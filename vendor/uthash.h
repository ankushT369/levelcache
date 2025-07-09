/*
Copyright (c) 2003-2022, Troy D. Hanson     http://troydhanson.github.com/uthash/
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/* a classroom-friendly hash table
 * 
 * The hash table is implemented using a single static array.
 * A chained approach is used for collisions.
 * 
 * The uthash data structure is a single-file implementation of a hash table
 * for C structures. It is easy to use and requires only that you add a
 * UT_hash_handle to your structure.
 * 
 * For more information, see the user guide at http://troydhanson.github.com/uthash
 */
#ifndef UTHASH_H
#define UTHASH_H

#include <string.h>   /* memcmp,strlen */
#include <stddef.h>   /* ptrdiff_t */
#include <stdlib.h>   /* exit */
#include <stdint.h>   /* uint32_t, uint64_t */

/* C++ compatibility */
#if defined(__cplusplus)
extern "C" {
#endif

/*
 * ***************************************************************************
 * 
 *                           configuration options
 * 
 * ***************************************************************************
 */

/*
 * C macro that is used to obtain the hash value for a key.
 * The default hash function is a string hash function, which is probably
 * the most common case.
 * 
 * If you have keys of a different type, you may want to replace this
 * with your own hash function.
 * 
 * For integer keys, the following hash function is a good choice:
 * 
 *   #define HASH_FUNCTION(key,keylen,hashv)                            \
 *     (hashv) = (unsigned)(key)
 * 
 * For pointers, the following hash function is a good choice:
 *
*    #define HASH_FUNCTION(key,keylen,hashv)                            \
*      (hashv) = (unsigned)(uintptr_t)(key)
*
* For any other key type, you can easily write your own hash function.
* For example, if you have a structure as a key, you can use the
* following hash function:
*
*   #define HASH_FUNCTION(key,keylen,hashv)                            \
*     do {                                                             \
*       unsigned _hf_i;                                                \
*       (hashv) = 0;                                                   \
*       for(_hf_i=0; _hf_i<keylen; _hf_i++) {                           \
*         (hashv) = (((hashv) << 5) + (hashv)) + ((char*)key)[_hf_i];   \
*       }                                                              \
*     } while (0)
*
* The hash function is not used for integer keys (when HASH_INT is used).
*/
#ifndef HASH_FUNCTION
#define HASH_FUNCTION(key,keylen,hashv) HASH_JEN(key,keylen,hashv)
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_INITIAL_NUM_BUCKETS
#define HASH_INITIAL_NUM_BUCKETS 256U
#endif
#ifndef HASH_INITIAL_NUM_BUCKETS_LOG2
#define HASH_INITIAL_NUM_BUCKETS_LOG2 8U
#endif
#ifndef HASH_LOAD_FACTOR
#define HASH_LOAD_FACTOR 0.5
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_RESIZE_FACTOR
#define HASH_RESIZE_FACTOR 2
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_SHRINK_FACTOR
#define HASH_SHRINK_FACTOR 2
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_RESIZE_THRESHOLD
#define HASH_RESIZE_THRESHOLD (HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS)
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_SHRINK_THRESHOLD
#define HASH_SHRINK_THRESHOLD (HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / HASH_SHRINK_FACTOR)
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_BLOOM_FILTERS
#define HASH_BLOOM_FILTERS 0
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_BLOOM_FILTER_SIZE
#define HASH_BLOOM_FILTER_SIZE 32
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_BLOOM_FILTER_HASHES
#define HASH_BLOOM_FILTER_HASHES 3
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_BLOOM_FILTER_SEED
#define HASH_BLOOM_FILTER_SEED 0xdeadbeef
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_BLOOM_FILTER_USE_MURMUR
#define HASH_BLOOM_FILTER_USE_MURMUR 1
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_MURMUR_SEED
#define HASH_MURMUR_SEED 0xdeadbeef
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_MURMUR_FINALIZE
#define HASH_MURMUR_FINALIZE 1
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_JEN_MIX
#define HASH_JEN_MIX(a,b,c)                                                    \
do {                                                                           \
  a -= b; a -= c; a ^= (c >> 13);                                               \
  b -= c; b -= a; b ^= (a << 8);                                                \
  c -= a; c -= b; c ^= (b >> 13);                                               \
  a -= b; a -= c; a ^= (c >> 12);                                               \
  b -= c; b -= a; b ^= (a << 16);                                               \
  c -= a; c -= b; c ^= (b >> 5);                                                \
  a -= b; a -= c; a ^= (c >> 3);                                                \
  b -= c; b -= a; b ^= (a << 10);                                               \
  c -= a; c -= b; c ^= (b >> 15);                                               \
} while (0)
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_SAX_MIX
#define HASH_SAX_MIX(h,d)                                                      \
do {                                                                           \
  (h) ^= ((h) << 5) + ((h) >> 2) + (d);                                         \
} while (0)
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_FNV_MIX
#define HASH_FNV_MIX(h,d)                                                      \
do {                                                                           \
  (h) *= 16777619;                                                             \
  (h) ^= (d);                                                                  \
} while (0)
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_OAT_MIX
#define HASH_OAT_MIX(h,d)                                                      \
do {                                                                           \
  (h) += (d);                                                                  \
  (h) += ((h) << 10);                                                          \
  (h) ^= ((h) >> 6);                                                           \
} while (0)
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_BERNSTEIN_MIX
#define HASH_BERNSTEIN_MIX(h,d)                                                \
do {                                                                           \
  (h) = 33 * (h) + (d);                                                        \
} while (0)
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_SFH_MIX
#define HASH_SFH_MIX(h,d)                                                      \
do {                                                                           \
  (h) += (d);                                                                  \
  (h) += ((h) << 10);                                                          \
  (h) ^= ((h) >> 6);                                                           \
} while (0)
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_CRC32_MIX
#define HASH_CRC32_MIX(h,d)                                                    \
do {                                                                           \
  (h) = ((h) << 5) + (h) + (d);                                                \
} while (0)
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_MURMUR
#define HASH_MURMUR(key,keylen,hashv)                                          \
do {                                                                           \
  const uint32_t m = 0x5bd1e995;                                               \
  const int32_t r = 24;                                                        \
  uint32_t h = HASH_MURMUR_SEED ^ (uint32_t)keylen;                             \
  const unsigned char *data = (const unsigned char *)key;                      \
  while(keylen >= 4) {                                                         \
    uint32_t k = *(uint32_t*)data;                                             \
    k *= m;                                                                    \
    k ^= k >> r;                                                               \
    k *= m;                                                                    \
    h *= m;                                                                    \
    h ^= k;                                                                    \
    data += 4;                                                                 \
    keylen -= 4;                                                               \
  }                                                                            \
  switch(keylen) {                                                             \
    case 3: h ^= data[2] << 16;                                                \
    case 2: h ^= data[1] << 8;                                                 \
    case 1: h ^= data[0];                                                      \
            h *= m;                                                            \
  };                                                                           \
  if (HASH_MURMUR_FINALIZE) {                                                  \
    h ^= h >> 13;                                                              \
    h *= m;                                                                    \
    h ^= h >> 15;                                                              \
  }                                                                            \
  (hashv) = h;                                                                 \
} while (0)
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_JEN
#define HASH_JEN(key,keylen,hashv)                                             \
do {                                                                           \
  unsigned _hj_i,_hj_j,_hj_k;                                                   \
  unsigned const char *_hj_key=(unsigned const char*)(key);                    \
  (hashv) = 0xfeedbeef;                                                        \
  _hj_i = _hj_j = 0x9e3779b9;                                                   \
  _hj_k = (unsigned)(keylen);                                                  \
  while (_hj_k >= 12) {                                                        \
    _hj_i += (_hj_key[0] + ((unsigned)_hj_key[1] << 8)                          \
        + ((unsigned)_hj_key[2] << 16)                                         \
        + ((unsigned)_hj_key[3] << 24));                                       \
    _hj_j += (_hj_key[4] + ((unsigned)_hj_key[5] << 8)                          \
        + ((unsigned)_hj_key[6] << 16)                                         \
        + ((unsigned)_hj_key[7] << 24));                                       \
    (hashv) += (_hj_key[8] + ((unsigned)_hj_key[9] << 8)                        \
        + ((unsigned)_hj_key[10] << 16)                                        \
        + ((unsigned)_hj_key[11] << 24));                                      \
                                                                               \
     HASH_JEN_MIX(_hj_i, _hj_j, (hashv));                                       \
                                                                               \
     _hj_key += 12;                                                            \
     _hj_k -= 12;                                                              \
  }                                                                            \
  (hashv) += (unsigned)(keylen);                                               \
  switch (_hj_k) {                                                             \
     case 11: (hashv) += ((unsigned)_hj_key[10] << 24);                         \
     case 10: (hashv) += ((unsigned)_hj_key[9] << 16);                          \
     case 9:  (hashv) += ((unsigned)_hj_key[8] << 8);                           \
     case 8:  _hj_j += ((unsigned)_hj_key[7] << 24);                            \
     case 7:  _hj_j += ((unsigned)_hj_key[6] << 16);                            \
     case 6:  _hj_j += ((unsigned)_hj_key[5] << 8);                             \
     case 5:  _hj_j += _hj_key[4];                                              \
     case 4:  _hj_i += ((unsigned)_hj_key[3] << 24);                            \
     case 3:  _hj_i += ((unsigned)_hj_key[2] << 16);                            \
     case 2:  _hj_i += ((unsigned)_hj_key[1] << 8);                             \
     case 1:  _hj_i += _hj_key[0];                                              \
  }                                                                            \
  HASH_JEN_MIX(_hj_i, _hj_j, (hashv));                                          \
} while (0)
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_SAX
#define HASH_SAX(key,keylen,hashv)                                             \
do {                                                                           \
  unsigned const char *_hs_key=(unsigned const char*)(key);                    \
  (hashv) = 0;                                                                 \
  while(keylen--) { HASH_SAX_MIX((hashv),*_hs_key++); }                         \
} while (0)
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_FNV
#define HASH_FNV(key,keylen,hashv)                                             \
do {                                                                           \
  unsigned const char *_hf_key=(unsigned const char*)(key);                    \
  (hashv) = 2166136261U;                                                       \
  while(keylen--) { HASH_FNV_MIX((hashv),*_hf_key++); }                         \
} while (0)
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_OAT
#define HASH_OAT(key,keylen,hashv)                                             \
do {                                                                           \
  unsigned const char *_ho_key=(unsigned const char*)(key);                    \
  (hashv) = 0;                                                                 \
  while(keylen--) { HASH_OAT_MIX((hashv),*_ho_key++); }                         \
  (hashv) += ((hashv) << 3);                                                   \
  (hashv) ^= ((hashv) >> 11);                                                  \
  (hashv) += ((hashv) << 15);                                                  \
} while (0)
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_BERNSTEIN
#define HASH_BERNSTEIN(key,keylen,hashv)                                       \
do {                                                                           \
  unsigned const char *_hb_key=(unsigned const char*)(key);                    \
  (hashv) = 0;                                                                 \
  while(keylen--) { HASH_BERNSTEIN_MIX((hashv),*_hb_key++); }                   \
} while (0)
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_SFH
#define HASH_SFH(key,keylen,hashv)                                             \
do {                                                                           \
  unsigned const char *_h_key=(unsigned const char*)(key);                     \
  unsigned _h_tmp;                                                             \
  int _h_rem = (int)keylen & 3;                                                \
  (hashv) = (unsigned)keylen;                                                  \
  keylen >>= 2;                                                                \
  /* Main loop */                                                              \
  while(keylen-- > 0) {                                                        \
    (hashv) += *(unsigned short*)_h_key;                                       \
    _h_tmp = (*(unsigned short*)(_h_key+2) << 11) ^ (hashv);                    \
    (hashv) = ((hashv) << 16) ^ _h_tmp;                                        \
    _h_key += 4;                                                               \
    (hashv) += (hashv) >> 11;                                                  \
  }                                                                            \
  /* Handle end cases */                                                       \
  switch (_h_rem) {                                                            \
    case 3: (hashv) += *(unsigned short*)_h_key;                               \
            (hashv) ^= (hashv) << 16;                                          \
            (hashv) ^= (unsigned)_h_key[2] << 18;                              \
            (hashv) += (hashv) >> 11;                                          \
            break;                                                             \
    case 2: (hashv) += *(unsigned short*)_h_key;                               \
            (hashv) ^= (hashv) << 11;                                          \
            (hashv) += (hashv) >> 17;                                          \
            break;                                                             \
    case 1: (hashv) += *_h_key;                                                \
            (hashv) ^= (hashv) << 10;                                          \
            (hashv) += (hashv) >> 1;                                           \
  }                                                                            \
  /* Force "avalanching" of final 127 bits */                                  \
  (hashv) ^= (hashv) << 3;                                                     \
  (hashv) += (hashv) >> 5;                                                     \
  (hashv) ^= (hashv) << 4;                                                     \
  (hashv) += (hashv) >> 17;                                                    \
  (hashv) ^= (hashv) << 25;                                                    \
  (hashv) += (hashv) >> 6;                                                     \
} while (0)
#endif

/*
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#ifndef HASH_CRC32
#define HASH_CRC32(key,keylen,hashv)                                           \
do {                                                                           \
  unsigned const char *_hc_key=(unsigned const char*)(key);                    \
  (hashv) = 0;                                                                 \
  while(keylen--) { HASH_CRC32_MIX((hashv),*_hc_key++); }                       \
} while (0)
#endif

/*
 * This is a macro that defines the hash table structure.
 * 
 * The hash table is a single static array.
 * The size of this array is given by HASH_INITIAL_NUM_BUCKETS.
 * 
 * The hash table is resized when the number of items in the table
 * exceeds HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS * 2.
 * 
 * The hash table is shrunk when the number of items in the table
 * is less than HASH_LOAD_FACTOR * HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The new size of the hash table is HASH_INITIAL_NUM_BUCKETS / 2.
 * 
 * The default load factor is 0.5.
 * 
 * The default initial number of buckets is 1024.
 * 
 * You can change these values by defining them before including this file.
 */
#define HASH_TABLE(name,type)                                                  \
struct name {                                                                  \
  type *table;                                                                 \
  unsigned num_buckets, log2_num_buckets;                                      \
  unsigned num_items;                                                          \
  struct {                                                                     \
    unsigned num_items, num_buckets;                                           \
    unsigned char *bloom_filter;                                               \
    uint32_t bloom_seed;                                                       \
  } *bloom;                                                                    \
  UT_hash_handle hh;                                                           \
}

/*
 * This is a macro that defines the hash table handle.
 * 
 * The hash table handle is a structure that is added to your structure.
 * 
 * The hash table handle contains the following fields:
 * 
 *   - hh.next: a pointer to the next item in the same bucket
 *   - hh.prev: a pointer to the previous item in the same bucket
 *   - hh.key: a pointer to the key
 *   - hh.keylen: the length of the key
 *   - hh.hashv: the hash value of the key
 */
typedef struct UT_hash_handle {
   struct UT_hash_table *tbl;
   void *prev;                       /* prev element in app order      */
   void *next;                       /* next element in app order      */
   struct UT_hash_handle *hh_prev;   /* previous hh in bucket order    */
   struct UT_hash_handle *hh_next;   /* next hh in bucket order        */
   void *key;                        /* ptr to enclosing struct's key  */
   unsigned keylen;                  /* length of key (0=int)          */
   unsigned hashv;                   /* result of hash function        */
} UT_hash_handle;

/*
 * This is a macro that defines the hash table bucket.
 * 
 * The hash table bucket is a structure that contains the following fields:
 * 
 *   - hh.hh_head: a pointer to the first item in the bucket
 *   - count: the number of items in the bucket
 *   - expand_mult: the expansion multiplier for this bucket
 */
typedef struct UT_hash_bucket {
   struct UT_hash_handle *hh_head;
   unsigned count;
   unsigned expand_mult;
} UT_hash_bucket;

/*
 * This is a macro that defines the hash table.
 * 
 * The hash table is a structure that contains the following fields:
 * 
 *   - buckets: a pointer to the array of buckets
 *   - num_buckets: the number of buckets in the array
 *   - log2_num_buckets: the log2 of the number of buckets
 *   - num_items: the number of items in the hash table
 *   - nonprofit: a pointer to a non-profit item
 *   - tail: a pointer to the tail of the list of items
 *   - hho: the offset of the hash handle in the structure
 *   - bloom_filter: a pointer to the bloom filter
 *   - bloom_seed: the seed for the bloom filter
 *   - bloom_nbuckets: the number of buckets in the bloom filter
 *   - bloom_log2_nbuckets: the log2 of the number of buckets in the bloom filter
 *   - bloom_hashv: the hash value for the bloom filter
 */
typedef struct UT_hash_table {
   UT_hash_bucket *buckets;
   unsigned num_buckets, log2_num_buckets;
   unsigned num_items;
   struct UT_hash_handle *tail; /* tail hh in app order */
   ptrdiff_t hho; /* hash handle offset */
#if HASH_BLOOM_FILTERS
   unsigned char *bloom_filter;
   uint32_t bloom_seed;
   unsigned bloom_nbuckets, bloom_log2_nbuckets;
#endif
} UT_hash_table;

/*
 * This is a macro that finds an item in the hash table.
 * 
 * The hash table is searched for an item with the given key.
 * 
 * If the item is found, a pointer to the item is returned.
 * If the item is not found, NULL is returned.
 * 
 * The key is a pointer to the key data.
 * The keylen is the length of the key data.
 * 
 * The hash value is calculated using the HASH_FUNCTION macro.
 * 
 * The bucket is found by taking the hash value modulo the number of buckets.
 * 
 * The bucket is then searched for the item.
 * 
 * If the item is found, a pointer to the item is returned.
 * If the item is not found, NULL is returned.
 */
#define HASH_FIND(hh,head,keyptr,keylen,out)                                    \
do {                                                                           \
  out=NULL;                                                                    \
  if (head) {                                                                  \
     unsigned _hf_bkt,_hf_hashv;                                               \
     HASH_FUNCTION(keyptr,keylen,_hf_hashv);                                   \
     _hf_bkt = _hf_hashv & (head)->hh.tbl->num_buckets-1;                       \
     if (HASH_BLOOM_FILTERS) {                                                 \
       if (!((head)->hh.tbl->bloom_filter[_hf_hashv & (head)->hh.tbl->bloom_nbuckets-1] & (1 << (_hf_hashv >> (head)->hh.tbl->bloom_log2_nbuckets & 7))))) { \
         out=NULL;                                                             \
       } else {                                                                \
         HASH_FIND_IN_BKT((head)->hh.tbl,hh,(head)->hh.tbl->buckets[_hf_bkt],keyptr,keylen,out); \
       }                                                                       \
     } else {                                                                  \
       HASH_FIND_IN_BKT((head)->hh.tbl,hh,(head)->hh.tbl->buckets[_hf_bkt],keyptr,keylen,out); \
     }                                                                         \
  }                                                                            \
} while (0)

