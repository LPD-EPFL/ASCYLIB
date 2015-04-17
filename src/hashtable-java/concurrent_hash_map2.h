/*   
 *   File: concurrent_hash_map2.h
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: Similar to Java's ConcurrentHashMap. 
 *   Doug Lea. 1.3.4. http://gee.cs.oswego.edu/dl/classes/EDU/oswego/
 *   cs/dl/util/concurrent/intro.html, 2003.
 *   concurrent_hash_map2.h is part of ASCYLIB
 *
 * Copyright (c) 2014 Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>,
 * 	     	      Tudor David <tudor.david@epfl.ch>
 *	      	      Distributed Programming Lab (LPD), EPFL
 *
 * ASCYLIB is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _CONCURRENT_HASH_MAP_H_
#define _CONCURRENT_HASH_MAP_H_

#include "utils.h"
#include "common.h"
#include "lock_if.h"
#include "ssmem.h"

#define DEFAULT_LOAD                    1
#define MAXHTLENGTH                     65536

/* 
 * parameters
 */

#define CHM_NUM_SEGMENTS                512
#if defined(__tile__)
/* on the tilera we need to keep the ht small to avoid the TLB issues */
#  define CHM_LOAD_FACTOR                 3
#else
#  define CHM_LOAD_FACTOR                 0.75
#endif
#define CHM_TRY_PREFETCH                0
#define CHM_MAX_SCAN_RETRIES            64
#define CHM_READ_ONLY_FAIL              RO_FAIL

/* 
 * structures
 */

typedef volatile struct chm_node
{
  skey_t key;
  sval_t val;
  uint8_t padding32[8];
  volatile struct chm_node* next;
} chm_node_t;

typedef volatile struct ALIGNED(CACHE_LINE_SIZE) chm_seg
{
  union
  {
    struct
    {
      size_t num_buckets;
      size_t hash;
      ptlock_t lock;
      uint32_t modifications;
      uint32_t size;
      float load_factor;
      uint32_t size_limit;
      chm_node_t** table;
    };
    uint8_t padding[CACHE_LINE_SIZE];
  };
} chm_seg_t;

typedef struct ALIGNED(CACHE_LINE_SIZE) chm
{
  union
  {
    struct
    {
      size_t num_segments;
      size_t hash;
      int hash_seed;
      chm_seg_t** segments;
    };
    uint8_t padding[CACHE_LINE_SIZE];
  };
} chm_t;

/* 
 * interface
 */

chm_t* chm_new(size_t capacity, size_t concurrency);
sval_t chm_get(chm_t* set, skey_t key);
int chm_put(chm_t* set, skey_t key, sval_t val);
sval_t chm_rem(chm_t* set, skey_t key);
size_t chm_size(chm_t* set);

extern __thread ssmem_allocator_t* alloc;
extern size_t maxhtlength;
#endif	/* _CONCURRENT_HASH_MAP_H_ */
