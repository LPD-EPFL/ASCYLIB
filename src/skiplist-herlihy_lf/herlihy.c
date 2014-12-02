/*   
 *   File: herlihy.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: based on Fraser's skiplist, ASCY, and
 *   Herlihy, M., Lev, Y., & Shavit, N. (2011). 
 *   Concurrent lock-free skiplist with wait-free contains operator. 
 *   US Patent 7,937,378, 2(12). 
 *   Retrieved from http://www.google.com/patents/US7937378
 *   herlihy.c is part of ASCYLIB
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

#include "herlihy.h"

RETRY_STATS_VARS;

#include "latency.h"
#if LATENCY_PARSING == 1
__thread size_t lat_parsing_get = 0;
__thread size_t lat_parsing_put = 0;
__thread size_t lat_parsing_rem = 0;
#endif	/* LATENCY_PARSING == 1 */

extern ALIGNED(CACHE_LINE_SIZE) unsigned int levelmax;

#define FRASER_MAX_MAX_LEVEL 64 /* covers up to 2^64 elements */

int				
fraser_search(sl_intset_t *set, skey_t key, sl_node_t **left_list, sl_node_t **right_list)
{
  int i;
  sl_node_t *left, *left_next, *right = NULL, *right_next;

 retry:
  PARSE_TRY();

  left = set->head;
  for (i = levelmax - 1; i >= 0; i--)
    {
      left_next = left->next[i];
      if ((is_marked((uintptr_t)left_next)))
	{
	  goto retry;
	}
      /* Find unmarked node pair at this level */
      for (right = left_next; ; right = right_next)
	{
	  /* Skip a sequence of marked nodes */
	  right_next = right->next[i];
	  while ((is_marked((uintptr_t)right_next)))
	    {
	      right = (sl_node_t*)unset_mark((uintptr_t)right_next);
	      right_next = right->next[i];
	    }

	  if (right->key >= key)
	    {
	      break;
	    }
	  left = right; 
	  left_next = right_next;
	}
      /* Ensure left and right nodes are adjacent */
      if ((left_next != right) && (!ATOMIC_CAS_MB(&left->next[i], left_next, right)))
	{
	  goto retry;
	}

      if (left_list != NULL)
      	{
	  left_list[i] = left;
      	}
      if (right_list != NULL)
      	{
	  right_list[i] = right;
	}
    }
  return (right->key == key);
}

int				
fraser_search_no_cleanup(sl_intset_t *set, skey_t key, sl_node_t **left_list, sl_node_t **right_list)
{
  PARSE_TRY();

  int i;
  sl_node_t *left, *left_next, *right = NULL;

  left = set->head;
  for (i = levelmax - 1; i >= 0; i--)
    {
      left_next = GET_UNMARKED(left->next[i]);
      right = left_next;
      while (1)
	{
	  if (likely(!IS_MARKED(right->next[i])))
	    {
	      if (right->key >= key)
		{
		  break;
		}
	      left = right;
	    }
	  right = GET_UNMARKED(right->next[i]);
	}

      /* if (left_list != NULL) */
      /* 	{ */
	  left_list[i] = left;
      /* 	} */
      /* if (right_list != NULL)	 */
      /* 	{ */
	  right_list[i] = right;
	/* } */
    }
  return (right->key == key);
}

int				
fraser_search_no_cleanup_succs(sl_intset_t *set, skey_t key, sl_node_t **right_list)
{
  PARSE_TRY();

  int i;
  sl_node_t *left, *left_next, *right = NULL;

  left = set->head;
  for (i = levelmax - 1; i >= 0; i--)
    {
      left_next = GET_UNMARKED(left->next[i]);
      right = left_next;
      while (1)
	{
	  if (likely(!IS_MARKED(right->next[i])))
	    {
	      if (right->key >= key)
		{
		  break;
		}
	      left = right;
	    }
	  right = GET_UNMARKED(right->next[i]);
	}


      right_list[i] = right;
    }
  return (right->key == key);
}