/*
 * This is a macro that finds an item in a bucket.
 * 
 * The bucket is searched for an item with the given key.
 * 
 * If the item is found, a pointer to the item is returned.
 * If the item is not found, NULL is returned.
 * 
 * The key is a pointer to the key data.
 * The keylen is the length of the key data.
 * 
 * The hash value is not used.
 * 
 * The bucket is searched by iterating through the linked list of items.
 * 
 * If the item is found, a pointer to the item is returned.
 * If the item is not found, NULL is returned.
 */
#define HASH_FIND_IN_BKT(tbl,hh,bkt,keyptr,keylen,out)                           \
do {                                                                           \
  if (bkt.hh_head) {                                                           \
    DECLTYPE_HH(tbl,hh) _hf_tmp;                                               \
    _hf_tmp = (bkt.hh_head);                                                   \
    while (_hf_tmp) {                                                          \
      if ((_hf_tmp->keylen == keylen) && (!memcmp(_hf_tmp->key,keyptr,keylen))) {\
        out = _hf_tmp;                                                         \
        break;                                                                 \
      }                                                                        \
      _hf_tmp = _hf_tmp->hh_next;                                              \
    }                                                                          \
  }                                                                            \
} while (0)

/*
 * This is a macro that adds an item to the hash table.
 * 
 * The item is added to the hash table.
 * 
 * If the item is already in the hash table, it is not added again.
 * 
 * The key is a pointer to the key data.
 * The keylen is the length of the key data.
 * 
 * The hash value is calculated using the HASH_FUNCTION macro.
 * 
 * The bucket is found by taking the hash value modulo the number of buckets.
 * 
 * The item is added to the head of the bucket's linked list.
 * 
 * The number of items in the hash table is incremented.
 * 
 * If the number of items in the hash table exceeds the resize threshold,
 * the hash table is resized.
 */
