/*
 * File:
 *   coupling.c
 * Author(s):
 */

#include "seq.h"

sval_t
seq_delete(intset_t *set, skey_t key)
{
  node_t *curr, *pred;
  sval_t res = 0;
	
  curr = set->head;
  pred = NULL;
	
  while (curr != NULL)
    {
      if (curr->key == key)
	{
	  res = curr->val;
	  if (pred != NULL)
	    {
	      pred->next = curr->next;
	    }
	  else
	    {
	      set->head = curr->next;
	    }
	  node_delete(curr);
	  return res;
	}
      pred = curr;
      curr = curr->next;
    }

  return res;
}

sval_t
seq_find(intset_t *set, skey_t key) 
{
  node_t *curr; 
  sval_t res = 0;
	
  curr = set->head;
  while (curr != NULL)
    {
      if (curr->key == key)
	{
	  res = curr->val;
	  break;
	}
      curr = curr->next;
    }	
  return res;
}

int
seq_insert(intset_t *set, skey_t key, sval_t val) 
{
  node_t *pred, *newnode;
  volatile node_t* curr;
	
  curr = set->head;
  pred = NULL;
	
  while (curr != NULL)
    {
      if (curr->key == key)
	{
	  return 0;
	}
      pred = curr;
      curr = curr->next;
    }

  newnode = new_node(key, val, NULL, 0);
  if (pred != NULL)
    {
      pred->next = newnode;
    }
  else
    {
      set->head = newnode;
    }

  return 1;
}
