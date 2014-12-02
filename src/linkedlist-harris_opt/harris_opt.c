/*   
 *   File: harris_opt.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: Timothy L Harris. A Pragmatic Implementation 
 *   of Non-blocking Linked Lists. DISC 2001.
 *   Optimized version
 *   harris_opt.c is part of ASCYLIB
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

#include "linkedlist.h"

RETRY_STATS_VARS;

/*
 * The five following functions handle the low-order mark bit that indicates
 * whether a node is logically deleted (1) or not (0).
 *  - is_marked_ref returns whether it is marked, 
 *  - (un)set_marked changes the mark,
 *  - get_(un)marked_ref sets the mark before returning the node.
 */
inline int
is_marked_ref(node_t* i) 
{
  return ((uintptr_t) i & 0x1L);
}

inline node_t*
get_unmarked_ref(node_t* w) 
{
  return (node_t*) ((uintptr_t) w & ~0x1L);
}

inline node_t*
get_marked_ref(node_t* w) 
{
  return (node_t*) ((uintptr_t) w | 0x1L);
}

static inline int
physical_delete_right(node_t* left_node, node_t* right_node) 
{
  node_t* new_next = get_unmarked_ref(right_node->next);
  node_t* res = CAS_PTR(&left_node->next, right_node, new_next);
  int removed = (res == right_node);
#if GC == 1
  if (likely(removed))
    {
      ssmem_free(alloc, (void*) res);
    }
#endif
  return removed;
}


/*
 * list_search looks for value val, it
 *  - returns right_node owning val (if present) or its immediately higher 
 *    value present in the list (otherwise) and 
 *  - sets the left_node to the node owning the value immediately lower than val. 
 * Encountered nodes that are marked as logically deleted are physically removed
 * from the list, yet not garbage collected.
 */
static inline node_t* 
list_search(intset_t* set, skey_t key, node_t** left_node_ptr) 
{
  PARSE_TRY();
  node_t* left_node = set->head;
  node_t* right_node = set->head->next;
  while(1)
    {
      if (likely(!is_marked_ref(right_node->next)))
	{
	  if (unlikely(right_node->key >= key))
	    {
	      break;
	    }
	  left_node = right_node;
	}
      else 
	{
	  CLEANUP_TRY();
	  physical_delete_right(left_node, right_node);
	}
      right_node = get_unmarked_ref(right_node->next);
    }
  *left_node_ptr = left_node;
  return right_node;
}

/*
 * returns a value different from 0 if there is a node in the list owning value val.
 */
sval_t
harris_find(intset_t* the_list, skey_t key)
{
  node_t* node = the_list->head->next;
  PARSE_TRY();
  while(likely(node->key < key))
    {
      node = get_unmarked_ref(node->next);
    }
  /* node_t* l; */
  /* node_t* node = list_search(the_list, key, &l); */

  if (node->key == key && !is_marked_ref(node->next)) 
    {
      return node->val;
    }
  return 0;
}


/*
 * inserts a new node with the given value val in the list
 * (if the value was absent) or does nothing (if the value is already present).
 */
int
harris_insert(intset_t *the_list, skey_t key, sval_t val)
{
  do
    {
      UPDATE_TRY();
      node_t* left_node;
      node_t* right_node = list_search(the_list, key, &left_node);
      if (right_node->key == key) 
	{
	  return 0;
	}

      node_t* node_to_add = new_node(key, val, right_node, 0);

#ifdef __tile__
      MEM_BARRIER;
#endif
      // Try to swing left_node's unmarked next pointer to a new node

      if (CAS_PTR(&left_node->next, right_node, node_to_add) == right_node)
	{
	  return 1;
	}

#if GC == 1
      ssmem_free(alloc, (void*) node_to_add);
#endif
    } 
  while (1);
}

/*
 * deletes a node with the given value val (if the value is present) 
 * or does nothing (if the value is already present).
 * The deletion is logical and consists of setting the node mark bit to 1.
 */
sval_t
harris_delete(intset_t *the_list, skey_t key)
{
  node_t* cas_result;
  node_t* unmarked_ref;
  node_t* left_node;
  node_t* right_node;
  
  do
    {
      UPDATE_TRY();
      right_node = list_search(the_list, key, &left_node);

      if (right_node->key != key)
	{
	  return 0;
	}
    
      // Try to mark right_node as logically deleted
      unmarked_ref = get_unmarked_ref(right_node->next);
      node_t* marked_ref = get_marked_ref(unmarked_ref);
      cas_result = CAS_PTR(&right_node->next, unmarked_ref, marked_ref);
    } 
  while(cas_result != unmarked_ref);

  sval_t ret = right_node->val;

  physical_delete_right(left_node, right_node);

  return ret;
}

int
set_size(intset_t *set)
{
  size_t size = 0;
  node_t* node;

  /* We have at least 2 elements */
  node = get_unmarked_ref(set->head->next);
  while (get_unmarked_ref(node->next) != NULL)
    {
      if (!is_marked_ref(node->next)) size++;
      node = get_unmarked_ref(node->next);
    }

  return size;
}
