#include "splitcache.h"
#include <leveldb/c.h>
#include <stdlib.h>
#include <string.h>

leveldb_t* splitcache_open(const char *path) {
    leveldb_t *db;
    leveldb_options_t *options;
    char *err = NULL;

    options = leveldb_options_create();
    leveldb_options_set_create_if_missing(options, 1);
    db = leveldb_open(options, path, &err);

    if (err != NULL) {
        // Handle error
        leveldb_free(err);
        return NULL;
    }

    leveldb_options_destroy(options);
    return db;
}

void splitcache_close(leveldb_t *db) {
    leveldb_close(db);
}

int splitcache_put(leveldb_t *db, const char *key, const void *value, size_t value_len) {
    leveldb_writeoptions_t *woptions;
    char *err = NULL;

    woptions = leveldb_writeoptions_create();
    leveldb_put(db, woptions, key, strlen(key), value, value_len, &err);
    leveldb_writeoptions_destroy(woptions);

    if (err != NULL) {
        leveldb_free(err);
        return -1;
    }
    return 0;
}

int splitcache_get(leveldb_t *db, const char *key, void **value, size_t *value_len) {
    leveldb_readoptions_t *roptions;
    char *err = NULL;
    char *read_value;

    roptions = leveldb_readoptions_create();
    read_value = leveldb_get(db, roptions, key, strlen(key), value_len, &err);
    leveldb_readoptions_destroy(roptions);

    if (err != NULL) {
        leveldb_free(err);
        return -1;
    }
    
    if (read_value == NULL) {
        return -1; // Not found
    }

    *value = read_value;
    return 0;
}

int splitcache_delete(leveldb_t *db, const char *key) {
    leveldb_writeoptions_t *woptions;
    char *err = NULL;

    woptions = leveldb_writeoptions_create();
    leveldb_delete(db, woptions, key, strlen(key), &err);
    leveldb_writeoptions_destroy(woptions);

    if (err != NULL) {
        leveldb_free(err);
        return -1;
    }
    return 0;
}