/*   
 *   File: intset.h
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: using the urcu library
 *   Mathieu Desnoyers, Paul E McKenney, Alan S Stern, Michel R Da-
 *   genais, and Jonathan Walpole. User-level implementations of read-
 *   copy update. Parallel and Distributed Systems, IEEE Transactions on,
 *   23(2):375–382, 2012.
 *   intset.h is part of ASCYLIB
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

/* #include "hashtable.h" */
#include "common.h"
#include "ssmem.h"

#define RCU_SIGNAL
#include <urcu.h>		/* RCU flavor */
/* #include <urcu-qsbr.h> */
#include <urcu/rculfhash.h>	/* RCU Lock-free hash table */

typedef struct node
{
  skey_t key;
  sval_t val;
  struct cds_lfht_node node;	/* Chaining in hash table */
} node_t;

typedef struct cds_lfht cds_lfht_t;

static inline int
match(struct cds_lfht_node* ht_node, const void *_key)
{
  node_t* node = caa_container_of(ht_node, node_t, node);
  skey_t key  = *(skey_t*) _key;
  return node->key == key;
}

static inline int
cds_lfht_size(cds_lfht_t* ht)
{
  struct cds_lfht_iter iter;
  struct cds_lfht_node* node;
  size_t size = 0;
  cds_lfht_for_each(ht, &iter, node)
    {
      size++;
    }

  return size;
}


/*******************************************************************************/
/* mem management **************************************************************/
/*******************************************************************************/
#define USE_RCU_GC 1
#define RCU_WAIT() synchronize_rcu();


#if USE_RCU_GC == 1
#  define RCU_RLOCK()  rcu_read_lock()
#  define RCU_RUNLOCK()  rcu_read_unlock()
#else
#  define RCU_RLOCK()
#  define RCU_RUNLOCK()
#endif

extern __thread ssmem_allocator_t *alloc, *alloc_data;

static inline void
node_init(node_t** node)
{
  if (*node == NULL)
    {
#if GC == 1 && USE_RCU_GC != 1
      *node = (node_t*) ssmem_alloc(alloc, sizeof(node_t));
#else
      *node = (node_t*) ssalloc(sizeof(node_t));
#endif
    }
}

static inline void
value_init(size_t** val, size_t size)
{
  if (*val == NULL)
    {
#if GC == 1 && USE_RCU_GC != 1
      *val = (size_t*) ssmem_alloc(alloc_data, size);
#else
      *val = (size_t*) ssalloc_alloc(1, size);
#endif
    }
}

static inline void
node_free(struct cds_lfht_node* ht_node)
{
  UNUSED node_t* node = caa_container_of(ht_node, node_t, node);
#if USE_RCU_GC == 1
  RCU_WAIT();
  ssfree(node);
#else
  ssmem_free(alloc, node);
#endif
}

static inline void
value_free(size_t* val)
{
#if USE_RCU_GC == 1
  RCU_WAIT();
  ssfree_alloc(1, val);
#else
  ssmem_free(alloc_data, val);
#endif
}

#define DEFAULT_LOAD 1
