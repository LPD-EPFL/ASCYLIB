/*
 * File:
 *   pugh.c
 * Author(s):
 * Description:
 *   Lazy linked list implementation of an integer set based on Heller et al. algorithm
 *   "A Lazy Concurrent List-Based Set Algorithm"
 *   S. Heller, M. Herlihy, V. Luchangco, M. Moir, W.N. Scherer III, N. Shavit
 *   p.3-16, OPODIS 2005
 *
 * Copyright (c) 2009-2010.
 *
 * lazy.c is part of Synchrobench
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


#include "lazy.h"

/********************************************************************************* 
 * help search functions
 *********************************************************************************/

static inline node_l_t*
search_weak_left(intset_l_t* set, skey_t key)
{
  node_l_t* pred = set->head;
  node_l_t* succ = pred->next;
  while (succ->key < key)
    {
      pred = succ;
      succ = succ->next;
    }

  return pred;
}

static inline node_l_t*
search_weak_right(intset_l_t* set, skey_t key)
{
  node_l_t* succ = set->head->next;
  while (succ->key < key)
    {
      succ = succ->next;
    }

  return succ;
}

static inline node_l_t*
search_strong(intset_l_t* set, skey_t key, node_l_t** right)
{
  node_l_t* pred = search_weak_left(set, key);
  GL_LOCK(set->lock);
  LOCK(ND_GET_LOCK(pred));
  node_l_t* succ = pred->next;
  while (unlikely(succ->key < key))
    {
      UNLOCK(ND_GET_LOCK(pred));
      pred = succ;
      LOCK(ND_GET_LOCK(pred));
      succ = pred->next;
    }
  *right = succ;
  return pred;
}

sval_t
list_search(intset_l_t* set, skey_t key)
{
  node_l_t* right = search_weak_right(set, key);
  if (right->key == key)
    {
      return right->val;
    }

  return false;
}

int
list_insert(intset_l_t* set, skey_t key, sval_t val)
{
  int result = 1;
  node_l_t* right;
  /* optimize for step-wise strong search:: if found, return before locking! */
  node_l_t* left = search_strong(set, key, &right);
  if (right->key == key)
    {
      result = 0;
    }
  else
    {
      node_l_t* n = new_node_l(key, val, left->next, 0);
      left->next = n;
    }

  UNLOCK(ND_GET_LOCK(left));
  GL_UNLOCK(set->lock);
  return result;
}

sval_t
list_delete(intset_l_t* set, skey_t key)
{
  sval_t result = 0;
  node_l_t* right;
  node_l_t* left = search_strong(set, key, &right);   /* TODO:: optimize for step-wise strong search!! */
  if (right->key == key)
    {
      LOCK(ND_GET_LOCK(right));
      result = right->val;
      left->next = right->next;
      right->next = left;
      UNLOCK(ND_GET_LOCK(right));
#if GC == 1
      ssmem_free(alloc, right);
#endif
    }
  UNLOCK(ND_GET_LOCK(left));
  GL_UNLOCK(set->lock);
  return result;
}
