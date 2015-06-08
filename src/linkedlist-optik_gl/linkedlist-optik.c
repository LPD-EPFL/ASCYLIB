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
  node_l_t *curr, *pred, *newnode;

 restart:
  PARSE_TRY();
  COMPILER_NO_REORDER(optik_t version = set->lock;);

  curr = set->head;

  do
    {
      pred = curr;
      curr = curr->next;
    }
  while (likely(curr->key < key));

  UPDATE_TRY();

#if OPTIK_RO_FAIL == 1 
  if (curr->key == key)
    {
      return false;
    }
#endif

  newnode = new_node_l(key, val, curr, 0);

  if ((!optik_trylock_version(&set->lock, version)))
    {
      cpause(rand() & 1023);
      goto restart;
    }

#ifdef __tile__
  MEM_BARRIER;
#endif
  pred->next = newnode;
  optik_unlock(&set->lock);

  return true;
}

sval_t
optik_delete(intset_l_t *set, skey_t key)
{
  node_l_t *pred, *curr;
  sval_t result = 0;

 restart:
  COMPILER_NO_REORDER(optik_t version = set->lock;);
  PARSE_TRY();

  curr = set->head;
  do
    {
      pred = curr;
      curr = curr->next;
    }
  while (likely(curr->key < key));

  UPDATE_TRY();

#if OPTIK_RO_FAIL == 1 
  if (curr->key != key)
    {
      return false;
    }
#endif

  if (unlikely(!optik_trylock_version(&set->lock, version)))
    {
      cpause(rand() & 1023);
      goto restart;
    }

  pred->next = curr->next;

  optik_unlock(&set->lock);

  result = curr->val;
      
#if GC == 1
  ssmem_free(alloc, (void*) curr);
#endif

  return result;
}
