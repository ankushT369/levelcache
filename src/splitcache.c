#include "splitcache.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

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

    KeyMetadata *current_entry, *tmp;
    HASH_ITER(hh, cache->mcache, current_entry, tmp) {
        HASH_DEL(cache->mcache, current_entry);
        free((void*)current_entry->hh.key);
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
    leveldb_put(cache->db, cache->woptions, key, strlen(key), value, strlen(value) + 1, &err);
    if (err != NULL) {
        leveldb_free(err);
        return -1;
    }

    // Add to in-memory cache
    KeyMetadata *entry;
    HASH_FIND_STR(cache->mcache, key, entry);
    if (entry == NULL) {
        entry = (KeyMetadata*)malloc(sizeof(KeyMetadata));
        char *key_copy = strdup(key);
        HASH_ADD_KEYPTR(hh, cache->mcache, key_copy, strlen(key_copy), entry);
    }
    entry->lru = (uint32_t)time(NULL);

    return 0;
}

char* splitcache_get(SplitCache *cache, const char *key) {
    KeyMetadata *entry;
    HASH_FIND_STR(cache->mcache, key, entry);

    if (entry != NULL) {
        entry->lru = (uint32_t)time(NULL);
    }

    char *err = NULL;
    size_t value_len;
    char *value_buffer = leveldb_get(cache->db, cache->roptions, key, strlen(key), &value_len, &err);

    if (err != NULL) {
        leveldb_free(err);
        return NULL;
    }

    if (value_buffer != NULL) {
        char *result = strndup(value_buffer, value_len);
        leveldb_free(value_buffer);

        if (entry == NULL) {
            entry = (KeyMetadata*)malloc(sizeof(KeyMetadata));
            char *key_copy = strdup(key);
            HASH_ADD_KEYPTR(hh, cache->mcache, key_copy, strlen(key_copy), entry);
        }
        entry->lru = (uint32_t)time(NULL);
        return result;
    }
    
    return NULL;
}

int splitcache_delete(SplitCache *cache, const char *key) {
    // Delete from in-memory cache
    KeyMetadata *entry;
    HASH_FIND_STR(cache->mcache, key, entry);
    if (entry != NULL) {
        HASH_DEL(cache->mcache, entry);
        free((void*)entry->hh.key);
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
