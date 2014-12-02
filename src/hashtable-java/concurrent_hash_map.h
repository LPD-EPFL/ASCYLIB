/*   
 *   File: concurrent_hash_map.h
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: Similar to Java's ConcurrentHashMap. 
 *   Doug Lea. 1.3.4. http://gee.cs.oswego.edu/dl/classes/EDU/oswego/
 *   cs/dl/util/concurrent/intro.html, 2003.
 *   concurrent_hash_map.h is part of ASCYLIB
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

#define CHM_READ_ONLY_FAIL              RO_FAIL
#define CHM_NUM_SEGMENTS                128

/* 
 * structures
 */

typedef volatile struct chm_node
{
  skey_t key;
  sval_t val;
  volatile struct chm_node* next;
} chm_node_t;

typedef struct ALIGNED(64) chm
{
  union
  {
    struct
    {
      size_t num_buckets;
      size_t hash;
      size_t num_segments;
      size_t hash_seg;
      chm_node_t** table;
      ptlock_t* locks;
    };
    uint8_t padding[CACHE_LINE_SIZE];
  };
} chm_t;

/* 
 * interface
 */

chm_t* chm_new();
sval_t chm_get(chm_t* set, skey_t key);
int chm_put(chm_t* set, skey_t key, sval_t val);
sval_t chm_rem(chm_t* set, skey_t key);
size_t chm_size(chm_t* set);

extern __thread ssmem_allocator_t* alloc;
extern size_t maxhtlength;
#endif	/* _CONCURRENT_HASH_MAP_H_ */
