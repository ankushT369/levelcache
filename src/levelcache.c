#include "levelcache.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "leveldb/c.h"
#include "uthash.h"
#include "log.h"

#define DEFAULT_TTL_SEC (24 * 60 * 60) // 1 day

void *cleanup_thread_function(void *arg) {
    LevelCache *cache = (LevelCache *)arg;
    log_info("Cleanup thread started with frequency %d seconds", cache->cleanup_frequency_sec);
    while (!cache->stop_cleanup_thread) {
        sleep(cache->cleanup_frequency_sec);
        log_debug("Running cleanup");
        
        KeyMetadata *current, *tmp;
        HASH_ITER(hh, cache->index, current, tmp) {
            if (current->expiration > 0 && time(NULL) > current->expiration) {
                log_info("Key '%s' expired, deleting.", current->key);
                levelcache_delete(cache, current->key);
            }
        }
    }
    log_info("Cleanup thread stopped");
    return NULL;
}

LevelCache* levelcache_open(const char *path, size_t max_memory_mb, uint32_t default_ttl_seconds, uint32_t cleanup_frequency_sec, int log_level) {
    log_set_level(log_level);
    log_info("Opening database at %s", path);

    LevelCache *cache = (LevelCache *) malloc(sizeof(LevelCache));
    if (cache == NULL) {
        log_error("Failed to allocate memory for cache");
        return NULL;
    }
    
    cache->index = NULL;
    cache->default_ttl = (default_ttl_seconds > 0) ? default_ttl_seconds : DEFAULT_TTL_SEC;
    cache->cleanup_frequency_sec = cleanup_frequency_sec;
    cache->stop_cleanup_thread = 0;
    cache->log_level = log_level;
    
    char *err = NULL;

    // Destroy the existing database if it exists
    leveldb_options_t* destroy_options = leveldb_options_create();
    leveldb_destroy_db(destroy_options, path, &err);
    leveldb_options_destroy(destroy_options);
    if (err != NULL) {
        log_warn("Could not destroy existing database: %s", err);
        leveldb_free(err);
        err = NULL; 
    }

    cache->options = leveldb_options_create();
    leveldb_options_set_create_if_missing(cache->options, 1);

    cache->max_memory_mb = max_memory_mb;
    if (cache->max_memory_mb > 0) {
        size_t cache_size = cache->max_memory_mb * 1024 * 1024;
        cache->lru_cache = leveldb_cache_create_lru(cache_size);
        leveldb_options_set_cache(cache->options, cache->lru_cache);
        cache->used_memory_bytes = cache_size;
        log_info("LRU cache created with size %zu MB", max_memory_mb);
    } else {
        cache->lru_cache = NULL;
        cache->used_memory_bytes = 0;
    }

    cache->db = leveldb_open(cache->options, path, &err);

    if (err != NULL) {
        log_error("Failed to open database: %s", err);
        leveldb_free(err);
        leveldb_options_destroy(cache->options);
        if(cache->lru_cache) {
            leveldb_cache_destroy(cache->lru_cache);
        }
        free(cache);
        return NULL;
    }

    cache->roptions = leveldb_readoptions_create();
    cache->woptions = leveldb_writeoptions_create();

    if (cache->cleanup_frequency_sec > 0) {
        if (pthread_create(&cache->cleanup_thread, NULL, cleanup_thread_function, cache)) {
            log_error("Failed to create cleanup thread");
            levelcache_close(cache);
            return NULL;
        }
    }

    log_info("Database opened successfully");
    return cache;
}

