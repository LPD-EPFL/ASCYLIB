/*
 * File:
 *   lazy.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Lazy linked list implementation of an integer set based on Heller et al. algorithm
 *   "A Lazy Concurrent List-Based Set Algorithm"
 *   S. Heller, M. Herlihy, V. Luchangco, M. Moir, W.N. Scherer III, N. Shavit
 *   p.3-16, OPODIS 2005
 *
 * Copyright (c) 2009-2010.
 *
 * lazy.c is part of Synchrobench
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

#include "lazy.h"

inline int
is_marked_ref(uintptr_t i) 
{
  /* if (r) { printf("is_marked_ref(%p) = %d\n", i, r); } */
  return (int) (i & 0x1L);
}

inline uintptr_t
unset_mark(uintptr_t* i) 
{
  *i &= ~0x1L;
  return *i;
}

inline uintptr_t
set_mark(uintptr_t* i)
{
  *i |= 0x1L;
  return *i;
}

inline uintptr_t
get_unmarked_ref(uintptr_t w) 
{
  return w & ~0x1L;
}

inline uintptr_t
get_marked_ref(uintptr_t w) 
{
  return w | 0x1L;
}

/*
 * Checking that both curr and pred are both unmarked and that pred's next pointer
 * points to curr to verify that the entries are adjacent and present in the list.
 */
inline int
parse_validate(node_l_t* pred, node_l_t* curr) 
{
  /* FIX: checking pred twice :: return (!is_marked_ref((long) pred) && !is_marked_ref((long) pred) && (pred->next == curr)); */
  return (!is_marked_ref((uintptr_t) pred->next) && !is_marked_ref((uintptr_t) curr->next) && (pred->next == curr));
}

int
parse_find(intset_l_t *set, val_t val)
{
  node_l_t *curr;
  curr = set->head;
  while (curr->val < val)
    {
      curr = (node_l_t*) get_unmarked_ref((uintptr_t) curr->next);
    }
  return ((curr->val == val) && !is_marked_ref((uintptr_t) curr->next));
}

int
parse_insert(intset_l_t *set, val_t val)
{
  node_l_t *curr, *pred, *newnode;
  int result;
	
  pred = set->head;
  curr = (node_l_t*) get_unmarked_ref((uintptr_t) pred->next);
  while (curr->val < val) 
    {
      pred = curr;
      curr = (node_l_t*) get_unmarked_ref((uintptr_t) curr->next);
    }

  GL_LOCK(set->lock);		/* when GL_[UN]LOCK is defined the [UN]LOCK is not ;-) */
  LOCK(ND_GET_LOCK(pred));
  LOCK(ND_GET_LOCK(curr));
  result = (parse_validate(pred, curr) && (curr->val != val));
  if (result) 
    {
      newnode = new_node_l(val, curr, 0);
      pred->next = newnode;
    } 
  GL_UNLOCK(set->lock);
  UNLOCK(ND_GET_LOCK(curr));
  UNLOCK(ND_GET_LOCK(pred));
  return result;
}

/*
 * Logically remove an element by setting a mark bit to 1 
 * before removing it physically.
 *
 * NB. it is not safe to free the element after physical deletion as a 
 * pre-empted find operation may currently be parsing the element.
 * TODO: must implement a stop-the-world garbage collector to correctly 
 * free the memory.
 */
int
parse_delete(intset_l_t *set, val_t val)
{
  node_l_t *pred, *curr;
  int result;
	
  pred = set->head;
  curr = (node_l_t*) get_unmarked_ref((uintptr_t) pred->next);
  while (curr->val < val)
    {
      pred = curr;
      curr = (node_l_t*) get_unmarked_ref((uintptr_t) curr->next);
    }

  GL_LOCK(set->lock);		/* when GL_[UN]LOCK is defined the [UN]LOCK is not ;-) */
  LOCK(ND_GET_LOCK(pred));
  LOCK(ND_GET_LOCK(curr));
  result = (parse_validate(pred, curr) && (val == curr->val));
  if (result)
    {
      node_l_t* c_nxt = curr->next;
      set_mark((uintptr_t*) &curr->next);
      pred->next = c_nxt;
    }
  GL_UNLOCK(set->lock);
  UNLOCK(ND_GET_LOCK(curr));
  UNLOCK(ND_GET_LOCK(pred));
  return result;
}
