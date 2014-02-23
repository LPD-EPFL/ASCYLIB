/*
 * File:
 *   fraser.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Lock-based skip list implementation of the Fraser algorithm
 *   "Practical Lock Freedom", K. Fraser, 
 *   PhD dissertation, September 2003
 *   Cambridge University Technical Report UCAM-CL-TR-579 
 *
 * Copyright (c) 2009-2010.
 *
 * fraser.c is part of Synchrobench
 * 
 * Synchrobench is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "fraser.h"

extern ALIGNED(CACHE_LINE_SIZE) unsigned int levelmax;

#define FRASER_MAX_MAX_LEVEL 64 /* covers up to 2^64 elements */

void
fraser_search(sl_intset_t *set, skey_t key, sl_node_t **left_list, sl_node_t **right_list)
{
  int i;
  sl_node_t *left, *left_next, *right, *right_next;

 retry:
  left = set->head;
  for (i = levelmax - 1; i >= 0; i--)
    {
      left_next = left->next[i];
      if (unlikely(is_marked((uintptr_t)left_next)))
	{
	  goto retry;
	}
      /* Find unmarked node pair at this level */
      for (right = left_next; ; right = right_next)
	{
	  /* Skip a sequence of marked nodes */
	  right_next = right->next[i];
	  while (unlikely(is_marked((uintptr_t)right_next)))
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
}

static sl_node_t* 
fraser_left_search(sl_intset_t *set, skey_t key)
{
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

      if (unlikely(left->key == key))
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
  sl_node_t* left = fraser_left_search(set, key);
  if (left->key == key && !left->deleted)
    {
      result = left->val;
    }
  return result;
}

inline void
mark_node_ptrs(sl_node_t *n)
{
  int i;
  sl_node_t *n_next;
	
  for (i = n->toplevel - 1; i >= 0; i--)
    {
      do
      	{
      	  n_next = n->next[i];
      	  if (is_marked((uintptr_t)n_next))
      	    {
      	      break;
      	    }
      	} 
      while (!ATOMIC_CAS_MB(&n->next[i], n_next, set_mark((uintptr_t)n_next)));
    }
}

sval_t
fraser_remove(sl_intset_t *set, skey_t key)
{
  sl_node_t* succs[FRASER_MAX_MAX_LEVEL];
  sval_t result = 0;

  fraser_search(set, key, NULL, succs);
  if (succs[0]->key != key)
    {
      goto end;
    }
  /* 1. Node is logically deleted when the deleted field is not 0 */
  if (succs[0]->deleted)
    {
      goto end;
    }


  if (ATOMIC_FETCH_AND_INC_FULL(&succs[0]->deleted) == 0)
    {
      /* 2. Mark forward pointers, then search will remove the node */
      mark_node_ptrs(succs[0]);

      result = succs[0]->val;
      fraser_search(set, key, NULL, NULL);
#if GC == 1
      ssmem_free(alloc, succs[0]);
#endif
    }

 end:
  return result;
}

int
fraser_insert(sl_intset_t *set, skey_t key, sval_t val) 
{
  sl_node_t *new, *new_next, *pred, *succ;
  /* sl_new_node **succs, **preds; */
  sl_node_t *succs[FRASER_MAX_MAX_LEVEL], *preds[FRASER_MAX_MAX_LEVEL];
  int i;
  int result = 0;

  new = sl_new_simple_node(key, val, get_rand_level(), 0);
 retry: 	
  fraser_search(set, key, preds, succs);
  /* Update the value field of an existing node */
  if (succs[0]->key == key) 
    {				/* Value already in list */
      if (succs[0]->deleted)
	{		   /* Value is deleted: remove it and retry */
	  mark_node_ptrs(succs[0]);
	  goto retry;
	}
      result = 0;
      sl_delete_node(new);
      goto end;
    }

  for (i = 0; i < new->toplevel; i++)
    {
      new->next[i] = succs[i];
    }

#if defined(__tile__)
  MEM_BARRIER;
#endif

  /* Node is visible once inserted at lowest level */
  if (!ATOMIC_CAS_MB(&preds[0]->next[0], succs[0], new))
    {
      goto retry;
    }

  for (i = 1; i < new->toplevel; i++) 
    {
      while (1) 
	{
	  pred = preds[i];
	  succ = succs[i];
	  /* Update the forward pointer if it is stale */
	  new_next = new->next[i];

	  if (unlikely(new->deleted))
	    {
	      goto success;
	    }
	  if ((new_next != succ) && 
	      (!ATOMIC_CAS_MB(&new->next[i], unset_mark((uintptr_t)new_next), succ)))
	    {
	      break; /* Give up if pointer is marked */
	    }
	  /* Check for old reference to a k node */
	  if (succ->key == key)
	    {
	      succ = (sl_node_t *)unset_mark((uintptr_t)succ->next);
	    }
	  /* We retry the search if the CAS fails */
	  if (ATOMIC_CAS_MB(&pred->next[i], succ, new))
	    break;

	  /* MEM_BARRIER; */
	  fraser_search(set, key, preds, succs);
	}
    }

 success:
  result = 1;
 end:
  return result;
}

