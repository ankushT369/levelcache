#ifndef STORAGE_ENGINE_H
#define STORAGE_ENGINE_H

#include <stddef.h>
#include <stdbool.h>

typedef enum engine_t { 
    ENGINE_LEVELDB,
    ENGINE_ROCKSDB,
    LIMIT,
} engine_t;

static const char* engine_names[LIMIT] = { "leveldb", "rocksdb" };

/**
 * @brief i will write it later.
 */

typedef struct StorageEngine {
    engine_t type;

    // open/close
    void* (*open)(void *options, const char *path, char **err);
    void  (*close)(void *db);

    // options
    void* (*options_create)();
    void  (*options_destroy)(void *options);
    void  (*options_set_create_if_missing)(void *options, int v);
    void  (*destroy_db)(void *options, const char *path, char **err);
    
    // Read/Write
    void* (*readoptions_create)();
    void* (*writeoptions_create)();
    void  (*readoptions_destroy)(void *roptions);
    void  (*writeoptions_destroy)(void *woptions);
    void  (*put)(void *db, void *woptions, const char *key, size_t keylen,
                const char *value, size_t valuelen, char **err);
    char* (*get)(void *db, void *roptions, const char *key, size_t keylen,
                size_t* valuelen, char **err);
    void  (*del)(void *db, void *woptions, const char *key, size_t keylen,
                char **err);

    //cache
    void* (*cache_create_lru)(size_t cache_size);
    void  (*options_set_cache)(void *options, void *cache);
    void  (*cache_destroy)(void *lru_cache);

    //free function for db-allocated bufers (errors) 
    //rocksdb uses free() but leveldb has its owd leveldb_free()
    void  (*free_fn)(void *ptr);

    bool supports_native_ttl;

} StorageEngine;

extern StorageEngine LEVELDB_ENGINE;
extern StorageEngine ROCKSDB_ENGINE;

#endif // STORAGE_ENGINE_H

