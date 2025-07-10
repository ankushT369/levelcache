#ifndef SPLITCACHE_H
#define SPLITCACHE_H

#include <stddef.h>

/**
 * @brief An opaque handle to the SplitCache database.
 */
typedef struct SplitCache SplitCache;

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
 * @return 0 on success, -1 on error.
 */
int splitcache_put(SplitCache *cache, const char *key, const char *value);

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