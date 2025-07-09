#ifndef SPLITCACHE_H
#define SPLITCACHE_H

#include <stddef.h>

/**
 * @brief An opaque handle to the LevelDB database.
 */
typedef struct leveldb_t leveldb_t;

/**
 * @brief Opens a LevelDB database at the specified path.
 *
 * @param path The filesystem path to the database.
 * @return A handle to the database, or NULL on error.
 */
leveldb_t* splitcache_open(const char *path);

/**
 * @brief Closes a LevelDB database.
 *
 * @param db The database handle.
 */
void splitcache_close(leveldb_t *db);

/**
 * @brief Stores a key-value pair in the database.
 *
 * @param db The database handle.
 * @param key The key to store.
 * @param value A pointer to the value data.
 * @param value_len The length of the value data.
 * @return 0 on success, -1 on error.
 */
int splitcache_put(leveldb_t *db, const char *key, const void *value, size_t value_len);

/**
 * @brief Retrieves a value from the database for a given key.
 *
 * @param db The database handle.
 * @param key The key to retrieve.
 * @param value A pointer to a variable that will be allocated and filled with the value data. The caller is responsible for freeing this memory.
 * @param value_len A pointer to a variable that will be filled with the length of the value data.
 * @return 0 on success, -1 if the key is not found or an error occurs.
 */
int splitcache_get(leveldb_t *db, const char *key, void **value, size_t *value_len);

/**
 * @brief Deletes a key-value pair from the database.
 *
 * @param db The database handle.
 * @param key The key to delete.
 * @return 0 on success, -1 on error.
 */
int splitcache_delete(leveldb_t *db, const char *key);

#endif // SPLITCACHE_H
