#ifndef LEVELCACHE_H
#define LEVELCACHE_H

#include <stddef.h>
#include <stdint.h>
#include <pthread.h>
#include "leveldb/c.h"
#include "../vendor/rocksdb/include/rocksdb/c.h"
#include "uthash.h"
#include "log.h"
#include "storage_engine.h"

static StorageEngine* ALL_ENGINES[LIMIT] = {
    &LEVELDB_ENGINE,
    &ROCKSDB_ENGINE
};

/**
 * @brief Metadata for each key, stored in the in-memory index.
 */
typedef struct KeyMetadata {
    char *key;
    uint64_t expiration;
    UT_hash_handle hh;
} KeyMetadata;

/**
 * @brief An opaque handle to the LevelCache database.
 */
typedef struct LevelCache {
    void *db;
    void *options;
    void *roptions;
    void *woptions;
    void *lru_cache;
    size_t max_memory_mb;
    size_t used_memory_bytes;
    uint32_t default_ttl;
    KeyMetadata *index;
    pthread_t cleanup_thread;
    int stop_cleanup_thread;
    uint32_t cleanup_frequency_sec;
    int log_level;
    size_t total_memory_bytes;
    StorageEngine* engine;
} LevelCache;


/**
 * @brief Opens a LevelCache database at the specified path.
 *
 * @param path The filesystem path to the database.
 * @param max_memory_mb The maximum memory capacity in megabytes.
 * @param default_ttl_seconds The default time-to-live in seconds for keys. 0 means no TTL.
 * @param cleanup_frequency_sec The frequency in seconds for the cleanup thread to run. 0 disables the cleanup thread.
 * @param log_level The initial log level. See log.h for levels.
 * @return A handle to the database, or NULL on error.
 */
LevelCache* levelcache_open(const char *path, size_t max_memory_mb, uint32_t default_ttl_seconds, uint32_t cleanup_frequency_sec, int log_level, engine_t engine);

/**
 * @brief Closes a LevelCache database.
 *
 * @param cache The database handle.
 */
void levelcache_close(LevelCache *cache);

/**
 * @brief Stores a key-value pair in the database.
 *
 * @param cache The database handle.
 * @param key The key to store.
 * @param value The null-terminated string value to store.
 * @param ttl_seconds The time-to-live in seconds. If 0, the default TTL is used.
 * @return 0 on success, -1 on error.
 */
int levelcache_put(LevelCache *cache, const char *key, const char *value, uint32_t ttl_seconds);

/**
 * @brief Retrieves a value from the database for a given key.
 *
 * The caller is responsible for freeing the returned string.
 *
 * @param cache The database handle.
 * @param key The key to retrieve.
 * @return A pointer to a null-terminated string, or NULL if the key is not found or an error occurs.
 */
char* levelcache_get(LevelCache *cache, const char *key);

/**
 * @brief Deletes a key-value pair from the database.
 *
 * @param cache The database handle.
 * @param key The key to delete.
 * @return 0 on success, -1 on error.
 */
int levelcache_delete(LevelCache *cache, const char *key);

/**
 * @brief Gets the current memory usage of the cache in bytes.
 *
 * @param cache The database handle.
 * @return The memory usage in bytes.
 */
size_t levelcache_get_memory_usage(LevelCache *cache);

#endif // LEVELCACHE_H
