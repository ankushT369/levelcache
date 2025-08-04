#include "levelcache.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "uthash.h"
#include "log.h"

#define DEFAULT_TTL_SEC (24 * 60 * 60) // 1 day

void *cleanup_thread_function(void *arg) {
    LevelCache *cache = (LevelCache *)arg;
    log_info("[cleanup] Thread started with frequency %d seconds", cache->cleanup_frequency_sec);
    while (!cache->stop_cleanup_thread) {
        sleep(cache->cleanup_frequency_sec);
        log_debug("[cleanup] Running cleanup cycle");
        
        KeyMetadata *current, *tmp;
        HASH_ITER(hh, cache->index, current, tmp) {
            if (current->expiration > 0 && time(NULL) > current->expiration) {
                log_info("[cleanup] Key '%s' expired, deleting", current->key);
                levelcache_delete(cache, current->key);
            }
        }
    }
    log_info("[cleanup] Thread stopped");
    return NULL;
}

LevelCache* levelcache_open(const char *path, size_t max_memory_mb, uint32_t default_ttl_seconds, uint32_t cleanup_frequency_sec, int log_level, engine_t engine_type) {
    log_set_level(log_level);
    log_info("[open] Opening database at '%s'", path);

    LevelCache *cache = (LevelCache *) malloc(sizeof(LevelCache));
    if (cache == NULL) {
        log_error("[open] Failed to allocate memory for cache");
        return NULL;
    }

    StorageEngine *engine = ALL_ENGINES[engine_type];
    
    cache->engine = engine;
    cache->index = NULL;
    cache->default_ttl = (default_ttl_seconds > 0) ? default_ttl_seconds : DEFAULT_TTL_SEC;
    cache->cleanup_frequency_sec = cleanup_frequency_sec;
    cache->stop_cleanup_thread = 0;
    cache->log_level = log_level;
    cache->total_memory_bytes = sizeof(LevelCache);
    
    char *err = NULL;

    void* destroy_options = cache->engine->options_create();
    cache->engine->destroy_db(destroy_options, path, &err);
    cache->engine->options_destroy(destroy_options);
    if (err != NULL) {
        log_warn("[open] Could not destroy existing database: %s", err);
        cache->engine->free_fn(err);
        err = NULL; 
    }

    cache->options = cache->engine->options_create();
    cache->engine->options_set_create_if_missing(cache->options, 1);

    cache->max_memory_mb = max_memory_mb;
    if (cache->max_memory_mb > 0) {
        size_t cache_size = cache->max_memory_mb * 1024 * 1024;
        cache->lru_cache = cache->engine->cache_create_lru(cache_size);
        cache->engine->options_set_cache(cache->options, cache->lru_cache);
        cache->total_memory_bytes += cache_size;
        log_info("[open] LRU cache created with size %zu MB", max_memory_mb);
    } else {
        cache->lru_cache = NULL;
        cache->used_memory_bytes = 0;
    }

    cache->db = cache->engine->open(cache->options, path, &err);

    if (err != NULL) {
        log_error("[open] Failed to open database: %s", err);
        cache->engine->free_fn(err);
        cache->engine->options_destroy(cache->options);
        if(cache->lru_cache) {
            cache->engine->cache_destroy(cache->lru_cache);
        }
        free(cache);
        return NULL;
    }

    cache->roptions = cache->engine->readoptions_create();
    cache->woptions = cache->engine->writeoptions_create();

    if (cache->cleanup_frequency_sec > 0) {
        if (pthread_create(&cache->cleanup_thread, NULL, cleanup_thread_function, cache)) {
            log_error("[open] Failed to create cleanup thread");
            cache->engine->close(cache);
            return NULL;
        }
    }

    log_info("[open] Database opened successfully");
    log_warn("[open] Memory usage tracking does not include all internal leveldb allocations.");
    return cache;
}

void levelcache_close(LevelCache *cache) {
    if (cache == NULL) {
        return;
    }
    log_info("[close] Closing database");

    if (cache->cleanup_frequency_sec > 0) {
        cache->stop_cleanup_thread = 1;
        pthread_join(cache->cleanup_thread, NULL);
    }

    KeyMetadata *current, *tmp;
    HASH_ITER(hh, cache->index, current, tmp) {
        HASH_DEL(cache->index, current);
        cache->total_memory_bytes -= (sizeof(KeyMetadata) + strlen(current->key) + 1);
        free(current->key);
        free(current);
    }

    cache->engine->close(cache->db);
    cache->engine->options_destroy(cache->options);
    cache->engine->readoptions_destroy(cache->roptions);
    cache->engine->writeoptions_destroy(cache->woptions);
    if (cache->lru_cache) {
        cache->engine->cache_destroy(cache->lru_cache);
    }
    free(cache);
    log_info("[close] Database closed");
}

