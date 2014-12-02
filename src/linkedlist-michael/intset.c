/*   
 *   File: intset.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   intset.c is part of ASCYLIB
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

#include "intset.h"

sval_t
set_contains(intset_t *set, skey_t key)
{
  sval_t result;
	
#ifdef DEBUG_PRINT
  printf("++> set_contains(%d)\n", (int)val);
  IO_FLUSH;
#endif
	
#ifdef SEQUENTIAL
  node_t *prev, *next;
	
  prev = set->head;
  next = prev->next;
  while (next->key < keyl) 
    {
      prev = next;
      next = prev->next;
    }
  result = (next->key == key) ? next->val : 0;
#elif defined LOCKFREE			
  result = michael_find(set, key);
#endif	
	
  return result;
}

inline int 
set_seq_add(intset_t* set, skey_t key, sval_t val)
{
  int result;
  node_t *prev, *next;
	
  prev = set->head;
  next = prev->next;
  while (next->key < key) 
    {
      prev = next;
      next = prev->next;
    }
  result = (next->key != key);
  if (result) 
    {
      prev->next = new_node(key, val, next, 0);
    }
  return result;
}	
		

int
set_add(intset_t *set, skey_t key, skey_t val)
{
  int result;
#ifdef DEBUG_PRINT
  printf("++> set_add(%d)\n", (int)val);
  IO_FLUSH;
#endif
#ifdef SEQUENTIAL /* Unprotected */
  result = set_seq_add(set, key, val);
#elif defined LOCKFREE
  result = michael_insert(set, key, val);
#endif
  return result;
}

sval_t
set_remove(intset_t *set, skey_t key)
{
  sval_t result = 0;
	
#ifdef DEBUG_PRINT
  printf("++> set_remove(%d)\n", (int)val);
  IO_FLUSH;
#endif
	
#ifdef SEQUENTIAL /* Unprotected */
  node_t *prev, *next;
  prev = set->head;
  next = prev->next;
  while (next->key < key) 
    {
      prev = next;
      next = prev->next;
    }
  result = (next->key == key) ? next->val : 0;
  if (result) 
    {
      prev->next = next->next;
      free(next);
    }
#elif defined LOCKFREE
  result = michael_delete(set, key);
#endif
	
  return result;
}


