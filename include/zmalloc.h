#ifndef ZMALLOC_H
#define ZMALLOC_H

#include <stddef.h>

// We need a forward declaration of SplitCache here.
struct SplitCache;

void *zmalloc(struct SplitCache *cache, size_t size);
void zfree(struct SplitCache *cache, void *ptr);

#endif // ZMALLOC_H