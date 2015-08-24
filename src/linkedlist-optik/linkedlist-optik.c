/*   
 *   File: optik.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
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

#include "linkedlist-optik.h"

RETRY_STATS_VARS;

sval_t
optik_find(intset_l_t *set, skey_t key)
{
  PARSE_TRY();
  node_l_t* curr = set->head;
  while (likely(curr->key < key))
    {
      curr = curr->next;
    }

  sval_t res = 0;
  if (curr->key == key)
    {
      res = curr->val;
    }
  
  return res;
}

int
optik_insert(intset_l_t *set, skey_t key, sval_t val)
{
  optik_t pred_ver = OPTIK_INIT;
	
 restart:
  PARSE_TRY();

  node_l_t* curr = set->head, *pred;

  do
    {
      COMPILER_NO_REORDER(optik_t curr_ver = curr->lock;);
	  
      pred = curr;
      pred_ver = curr_ver;

      curr = curr->next;
    }
  while (likely(curr->key < key));

  UPDATE_TRY();

  if (curr->key == key)
    {
      return false;
    }

  node_l_t* newnode = new_node_l(key, val, curr, 0);

  if ((!optik_trylock_version(&pred->lock, pred_ver)))
    {
      node_delete_l(newnode);
      goto restart;
    }

#ifdef __tile__
  MEM_BARRIER;
#endif
  pred->next = newnode;
  optik_unlock(&pred->lock);

  return true;
}

sval_t
optik_delete(intset_l_t *set, skey_t key)
{
  optik_t pred_ver = OPTIK_INIT, curr_ver = OPTIK_INIT;
 restart:
  PARSE_TRY();

  node_l_t* curr = set->head, *pred;
  curr_ver = curr->lock;

  do
    {
      pred = curr;
      pred_ver = curr_ver;

      curr = curr->next;
      curr_ver = curr->lock;
    }
  while (likely(curr->key < key));

  UPDATE_TRY();

  if (curr->key != key)
    {
      return false;
    }

  node_l_t* cnxt = curr->next;

  if (unlikely(!optik_trylock_version(&pred->lock, pred_ver)))
    {
      goto restart;
    }

  if (unlikely(!optik_trylock_version(&curr->lock, curr_ver)))
    {
      optik_revert(&pred->lock);
      goto restart;
    }

  pred->next = cnxt;
  optik_unlock(&pred->lock);
      
  sval_t result = curr->val;
  node_delete_l(curr);
  return result;
}
