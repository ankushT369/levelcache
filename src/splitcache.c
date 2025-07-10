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

    return cache;
}

void splitcache_close(SplitCache *cache) {
    if (cache == NULL) {
        return;
    }

    leveldb_close(cache->db);
    leveldb_options_destroy(cache->options);
    leveldb_readoptions_destroy(cache->roptions);
    leveldb_writeoptions_destroy(cache->woptions);
    free(cache);
}

int splitcache_put(SplitCache *cache, const char *key, const char *value, uint32_t ttl_seconds) {
    char *err = NULL;
    
    size_t value_len = strlen(value);
    
    if (ttl_seconds > 0) {
        uint64_t expiration = time(NULL) + ttl_seconds;
        size_t new_value_len = value_len + sizeof(expiration);
        char *new_value = malloc(new_value_len);
        memcpy(new_value, value, value_len);
        memcpy(new_value + value_len, &expiration, sizeof(expiration));
        
        leveldb_put(cache->db, cache->woptions, key, strlen(key), new_value, new_value_len, &err);
        free(new_value);
    } else {
        leveldb_put(cache->db, cache->woptions, key, strlen(key), value, value_len, &err);
    }

    if (err != NULL) {
        leveldb_free(err);
        return -1;
    }

    return 0;
}

char* splitcache_get(SplitCache *cache, const char *key) {
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

    if (value_len > sizeof(uint64_t)) {
        uint64_t expiration;
        memcpy(&expiration, value_buffer + value_len - sizeof(expiration), sizeof(expiration));

        if (time(NULL) > expiration) {
            leveldb_free(value_buffer);
            splitcache_delete(cache, key);
            return NULL;
        }

        size_t real_value_len = value_len - sizeof(expiration);
        char *result = (char *)malloc(real_value_len + 1);
        memcpy(result, value_buffer, real_value_len);
        result[real_value_len] = '\0';
        leveldb_free(value_buffer);
        return result;
    }
    
    char *result = (char *)malloc(value_len + 1);
    memcpy(result, value_buffer, value_len);
    result[value_len] = '\0';
    leveldb_free(value_buffer);
    return result;
}

int splitcache_delete(SplitCache *cache, const char *key) {
    char *err = NULL;
    leveldb_delete(cache->db, cache->woptions, key, strlen(key), &err);
    if (err != NULL) {
        leveldb_free(err);
        return -1;
    }

    return 0;
}