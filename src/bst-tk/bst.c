/*   
 *   File: bst.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   bst.c is part of ASCYLIB
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

#include "intset.h"
#include "utils.h"

__thread ssmem_allocator_t* alloc;

node_t*
new_node(skey_t key, sval_t val, node_t* l, node_t* r, int initializing)
{
  node_t* node;
#if GC == 1
  if (likely(!initializing))		/* for initialization AND the coupling algorithm */
    {
      node = (node_t*) ssmem_alloc(alloc, sizeof(node_t));
    }
  else
    {
      node = (node_t*) ssalloc(sizeof(node_t));
    }
#else
  node = (node_t*) ssalloc(sizeof(node_t));
#endif
  
  if (node == NULL) 
    {
      perror("malloc @ new_node");
      exit(1);
    }

  node->key = key;
  node->val = val;
  node->left = l;
  node->right = r;
  node->lock.to_uint64 = 0;

  return (node_t*) node;
}

node_t*
new_node_no_init()
{
  node_t* node;
#if GC == 1
  node = (node_t*) ssmem_alloc(alloc, sizeof(node_t));
#else
  node = (node_t*) ssalloc(sizeof(node_t));
#endif
  if (unlikely(node == NULL))
    {
      perror("malloc @ new_node");
      exit(1);
    }

  node->val = 0;
  node->lock.to_uint64 = 0;

  return (node_t*) node;
}



intset_t* set_new()
{
  intset_t *set;

  if ((set = (intset_t *)ssalloc_aligned(CACHE_LINE_SIZE, sizeof(intset_t))) == NULL) 
    {
      perror("malloc");
      exit(1);
    }

  node_t* min = new_node(INT_MIN, 1, NULL, NULL, 1);
  node_t* max = new_node(INT_MAX, 1, NULL, NULL, 1);
  set->head = new_node(INT_MAX, 0, min, max, 1);
  MEM_BARRIER;
  return set;
}

void
node_delete(node_t *node) 
{
#if GC == 1
  ssmem_free(alloc, node);
#else
  /* ssfree(node); */
#endif
}

void
set_delete_l(intset_t *set)
{
  /* TODO: implement */
}

static int
node_size(node_t* n)
{
 if (n->leaf != 0)
    {
      return 1;
    }
  else
    {
      return node_size((node_t*) n->left) + node_size((node_t*) n->right);
    }
}

int 
set_size(intset_t* set)
{
  int size = node_size(set->head) - 2;
  return size;
}



	