#define HASH_ADD(hh,head,fieldname,keylen_in,add)                              \
        HASH_ADD_KEYPTR(hh,head,&((add)->fieldname),keylen_in,add)

/*
 * This is a macro that adds an item to the hash table.
 * 
 * The item is added to the hash table.
 * 
 * If the item is already in the hash table, it is not added again.
 * 
 * The key is a pointer to the key data.
 * The keylen is the length of the key data.
 * 
 * The hash value is calculated using the HASH_FUNCTION macro.
 * 
 * The bucket is found by taking the hash value modulo the number of buckets.
 * 
 * The item is added to the head of the bucket's linked list.
 * 
 * The number of items in the hash table is incremented.
 * 
 * If the number of items in the hash table exceeds the resize threshold,
 * the hash table is resized.
 */
#define HASH_ADD_KEYPTR(hh,head,keyptr,keylen_in,add)                            \
do {                                                                           \
    unsigned _ha_hashv;                                                        \
    (add)->hh.key = (keyptr);                                                  \
    (add)->hh.keylen = (keylen_in);                                            \
    if (!(head)) {                                                             \
        head = (add);                                                          \
        (head)->hh.prev = NULL;                                                \
        (head)->hh.next = NULL;                                                \
        HASH_MAKE_TABLE(hh,head);                                              \
    } else {                                                                   \
        (head)->hh.tbl->tail->next = (add);                                    \
        (add)->hh.prev = (head)->hh.tbl->tail;                                 \
        (head)->hh.tbl->tail = (add);                                          \
        (add)->hh.next = NULL;                                                 \
    }                                                                          \
    (head)->hh.tbl->num_items++;                                               \
    HASH_FUNCTION((keyptr), (keylen_in), _ha_hashv);                           \
    (add)->hh.hashv = _ha_hashv;                                               \
    HASH_ADD_TO_BKT(head,add);                                                 \
    HASH_BLOOM_ADD(head,add);                                                  \
    HASH_RESIZE_IF_NEEDED(hh,head);                                            \
} while (0)

