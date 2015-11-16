/*   
 *   File: sl_optik.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description:  A skip-list algorithm design with OPTIK.
 *   Algorithm: High-level description:
 *   -Search: Simply traverse the levels of the skip list
 *   -Parse (i.e., traverse to the point you want to modify): Traverse
 *   and keep track of the predecessor node for the target key at each level
 *   as well as the OPTIK version of each predecessor. Unlike other skip lists
 *   this one does not need to keep track of successor nodes for validation.
 *   optik_trylock_version takes care of validation.
 *   -insert: do the parse and the start from level 0, lock with trylock_version
 *   and insert the new node. If the trylock fails, reparse and continue from the 
 *   previous level. The state flag of a node indicates whether a node is fully
 *   linked.
 *   -delete: parse and then try to do optik_trylock_vdelete on the node. If 
 *   successful, try to grab the lock with optik_trylock_version on all levels
 *   and then unlink the node. If one of the trylock calls fail, release all locks
 *   and retry.
 *   sl_optik.c is part of ASCYLIB
 *
 * Copyright (c) 2015 Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>,
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

#include "skiplist-optik.h"
#include "utils.h"

RETRY_STATS_VARS;

#include "latency.h"
#if LATENCY_PARSING == 1
__thread size_t lat_parsing_get = 0;
__thread size_t lat_parsing_put = 0;
__thread size_t lat_parsing_rem = 0;
#endif	/* LATENCY_PARSING == 1 */

extern ALIGNED(CACHE_LINE_SIZE) unsigned int levelmax;

#define MAX_BACKOFF 131071
#define OPTIK_MAX_MAX_LEVEL 64 /* covers up to 2^64 elements */

/*
 * finds the predecessors of a key, 
 * if return value is >= 0, then the contains the node with the key we are looking for
 */
static sl_node_t*
sl_optik_search(sl_intset_t* set, skey_t key, sl_node_t** preds, optik_t* predsv, optik_t* node_foundv)
{
 restart:
  PARSE_TRY();
	
  sl_node_t* node_found = NULL;
  sl_node_t* pred = set->head;
  optik_t predv = set->head->lock;
	
  int i;
  for (i = (pred->toplevel - 1); i >= 0; i--)
    {
      sl_node_t* curr = pred->next[i];
      optik_t currv = curr->lock; 

      while (key > curr->key)
	{
	  predv = currv;
	  pred = curr;

	  curr = pred->next[i];
	  currv = curr->lock;  
	}

      if (unlikely(optik_is_deleted(predv)))
      	{
      	  goto restart;
      	}
      preds[i] = pred;
      predsv[i] = predv;
      if (key == curr->key)
	{
	  node_found = curr;
	  *node_foundv = currv;
	}
    }
  return node_found;
}

inline sl_node_t*
sl_optik_left_search(sl_intset_t* set, skey_t key)
{
  PARSE_TRY();
  int i;
  sl_node_t* pred, *curr, *nd = NULL;
	
  pred = set->head;
	
  for (i = (pred->toplevel - 1); i >= 0; i--)
    {
      curr = pred->next[i];
      while (key > curr->key)
	{
	  pred = curr;
	  curr = pred->next[i];
	}

      if (key == curr->key)
	{
	  nd = curr;
	  break;
	}
    }

  return nd;
}

sval_t
sl_optik_find(sl_intset_t* set, skey_t key)
{ 
  PARSE_START_TS(0);
  sl_node_t* nd = sl_optik_left_search(set, key);
  PARSE_END_TS(0, lat_parsing_get++);

  sval_t result = 0;
  if (nd != NULL && !optik_is_deleted(nd->lock))
    {
      result = nd->val;
    }
  return result;
}

static inline void
unlock_levels_down(sl_node_t** nodes, int low, int high)
{
  sl_node_t* old = NULL;
  int i;
  for (i = high; i >= low; i--)
    {
      if (old != nodes[i])
	{
	  optik_unlock(&nodes[i]->lock);
	}
      old = nodes[i];
    }
}

