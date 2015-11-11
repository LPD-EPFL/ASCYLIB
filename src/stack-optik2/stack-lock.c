/*   
 *   File: skiplist-lock.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   skiplist-lock.c is part of ASCYLIB
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

#include "stack-lock.h"
#include "utils.h"

__thread ssmem_allocator_t* alloc;


mstack_node_t*
mstack_new_node(skey_t key, sval_t val, mstack_node_t* next)
{
  mstack_node_t *node = ssmem_alloc(alloc, sizeof(*node));
  node->key = key;
  node->val = val;
  node->next = next;
	
#ifdef __tile__
  MEM_BARRIER;
#endif

  return node;
}

void
mstack_delete_node(mstack_node_t *n)
{
  ssfree_alloc(1, (void*) n);
}

mstack_t*
mstack_new()
{
  mstack_t *set;
	
  if ((set = (mstack_t*) ssalloc_aligned(CACHE_LINE_SIZE, sizeof(mstack_t))) == NULL)
    {
      perror("malloc");
      exit(1);
    }

  /* mstack_node_t* node = (mstack_node_t*) ssalloc_aligned(CACHE_LINE_SIZE, sizeof(mstack_node_t)); */
  /* node->next = NULL; */
  set->top = NULL;
  int i;
  for (i = 0; i < ELIM_SPOTS; i++)
    {
      set->pushe[i] = NULL;
      set->pope[i] = NULL;
    }

  return set;
}

void
mstack_delete(mstack_t *set)
{
  printf("mstack_delete - implement me\n");
}

int mstack_size(mstack_t *set)
{
  int size = 0;
  mstack_node_t *node;
	
  /* We have at least 2 elements */
  node = set->top;
  while (node != NULL) 
    {
      size++;
      node = node->next;
    }
  return size;
}