/*
 * This is a macro that adds an item to a bucket.
 * 
 * The item is added to the head of the bucket's linked list.
 * 
 * The number of items in the bucket is incremented.
 */
#define HASH_ADD_TO_BKT(head,add)                                              \
do {                                                                           \
    unsigned _ha_bkt;                                                          \
    _ha_bkt = (add)->hh.hashv & ((head)->hh.tbl->num_buckets-1);                \
    (add)->hh.hh_next = (head)->hh.tbl->buckets[_ha_bkt].hh_head;               \
    (add)->hh.hh_prev = NULL;                                                  \
    if ((head)->hh.tbl->buckets[_ha_bkt].hh_head) {                             \
        (head)->hh.tbl->buckets[_ha_bkt].hh_head->hh_prev = (add);              \
    }                                                                          \
    (head)->hh.tbl->buckets[_ha_bkt].hh_head = (add);                           \
    (head)->hh.tbl->buckets[_ha_bkt].count++;                                  \
} while (0)

/*
 * This is a macro that replaces an item in the hash table.
 * 
 * The item is replaced in the hash table.
 * 
 * If the item is not in the hash table, it is added.
 * 
 * The key is a pointer to the key data.
 * The keylen is the length of the key data.
 * 
 * The hash value is calculated using the HASH_FUNCTION macro.
 * 
 * The bucket is found by taking the hash value modulo the number of buckets.
 * 
 * The item is added to the head of the bucket's linked list.
 * 
 * The number of items in the hash table is incremented.
 * 
 * If the number of items in the hash table exceeds the resize threshold,
 * the hash table is resized.
 */
