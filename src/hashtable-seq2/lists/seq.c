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
  node_t *pred = NULL, *next;
  sval_t res = 0;
	
  next = set->head;

  while (next != NULL && next->key < key)
    {
      pred = next;
      next = next->next;
    }

  if (next != NULL && key == next->key)
    {
      res = next->val;
      if (pred != NULL)
	{
	  pred->next = next->next;
	}
      else
	{
	  set->head = next->next;
	}
      node_delete(next);
    } 

  return res;
}

sval_t
seq_find(intset_t *set, skey_t key) 
{
  node_t *curr = set->head;
  sval_t res = 0;

  while (curr != NULL && curr->key < key) 
    {
      curr = curr->next;
    }	
  if (curr != NULL && key == curr->key)
    {
      res = curr->val;
    }
  return res;
}

int
seq_insert(intset_t *set, skey_t key, sval_t val) 
{
  node_t *next, *newnode;
  node_t* pred = NULL;
	
  next = set->head;
	
  while (next != NULL && next->key < key) 
    {
      pred = next;
      next = pred->next;
    }
  int found = (next != NULL && key == next->key);
  if (!found) 
    {
      newnode = new_node(key, val, next);
      if (pred != NULL)
	{
	  pred->next = newnode;
	}
      else
	{
	  set->head = newnode;
	}
    }
  return !found;
}
