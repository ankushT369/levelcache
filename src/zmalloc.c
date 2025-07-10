#include "zmalloc.h"
#include <stdlib.h>
#include <string.h>

#define PREFIX_SIZE sizeof(size_t)

size_t used_memory = 0;

void *zmalloc(size_t size) {
    void *ptr = malloc(size + PREFIX_SIZE);
    if (!ptr) return NULL;

    *((size_t*)ptr) = size;
    used_memory += size + PREFIX_SIZE;

    return (char*)ptr + PREFIX_SIZE;
}

void zfree(void *ptr) {
    if (ptr == NULL) return;

    void *real_ptr = (char*)ptr - PREFIX_SIZE;
    size_t old_size = *((size_t*)real_ptr);
    used_memory -= old_size + PREFIX_SIZE;
    free(real_ptr);
}