#define HASH_REPLACE(hh,head,fieldname,keylen_in,add,replaced)                 \
do {                                                                           \
    replaced=NULL;                                                             \
    HASH_FIND(hh,head,&((add)->fieldname),keylen_in,replaced);                  \
    if (replaced) {                                                            \
        HASH_DELETE(hh,head,replaced);                                         \
    }                                                                          \
    HASH_ADD(hh,head,fieldname,keylen_in,add);                                 \
} while (0)

/*
 * This is a macro that replaces an item in the hash table.
 * 
 * The item is replaced in the hash table.
 * 
 * If the item is not in the hash table, it is added.
 * 
 * The key is a pointer to the key data.
 * The keylen is the length of the key data.
 * 
 * The hash value is calculated using the HASH_FUNCTION macro.
 * 
 * The bucket is found by taking the hash value modulo the number of buckets.
 * 
 * The item is added to the head of the bucket's linked list.
 * 
 * The number of items in the hash table is incremented.
 * 
 * If the number of items in the hash table exceeds the resize threshold,
 * the hash table is resized.
 */
#define HASH_REPLACE_KEYPTR(hh,head,keyptr,keylen_in,add,replaced)               \
do {                                                                           \
    replaced=NULL;                                                             \
    HASH_FIND(hh,head,keyptr,keylen_in,replaced);                              \
    if (replaced) {                                                            \
        HASH_DELETE(hh,head,replaced);                                         \
    }                                                                          \
    HASH_ADD_KEYPTR(hh,head,keyptr,keylen_in,add);                             \
} while (0)

/*
 * This is a macro that deletes an item from the hash table.
 * 
 * The item is deleted from the hash table.
 * 
 * The number of items in the hash table is decremented.
 * 
 * If the number of items in the hash table is less than the shrink threshold,
 * the hash table is shrunk.
 */
