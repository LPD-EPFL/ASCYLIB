/*   
 *   File: seq.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   seq.c is part of ASCYLIB
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

#include "seq.h"

sval_t
seq_delete(intset_t *set, skey_t key)
{
  node_t *curr, *next;
  sval_t res = 0;
	
  curr = set->head;
  next = curr->next;
	
  while (next != NULL && next->key < key) 
    {
      curr = next;
      next = next->next;
    }

  if (next != NULL && key == next->key)
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
	
  while (next != NULL && next->key < key) 
    {
      curr = next;
      next = curr->next;
    }	
  if (next != NULL && key == next->key)
    {
      res = next->val;
    }
  return res;
}

int
seq_insert(intset_t *set, skey_t key, sval_t val) 
{
  node_t *next, *newnode;
  volatile node_t* curr;
  int found;
	
  curr = set->head;
  next = curr->next;
	
  while (next != NULL && next->key < key) 
    {
      curr = next;
      next = curr->next;
    }
  found = (next != NULL && key == next->key);
  if (!found) 
    {
      newnode = new_node(key, val, next, 0);
      curr->next = newnode;
    }
  return !found;
}