int levelcache_put(LevelCache *cache, const char *key, const char *value, uint32_t ttl_seconds) {
    log_trace("[put] Putting key '%s'", key);
    KeyMetadata *meta;
    HASH_FIND_STR(cache->index, key, meta);

    uint32_t __ttl_seconds = (ttl_seconds > 0) ? ttl_seconds : cache->default_ttl;
    uint64_t expiration = time(NULL) + __ttl_seconds;

    int new_key = 0;
    if (meta == NULL) {
        log_debug("[put] Key '%s' not found, creating new entry", key);
        meta = (KeyMetadata *) malloc(sizeof(KeyMetadata));
        if (meta == NULL) {
            log_error("[put] Failed to allocate memory for key metadata");
            return -1;
        }
        meta->key = strdup(key);
        if (meta->key == NULL) {
            log_error("[put] Failed to duplicate key string");
            free(meta);
            return -1;
        }
        new_key = 1;
        cache->total_memory_bytes += (sizeof(KeyMetadata) + strlen(key) + 1);
    } else {
        log_debug("[put] Key '%s' found, updating expiration", key);
    }
    meta->expiration = expiration;

    char *err = NULL;
    cache->engine->put(cache->db, cache->woptions, key, strlen(key), value, strlen(value), &err);

    if (err != NULL) {
        log_error("[put] Failed to put key '%s' into leveldb: %s", key, err);
        cache->engine->free_fn(err);
        if (new_key) {
            log_debug("[put] Rolling back in-memory insert for key '%s'", key);
            cache->total_memory_bytes -= (sizeof(KeyMetadata) + strlen(key) + 1);
            free(meta->key);
            free(meta);
        }
        return -1;
    }

    if (new_key) {
        HASH_ADD_KEYPTR(hh, cache->index, meta->key, strlen(meta->key), meta);
    }
    log_info("[put] Key '%s' put successfully with TTL %u seconds", key, __ttl_seconds);

    return 0;
}

char* levelcache_get(LevelCache *cache, const char *key) {
    log_trace("[get] Getting key '%s'", key);
    KeyMetadata *meta;
    HASH_FIND_STR(cache->index, key, meta);

    if (meta != NULL) {
        if (meta->expiration > 0 && time(NULL) > meta->expiration) {
            log_info("[get] Key '%s' expired, deleting", key);
            levelcache_delete(cache, key);
            return NULL;
        }
    } else {
        log_debug("[get] Key '%s' not found in index", key);
        return NULL;
    }

    char *err = NULL;
    size_t value_len;
    char *value_buffer = cache->engine->get(cache->db, cache->roptions, key, strlen(key), &value_len, &err);

    if (err != NULL) {
        log_error("[get] Failed to get key '%s' from leveldb: %s", key, err);
        cache->engine->free_fn(err);
        return NULL;
    }

    if (value_buffer == NULL) {
        log_warn("[get] Key '%s' not found in db, but present in index. Inconsistency.", key);
        return NULL;
    }
    
    char *result = (char *)malloc(value_len + 1);
    if (result == NULL) {
        log_error("[get] Failed to allocate memory for result");
        cache->engine->free_fn(value_buffer);
        return NULL;
    }
    memcpy(result, value_buffer, value_len);
    result[value_len] = '\0';
    cache->engine->free_fn(value_buffer);
    log_info("[get] Key '%s' retrieved successfully", key);
    return result;
}

int levelcache_delete(LevelCache *cache, const char *key) {
    log_trace("[delete] Deleting key '%s'", key);
    KeyMetadata *meta;
    HASH_FIND_STR(cache->index, key, meta);

    if (meta != NULL) {
        HASH_DEL(cache->index, meta);
        cache->total_memory_bytes -= (sizeof(KeyMetadata) + strlen(key) + 1);
    }

    char *err = NULL;
    cache->engine->del(cache->db, cache->woptions, key, strlen(key), &err);
    
    if (err != NULL) {
        log_error("[delete] Failed to delete key '%s' from leveldb: %s", key, err);
        cache->engine->free_fn(err);
        if (meta != NULL) {
            log_debug("[delete] Rolling back in-memory delete for key '%s'", key);
            HASH_ADD_KEYPTR(hh, cache->index, meta->key, strlen(meta->key), meta);
            cache->total_memory_bytes += (sizeof(KeyMetadata) + strlen(key) + 1);
        }
        return -1;
    }

    if (meta != NULL) {
        free(meta->key);
        free(meta);
    }
    log_info("[delete] Key '%s' deleted successfully", key);

    return 0;
}

size_t levelcache_get_memory_usage(LevelCache *cache) {
    if (cache == NULL) {
        return 0;
    }
    return cache->total_memory_bytes;
}
