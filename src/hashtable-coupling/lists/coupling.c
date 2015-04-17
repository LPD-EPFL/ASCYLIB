/*   
 *   File: coupling.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   Hand-over-hand lock-based linked list implementation of an integer set
 *   coupling.c is part of ASCYLIB
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

#include "coupling.h"

/* 
 * Similar algorithm for the delete, find, and insert:
 * Lock the first two elements (locking each before getting the copy of the element)
 * then unlock previous, keep ownership of the current, and lock next in a loop.
 */
sval_t
lockc_delete(intset_l_t *set, skey_t key)
{
  node_l_t *curr, *next;
  sval_t res = 0;
	
  GL_LOCK(&set->lock);		/* when GL_[UN]LOCK is defined the [UN]LOCK is not ;-) */
  LOCK(ND_GET_LOCK(set->head));
  curr = set->head;
  if (curr->next != NULL)
    {
      LOCK(ND_GET_LOCK(curr->next));
    }
  next = curr->next;
	
  while (next != NULL && next->key < key) 
    {
      UNLOCK(ND_GET_LOCK(curr));
      curr = next;
      if (next->next != NULL)
	{
	  LOCK(ND_GET_LOCK(next->next));
	}
      next = next->next;
    }

  if (next != NULL && key == next->key)
    {
      res = next->val;
      curr->next = next->next;
      UNLOCK(ND_GET_LOCK(next));
      node_delete_l(next);
      UNLOCK(ND_GET_LOCK(curr));
    } 
  else 
    {
      UNLOCK(ND_GET_LOCK(curr));
      if (next != NULL)
	{
	  UNLOCK(ND_GET_LOCK(next));
	}
    }  
  GL_UNLOCK(&set->lock);

  return res;
}

sval_t
lockc_find(intset_l_t *set, skey_t key) 
{
  node_l_t *curr, *next; 
  sval_t res = 0;
	
  GL_LOCK(&set->lock);		/* when GL_[UN]LOCK is defined the [UN]LOCK is not ;-) */
  LOCK(ND_GET_LOCK(set->head));
  curr = set->head;
  if (curr->next != NULL)
    {
      LOCK(ND_GET_LOCK(curr->next));
    }
  next = curr->next;
	
  while (next != NULL && next->key < key) 
    {
      UNLOCK(ND_GET_LOCK(curr));
      curr = next;
      if (next->next != NULL)
	{
	  LOCK(ND_GET_LOCK(next->next));
	}
      next = curr->next;
    }	
  if (next != NULL && key == next->key)
    {
      res = next->val;
    }
  GL_UNLOCK(&set->lock);
  UNLOCK(ND_GET_LOCK(curr));
  if (next != NULL)
    {
      UNLOCK(ND_GET_LOCK(next));
    }
  return res;
}

int
lockc_insert(intset_l_t *set, skey_t key, sval_t val) 
{
  node_l_t *next, *newnode;
  volatile node_l_t* curr;
  int found;
	
  GL_LOCK(&set->lock);		/* when GL_[UN]LOCK is defined the [UN]LOCK is not ;-) */
  LOCK(ND_GET_LOCK(set->head));
  curr = set->head;
  if (curr->next != NULL)
    {
      LOCK(ND_GET_LOCK(curr->next));
    }
  next = curr->next;
	
  while (next != NULL && next->key < key) 
    {
      UNLOCK(ND_GET_LOCK(curr));
      curr = next;
      if (next->next != NULL)
	{
	  LOCK(ND_GET_LOCK(next->next));
	}
      next = curr->next;
    }
  found = (next != NULL && key == next->key);
  if (!found) 
    {
      newnode =  new_node_l(key, val, next, 1);
      curr->next = newnode;
    }
  GL_UNLOCK(&set->lock);
  UNLOCK(ND_GET_LOCK(curr));
  if (next != NULL)
    {
      UNLOCK(ND_GET_LOCK(next));
    }
  return !found;
}
