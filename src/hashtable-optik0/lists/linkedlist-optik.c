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

/*
 * Checking that both curr and pred are both unmarked and that pred's next pointer
 * points to curr to verify that the entries are adjacent and present in the list.
 */
inline int
optik_validate(node_l_t* pred, node_l_t* curr) 
{
  return 1;
}

sval_t
optik_find(intset_l_t *set, skey_t key)
{
  PARSE_TRY();
  node_l_t* curr = set->head;
  while (curr != NULL && (curr->key < key))
    {
      curr = curr->next;
    }

  sval_t res = 0;
  if (curr != NULL && curr->key == key)
    {
      res = curr->val;
    }

  return res;
}

/* static int __dc = 0, __r = 0; */

int
optik_insert(intset_l_t *set, skey_t key, sval_t val)
{
  node_l_t *curr, *pred, *newnode;
  optik_t pred_ver = OPTIK_INIT;
	
 restart:
  PARSE_TRY();

  curr = set->head;

  do
    {
      //      PREFETCH(curr->next);
      COMPILER_NO_REORDER(optik_t curr_ver = curr->lock;);
	  
      pred = curr;
      pred_ver = curr_ver;

      curr = curr->next;
    }
  while (curr != NULL && (curr->key < key));

  UPDATE_TRY();

#if OPTIK_RO_FAIL == 1 
  if (curr != NULL && curr->key == key)
    {
      return false;
    }
#endif

  if ((!optik_trylock_version((optik_t*) &pred->lock, pred_ver)))
    {
      goto restart;
    }

  newnode = new_node_l(key, val, curr, 0);
#ifdef __tile__
  MEM_BARRIER;
#endif
  pred->next = newnode;
  optik_unlock((optik_t*) &pred->lock);

  return true;
}

sval_t
optik_delete(intset_l_t *set, skey_t key)
{
  node_l_t *pred, *curr;
  optik_t pred_ver = OPTIK_INIT, curr_ver = OPTIK_INIT;
  sval_t result = 0;

 restart:
  PARSE_TRY();

  curr = set->head;
  curr_ver = curr->lock;

  do
    {
      //      PREFETCH(curr->next);
      pred = curr;
      pred_ver = curr_ver;

      curr = curr->next;
      if (curr == NULL)
	{
	  break;
	}
      curr_ver = curr->lock;
    }
  while (likely(curr->key < key));

  UPDATE_TRY();

#if OPTIK_RO_FAIL == 1 
  if (curr == NULL || curr->key != key)
    {
      return false;
    }
#endif

  if (unlikely(!optik_trylock_version((optik_t*) &pred->lock, pred_ver)))
    {
      goto restart;
    }

  if (unlikely(!optik_trylock_version((optik_t*) &curr->lock, curr_ver)))
    {
      optik_revert((optik_t*) &pred->lock);
      goto restart;
    }

  result = curr->val;
  pred->next = curr->next;
#if GC == 1
  ssmem_free(alloc, (void*) curr);
#endif

  optik_unlock((optik_t*) &pred->lock);
      
  return result;
}
