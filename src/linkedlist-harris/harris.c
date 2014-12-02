/*   
 *   File: harris.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: Timothy L Harris. A Pragmatic Implementation 
 *   of Non-blocking Linked Lists. DISC 2001.
 *   harris.c is part of ASCYLIB
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

#include "harris.h"

RETRY_STATS_VARS;

/*
 * The five following functions handle the low-order mark bit that indicates
 * whether a node is logically deleted (1) or not (0).
 *  - is_marked_ref returns whether it is marked, 
 *  - (un)set_marked changes the mark,
 *  - get_(un)marked_ref sets the mark before returning the node.
 */
inline int
is_marked_ref(long i) 
{
  /* return (int) (i & (LONG_MIN+1)); */
  return (int) (i & 0x1L);
}

inline long
unset_mark(long i)
{
  /* i &= LONG_MAX-1; */
  i &= ~0x1L;
  return i;
}

inline long
set_mark(long i) 
{
  /* i = unset_mark(i); */
  /* i += 1; */
  i |= 0x1L;
  return i;
}

inline long
get_unmarked_ref(long w) 
{
  /* return unset_mark(w); */
  return w & ~0x1L;
}

inline long
get_marked_ref(long w) 
{
  /* return set_mark(w); */
  return w | 0x1L;
}

/*
 * harris_search looks for value val, it
 *  - returns right_node owning val (if present) or its immediately higher 
 *    value present in the list (otherwise) and 
 *  - sets the left_node to the node owning the value immediately lower than val. 
 * Encountered nodes that are marked as logically deleted are physically removed
 * from the list, yet not garbage collected.
 */
node_t*
harris_search(intset_t *set, skey_t key, node_t **left_node) 
{
  node_t *left_node_next, *right_node;
  left_node_next = set->head;
	
  do
    {
      PARSE_TRY();
      node_t *t = set->head;
      node_t *t_next = set->head->next;
		
      /* Find left_node and right_node */
      do 
	{
	  if (!is_marked_ref((long) t_next)) 
	    {
	      (*left_node) = t;
	      left_node_next = t_next;
	    }
	  t = (node_t *) get_unmarked_ref((long) t_next);
	  if (!t->next) 
	    {
	      break;
	    }
	  t_next = t->next;
	} 
      while (is_marked_ref((long) t_next) || (t->key < key));
      right_node = t;
		
      /* Check that nodes are adjacent */
      if (left_node_next == right_node) 
	{
	  if (right_node->next && is_marked_ref((long) right_node->next))
	    {
	      continue;
	    }
	  else
	    {
	      return right_node;
	    }
	}
		
      CLEANUP_TRY();
      /* Remove one or more marked nodes */
      if (ATOMIC_CAS_MB(&(*left_node)->next, left_node_next, right_node)) 
	{
#if GC == 1
	  node_t* cur = left_node_next;
	  do 
	    {
	      node_t* free = cur;
	      cur = (node_t*) get_unmarked_ref((long) cur->next);
	      ssmem_free(alloc, (void*) free);
	    }
	  while (cur != right_node);
#endif

	  if (!(right_node->next && is_marked_ref((long) right_node->next)))
	    {
	      return right_node;
	    }
	} 
    } 
  while (1);
}

/*
 * harris_find returns whether there is a node in the list owning value val.
 */
sval_t
harris_find(intset_t *set, skey_t key) 
{
  node_t *right_node, *left_node;
  left_node = set->head;
	
  right_node = harris_search(set, key, &left_node);
  if ((right_node->next == NULL) || right_node->key != key)
    {
      return 0;
    }
  else 
    {
      return right_node->val;
    }
}

/*
 * harris_find inserts a new node with the given value val in the list
 * (if the value was absent) or does nothing (if the value is already present).
 */
int
harris_insert(intset_t *set, skey_t key, sval_t val) 
{
  node_t *newnode = NULL, *right_node, *left_node;
  left_node = set->head;
	
  do 
    {
      UPDATE_TRY();
      right_node = harris_search(set, key, &left_node);
      if (right_node->key == key)
	{
#if GC == 1
	  if (unlikely(newnode != NULL))
	    {
	      ssmem_free(alloc, (void*) newnode);
	    }
#endif
	  return 0;
	}

      if (likely(newnode == NULL))
	{
	  newnode = new_node(key, val, right_node, 0);
	}
      else
	{
	  newnode->next = right_node;
	}
#ifdef __tile__
    MEM_BARRIER;
#endif
      if (ATOMIC_CAS_MB(&left_node->next, right_node, newnode))
	return 1;
    } 
  while(1);
}

/*
 * harris_find deletes a node with the given value val (if the value is present) 
 * or does nothing (if the value is already present).
 * The deletion is logical and consists of setting the node mark bit to 1.
 */
sval_t
harris_delete(intset_t *set, skey_t key)
{
  node_t *right_node, *right_node_next, *left_node;
  left_node = set->head;
  sval_t ret = 0;
	
  do 
    {
      UPDATE_TRY();
      right_node = harris_search(set, key, &left_node);
      if (right_node->key != key)
	{
	  return 0;
	}
      right_node_next = right_node->next;
      if (!is_marked_ref((long) right_node_next))
	{
	  if (ATOMIC_CAS_MB(&right_node->next, right_node_next, get_marked_ref((long) right_node_next)))
	    {
	      ret = right_node->val;
	      break;
	    }
	}
    } 
  while(1);

  if (likely(ATOMIC_CAS_MB(&left_node->next, right_node, right_node_next)))
    {
#if GC == 1
      ssmem_free(alloc, (void*) get_unmarked_ref((long) right_node));
#endif
      ;
    }
  else
    {
      harris_search(set, key, &left_node);
    }

  return ret;
}

int
set_size(intset_t *set)
{
  int size = 0;
  node_t* node;

  /* We have at least 2 elements */
  node = (node_t*) get_unmarked_ref((long) set->head->next);
  while ((node_t*) get_unmarked_ref((long) node->next) != NULL)
    {
      if (!is_marked_ref((long) node->next)) size++;
      node = (node_t*) get_unmarked_ref((long) node->next);
    }
  return size;
}