static inline void
unlock_levels_up(sl_node_t** nodes, int low, int high)
{
  sl_node_t* old = NULL;
  int i;
  for (i = low; i < high; i++)
    {
      if (old != nodes[i])
	{
	  optik_unlock(&nodes[i]->lock);
	}
      old = nodes[i];
    }
}


/*
 * 
 */
int
sl_optik_insert(sl_intset_t* set, skey_t key, sval_t val)
{
  sl_node_t* preds[OPTIK_MAX_MAX_LEVEL];
  optik_t predsv[OPTIK_MAX_MAX_LEVEL], unused;
  sl_node_t* node_new = NULL;

  int toplevel = get_rand_level();
  int inserted_upto = 0;
  /* printf("++> inserting %zu\n", key); */

 restart:
  UPDATE_TRY();
  sl_node_t* node_found = sl_optik_search(set, key, preds, predsv, &unused);
  if (node_found != NULL)
    {
      if (!inserted_upto)
	{
	  if (!optik_is_deleted(node_found->lock))
	    {
	      if (unlikely(node_new != NULL))
		{
#if GC == 1
		  ssmem_free(alloc, (void*) node_new);
#else
		  ssfree(node_new);
#endif
		} 
	      return 0;
	    }
	  else		/* there is a logically deleted node -- wait for it to be physically removed */
	    {
	      goto restart;
	    }
	}
    }

  if (node_new == NULL)
    {
      node_new = sl_new_simple_node(key, val, toplevel, 0);
    }

  sl_node_t* pred_prev = NULL;
  int i;
  for (i = inserted_upto; i < toplevel; i++)
    {
      sl_node_t* pred = preds[i];
      if (pred_prev != pred && !optik_trylock_version(&pred->lock, predsv[i]))
	{
	  unlock_levels_down(preds, inserted_upto, i - 1);
	  inserted_upto = i;
	  goto restart;
	}
      node_new->next[i] = pred->next[i];
      pred->next[i] = node_new;
      pred_prev = pred;
    }

  node_new->state = 1;
  unlock_levels_down(preds, inserted_upto, toplevel - 1);

  return 1;

}


sval_t
sl_optik_delete(sl_intset_t* set, skey_t key)
{
  sl_node_t* preds[OPTIK_MAX_MAX_LEVEL];
  optik_t predsv[OPTIK_MAX_MAX_LEVEL], node_foundv;
  int my_delete = 0;

 restart:
  UPDATE_TRY();
  sl_node_t* node_found = sl_optik_search(set, key, preds, predsv, &node_foundv);
  if (node_found == NULL)
    {
      return 0;
    }

  if (!my_delete)
    {
      if (optik_is_deleted(node_found->lock) || (!node_found->state))
	{
	  return 0;
	}

      if (!optik_trylock_vdelete(&node_found->lock, node_foundv))
	{
	  if (optik_is_deleted(node_found->lock))
	    {
	      /* printf("+[del-%zu]+> someone else did the deletion\n", key); */
	      return 0;
	    }
	  else
	    {
	      goto restart;
	    }
	}
    }

  my_delete = 1;

  const int toplevel_nf = node_found->toplevel;
  sl_node_t* pred_prev = NULL;
  int i;
  for (i = 0; i < toplevel_nf; i++)
    {
      sl_node_t* pred = preds[i];
      if (pred_prev != pred && !optik_trylock_version(&pred->lock, predsv[i]))
	{
	  unlock_levels_down(preds, 0, i - 1);
	  goto restart;
	}
      pred_prev = pred;
    }

  /* for (i = (node_found->toplevel - 1); i >= 0; i--) */
  for (i = 0; i < toplevel_nf; i++)
    {
      preds[i]->next[i] = node_found->next[i];
    }

  unlock_levels_down(preds, 0, toplevel_nf - 1);

#if GC == 1
  ssmem_free(alloc, (void*) node_found);
#endif

  return node_found->val;
}
