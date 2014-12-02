/*   
 *   File: seq.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   seq.c is part of ASCYLIB
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

/*
 * File:
 *   intset.c
 * Author(s):
 */

#include "seq.h"
#include "latency.h"
#if LATENCY_PARSING == 1
__thread size_t lat_parsing_get = 0;
__thread size_t lat_parsing_put = 0;
__thread size_t lat_parsing_rem = 0;
#endif	/* LATENCY_PARSING == 1 */

#define MAXLEVEL    32

sval_t
sl_contains(sl_intset_t *set, skey_t key)
{
  PARSE_START_TS(0);
  sval_t result = 0;

  int i;
  sl_node_t *node, *next;
	
  node = set->head;
  for (i = node->toplevel-1; i >= 0; i--) 
    {
      next = node->next[i];
      while (next->key < key) 
	{
	  node = next;
	  next = node->next[i];
	}
    }
  node = node->next[0];
  PARSE_END_TS(0, lat_parsing_get++);

  if (node->key == key)
    {
      result = node->val;
    }
		
  return result;
}


int
sl_add(sl_intset_t *set, skey_t key, sval_t val)
{
  PARSE_START_TS(1);
  int i, l, result;
  sl_node_t *node, *next;
  sl_node_t *preds[MAXLEVEL], *succs[MAXLEVEL];
	
  node = set->head;
  for (i = node->toplevel-1; i >= 0; i--) 
    {
      next = node->next[i];
      while (next->key < key) 
	{
	  node = next;
	  next = node->next[i];
	}
      preds[i] = node;
      succs[i] = node->next[i];
    }
  node = node->next[0];
  PARSE_END_TS(1, lat_parsing_put++);

  result = (node->key != key);
  if (result == 1)
    {
      l = get_rand_level();
      node = sl_new_simple_node(key, val, l, 0);
      for (i = 0; i < l; i++) 
	{
	  node->next[i] = succs[i];
#ifdef __tile__
    MEM_BARRIER;
#endif
	  preds[i]->next[i] = node;
	}
    }
  return result;
}

sval_t
sl_remove(sl_intset_t *set, skey_t key)
{
  PARSE_START_TS(2);
  sval_t result = 0;
  int i;
  sl_node_t *node, *next = NULL;
  sl_node_t *preds[MAXLEVEL], *succs[MAXLEVEL];
	
  node = set->head;
  for (i = node->toplevel-1; i >= 0; i--) 
    {
      next = node->next[i];
      while (next->key < key) 
	{
	  node = next;
	  next = node->next[i];
	}
      preds[i] = node;
      succs[i] = node->next[i];
    }

  PARSE_END_TS(2, lat_parsing_rem++);
  if (next->key == key)
    {
      result = next->val;
      for (i = 0; i < set->head->toplevel; i++) 
	if (succs[i]->key == key)
	  {
	    preds[i]->next[i] = succs[i]->next[i];
	  }
      sl_delete_node(next); 
    }
  return result;
}


