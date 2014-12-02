/*   
 *   File: pugh.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   pugh.c is part of ASCYLIB
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

#include "pugh.h"

RETRY_STATS_VARS;

/********************************************************************************* 
 * help search functions
 *********************************************************************************/

static inline node_l_t*
search_weak_left(intset_l_t* set, skey_t key)
{
  node_l_t* pred = set->head;
  node_l_t* succ = pred->next;
  while (succ != NULL && succ->key < key)
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
  while (succ != NULL && succ->key < key)
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
  while (succ != NULL && unlikely(succ->key < key))
    {
      UNLOCK(ND_GET_LOCK(pred));
      pred = succ;
      LOCK(ND_GET_LOCK(pred));
      succ = pred->next;
    }
  *right = succ;
  return pred;
}

static inline node_l_t*
search_strong_cond(intset_l_t* set, skey_t key, node_l_t** right, int equal)
{
  node_l_t* pred = search_weak_left(set, key);
  node_l_t* succ = pred->next;
  if (((succ != NULL) && (succ->key == key)) == equal)
    {
      return 0;
    }

  GL_LOCK(set->lock);
  LOCK(ND_GET_LOCK(pred));
  succ = pred->next;
  while (succ != NULL && unlikely(succ->key < key))
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
  PARSE_TRY();
  node_l_t* right = search_weak_right(set, key);
  if (right != NULL && right->key == key)
    {
      return right->val;
    }

  return false;
}

int
list_insert(intset_l_t* set, skey_t key, sval_t val)
{
  PARSE_TRY();
  UPDATE_TRY();
  int result = 1;
  node_l_t* right;
  /* optimize for step-wise strong search:: if found, return before locking! */
#if PUGH_RO_FAIL == 1
  node_l_t* left = search_strong_cond(set, key, &right, 1);
  if (left == NULL)
    { 
      return 0; 
    }
#else
  node_l_t* left = search_strong(set, key, &right);
#endif

  if (right != NULL && right->key == key)
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
  PARSE_TRY();
  UPDATE_TRY();
  sval_t result = 0;
  node_l_t* right;

#if PUGH_RO_FAIL == 1
  node_l_t* left = search_strong_cond(set, key, &right, 0);
  if (left == NULL)
    { 
      return 0; 
    }
#else
  node_l_t* left = search_strong(set, key, &right);
#endif

  if (right != NULL && right->key == key)
    {
      LOCK(ND_GET_LOCK(right));
      result = right->val;
      left->next = right->next;
      right->next = left;
      UNLOCK(ND_GET_LOCK(right));
#if GC == 1
      ssmem_free(alloc, (void*) right);
#endif
    }
  UNLOCK(ND_GET_LOCK(left));
  GL_UNLOCK(set->lock);
  return result;
}
