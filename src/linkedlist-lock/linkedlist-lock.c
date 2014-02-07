/*
 * File:
 *   linkedlist-lock.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Lock-based linked list implementation of an integer set
 *
 * Copyright (c) 2009-2010.
 *
 * linkedlist-lock.c is part of Synchrobench
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

#include "intset.h"
#include "utils.h"

__thread ssmem_allocator_t* alloc;

node_l_t*
new_node_l(val_t val, node_l_t* next, int transactional)
{
  volatile node_l_t *node;
#if GC == 1
  if (transactional)		/* for initialization AND the coupling algorithm */
    {
      node = (volatile node_l_t *) ssalloc(sizeof(node_l_t));
    }
  else
    {
      node = (volatile node_l_t *) ssmem_alloc(alloc, sizeof(node_l_t));
    }

  if (node == NULL) 
    {
      perror("malloc @ new_node");
      exit(1);
    }
#else
  node = (volatile node_l_t *) ssalloc(sizeof(node_l_t));
#endif
  
  node->val = val;
  node->next = next;

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

  if ((set = (intset_l_t *)ssalloc(sizeof(intset_l_t))) == NULL) 
    {
      perror("malloc");
      exit(1);
    }

  ssalloc_align_alloc(0);
  max = new_node_l(VAL_MAX, NULL, 1);
  ssalloc_align_alloc(0);
  min = new_node_l(VAL_MIN, max, 1);
  set->head = min;

  ssalloc_align_alloc(0);
#if defined(LL_GLOBAL_LOCK)
  set->lock = (volatile ptlock_t*) ssalloc(sizeof(ptlock_t));
  if (set->lock == NULL)
    {
      perror("malloc");
      exit(1);
    }
  GL_INIT_LOCK(set->lock);
  ssalloc_align_alloc(0);
#endif

  MEM_BARRIER;
  return set;
}

void
node_delete_l(node_l_t *node) 
{
  DESTROY_LOCK(&node->lock);
#if GC == 1
  ssfree(node);
#endif
  /* free(node); */
}

void set_delete_l(intset_l_t *set)
{
  node_l_t *node, *next;

  node = set->head;
  while (node != NULL) {
    next = (node_l_t*) get_unmarked_ref((uintptr_t) node->next);
    DESTROY_LOCK(&node->lock);
    /* free(node); */
    ssfree(node);		/* TODO : fix with ssmem */
    node = next;
  }
  ssfree(set);
}

int set_size_l(intset_l_t *set)
{
  int size = 0;
  node_l_t *node;

  /* We have at least 2 elements */
  node = (node_l_t*) get_unmarked_ref((uintptr_t) set->head->next);
  while (node->next != NULL) 
    {
      size++;
      node = (node_l_t*) get_unmarked_ref((uintptr_t) node->next);
    }

  return size;
}



	
