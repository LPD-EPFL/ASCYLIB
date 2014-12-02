/*   
 *   File: linkedlist-seq.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   linkedlist-seq.c is part of ASCYLIB
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
new_node(skey_t key, sval_t val, node_t* next, int initializing)
{
  volatile node_t *node;
#if GC == 1
  if (unlikely(initializing))		/* for initialization AND the coupling algorithm */
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


#if defined(__tile__)
  /* on tilera you may have store reordering causing the pointer to a new node
     to become visible, before the contents of the node are visible */
  MEM_BARRIER;
#endif	/* __tile__ */

  return (node_t*) node;
}

void
bucket_set_init(intset_t* set)
{
  node_t *min;

  min = new_node(KEY_MIN, 0, NULL, 1);
  set->head = min;

  MEM_BARRIER;
}

void
node_delete(node_t *node) 
{
#if GC == 1 && SEQ_SSMEM_NO_FREE != 1
  ssmem_free(alloc, (void*) node);
#else

#endif
}

void set_delete(intset_t *set)
{
  node_t *node, *next;

  node = set->head;
  while (node != NULL) 
    {
      next = node->next;
      ssfree((void*) node);		/* TODO : fix with ssmem */
      node = next;
    }
  ssfree(set);
}

int set_size(intset_t *set)
{
  int size = 0;
  node_t *node;

  /* We have at least 2 elements */
  node = set->head->next;
  while (node->next != NULL) 
    {
      size++;
      node = node->next;
    }

  return size;
}



	
