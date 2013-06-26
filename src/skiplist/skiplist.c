/*
 * File:
 *   skiplist.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Skip list definition 
 *
 * Copyright (c) 2009-2010.
 *
 * skiplist.c is part of Synchrobench
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

#include "skiplist.h"	

ALIGNED(64) uint8_t levelmax[64];

int
get_rand_level()
{
  int i, level = 1;
  for (i = 0; i < *levelmax - 1; i++)
    {
      if ((rand_range(100)-1) < 50)
	level++;
      else
	break;
    }
  /* 1 <= level <= *levelmax */
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
sl_new_simple_node(val_t val, int toplevel, int transactional)
{
  sl_node_t *node;

  if (transactional)
    {
      node = (sl_node_t *)MALLOC(sizeof(sl_node_t) + toplevel * sizeof(sl_node_t *));
    }
  else 
    {
      /* node = (sl_node_t *)malloc(sizeof(sl_node_t) + toplevel * sizeof(sl_node_t *)); */
      /* node = (sl_node_t *)ssalloc_alloc(1, sizeof(sl_node_t) + toplevel * sizeof(sl_node_t *)); */

      /* use *levelmax instead of toplevel in order to be able to use the ssalloc allocator*/
      size_t ns = sizeof(sl_node_t) + *levelmax * sizeof(sl_node_t *);
      size_t ns_rm = ns % 64;
      if (ns_rm)
	{
	  ns += 64 - ns_rm;
	}
      node = (sl_node_t *)ssalloc_alloc(1, ns);
    }

  if (node == NULL)
    {
      perror("malloc");
      exit(1);
    }

  node->val = val;
  node->toplevel = toplevel;
  node->deleted = 0;

  MEM_BARRIER;

  return node;
}

/* 
 * Create a new node with its next field. 
 * If next=NULL, then this create a tail node. 
 */
sl_node_t*
sl_new_node(val_t val, sl_node_t *next, int toplevel, int transactional)
{
  volatile sl_node_t *node;
  int i;

  /* if (transactional) */
  /*   node = (sl_node_t *)MALLOC(sizeof(sl_node_t) + toplevel * sizeof(sl_node_t *)); */
  /* else  */
  /*   { */
  /*     /\* node = (sl_node_t *)malloc(sizeof(sl_node_t) + toplevel * sizeof(sl_node_t *)); *\/ */
  /*     node = (sl_node_t *)ssalloc(sizeof(sl_node_t) + toplevel * sizeof(sl_node_t *)); */
  /*     /\* node = (sl_node_t *)ssalloc_alloc(1, sizeof(sl_node_t) + toplevel * sizeof(sl_node_t *)); *\/ */
  /*   } */
  /* if (node == NULL) */
  /*   { */
  /*     perror("malloc"); */
  /*     exit(1); */
  /*   } */

  /* node->val = val; */
  /* node->toplevel = toplevel; */
  /* node->deleted = 0; */
  node = sl_new_simple_node(val, toplevel, transactional);

  for (i = 0; i < *levelmax; i++)
    node->next[i] = next;
	
  MEM_BARRIER;

  return (sl_node_t*) node;
}

void
sl_delete_node(sl_node_t *n)
{
  /* free(n); */
  ssfree_alloc(1, n);
}

sl_intset_t*
sl_set_new()
{
  sl_intset_t *set;
  sl_node_t *min, *max;
	
  /* if ((set = (sl_intset_t *)malloc(sizeof(sl_intset_t))) == NULL) */
  if ((set = (sl_intset_t *)ssalloc_alloc(1, sizeof(sl_intset_t))) == NULL)
  /* if ((set = (sl_intset_t *)ssalloc_alloc(0, sizeof(sl_intset_t))) == NULL) */
    {
      perror("malloc");
      exit(1);
    }

  ssalloc_align_alloc(1);

  max = sl_new_node(VAL_MAX, NULL, *levelmax, 0);
  min = sl_new_node(VAL_MIN, max, *levelmax, 0);
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
  ssfree_alloc(1, set);
}

int
sl_set_size(sl_intset_t *set)
{
  int size = 0;
  sl_node_t *node;

  node = set->head->next[0];
  while (node->next[0] != NULL)
    {
      if (!node->deleted)
	size++;
      node = node->next[0];
    }

  return size;
}
