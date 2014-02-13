/*
 * File:
 *   intset.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Integer set operations accessing the hashtable
 *
 * Copyright (c) 2009-2010.
 *
 * intset.c is part of Synchrobench
 * 
 * Synchrobench is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */


/* #include "hashtable.h" */
#include "common.h"
#include "ssmem.h"

#include <urcu.h>		/* RCU flavor */
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
#define USE_RCU_GC 0

extern __thread ssmem_allocator_t* alloc;

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
node_free(struct cds_lfht_node* ht_node)
{
  UNUSED node_t* node = caa_container_of(ht_node, node_t, node);
#if GC == 1
#if USE_RCU_GC == 1
  rcu_barrier();  /* synchronize_rcu(); */
  ssfree(node);
#else
  ssmem_free(alloc, node);
#endif
#endif
}


#define DEFAULT_LOAD 1