#define HASH_DELETE(hh,head,del)                                               \
do {                                                                           \
    if ((del)->hh.prev) {                                                      \
        (del)->hh.prev->next = (del)->hh.next;                                 \
    } else {                                                                   \
        (head) = (del)->hh.next;                                               \
    }                                                                          \
    if ((del)->hh.next) {                                                      \
        (del)->hh.next->prev = (del)->hh.prev;                                 \
    } else {                                                                   \
        (head)->hh.tbl->tail = (del)->hh.prev;                                 \
    }                                                                          \
    HASH_REMOVE_FROM_BKT(head,del);                                            \
    (head)->hh.tbl->num_items--;                                               \
    HASH_BLOOM_DEL(head,del);                                                  \
    HASH_SHRINK_IF_NEEDED(hh,head);                                            \
} while (0)

/*
 * This is a macro that removes an item from a bucket.
 * 
 * The item is removed from the bucket's linked list.
 * 
 * The number of items in the bucket is decremented.
 */
#define HASH_REMOVE_FROM_BKT(head,del)                                         \
do {                                                                           \
    unsigned _hd_bkt;                                                          \
    _hd_bkt = (del)->hh.hashv & ((head)->hh.tbl->num_buckets-1);                \
    if ((del)->hh.hh_prev) {                                                   \
        (del)->hh.hh_prev->hh_next = (del)->hh.hh_next;                         \
    } else {                                                                   \
        (head)->hh.tbl->buckets[_hd_bkt].hh_head = (del)->hh.hh_next;           \
    }                                                                          \
    if ((del)->hh.hh_next) {                                                   \
        (del)->hh.hh_next->hh_prev = (del)->hh.hh_prev;                         \
    }                                                                          \
    (head)->hh.tbl->buckets[_hd_bkt].count--;                                  \
} while (0)

/*
 * This is a macro that deletes all items from the hash table.
 * 
 * The hash table is cleared of all items.
 * 
 * The hash table is not freed.
 */
#define HASH_CLEAR(hh,head)                                                    \
do {                                                                           \
  if (head) {                                                                  \
    DECLTYPE_HH(head,hh) _tmp, _safe;                                          \
    _tmp = (head);                                                             \
    while(_tmp) {                                                              \
      _safe = _tmp->hh.next;                                                   \
      HASH_DELETE(hh,head,_tmp);                                               \
      _tmp = _safe;                                                            \
    }                                                                          \
  }                                                                            \
} while (0)

/*
 * This is a macro that counts the number of items in the hash table.
 * 
 * The number of items in the hash table is returned.
 */
#define HASH_COUNT(head) ((head)?((head)->hh.tbl->num_items):0)

/*
 * This is a macro that iterates over the items in the hash table.
 * 
 * The user-supplied function is called for each item in the hash table.
 * 
 * The user-supplied function is passed a pointer to the item.
 * 
 * The user-supplied function should return 0 to continue iterating,
 * or non-zero to stop iterating.
 */
#define HASH_ITER(hh,head,el,tmp)                                              \
  for((el)=(head), (tmp)=(((el))?((el)->hh.next):NULL);                         \
      (el); (el)=(tmp), (tmp)=(((el))?((el)->hh.next):NULL))

/*
 * This is a macro that sorts the items in the hash table.
 * 
 * The items in the hash table are sorted using the user-supplied
 * comparison function.
 * 
 * The user-supplied comparison function is passed two pointers to items.
 * 
 * The user-supplied comparison function should return a negative value
 * if the first item is less than the second item, a positive value if
 * the first item is greater than the second item, and zero if the
 * two items are equal.
 */
#define HASH_SORT(head,cmp)                                                    \
do {                                                                           \
  if (head) {                                                                  \
    DECLTYPE_HH(head,hh) _hs_p, _hs_q;                                          \
    unsigned _hs_i, _hs_n;                                                     \
    _hs_n = HASH_COUNT(head);                                                  \
    for(_hs_i=0; _hs_i<_hs_n-1; _hs_i++) {                                      \
      _hs_p = head;                                                            \
      for(_hs_j=0; _hs_j<_hs_n-_hs_i-1; _hs_j++) {                              \
        _hs_q = _hs_p->hh.next;                                                \
        if (cmp(_hs_p,_hs_q) > 0) {                                            \
          if (_hs_p->hh.prev) {                                                \
            _hs_p->hh.prev->next = _hs_q;                                      \
          } else {                                                             \
            head = _hs_q;                                                      \
          }                                                                    \
          if (_hs_q->hh.next) {                                                \
            _hs_q->hh.next->prev = _hs_p;                                      \
          } else {                                                             \
            head->hh.tbl->tail = _hs_p;                                        \
          }                                                                    \
          _hs_p->hh.next = _hs_q->hh.next;                                     \
          _hs_q->hh.prev = _hs_p->hh.prev;                                     \
          _hs_p->hh.prev = _hs_q;                                              \
          _hs_q->hh.next = _hs_p;                                              \
        } else {                                                               \
          _hs_p = _hs_q;                                                       \
        }                                                                      \
      }                                                                        \
    }                                                                          \
  }                                                                            \
} while (0)

/*
 * This is a macro that selects a non-profit item from the hash table.
 * 
 * A non-profit item is an item that is not in any bucket.
 * 
 * This is used to avoid having to allocate a new item when the hash table
 * is resized.
 */
#define HASH_SELECT_NONPROFIT(tbl)                                             \
do {                                                                           \
  if ((tbl)->nonprofit) {                                                      \
    (tbl)->nonprofit->hh.next = (tbl)->tail;                                   \
    (tbl)->tail->next = (tbl)->nonprofit;                                      \
    (tbl)->tail = (tbl)->nonprofit;                                            \
    (tbl)->nonprofit = NULL;                                                   \
  }                                                                            \
} while (0)

