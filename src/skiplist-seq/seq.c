/*
 * File:
 *   intset.c
 * Author(s):
 */

#include "seq.h"

#define MAXLEVEL    32

sval_t
sl_contains(sl_intset_t *set, skey_t key)
{
  sval_t result = 0;

  int i;
  sl_node_t *node, *next;
	
  node = set->head;
  for (i = node->toplevel-1; i >= 0; i--) 
    {
      next = node->next[i];
      while (next->key < key) 
	{
	  node = next;
	  next = node->next[i];
	}
    }
  node = node->next[0];

  if (node->key == key)
    {
      result = node->val;
    }
		
  return result;
}


int
sl_add(sl_intset_t *set, skey_t key, sval_t val)
{
  int i, l, result;
  sl_node_t *node, *next;
  sl_node_t *preds[MAXLEVEL], *succs[MAXLEVEL];
	
  node = set->head;
  for (i = node->toplevel-1; i >= 0; i--) 
    {
      next = node->next[i];
      while (next->key < key) 
	{
	  node = next;
	  next = node->next[i];
	}
      preds[i] = node;
      succs[i] = node->next[i];
    }
  node = node->next[0];
  result = (node->key != key);
  if (result == 1)
    {
      l = get_rand_level();
      node = sl_new_simple_node(key, val, l, 1);
      for (i = 0; i < l; i++) 
	{
	  node->next[i] = succs[i];
#ifdef __tile__
    MEM_BARRIER;
#endif
	  preds[i]->next[i] = node;
	}
    }
  return result;
}

sval_t
sl_remove(sl_intset_t *set, skey_t key)
{
  sval_t result = 0;
  int i;
  sl_node_t *node, *next = NULL;
  sl_node_t *preds[MAXLEVEL], *succs[MAXLEVEL];
	
  node = set->head;
  for (i = node->toplevel-1; i >= 0; i--) 
    {
      next = node->next[i];
      while (next->key < key) 
	{
	  node = next;
	  next = node->next[i];
	}
      preds[i] = node;
      succs[i] = node->next[i];
    }

  if (next->key == key)
    {
      result = next->val;
      for (i = 0; i < set->head->toplevel; i++) 
	if (succs[i]->key == key)
	  {
	    preds[i]->next[i] = succs[i]->next[i];
	  }
      sl_delete_node(next); 
    }
  return result;
}


