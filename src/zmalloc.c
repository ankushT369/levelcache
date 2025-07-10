#include "zmalloc.h"
#include "splitcache.h"
#include <stdlib.h>
#include <string.h>

#define PREFIX_SIZE sizeof(size_t)

void *zmalloc(SplitCache *cache, size_t size) {
    if (cache->max_memory > 0 && cache->used_memory + size + PREFIX_SIZE > cache->max_memory) {
        splitcache_evict(cache);
    }

    void *ptr = malloc(size + PREFIX_SIZE);
    if (!ptr) return NULL;

    *((size_t*)ptr) = size;
    cache->used_memory += size + PREFIX_SIZE;

    return (char*)ptr + PREFIX_SIZE;
}

void zfree(SplitCache *cache, void *ptr) {
    if (ptr == NULL) return;

    void *real_ptr = (char*)ptr - PREFIX_SIZE;
    size_t old_size = *((size_t*)real_ptr);
    cache->used_memory -= old_size + PREFIX_SIZE;
    free(real_ptr);
}
