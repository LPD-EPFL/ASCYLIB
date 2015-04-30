/*   
 *   File: optik.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *
 * Copyright (c) 2014 Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>,
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
  while (curr != NULL && curr->key < key)
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

#if !defined(LL_GLOBAL_LOCK)
int
optik_insert(intset_l_t *set, skey_t key, sval_t val)
{	
 restart:
  PARSE_TRY();
  volatile node_l_t *curr, *pred = NULL;

  COMPILER_NO_REORDER(optik_t pred_ver = set->lock);
  curr = set->head;

  while (curr != NULL && (curr->key < key))
    {
      pred = curr;
      curr = curr->next;
    }

  UPDATE_TRY();

  if (curr != NULL && curr->key == key)
    {
      return false;
    }

  if (!optik_trylock_version(&set->lock, pred_ver))
    {
      goto restart;
    }

  node_l_t* newnode = new_node_l(key, val, curr, 0);
#ifdef __tile__
  MEM_BARRIER;
#endif

  if (pred != NULL)
    {
      pred->next = newnode;
    }
  else
    {
      set->head = newnode;
    }

  optik_unlock(&set->lock);

  return true;
}
#else  /* LL_GLOBAL_LOCK == 1 :: pessimistic */
int
optik_insert(intset_l_t *set, skey_t key, sval_t val)
{	
  volatile node_l_t *curr, *pred;
  COMPILER_NO_REORDER(optik_t pred_ver = set->lock);

  int r;
  for (r = 0; r < 2; r++)
    {
      PARSE_TRY();

      pred = NULL;
      curr = set->head;

      while (curr != NULL && curr->key < key)
	{
	  pred = curr;
	  curr = curr->next;
	}

      UPDATE_TRY();

      if (curr != NULL && curr->key == key)
	{
	  if (r)
	    {
	      optik_unlock(&set->lock);
	    }
	  return false;
	}

      if (!r && optik_lock_version(&set->lock, pred_ver))
	{
	  break;
	}
    }

  node_l_t* newnode = new_node_l(key, val, curr, 0);
#ifdef __tile__
  MEM_BARRIER;
#endif

  if (pred != NULL)
    {
      pred->next = newnode;
    }
  else
    {
      set->head = newnode;
    }

  optik_unlock(&set->lock);

  return true;
}
#endif	/* LL_GLOBAL_LOCK */

#if !defined(LL_GLOBAL_LOCK)
sval_t
optik_delete(intset_l_t *set, skey_t key)
{
 restart:
  PARSE_TRY();
  volatile node_l_t *pred = NULL, *curr;  
  sval_t result = 0;

  COMPILER_NO_REORDER(optik_t pred_ver = set->lock);
  curr = set->head;

  while (curr != NULL && curr->key < key)
    {
      pred = curr;
      curr = curr->next;
    }

  UPDATE_TRY();

  if (curr == NULL || curr->key != key)
    {
      return false;
    }

  if ((!optik_trylock_version(&set->lock, pred_ver)))
    {
      goto restart;
    }

  result = curr->val;
  if (pred != NULL)
    {
      pred->next = curr->next;
    }
  else
    {
      set->head = curr->next;
    }

  optik_unlock(&set->lock);
     
#if GC == 1
   ssmem_free(alloc, (void*) curr);
#endif
 
  return result;
}
#else  /* LL_GLOBAL_LOCK = 1 :: pessimistic locking */
sval_t
optik_delete(intset_l_t *set, skey_t key)
{
  volatile node_l_t *pred, *curr;  
  COMPILER_NO_REORDER(optik_t pred_ver = set->lock);

  int r;
  for (r = 0; r < 2; r++)
    {
      PARSE_TRY();

      pred = NULL;
      curr = set->head;

      while (curr != NULL && curr->key < key)
	{
	  pred = curr;
	  curr = curr->next;
	}

      UPDATE_TRY();

      if (curr == NULL || curr->key != key)
	{
	  if (r)
	    {
	      optik_unlock(&set->lock);
	    }
	  return false;
	}

      if (!r && optik_lock_version(&set->lock, pred_ver))
	{
	  break;
	}
    }

  sval_t result = curr->val;
  if (pred != NULL)
    {
      pred->next = curr->next;
    }
  else
    {
      set->head = curr->next;
    }

  optik_unlock(&set->lock);
     
#if GC == 1
  ssmem_free(alloc, (void*) curr);
#endif
 
  return result;
}
#endif	/* LL_GLOBAL_LOCK */
