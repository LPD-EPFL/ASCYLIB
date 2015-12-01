/*   
 *   File: linkedlist-optik.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
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

int
optik_insert(intset_l_t *set, skey_t key, sval_t val)
{
  node_l_t *curr, *pred, *newnode;
  optik_t pred_ver = OPTIK_INIT;
	
 restart:
  PARSE_TRY();

  curr = set->head;
  OPTIK_WITH_GL_DO(COMPILER_NO_REORDER(pred_ver = set->lock));

  do
    {
      OPTIK_WITHOUT_GL_DO(COMPILER_NO_REORDER(optik_t curr_ver = curr->lock;););
	  
      pred = curr;
      OPTIK_WITHOUT_GL_DO(pred_ver = curr_ver;);

      curr = curr->next;
    }
  while (curr != NULL && (curr->key < key));

  UPDATE_TRY();

  if (curr != NULL && curr->key == key)
    {
      return false;
    }

  OPTIK_WITHOUT_GL_DO(
		      if ((!optik_trylock_version(&pred->lock, pred_ver)))
			{
			  goto restart;
			}
		      );

  OPTIK_WITH_GL_DO(
		   if ((!optik_trylock_version(&set->lock, pred_ver)))
		     {
		       goto restart;
		     }
		   );

  newnode = new_node_l(key, val, curr, 0);
#ifdef __tile__
  MEM_BARRIER;
#endif
  pred->next = newnode;

  OPTIK_WITHOUT_GL_DO(optik_unlock(&pred->lock););
  OPTIK_WITH_GL_DO(optik_unlock(&set->lock););

  return true;
}

sval_t
optik_delete(intset_l_t *set, skey_t key)
{
  node_l_t *pred, *curr;
  optik_t pred_ver = OPTIK_INIT;
  OPTIK_WITHOUT_GL_DO(optik_t curr_ver = OPTIK_INIT;);
  sval_t result = 0;

 restart:
  PARSE_TRY();

  curr = set->head;
  OPTIK_WITHOUT_GL_DO(curr_ver = curr->lock;);
  OPTIK_WITH_GL_DO(COMPILER_NO_REORDER(pred_ver = set->lock));


  do
    {
      //      PREFETCH(curr->next);
      pred = curr;
      OPTIK_WITHOUT_GL_DO(pred_ver = curr_ver;);

      curr = curr->next;
      if (curr == NULL)
	{
	  break;
	}
      OPTIK_WITHOUT_GL_DO(curr_ver = curr->lock;);
    }
  while (curr->key < key);

  UPDATE_TRY();

  if (curr == NULL || curr->key != key)
    {
      return false;
    }

  OPTIK_WITHOUT_GL_DO(
		      if (unlikely(!optik_trylock_version(&pred->lock, pred_ver)))
			{
			  goto restart;
			}

		      if (unlikely(!optik_trylock_version(&curr->lock, curr_ver)))
			{
			  optik_revert(&pred->lock);
			  goto restart;
			}
		      );

  OPTIK_WITH_GL_DO(
		   if ((!optik_trylock_version(&set->lock, pred_ver)))
		     {
		       goto restart;
		     }
		   );


  result = curr->val;
  pred->next = curr->next;
  OPTIK_WITHOUT_GL_DO(optik_unlock(&pred->lock););
  OPTIK_WITH_GL_DO(optik_unlock(&set->lock););

#if GC == 1
  ssmem_free(alloc, (void*) curr);
#endif

  return result;
}
