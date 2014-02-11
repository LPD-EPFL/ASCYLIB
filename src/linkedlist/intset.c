/*
 *  intset.c
 *  
 *  Integer set operations (contain, insert, delete) 
 *  that call stm-based / lock-free counterparts.
 *
 *  Created by Vincent Gramoli on 1/12/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "intset.h"

int
set_contains(intset_t *set, skey_t key)
{
  int result;
	
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
  result = (next->key == key);
#elif defined LOCKFREE			
  result = harris_find(set, key);
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
      result = harris_insert(set, key, val);
#endif
  return result;
}

int
set_remove(intset_t *set, skey_t key)
{
  int result = 0;
	
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
  result = (next->key == key);
  if (result) 
    {
      prev->next = next->next;
      free(next);
    }
#elif defined LOCKFREE
  result = harris_delete(set, key);
#endif
	
  return result;
}