void levelcache_close(LevelCache *cache) {
    if (cache == NULL) {
        return;
    }
    log_info("Closing database");

    if (cache->cleanup_frequency_sec > 0) {
        cache->stop_cleanup_thread = 1;
        pthread_join(cache->cleanup_thread, NULL);
    }

    KeyMetadata *current, *tmp;
    HASH_ITER(hh, cache->index, current, tmp) {
        HASH_DEL(cache->index, current);
        free(current->key);
        free(current);
    }

    leveldb_close(cache->db);
    leveldb_options_destroy(cache->options);
    leveldb_readoptions_destroy(cache->roptions);
    leveldb_writeoptions_destroy(cache->woptions);
    if (cache->lru_cache) {
        leveldb_cache_destroy(cache->lru_cache);
    }
    free(cache);
    log_info("Database closed");
}

int levelcache_put(LevelCache *cache, const char *key, const char *value, uint32_t ttl_seconds) {
    log_debug("Putting key '%s'", key);
    KeyMetadata *meta;
    HASH_FIND_STR(cache->index, key, meta);

    uint32_t __ttl_seconds = (ttl_seconds > 0) ? ttl_seconds : cache->default_ttl;
    uint64_t expiration = time(NULL) + __ttl_seconds;

    int new_key = 0;
    if (meta == NULL) {
        meta = (KeyMetadata *) malloc(sizeof(KeyMetadata));
        if (meta == NULL) {
            log_error("Failed to allocate memory for key metadata");
            return -1;
        }
        meta->key = strdup(key);
        if (meta->key == NULL) {
            log_error("Failed to duplicate key string");
            free(meta);
            return -1;
        }
        new_key = 1;
    }
    meta->expiration = expiration;

    char *err = NULL;
    leveldb_put(cache->db, cache->woptions, key, strlen(key), value, strlen(value), &err);

    if (err != NULL) {
        log_error("Failed to put key '%s': %s", key, err);
        leveldb_free(err);
        // Rollback in-memory change
        if (new_key) {
            free(meta->key);
            free(meta);
        }
        return -1;
    }

    if (new_key) {
        HASH_ADD_KEYPTR(hh, cache->index, meta->key, strlen(meta->key), meta);
    }
    log_info("Key '%s' put successfully with TTL %u", key, __ttl_seconds);

    return 0;
}

char* levelcache_get(LevelCache *cache, const char *key) {
    log_debug("Getting key '%s'", key);
    KeyMetadata *meta;
    HASH_FIND_STR(cache->index, key, meta);

    if (meta != NULL) {
        if (meta->expiration > 0 && time(NULL) > meta->expiration) {
            log_info("Key '%s' expired, deleting.", key);
            levelcache_delete(cache, key);
            return NULL;
        }
    } else {
        log_debug("Key '%s' not found in index", key);
        return NULL;
    }

    char *err = NULL;
    size_t value_len;
    char *value_buffer = leveldb_get(cache->db, cache->roptions, key, strlen(key), &value_len, &err);

    if (err != NULL) {
        log_error("Failed to get key '%s': %s", key, err);
        leveldb_free(err);
        return NULL;
    }

    if (value_buffer == NULL) {
        log_debug("Key '%s' not found in db", key);
        return NULL;
    }
    
    char *result = (char *)malloc(value_len + 1);
    memcpy(result, value_buffer, value_len);
    result[value_len] = '\0';
    leveldb_free(value_buffer);
    log_info("Key '%s' retrieved successfully", key);
    return result;
}

int levelcache_delete(LevelCache *cache, const char *key) {
    log_debug("Deleting key '%s'", key);
    KeyMetadata *meta;
    HASH_FIND_STR(cache->index, key, meta);

    if (meta != NULL) {
        HASH_DEL(cache->index, meta);
    }

    char *err = NULL;
    leveldb_delete(cache->db, cache->woptions, key, strlen(key), &err);
    
    if (err != NULL) {
        log_error("Failed to delete key '%s': %s", key, err);
        leveldb_free(err);
        if (meta != NULL) {
            HASH_ADD_KEYPTR(hh, cache->index, meta->key, strlen(meta->key), meta);
        }
        return -1;
    }

    if (meta != NULL) {
        free(meta->key);
        free(meta);
    }
    log_info("Key '%s' deleted successfully", key);

    return 0;
}
