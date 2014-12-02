/*   
 *   File: skiplist.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   skiplist.c is part of ASCYLIB
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

#include "skiplist.h"	

unsigned int levelmax;
unsigned int size_pad_32;
__thread ssmem_allocator_t* alloc;

inline int
get_rand_level()
{
  int i, level = 1;
  for (i = 0; i < levelmax - 1; i++)
    {
      if ((rand_range(101)) < 50)
  	level++;
      else
  	break;
    }
  /* 1 <= level <= levelmax */

  return level;
}

int
floor_log_2(unsigned int n)
{
  int pos = 0;
  if (n >= 1<<16) { n >>= 16; pos += 16; }
  if (n >= 1<< 8) { n >>=  8; pos +=  8; }
  if (n >= 1<< 4) { n >>=  4; pos +=  4; }
  if (n >= 1<< 2) { n >>=  2; pos +=  2; }
  if (n >= 1<< 1) {           pos +=  1; }
  return ((n == 0) ? (-1) : pos);
}

/* 
 * Create a new node without setting its next fields. 
 */
sl_node_t*
sl_new_simple_node(skey_t key, sval_t val, int toplevel, int initializing)
{
  sl_node_t *node;

#if GC == 1
  if (unlikely(initializing))
    {
      /* use levelmax instead of toplevel in order to be able to use the ssalloc allocator*/
      size_t ns = size_pad_32;
      size_t ns_rm = ns & 63;
      if (ns_rm)
	{
	  ns += 64 - ns_rm;
	}
      node = (sl_node_t*) ssalloc(ns);
    }
  else 
    {
      size_t ns = size_pad_32;
#  if defined(DO_PAD)
      size_t ns_rm = ns & 63;
      if (ns_rm)
	{
	  ns += 64 - ns_rm;
	}
#  endif
      node = (sl_node_t*) ssmem_alloc(alloc, ns);
    }
#else
  size_t ns = size_pad_32;
  node = (sl_node_t *)ssalloc(ns);
#endif

  if (node == NULL)
    {
      perror("malloc");
      exit(1);
    }

  node->key = key;
  node->val = val;
  node->toplevel = toplevel;

#if defined(__tile__)
  MEM_BARRIER;
#endif

  return node;
}

/* 
 * Create a new node with its next field. 
 * If next=NULL, then this create a tail node. 
 */
sl_node_t*
sl_new_node(skey_t key, sval_t val, sl_node_t *next, int toplevel, int transactional)
{
  volatile sl_node_t *node;
  int i;

  node = sl_new_simple_node(key, val, toplevel, transactional);

  for (i = 0; i < levelmax; i++)
    {
      node->next[i] = next;
    }
	
  MEM_BARRIER;

  return (sl_node_t*) node;
}

void
sl_delete_node(sl_node_t *n)
{
#if GC == 1
#  if SEQ_SSMEM_NO_FREE != 1
#    warning ssmem_free() is never called in sl_delete_node
  /* ssmem_free(alloc, (void*) n); *\/ */
#  endif
#else
#endif
}

sl_intset_t*
sl_set_new()
{
  sl_intset_t *set;
  sl_node_t *min, *max;
	
  if ((set = (sl_intset_t *)ssalloc_aligned(CACHE_LINE_SIZE, sizeof(sl_intset_t))) == NULL)
    {
      perror("malloc");
      exit(1);
    }

  max = sl_new_node(KEY_MAX, 0, NULL, levelmax, 1);
  min = sl_new_node(KEY_MIN, 0, max, levelmax, 1);
  set->head = min;
  return set;
}

void
sl_set_delete(sl_intset_t *set)
{
  sl_node_t *node, *next;

  node = set->head;
  while (node != NULL)
    {
      next = node->next[0];
      sl_delete_node(node);
      node = next;
    }
  ssfree(set);
}

int
sl_set_size(sl_intset_t *set)
{
  int size = 0;
  sl_node_t *node;

  node = GET_UNMARKED(set->head->next[0]);
  while (node->next[0] != NULL)
    {
      if (!IS_MARKED(node->next[0]))
	{
	  size++;
	}
      node = GET_UNMARKED(node->next[0]);
    }

  return size;
}
