/*   
 *   File: bst-seq.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   bst-seq.c is part of ASCYLIB
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
new_node(skey_t key, sval_t val, int initializing)
{
  volatile node_t* node;
#if GC == 1
  if (unlikely(initializing))		/* for initialization AND the coupling algorithm */
    {
      node = (volatile node_t*) ssalloc(sizeof(node_t));
    }
  else
    {
      node = (volatile node_t*) ssmem_alloc(alloc, sizeof(node_t));
    }
#else
  node = (volatile node_t*) ssalloc(sizeof(node_t));
#endif
  
  if (node == NULL) 
    {
      perror("malloc @ new_node");
      exit(1);
    }

  node->key = key;
  node->val = val;
  node->left = node->right = NULL;

#if defined(__tile__)
  /* on tilera you may have store reordering causing the pointer to a new node
     to become visible, before the contents of the node are visible */
  MEM_BARRIER;
#endif	/* __tile__ */

  return (node_t*) node;
}

intset_t *set_new()
{
  intset_t *set;

  if ((set = (intset_t *)ssalloc_aligned(CACHE_LINE_SIZE, sizeof(intset_t))) == NULL) 
    {
      perror("malloc");
      exit(1);
    }

  set->head = NULL;

  MEM_BARRIER;
  return set;
}

void
node_delete(node_t *node) 
{
#if GC == 1 && SEQ_SSMEM_NO_FREE != 1
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
node_size(volatile node_t* n)
{
  if (n == NULL)
    {
      return 0;
    }
  else
    {
      return 1 + node_size(n->left) + node_size(n->right);
    }
}

int 
set_size(intset_t* set)
{
  int size = node_size(set->head);;
  return size;
}



	
