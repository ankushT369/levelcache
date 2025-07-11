#include "levelcache.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "leveldb/c.h"
#include "uthash.h"

#define DEFAULT_TTL_SEC (24 * 60 * 60) // 1 day

LevelCache* levelcache_open(const char *path, size_t max_memory_mb, uint32_t default_ttl_seconds) {
    LevelCache *cache = (LevelCache *) malloc(sizeof(LevelCache));
    if (cache == NULL) {
        return NULL;
    }
    
    cache->index = NULL;
    cache->default_ttl = (default_ttl_seconds > 0) ? default_ttl_seconds : DEFAULT_TTL_SEC;
    
    char *err = NULL;

    // Destroy the existing database if it exists
    leveldb_options_t* destroy_options = leveldb_options_create();
    leveldb_destroy_db(destroy_options, path, &err);
    leveldb_options_destroy(destroy_options);
    if (err != NULL) {
        // This is not a fatal error, maybe the db didn't exist.
        // We can log this if we have a logging mechanism.
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
    } else {
        cache->lru_cache = NULL;
        cache->used_memory_bytes = 0;
    }

    cache->db = leveldb_open(cache->options, path, &err);

    if (err != NULL) {
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

    return cache;
}

void levelcache_close(LevelCache *cache) {
    if (cache == NULL) {
        return;
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
}

int levelcache_put(LevelCache *cache, const char *key, const char *value, uint32_t ttl_seconds) {
    KeyMetadata *meta;
    HASH_FIND_STR(cache->index, key, meta);

    uint32_t __ttl_seconds = (ttl_seconds > 0) ? ttl_seconds : cache->default_ttl;
    uint64_t expiration = time(NULL) + __ttl_seconds;

    int new_key = 0;
    if (meta == NULL) {
        meta = (KeyMetadata *) malloc(sizeof(KeyMetadata));
        if (meta == NULL) return -1; // Allocation failed
        meta->key = strdup(key);
        if (meta->key == NULL) { // Allocation failed
            free(meta);
            return -1;
        }
        new_key = 1;
    }
    meta->expiration = expiration;

    char *err = NULL;
    leveldb_put(cache->db, cache->woptions, key, strlen(key), value, strlen(value), &err);

    if (err != NULL) {
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

    return 0;
}

char* levelcache_get(LevelCache *cache, const char *key) {
    KeyMetadata *meta;
    HASH_FIND_STR(cache->index, key, meta);

    if (meta != NULL) {
        if (meta->expiration > 0 && time(NULL) > meta->expiration) {
            levelcache_delete(cache, key);
            return NULL;
        }
    } else {
        // Key not in index, so it's considered not found.
        return NULL;
    }

    char *err = NULL;
    size_t value_len;
    char *value_buffer = leveldb_get(cache->db, cache->roptions, key, strlen(key), &value_len, &err);

    if (err != NULL) {
        leveldb_free(err);
        return NULL;
    }

    if (value_buffer == NULL) {
        return NULL;
    }
    
    char *result = (char *)malloc(value_len + 1);
    memcpy(result, value_buffer, value_len);
    result[value_len] = '\0';
    leveldb_free(value_buffer);
    return result;
}

int levelcache_delete(LevelCache *cache, const char *key) {
    KeyMetadata *meta;
    HASH_FIND_STR(cache->index, key, meta);

    if (meta != NULL) {
        HASH_DEL(cache->index, meta);
    }

    char *err = NULL;
    leveldb_delete(cache->db, cache->woptions, key, strlen(key), &err);
    
    if (err != NULL) {
        leveldb_free(err);
        // Rollback not strictly necessary for delete, but if we wanted to be
        // perfectly consistent, we would re-add the metadata to the index.
        // For now, we'll just report the error.
        if (meta != NULL) {
            // Re-add to index to rollback
            HASH_ADD_KEYPTR(hh, cache->index, meta->key, strlen(meta->key), meta);
        }
        return -1;
    }

    if (meta != NULL) {
        free(meta->key);
        free(meta);
    }

    return 0;
}
