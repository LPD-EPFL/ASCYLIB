/*
 * File:
 *   coupling.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Hand-over-hand lock-based linked list implementation of an integer set
 *
 * Copyright (c) 2009-2010.
 *
 * coupling.c is part of Synchrobench
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

#include "seq.h"

sval_t
seq_delete(intset_t *set, skey_t key)
{
  node_t *curr, *next;
  sval_t res = 0;
	
  curr = set->head;
  next = curr->next;
	
  while (likely(next->key < key))
    {
      curr = next;
      next = next->next;
    }

  if (key == next->key)
    {
      res = next->val;
      curr->next = next->next;
      node_delete(next);
    } 

  return res;
}

sval_t
seq_find(intset_t *set, skey_t key) 
{
  node_t *curr, *next; 
  sval_t res = 0;
	
  curr = set->head;
  next = curr->next;
	
  while (likely(next->key < key))
    {
      curr = next;
      next = curr->next;
    }	

  if (key == next->key)
    {
      res = next->val;
    }

  return res;
}

int
seq_insert(intset_t *set, skey_t key, sval_t val) 
{
  node_t *curr, *next, *newnode;
  int found;
	
  curr = set->head;
  next = curr->next;
	
  while (likely(next->key < key))
    {
      curr = next;
      next = curr->next;
    }

  found = (key == next->key);
  if (!found) 
    {
      newnode = new_node(key, val, next, 0);
      curr->next = newnode;
    }
  return !found;
}
