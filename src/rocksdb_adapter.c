#include "../include/storage_engine.h"
#include <stdlib.h>
#include "rocksdb/c.h"

static void* rdb_open(void *options, const char *path, char **err) {
    return rocksdb_open((rocksdb_options_t*)options, path, err);
}
static void rdb_close(void* db) { rocksdb_close((rocksdb_t*)db); }

static void* rdb_options_create() { return rocksdb_options_create(); }
static void rdb_options_destroy(void* options) { rocksdb_options_destroy((rocksdb_options_t*)options); }
static void rdb_options_set_create_if_missing(void *options, int v) { rocksdb_options_set_create_if_missing((rocksdb_options_t*)options, v); }
static void rdb_destroy_db(void *options, const char *path, char **err) { rocksdb_destroy_db((rocksdb_options_t*)options, path, err); }

static void* rdb_readoptions_create() { return rocksdb_readoptions_create(); }
static void* rdb_writeoptions_create() { return rocksdb_writeoptions_create(); }
static void rdb_readoptions_destroy(void *roptions) { rocksdb_readoptions_destroy((rocksdb_readoptions_t*)roptions); }
static void rdb_writeoptions_destroy(void *woptions) { rocksdb_writeoptions_destroy((rocksdb_writeoptions_t*)woptions); }
static void rdb_put(void *db, void *woptions, const char *key, size_t keylen, const char *value, size_t valuelen, char **err) {
    rocksdb_put((rocksdb_t*)db, (rocksdb_writeoptions_t*)woptions, key, keylen, value, valuelen, err);
}
static char* rdb_get(void *db, void *roptions, const char *key, size_t keylen, size_t *valuelen, char **err) {
    return rocksdb_get((rocksdb_t*)db, (rocksdb_readoptions_t*)roptions, key, keylen, valuelen, err);
}
static void rdb_del(void *db, void *woptions, const char *key, size_t klen, char **err) {
    rocksdb_delete((rocksdb_t*)db, (rocksdb_writeoptions_t*)woptions, key, klen, err);
}

static void* rdb_cache_create_lru(size_t capacity) { return rocksdb_cache_create_lru(capacity); }
//static void rdb_options_set_cache(void *options, void *cache) { rocksdb_options_set_cache((rocksdb_options_t*)options, (rocksdb_cache_t*)cache); }
static void rdb_options_set_cache(void *options, void *cache) {
    rocksdb_block_based_table_options_t* bbt_opts = rocksdb_block_based_options_create();
    rocksdb_block_based_options_set_block_cache(bbt_opts, (rocksdb_cache_t*)cache);
    rocksdb_options_set_block_based_table_factory((rocksdb_options_t*)options, bbt_opts);
}

static void rdb_cache_destroy(void *cache) { rocksdb_cache_destroy((rocksdb_cache_t*)cache); }

static void rdb_free(void *ptr) { free(ptr); }

StorageEngine ROCKSDB_ENGINE = {
    .type = ENGINE_ROCKSDB,
    .open = rdb_open,
    .close = rdb_close,
    .options_create = rdb_options_create,
    .options_destroy = rdb_options_destroy,
    .options_set_create_if_missing = rdb_options_set_create_if_missing,
    .destroy_db = rdb_destroy_db,
    .readoptions_create = rdb_readoptions_create,
    .writeoptions_create = rdb_writeoptions_create,
    .readoptions_destroy = rdb_readoptions_destroy,
    .writeoptions_destroy = rdb_writeoptions_destroy,
    .put = rdb_put,
    .get = rdb_get,
    .del = rdb_del,
    .cache_create_lru = rdb_cache_create_lru,
    .options_set_cache = rdb_options_set_cache,
    .cache_destroy = rdb_cache_destroy,
    .free_fn = rdb_free,
    .supports_native_ttl = true, // RocksDB supports TTL natively
};


