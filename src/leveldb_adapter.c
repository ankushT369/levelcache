#include "../include/storage_engine.h"
#include "leveldb/c.h"

static void* ldb_open(void *options, const char *path, char **err) {
    return leveldb_open((leveldb_options_t*)options, path, err);
}
static void ldb_close(void* db) { leveldb_close((leveldb_t*)db); }

static void* ldb_options_create() { return leveldb_options_create(); }
static void ldb_options_destroy(void* options) { leveldb_options_destroy((leveldb_options_t*)options); }
static void ldb_options_set_create_if_missing(void *options, int v) { leveldb_options_set_create_if_missing((leveldb_options_t*)options, v); }
static void ldb_destroy_db(void *options, const char *path, char **err) { leveldb_destroy_db((leveldb_options_t*)options, path, err); }

static void* ldb_readoptions_create() { return leveldb_readoptions_create(); }
static void* ldb_writeoptions_create() { return leveldb_writeoptions_create(); }
static void ldb_readoptions_destroy(void *roptions) { leveldb_readoptions_destroy((leveldb_readoptions_t*)roptions); }
static void ldb_writeoptions_destroy(void *woptions) { leveldb_writeoptions_destroy((leveldb_writeoptions_t*)woptions); }
static void ldb_put(void *db, void *woptions, const char *key, size_t keylen, const char *value, size_t valuelen, char **err) {
    leveldb_put((leveldb_t*)db, (leveldb_writeoptions_t*)woptions, key, keylen, value, valuelen, err);
}
static char* ldb_get(void *db, void *roptions, const char *key, size_t keylen, size_t *valuelen, char **err) {
    return leveldb_get((leveldb_t*)db, (leveldb_readoptions_t*)roptions, key, keylen, valuelen, err);
}
static void ldb_del(void *db, void *woptions, const char *key, size_t klen, char **err) {
    leveldb_delete((leveldb_t*)db, (leveldb_writeoptions_t*)woptions, key, klen, err);
}

static void* ldb_cache_create_lru(size_t capacity) { return leveldb_cache_create_lru(capacity); }
static void ldb_options_set_cache(void *options, void *cache) { leveldb_options_set_cache((leveldb_options_t*)options, (leveldb_cache_t*)cache); }
static void ldb_cache_destroy(void *cache) { leveldb_cache_destroy((leveldb_cache_t*)cache); }

static void ldb_free(void *ptr) { leveldb_free(ptr); }

StorageEngine LEVELDB_ENGINE = {
    .type = ENGINE_LEVELDB,
    .open = ldb_open,
    .close = ldb_close,
    .options_create = ldb_options_create,
    .options_destroy = ldb_options_destroy,
    .options_set_create_if_missing = ldb_options_set_create_if_missing,
    .destroy_db = ldb_destroy_db,
    .readoptions_create = ldb_readoptions_create,
    .writeoptions_create = ldb_writeoptions_create,
    .readoptions_destroy = ldb_readoptions_destroy,
    .writeoptions_destroy = ldb_writeoptions_destroy,
    .put = ldb_put,
    .get = ldb_get,
    .del = ldb_del,
    .cache_create_lru = ldb_cache_create_lru,
    .options_set_cache = ldb_options_set_cache,
    .cache_destroy = ldb_cache_destroy,
    .free_fn = ldb_free,
    .supports_native_ttl = false,
};
