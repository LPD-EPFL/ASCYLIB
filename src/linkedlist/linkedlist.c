/*
 *  linkedlist.c
 *  
 *  Linked list data structure
 *
 *  Created by Vincent Gramoli on 1/12/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "linkedlist.h"

__thread ssmem_allocator_t* alloc;

node_t*
new_node(skey_t key, sval_t val, node_t *next, int transactional)
{
  volatile node_t *node;
#if GC == 1
  if (unlikely(transactional))
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
	
  if ((set = (intset_t*)ssalloc(sizeof(intset_t))) == NULL)
    {
      perror("malloc");
      exit(1);
    }

#define CL_NUM(x) ((uintptr_t) (x) / 64)
#define CL_OFF(x)((uintptr_t) (x) % 64)
  /* printf("* set @ %p (%lu / %lu )\n", set, CL_NUM(set), CL_OFF(set)); */

  max = new_node(KEY_MAX, 0, NULL, 1);
  min = new_node(KEY_MIN, 0, max, 1);
  set->head = min;

  ssalloc_align_alloc(0);

  return set;
}

void set_delete(intset_t *set)
{
  node_t *node, *next;

  node = set->head;
  while (node != NULL) {
    next = node->next;
    free(node);
    node = next;
  }
  free(set);
}


