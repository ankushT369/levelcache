#ifndef ZMALLOC_H
#define ZMALLOC_H

#include <stddef.h>

void *zmalloc(size_t size);
void zfree(void *ptr);
extern size_t used_memory;

#endif // ZMALLOC_H