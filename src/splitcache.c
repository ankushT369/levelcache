#include "splitcache.h"
#include <stdlib.h>
#include <string.h>

SplitCache* splitcache_open(const char *path) {
    SplitCache *cache = (SplitCache *) malloc(sizeof(SplitCache));
    if (cache == NULL) {
        return NULL;
    }
    
    char *err = NULL;

    cache->options = leveldb_options_create();
    leveldb_options_set_create_if_missing(cache->options, 1);
    cache->db = leveldb_open(cache->options, path, &err);

    if (err != NULL) {
        leveldb_free(err);
        leveldb_options_destroy(cache->options);
        free(cache);
        return NULL;
    }

    cache->roptions = leveldb_readoptions_create();
    cache->woptions = leveldb_writeoptions_create();
    cache->mcache = NULL;

    return cache;
}

void splitcache_close(SplitCache *cache) {
    if (cache == NULL) {
        return;
    }

    CacheEntry *current_entry, *tmp;
    HASH_ITER(hh, cache->mcache, current_entry, tmp) {
        HASH_DEL(cache->mcache, current_entry);
        free(current_entry->key);
        free(current_entry->value);
        free(current_entry);
    }

    leveldb_close(cache->db);
    leveldb_options_destroy(cache->options);
    leveldb_readoptions_destroy(cache->roptions);
    leveldb_writeoptions_destroy(cache->woptions);
    free(cache);
}

int splitcache_put(SplitCache *cache, const char *key, const char *value) {
    char *err = NULL;

    // Add to LevelDB
    leveldb_put(cache->db, cache->woptions, key, strlen(key), value, strlen(value), &err);
    if (err != NULL) {
        leveldb_free(err);
        return -1;
    }

    // Add to in-memory cache
    CacheEntry *entry;
    HASH_FIND_STR(cache->mcache, key, entry);
    if (entry == NULL) {
        entry = (CacheEntry*)malloc(sizeof(CacheEntry));
        entry->key = strdup(key);
        HASH_ADD_KEYPTR(hh, cache->mcache, entry->key, strlen(entry->key), entry);
    }
    entry->value = strdup(value);

    return 0;
}

char* splitcache_get(SplitCache *cache, const char *key) {
    CacheEntry *entry;
    HASH_FIND_STR(cache->mcache, key, entry);

    if (entry != NULL) {
        return strdup(entry->value);
    }

    char *err = NULL;
    size_t value_len;
    char *value_buffer = leveldb_get(cache->db, cache->roptions, key, strlen(key), &value_len, &err);

    if (err != NULL) {
        leveldb_free(err);
        return NULL;
    }

    if (value_buffer != NULL) {
        // Add to in-memory cache
        entry = (CacheEntry*)malloc(sizeof(CacheEntry));
        entry->key = strdup(key);
        entry->value = strndup(value_buffer, value_len);
        HASH_ADD_KEYPTR(hh, cache->mcache, entry->key, strlen(entry->key), entry);
    }
    
    return value_buffer;
}

int splitcache_delete(SplitCache *cache, const char *key) {
    // Delete from in-memory cache
    CacheEntry *entry;
    HASH_FIND_STR(cache->mcache, key, entry);
    if (entry != NULL) {
        HASH_DEL(cache->mcache, entry);
        free(entry->key);
        free(entry->value);
        free(entry);
    }

    // Delete from LevelDB
    char *err = NULL;
    leveldb_delete(cache->db, cache->woptions, key, strlen(key), &err);
    if (err != NULL) {
        leveldb_free(err);
        return -1;
    }

    return 0;
}