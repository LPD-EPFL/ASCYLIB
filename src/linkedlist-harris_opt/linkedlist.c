/*   
 *   File: linkedlist.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   linkedlist.c is part of ASCYLIB
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

#include "linkedlist.h"

__thread ssmem_allocator_t* alloc;

node_t*
new_node(skey_t key, sval_t val, node_t *next, int initializing)
{
  volatile node_t *node;

#if GC == 1
  if (unlikely(initializing))
    {
      node = (volatile node_t *) ssalloc(sizeof(node_t));
    }
  else
    {
      node = (volatile node_t *) ssmem_alloc(alloc, sizeof(node_t));
    }

#else
  node = (volatile node_t *) ssalloc(sizeof(node_t));
#endif

  if (node == NULL) 
    {
      perror("malloc @ new_node");
      exit(1);
    }

  node->key = key;
  node->val = val;
  node->next = next;
  return (node_t*) node;
}

intset_t* 
set_new()
{
  intset_t *set;
  node_t *min, *max;
	
  if ((set = (intset_t*)ssalloc_aligned(CACHE_LINE_SIZE, sizeof(intset_t))) == NULL)
    {
      perror("malloc");
      exit(1);
    }

  max = new_node(KEY_MAX, 0, NULL, 1);
  min = new_node(KEY_MIN, 0, max, 1);
  set->head = min;

  return set;
}

void set_delete(intset_t *set)
{
  node_t *node, *next;

  node = set->head;
  while (node != NULL) {
    next = node->next;
    free((void*) node);
    node = next;
  }
  free(set);
}