/*
 * This is a macro that makes a hash table.
 * 
 * The hash table is initialized.
 * 
 * The number of buckets is set to the initial number of buckets.
 * The log2 of the number of buckets is calculated.
 * The number of items is set to 0.
 * The tail is set to the head.
 * The hash handle offset is calculated.
 * The bloom filter is initialized.
 */
#define HASH_MAKE_TABLE(hh,head)                                               \
do {                                                                           \
    (head)->hh.tbl = (UT_hash_table*)malloc(sizeof(UT_hash_table));             \
    if (!((head)->hh.tbl)) { exit(-1); }                                        \
    (head)->hh.tbl->tail = (head);                                             \
    (head)->hh.tbl->num_items = 0;                                             \
    (head)->hh.tbl->hho = (char*)(&(head)->hh) - (char*)(head);                 \
    (head)->hh.tbl->num_buckets = HASH_INITIAL_NUM_BUCKETS;                     \
    (head)->hh.tbl->log2_num_buckets = HASH_INITIAL_NUM_BUCKETS_LOG2;           \
    (head)->hh.tbl->buckets = (UT_hash_bucket*)calloc(HASH_INITIAL_NUM_BUCKETS, sizeof(UT_hash_bucket)); \
    if (!((head)->hh.tbl->buckets)) { exit(-1); }                               \
    HASH_BLOOM_MAKE(head);                                                     \
} while (0)

/*
 * This is a macro that resizes the hash table if needed.
 * 
 * If the number of items in the hash table exceeds the resize threshold,
 * the hash table is resized.
 * 
 * The new number of buckets is twice the old number of buckets.
 * The new log2 of the number of buckets is one more than the old log2.
 * 
 * The items are re-hashed into the new buckets.
 */
#define HASH_RESIZE_IF_NEEDED(hh,head)                                         \
do {                                                                           \
  if (((head)->hh.tbl->num_items > ((head)->hh.tbl->num_buckets * HASH_LOAD_FACTOR))) { \
    HASH_EXPAND_BUCKETS(hh,head);                                              \
  }                                                                            \
} while (0)

/*
 * This is a macro that expands the number of buckets in the hash table.
 * 
 * The new number of buckets is twice the old number of buckets.
 * The new log2 of the number of buckets is one more than the old log2.
 * 
 * The items are re-hashed into the new buckets.
 */
#define HASH_EXPAND_BUCKETS(hh,head)                                           \
do {                                                                           \
    unsigned _he_bkt;                                                          \
    unsigned _he_new_num_buckets;                                              \
    UT_hash_bucket *_he_new_buckets;                                           \
    DECLTYPE_HH(head,hh) _he_thh, _he_hh_next;                                  \
                                                                               \
    _he_new_num_buckets = (head)->hh.tbl->num_buckets * 2;                     \
    _he_new_buckets = (UT_hash_bucket*)realloc((head)->hh.tbl->buckets,         \
            _he_new_num_buckets * sizeof(UT_hash_bucket));                     \
    if (!_he_new_buckets) { exit(-1); }                                         \
    (head)->hh.tbl->buckets = _he_new_buckets;                                 \
    (head)->hh.tbl->num_buckets = _he_new_num_buckets;                         \
    (head)->hh.tbl->log2_num_buckets++;                                        \
    memset(&((head)->hh.tbl->buckets[(head)->hh.tbl->num_buckets/2]), 0,        \
           (head)->hh.tbl->num_buckets/2 * sizeof(UT_hash_bucket));             \
                                                                               \
    for(_he_bkt=0; _he_bkt < (head)->hh.tbl->num_buckets/2; _he_bkt++) {         \
        _he_thh = (head)->hh.tbl->buckets[_he_bkt].hh_head;                     \
        while(_he_thh) {                                                       \
            _he_hh_next = _he_thh->hh_next;                                    \
            if ((_he_thh->hashv & ((head)->hh.tbl->num_buckets-1)) != _he_bkt) {\
                HASH_REMOVE_FROM_BKT(head,_he_thh);                            \
                HASH_ADD_TO_BKT(head,_he_thh);                                 \
            }                                                                  \
            _he_thh = _he_hh_next;                                             \
        }                                                                      \
    }                                                                          \
} while (0)

/*
 * This is a macro that shrinks the hash table if needed.
 * 
 * If the number of items in the hash table is less than the shrink threshold,
 * the hash table is shrunk.
 * 
 * The new number of buckets is half the old number of buckets.
 * The new log2 of the number of buckets is one less than the old log2.
 * 
 * The items are re-hashed into the new buckets.
 */
#define HASH_SHRINK_IF_NEEDED(hh,head)                                         \
do {                                                                           \
  if (((head)->hh.tbl->num_items < ((head)->hh.tbl->num_buckets * HASH_LOAD_FACTOR / 2)) && \
      ((head)->hh.tbl->num_buckets > HASH_INITIAL_NUM_BUCKETS)) {               \
    HASH_SHRINK_BUCKETS(hh,head);                                              \
  }                                                                            \
} while (0)

/*
 * This is a macro that shrinks the number of buckets in the hash table.
 * 
 * The new number of buckets is half the old number of buckets.
 * The new log2 of the number of buckets is one less than the old log2.
 * 
 * The items are re-hashed into the new buckets.
 */
