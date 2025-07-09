#ifndef SPLITCACHE_H
#define SPLITCACHE_H

#include <stddef.h>

typedef struct ht_t ht_t;

ht_t *ht_create(void);
void ht_free(ht_t *hashtable);
void ht_set(ht_t *hashtable, const char *key, const void *value, size_t value_len);
int ht_get(ht_t *hashtable, const char *key, void **value, size_t *value_len);
void ht_update(ht_t *hashtable, const char *key, const void *value, size_t value_len);
int ht_delete(ht_t *hashtable, const char *key);

#endif // SPLITCACHE_H