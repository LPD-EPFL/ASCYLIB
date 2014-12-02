/*   
 *   File: linkedlist-lock.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   linkedlist-lock.c is part of ASCYLIB
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

node_l_t*
new_node_l(skey_t key, sval_t val, node_l_t* next, int initializing)
{
  volatile node_l_t *node;
#if GC == 1
  if (initializing)		/* for initialization AND the coupling algorithm */
    {
      node = (volatile node_l_t *) ssalloc(sizeof(node_l_t));
    }
  else
    {
      node = (volatile node_l_t *) ssmem_alloc(alloc, sizeof(node_l_t));
    }
#else
  node = (volatile node_l_t *) ssalloc(sizeof(node_l_t));
#endif
  
  if (node == NULL) 
    {
      perror("malloc @ new_node");
      exit(1);
    }

  node->key = key;
  node->val = val;
  node->next = next;
  node->marked = 0;

  INIT_LOCK(ND_GET_LOCK(node));

#if defined(__tile__)
  /* on tilera you may have store reordering causing the pointer to a new node
     to become visible, before the contents of the node are visible */
  MEM_BARRIER;
#endif	/* __tile__ */

  return (node_l_t*) node;
}

intset_l_t *set_new_l()
{
  intset_l_t *set;
  node_l_t *min, *max;

  if ((set = (intset_l_t *)ssalloc_aligned(CACHE_LINE_SIZE, sizeof(intset_l_t))) == NULL) 
    {
      perror("malloc");
      exit(1);
    }

  max = new_node_l(KEY_MAX, 0, NULL, 1);
  /* ssalloc_align_alloc(0); */
  min = new_node_l(KEY_MIN, 0, max, 1);
  set->head = min;

#if defined(LL_GLOBAL_LOCK)
  set->lock = (volatile ptlock_t*) ssalloc_aligned(CACHE_LINE_SIZE, sizeof(ptlock_t));
  if (set->lock == NULL)
    {
      perror("malloc");
      exit(1);
    }
  GL_INIT_LOCK(set->lock);
#endif

  MEM_BARRIER;
  return set;
}

void
node_delete_l(node_l_t *node) 
{
  DESTROY_LOCK(&node->lock);
#if GC == 1
  ssfree((void*) node);
#endif
}

void set_delete_l(intset_l_t *set)
{
  node_l_t *node, *next;

  node = set->head;
  while (node != NULL) 
    {
      next = node->next;
      DESTROY_LOCK(&node->lock);
      /* free(node); */
      ssfree((void*) node);		/* TODO : fix with ssmem */
      node = next;
    }
  ssfree(set);
}

int set_size_l(intset_l_t *set)
{
  int size = 0;
  node_l_t *node;

  /* We have at least 2 elements */
  node = set->head->next;
  while (node->next != NULL) 
    {
      size++;
      node = node->next;
    }

  return size;
}



	
