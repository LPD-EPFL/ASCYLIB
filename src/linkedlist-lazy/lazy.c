/*   
 *   File: lazy.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: A Lazy Concurrent List-Based Set Algorithm,
 *   S. Heller, M. Herlihy, V. Luchangco, M. Moir, W.N. Scherer III, N. Shavit
 *   p.3-16, OPODIS 2005
 *   lazy.c is part of ASCYLIB
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

#include "lazy.h"

RETRY_STATS_VARS;

/*
 * Checking that both curr and pred are both unmarked and that pred's next pointer
 * points to curr to verify that the entries are adjacent and present in the list.
 */
inline int
parse_validate(node_l_t* pred, node_l_t* curr) 
{
  return (!pred->marked && !curr->marked && (pred->next == curr));
}

sval_t
parse_find(intset_l_t *set, skey_t key)
{
  PARSE_TRY();
  node_l_t* curr = set->head;
  while (curr->key < key)
    {
      curr = curr->next;
    }

  sval_t res = 0;
  if ((curr->key == key) && !curr->marked)
    {
      res = curr->val;
    }
  
  return res;
}

int
parse_insert(intset_l_t *set, skey_t key, sval_t val)
{
  node_l_t *curr, *pred, *newnode;
  int result = -1;
	
  do
    {
      PARSE_TRY();
      pred = set->head;
      curr = pred->next;
      while (likely(curr->key < key))
	{
	  pred = curr;
	  curr = curr->next;
	}

      UPDATE_TRY();

#if LAZY_RO_FAIL ==1 
      if (curr->key == key)
	{
	  if (unlikely(curr->marked))
	    {
	      continue;
	    }
	  return false;
	}
#endif

      GL_LOCK(set->lock);		/* when GL_[UN]LOCK is defined the [UN]LOCK is not ;-) */
      LOCK(ND_GET_LOCK(pred));

      if (parse_validate(pred, curr))
	{
	  result = (curr->key != key);
	  if (result) 
	    {
	      newnode = new_node_l(key, val, curr, 0);
#ifdef __tile__
  MEM_BARRIER;
#endif
	      pred->next = newnode;
	    } 
	}
      GL_UNLOCK(set->lock);
      UNLOCK(ND_GET_LOCK(pred));
    }
  while (result < 0);
  return result;
}

/*
 * Logically remove an element by setting a mark bit to 1 
 * before removing it physically.
 */
sval_t
parse_delete(intset_l_t *set, skey_t key)
{
  node_l_t *pred, *curr;
  sval_t result = 0;
  int done = 0;
	
  do
    {
      PARSE_TRY();
      pred = set->head;
      curr = pred->next;
      while (likely(curr->key < key))
	{
	  pred = curr;
	  curr = curr->next;
	}

      UPDATE_TRY();

#if LAZY_RO_FAIL ==1 
      if (curr->key != key)
	{
	  if (unlikely(curr->marked))
	    {
	      continue;
	    }
	  return false;
	}
#endif

      GL_LOCK(set->lock);		/* when GL_[UN]LOCK is defined the [UN]LOCK is not ;-) */
      LOCK(ND_GET_LOCK(pred));
      LOCK(ND_GET_LOCK(curr));

      if (parse_validate(pred, curr))
	{
	  if (key == curr->key)
	    {
	      result = curr->val;
	      node_l_t* c_nxt = curr->next;
	      curr->marked = 1;
	      pred->next = c_nxt;
#if GC == 1
	      ssmem_free(alloc, (void*) curr);
#endif
	    }
	  done = 1;
	}

      GL_UNLOCK(set->lock);
      UNLOCK(ND_GET_LOCK(curr));
      UNLOCK(ND_GET_LOCK(pred));
    }
  while (!done);
  return result;
}
