#include "splitcache.h"
#include "../vendor/ht.h"
#include <leveldb/c.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    time_t last_accessed_at;
} KeyMetadata;

struct SplitCache {
    leveldb_t *db;
    ht_t *in_memory_index;
};

SplitCache* splitcache_open(const char *path) {
    SplitCache *cache = malloc(sizeof(SplitCache));
    if (cache == NULL) {
        return NULL;
    }
    
    cache->in_memory_index = ht_create();
    if (cache->in_memory_index == NULL) {
        free(cache);
        return NULL;
    }

    leveldb_options_t *options;
    char *err = NULL;

    options = leveldb_options_create();
    leveldb_options_set_create_if_missing(options, 1);
    cache->db = leveldb_open(options, path, &err);

    if (err != NULL) {
        leveldb_free(err);
        ht_free(cache->in_memory_index);
        free(cache);
        return NULL;
    }

    leveldb_options_destroy(options);
    return cache;
}

void splitcache_close(SplitCache *cache) {
    if (cache == NULL) {
        return;
    }
    ht_iterator_t* it = ht_iterator(cache->in_memory_index);
    while (ht_next(it)) {
        free(ht_iterator_get_value(it));
    }
    ht_free(cache->in_memory_index);
    leveldb_close(cache->db);
    free(cache);
}

int splitcache_put(SplitCache *cache, const char *key, const char *value) {
    leveldb_writeoptions_t *woptions;
    char *err = NULL;
    size_t value_len = strlen(value);

    woptions = leveldb_writeoptions_create();
    leveldb_put(cache->db, woptions, key, strlen(key), value, value_len + 1, &err);
    leveldb_writeoptions_destroy(woptions);

    if (err != NULL) {
        leveldb_free(err);
        return -1;
    }

    KeyMetadata *meta = ht_get(cache->in_memory_index, key);
    if (meta == NULL) {
        meta = malloc(sizeof(KeyMetadata));
        if (meta == NULL) {
            return -1;
        }
        ht_set(cache->in_memory_index, key, meta);
    }

    meta->last_accessed_at = time(NULL);

    return 0;
}

char* splitcache_get(SplitCache *cache, const char *key) {
    KeyMetadata *meta = ht_get(cache->in_memory_index, key);
    if (meta == NULL) {
        return NULL;
    }

    leveldb_readoptions_t *roptions;
    char *err = NULL;
    char *value_buffer;
    size_t value_len = 0;

    roptions = leveldb_readoptions_create();
    value_buffer = leveldb_get(cache->db, roptions, key, strlen(key), &value_len, &err);
    leveldb_readoptions_destroy(roptions);

    if (err != NULL) {
        leveldb_free(err);
        return NULL;
    }
    
    if (value_buffer == NULL) {
        ht_set(cache->in_memory_index, key, NULL);
        return NULL;
    }

    meta->last_accessed_at = time(NULL);
    return value_buffer;
}

int splitcache_delete(SplitCache *cache, const char *key) {
    leveldb_writeoptions_t *woptions;
    char *err = NULL;

    woptions = leveldb_writeoptions_create();
    leveldb_delete(cache->db, woptions, key, strlen(key), &err);
    leveldb_writeoptions_destroy(woptions);

    if (err != NULL) {
        leveldb_free(err);
        return -1;
    }

    KeyMetadata *meta = ht_get(cache->in_memory_index, key);
    if (meta != NULL) {
        ht_set(cache->in_memory_index, key, NULL);
        free(meta);
    }

    return 0;
}