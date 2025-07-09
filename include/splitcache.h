#ifndef SPLITCACHE_H
#define SPLITCACHE_H

#include <stddef.h>

typedef struct leveldb_t leveldb_t;

leveldb_t* splitcache_open(const char *path);
void splitcache_close(leveldb_t *db);
int splitcache_put(leveldb_t *db, const char *key, const void *value, size_t value_len);
int splitcache_get(leveldb_t *db, const char *key, void **value, size_t *value_len);
int splitcache_delete(leveldb_t *db, const char *key);

#endif // SPLITCACHE_H