#define HASH_SHRINK_BUCKETS(hh,head)                                           \
do {                                                                           \
    unsigned _hs_bkt;                                                          \
    unsigned _hs_new_num_buckets;                                              \
    UT_hash_bucket *_hs_new_buckets;                                           \
    DECLTYPE_HH(head,hh) _hs_thh, _hs_hh_next;                                  \
                                                                               \
    _hs_new_num_buckets = (head)->hh.tbl->num_buckets / 2;                     \
    _hs_new_buckets = (UT_hash_bucket*)realloc((head)->hh.tbl->buckets,         \
            _hs_new_num_buckets * sizeof(UT_hash_bucket));                     \
    if (!_hs_new_buckets) { exit(-1); }                                         \
    (head)->hh.tbl->buckets = _hs_new_buckets;                                 \
    (head)->hh.tbl->num_buckets = _hs_new_num_buckets;                         \
    (head)->hh.tbl->log2_num_buckets--;                                        \
                                                                               \
    for(_hs_bkt=0; _hs_bkt < (head)->hh.tbl->num_buckets; _hs_bkt++) {           \
        _hs_thh = (head)->hh.tbl->buckets[_hs_bkt].hh_head;                     \
        while(_hs_thh) {                                                       \
            _hs_hh_next = _hs_thh->hh_next;                                    \
            if ((_hs_thh->hashv & ((head)->hh.tbl->num_buckets-1)) != _hs_bkt) {\
                HASH_REMOVE_FROM_BKT(head,_hs_thh);                            \
                HASH_ADD_TO_BKT(head,_hs_thh);                                 \
            }                                                                  \
            _hs_thh = _hs_hh_next;                                             \
        }
    }
} while (0)

/*
 * This is a macro that makes a bloom filter.
 * 
 * The bloom filter is initialized.
 * 
 * The number of buckets is set to HASH_BLOOM_FILTER_SIZE.
 * The log2 of the number of buckets is calculated.
 * The seed is set to HASH_BLOOM_FILTER_SEED.
 * The bloom filter is allocated.
 */
#define HASH_BLOOM_MAKE(head)                                                  \
do {                                                                           \
  if (HASH_BLOOM_FILTERS) {                                                    \
    (head)->hh.tbl->bloom_nbuckets = HASH_BLOOM_FILTER_SIZE;                    \
    (head)->hh.tbl->bloom_log2_nbuckets = 0;                                   \
    while ((1U << (head)->hh.tbl->bloom_log2_nbuckets) < HASH_BLOOM_FILTER_SIZE) { \
      (head)->hh.tbl->bloom_log2_nbuckets++;                                   \
    }                                                                          \
    (head)->hh.tbl->bloom_seed = HASH_BLOOM_FILTER_SEED;                        \
    (head)->hh.tbl->bloom_filter = (unsigned char*)calloc((head)->hh.tbl->bloom_nbuckets, sizeof(unsigned char)); \
    if (!((head)->hh.tbl->bloom_filter)) { exit(-1); }                          \
  }                                                                            \
} while (0)

/*
 * This is a macro that adds an item to the bloom filter.
 * 
 * The item is added to the bloom filter.
 * 
 * The hash value is calculated using the HASH_FUNCTION macro.
 * 
 * The bloom filter is updated.
 */
#define HASH_BLOOM_ADD(head,add)                                               \
do {                                                                           \
  if (HASH_BLOOM_FILTERS) {                                                    \
    unsigned _hb_bkt;                                                          \
    _hb_bkt = (add)->hh.hashv & ((head)->hh.tbl->bloom_nbuckets-1);             \
    (head)->hh.tbl->bloom_filter[_hb_bkt] |= (1 << ((add)->hh.hashv >> (head)->hh.tbl->bloom_log2_nbuckets & 7)); \
  }                                                                            \
} while (0)

/*
 * This is a macro that deletes an item from the bloom filter.
 * 
 * The item is deleted from the bloom filter.
 * 
 * The hash value is calculated using the HASH_FUNCTION macro.
 * 
 * The bloom filter is updated.
 */
#define HASH_BLOOM_DEL(head,del)                                               \
do {                                                                           \
  if (HASH_BLOOM_FILTERS) {                                                    \
    unsigned _hb_bkt;                                                          \
    _hb_bkt = (del)->hh.hashv & ((head)->hh.tbl->bloom_nbuckets-1);             \
    (head)->hh.tbl->bloom_filter[_hb_bkt] &= ~(1 << ((del)->hh.hashv >> (head)->hh.tbl->bloom_log2_nbuckets & 7)); \
  }                                                                            \
} while (0)

/*
 * This is a macro that frees the hash table.
 * 
 * The hash table is freed.
 */
#define HASH_FREE_TABLE(hh,head)                                               \
do {                                                                           \
  if (head) {                                                                  \
    free((head)->hh.tbl->buckets);                                             \
    if (HASH_BLOOM_FILTERS) {                                                  \
      free((head)->hh.tbl->bloom_filter);                                      \
    }                                                                          \
    free((head)->hh.tbl);                                                      \
  }                                                                            \
} while (0)

/*
 * This is a macro that gets the type of the hash handle.
 */
#define DECLTYPE_HH(head,hh) typeof(head)

#if defined(__cplusplus)
}
#endif

#endif /* UTHASH_H */
