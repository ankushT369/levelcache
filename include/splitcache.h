#ifndef SPLITCACHE_H
#define SPLITCACHE_H

#include <stddef.h>
#include <stdint.h>
#include "leveldb/c.h"

/**
 * @brief An opaque handle to the SplitCache database.
 */
typedef struct SplitCache {
    leveldb_t *db;
    leveldb_options_t *options;
    leveldb_readoptions_t *roptions;
    leveldb_writeoptions_t *woptions;
} SplitCache;

/**
 * @brief Opens a SplitCache database at the specified path.
 *
 * @param path The filesystem path to the database.
 * @return A handle to the database, or NULL on error.
 */
SplitCache* splitcache_open(const char *path);

/**
 * @brief Closes a SplitCache database.
 *
 * @param cache The database handle.
 */
void splitcache_close(SplitCache *cache);

/**
 * @brief Stores a key-value pair in the database.
 *
 * @param cache The database handle.
 * @param key The key to store.
 * @param value The null-terminated string value to store.
 * @param ttl_seconds The time-to-live in seconds. 0 means no TTL.
 * @return 0 on success, -1 on error.
 */
int splitcache_put(SplitCache *cache, const char *key, const char *value, uint32_t ttl_seconds);

/**
 * @brief Retrieves a value from the database for a given key.
 *
 * The caller is responsible for freeing the returned string.
 *
 * @param cache The database handle.
 * @param key The key to retrieve.
 * @return A pointer to a null-terminated string, or NULL if the key is not found or an error occurs.
 */
char* splitcache_get(SplitCache *cache, const char *key);

/**
 * @brief Deletes a key-value pair from the database.
 *
 * @param cache The database handle.
 * @param key The key to delete.
 * @return 0 on success, -1 on error.
 */
int splitcache_delete(SplitCache *cache, const char *key);

#endif // SPLITCACHE_H