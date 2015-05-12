/*   
 *   File: optik.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description:  
 *   optik.c is part of ASCYLIB
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

static int
sl_optik_search(sl_intset_t* set, skey_t key, sl_node_t** preds, sl_node_t** succs, optik_t* predsv)
{
 restart:
  PARSE_TRY();
  int found, i;
  sl_node_t* pred, *curr;
  optik_t predv, currv;
	
  found = -1;
  pred = set->head;
  predv = set->head->lock;
	
  for (i = (pred->toplevel - 1); i >= 0; i--)
    {
      curr = pred->next[i];
      currv = curr->lock; 

      while (key > curr->key)
	{
	  predv = currv;
	  pred = curr;

	  curr = pred->next[i];
	  currv = curr->lock;  
	}

      /* if (unlikely(optik_is_deleted(predv))) */
      /* 	{ */
      /* 	  goto restart; */
      /* 	} */
      preds[i] = pred;
      predsv[i] = predv;
      succs[i] = curr;
      if (found == -1 && key == curr->key)
	{
	  found = i;
	}
    }
  return found;
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

/*
 * Function sl_optik_find corresponds to the contains method of the original 
 * paper. In contrast with the original version, it allocates and frees the 
 * memory at right places to avoid the use of a stop-the-world garbage 
 * collector. 
 */
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

/*
 * Function unlock_levels is an helper function for the insert and delete 
 * functions.
 */ 
inline void
unlock_levels(sl_intset_t* set, sl_node_t** nodes, int highestlevel)
{
  int i;
  sl_node_t* old = NULL;
  for (i = 0; i <= highestlevel; i++)
    {
      if (old != nodes[i])
	{
	  optik_unlock(&nodes[i]->lock);
	}
      old = nodes[i];
    }
}

/*
 * Function sl_optik_insert stands for the add method of the original paper.
 * Unlocking and freeing the memory are done at the right places.
 */
int
sl_optik_insert(sl_intset_t* set, skey_t key, sval_t val)
{
  sl_node_t* preds[OPTIK_MAX_MAX_LEVEL], *succs[OPTIK_MAX_MAX_LEVEL];;
  optik_t predsv[OPTIK_MAX_MAX_LEVEL];
  unsigned int backoff = 1;
	
  int toplevel = get_rand_level();

  PARSE_START_TS(1);
  while (1) 
    {
      UPDATE_TRY();
      int found = sl_optik_search(set, key, preds, succs, predsv);
      PARSE_END_TS(1, lat_parsing_put);

      if (found != -1)
	{
	  if (!optik_is_deleted(succs[found]->lock))
	    {
	      PARSE_END_INC(lat_parsing_put);
	      return 0;
	    }
	  continue;
	}

      int highest_locked = -1;
      sl_node_t* prev_pred = NULL;
      int i, valid = 1;
      for (i = 0; valid && (i < toplevel); i++)
	{
	  sl_node_t* pred = preds[i];
	  /* sl_node_t* succ = succs[i]; */
	  if (pred != prev_pred)
	    {
	      /* valid = optik_trylock_version(&pred->lock, predsv[i]);/ */
	      /* valid = optik_lock_if_not_deleted(&pred->lock); */
	      /* assert(!optik_is_deleted(predsv[i])); */
	      valid = optik_trylock_version(&pred->lock, predsv[i]);
	      if (valid)
		{
		  highest_locked = i;
		  prev_pred = pred;
		  /* valid = !optik_is_deleted(pred->lock) && !optik_is_deleted(succ->lock) && (pred->next[i] == succ); */
		  /* valid = !optik_is_deleted(succ->lock); */
		}
	    }	
			
	}
	
      if (!valid) 
	{			 /* Unlock the predecessors before leaving */ 
	  unlock_levels(set, preds, highest_locked); 
	  if (backoff > 5000)
	    {
	      nop_rep(backoff & MAX_BACKOFF);
	    }
	  backoff <<= 1;
	  continue;
	}
		
      sl_node_t* new_node = sl_new_simple_node(key, val, toplevel, 0);

      for (i = 0; i < toplevel; i++)
	{
	  new_node->next[i] = succs[i];
	}

#if defined(__tile__)
      MEM_BARRIER;
#endif

      for (i = 0; i < toplevel; i++)
	{
	  preds[i]->next[i] = new_node;
	}
		
      unlock_levels(set, preds, highest_locked);
      PARSE_END_INC(lat_parsing_put);
      return 1;
    }
}


inline int 
ok_to_delete(sl_node_t* node, int found)
{
  return (((node->toplevel-1) == found) && !optik_is_deleted(node->lock));
}

sval_t
sl_optik_delete(sl_intset_t* set, skey_t key)
{
  sl_node_t* succs[OPTIK_MAX_MAX_LEVEL], *preds[OPTIK_MAX_MAX_LEVEL], *node_todel = NULL;
  optik_t predsv[OPTIK_MAX_MAX_LEVEL];
  int is_marked = 0, toplevel = -1;	
  unsigned int backoff = 1;

  PARSE_START_TS(2);
  while(1)
    {
      UPDATE_TRY();
      int found = sl_optik_search(set, key, preds, succs, predsv);
      PARSE_END_TS(2, lat_parsing_rem);

      /* If not marked and ok to delete, then mark it */
      if (is_marked || (found != -1 && ok_to_delete(succs[found], found)))
	{	
	  if (!is_marked)
	    {
	      node_todel = succs[found];
	      toplevel = node_todel->toplevel;

	      if (!optik_lock_vdelete(&node_todel->lock))
		{
		  PARSE_END_INC(lat_parsing_rem);
		  return 0;
		}

	      is_marked = 1;
	    }

	  /* Physical deletion */
	  sl_node_t* prev_pred = NULL;
	  int i, valid = 1, highest_locked = -1;
	  for (i = 0; valid && (i < toplevel); i++)
	    {
	      sl_node_t* pred = preds[i];
	      if (pred != prev_pred)
		{
		  valid = optik_trylock_version(&pred->lock, predsv[i]);
		  if (valid)
		    {
		      highest_locked = i;
		      prev_pred = pred;
		    }
		}
	    }

	  if (!valid)
	    {	
	      unlock_levels(set, preds, highest_locked);
	      if (backoff > 5000) 
		{
		  nop_rep(backoff & MAX_BACKOFF);
		}
	      backoff <<= 1;
	      continue;
	    }
			
	  for (i = (toplevel-1); i >= 0; i--)
	    {
	      preds[i]->next[i] = node_todel->next[i];
	    }

	  sval_t val = node_todel->val;
	  /* optik_unlock(&node_todel->lock); */
	  unlock_levels(set, preds, highest_locked);
	  PARSE_END_INC(lat_parsing_rem);
#if GC == 1
	  ssmem_free(alloc, (void*) node_todel);
#endif
	  return val;
	}
      else
	{
	  PARSE_END_INC(lat_parsing_rem);
	  return 0;
	}
    }
}