static sl_node_t* 
fraser_left_search(sl_intset_t *set, skey_t key)
{
  PARSE_TRY();
  sl_node_t* left = NULL;
  sl_node_t* left_prev;
  
  left_prev = set->head;
  int lvl;  
  for (lvl = levelmax - 1; lvl >= 0; lvl--)
    {
      left = GET_UNMARKED(left_prev->next[lvl]);
      while(left->key < key || IS_MARKED(left->next[lvl]))
      	{
      	  if (!IS_MARKED(left->next[lvl]))
      	    {
      	      left_prev = left;
      	    }
      	  left = GET_UNMARKED(left->next[lvl]);
      	}

      if ((left->key == key))
	{
	  break;
	}
    }
  return left;
}


sval_t
fraser_find(sl_intset_t *set, skey_t key)
{
  sval_t result = 0;
  PARSE_START_TS(0);
  sl_node_t* left = fraser_left_search(set, key);
  PARSE_END_TS(0, lat_parsing_get++);

  if (left->key == key)
    {
      result = left->val;
    }
  return result;
}

inline int
mark_node_ptrs(sl_node_t *n)
{
  int i, cas = 0;
  sl_node_t* n_next;
	
  for (i = n->toplevel - 1; i >= 0; i--)
    {
      do
      	{
      	  n_next = n->next[i];
      	  if (is_marked((uintptr_t)n_next))
      	    {
	      cas = 0;
      	      break;
      	    }
	  cas = ATOMIC_CAS_MB(&n->next[i], GET_UNMARKED(n_next), set_mark((uintptr_t)n_next));
      	} 
      while (!cas);
    }

  return (cas);	/* if I was the one that marked lvl 0 */
}

sval_t
fraser_remove(sl_intset_t *set, skey_t key)
{
  UPDATE_TRY();

  sl_node_t* succs[FRASER_MAX_MAX_LEVEL];
  sval_t result = 0;

  PARSE_START_TS(2);
  int found = fraser_search_no_cleanup_succs(set, key, succs);
  PARSE_END_TS(2, lat_parsing_rem++);

  if (!found)
    {
      return false;
    }

  sl_node_t* node_del = succs[0];
  int my_delete = mark_node_ptrs(node_del);

  if (my_delete)
    {
      result = node_del->val;
      fraser_search(set, key, NULL, NULL);
#if GC == 1
      ssmem_free(alloc, (void*) succs[0]);
#endif
    }

  return result;
}

int
fraser_insert(sl_intset_t *set, skey_t key, sval_t val) 
{
  sl_node_t *new, *pred, *succ;
  sl_node_t *succs[FRASER_MAX_MAX_LEVEL], *preds[FRASER_MAX_MAX_LEVEL];
  int i, found;

  PARSE_START_TS(1);
 retry: 	
  UPDATE_TRY();

  found = fraser_search_no_cleanup(set, key, preds, succs);
  PARSE_END_TS(1, lat_parsing_put);

  if(found)
    {
      PARSE_END_INC(lat_parsing_put);
      return false;
    }


  new = sl_new_simple_node(key, val, get_rand_level(), 0);

  for (i = 0; i < new->toplevel; i++)
    {
      new->next[i] = succs[i];
    }

#if defined(__tile__)
  MEM_BARRIER;
#endif

  /* Node is visible once inserted at lowest level */
  if (!ATOMIC_CAS_MB(&preds[0]->next[0], GET_UNMARKED(succs[0]), new))
    {
      sl_delete_node(new);
      goto retry;
    }

  for (i = 1; i < new->toplevel; i++) 
    {
      while (1) 
	{
	  pred = preds[i];
	  succ = succs[i];
	  if (IS_MARKED(new->next[i]))
	    {
	      PARSE_END_INC(lat_parsing_put);
	      return true;
	    }
	  if (ATOMIC_CAS_MB(&pred->next[i], succ, new))
	    break;

	  fraser_search(set, key, preds, succs);
	}
    }
  PARSE_END_INC(lat_parsing_put);
  return true;
}

