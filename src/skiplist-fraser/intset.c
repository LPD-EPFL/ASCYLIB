/*   
 *   File: intset.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   intset.c is part of ASCYLIB
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

#define MAXLEVEL    32

sval_t
sl_contains(sl_intset_t *set, skey_t key)
{
  sval_t result = 0;
	
#ifdef SEQUENTIAL /* Unprotected */
	
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
  result = (node->key == key);
		
#elif defined LOCKFREE /* fraser lock-free */
  result = fraser_find(set, key);
#endif
	
  return result;
}

inline int
sl_seq_add(sl_intset_t *set, skey_t key, sval_t val) 
{
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
  if ((result = (node->key != key)) == 1)
    {
      l = get_rand_level();
      node = sl_new_simple_node(key, val, l, 1);
      for (i = 0; i < l; i++) 
	{
	  node->next[i] = succs[i];
	  preds[i]->next[i] = node;
	}
    }
  return result;
}

int
sl_add(sl_intset_t *set, skey_t key, sval_t val)
{
  int result = 0;
#ifdef SEQUENTIAL
  result = sl_seq_add(set, key, val);
#elif defined LOCKFREE /* fraser lock-free */
  result = fraser_insert(set, key, val);
#endif
  return result;
}

sval_t
sl_remove(sl_intset_t *set, skey_t key)
{
  sval_t result = 0;
	
#ifdef SEQUENTIAL
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
  if ((result = (next->key == key)) == 1) 
    {
      for (i = 0; i < set->head->toplevel; i++) 
	if (succs[i]->key == key)
	  preds[i]->next[i] = succs[i]->next[i];
      sl_delete_node(next); 
    }

#elif defined LOCKFREE
  result = fraser_remove(set, key);
#endif
	
  return result;
}


