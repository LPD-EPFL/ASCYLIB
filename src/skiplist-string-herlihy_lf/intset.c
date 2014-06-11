/*
 * File:
 *   intset.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Skip list integer set operations 
 *
 * Copyright (c) 2009-2010.
 *
 * intset.c is part of Synchrobench
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

#include "intset.h"

#define MAXLEVEL    32

strval_t
sl_contains(sl_intset_t *set, strkey_t key)
{
  // strval_t result = {""};
	
// #ifdef SEQUENTIAL /* Unprotected */
	
//   int i;
//   sl_node_t *node, *next;
	
//   node = set->head;
//   for (i = node->toplevel-1; i >= 0; i--) 
//     {
//       next = node->next[i];
//       // while (next->key < key)
//       while (strkey_compare(next->key, key) < 0) 
// 	{
// 	  node = next;
// 	  next = node->next[i];
// 	}
//     }
//   node = node->next[0];
//   result = (node->key == key);

// #elif defined LOCKFREE /* fraser lock-free */
  // result = fraser_find(set, key);
// #endif
	
  // return result;

  return fraser_find(set, key);
}

// inline int
// sl_seq_add(sl_intset_t *set, strkey_t key, strval_t val) 
// {
//   int i, l, result;
//   sl_node_t *node, *next;
//   sl_node_t *preds[MAXLEVEL], *succs[MAXLEVEL];
	
//   node = set->head;
//   for (i = node->toplevel-1; i >= 0; i--) 
//     {
//       next = node->next[i];
//       // while (next->key < key)
//       while (strkey_compare(next->key, key) < 0) 
// 	{
// 	  node = next;
// 	  next = node->next[i];
// 	}
//       preds[i] = node;
//       succs[i] = node->next[i];
//     }
//   node = node->next[0];
//   if ((result = (node->key != key)) == 1)
//     {
//       l = get_rand_level();
//       node = sl_new_simple_node(key, val, l, 1);
//       for (i = 0; i < l; i++) 
// 	{
// 	  node->next[i] = succs[i];
// 	  preds[i]->next[i] = node;
// 	}
//     }
//   return result;
// }

int
sl_add(sl_intset_t *set, strkey_t key, strval_t val)
{
//   int result = 0;
// // #ifdef SEQUENTIAL
//   // result = sl_seq_add(set, key, val);
// // #elif defined LOCKFREE /* fraser lock-free */
//   result = fraser_insert(set, key, val);
// // #endif
//   return result;

  return fraser_insert(set, key, val);
}

strval_t
sl_remove(sl_intset_t *set, strkey_t key)
{
  // strval_t result = {""};
	
// #ifdef SEQUENTIAL
//   int i;
//   sl_node_t *node, *next = NULL;
//   sl_node_t *preds[MAXLEVEL], *succs[MAXLEVEL];
	
//   node = set->head;
//   for (i = node->toplevel-1; i >= 0; i--) 
//     {
//       next = node->next[i];
//       while (next->key < key) 
// 	{
// 	  node = next;
// 	  next = node->next[i];
// 	}
//       preds[i] = node;
//       succs[i] = node->next[i];
//     }
//   if ((result = (next->key == key)) == 1) 
//     {
//       for (i = 0; i < set->head->toplevel; i++) 
// 	if (succs[i]->key == key)
// 	  preds[i]->next[i] = succs[i]->next[i];
//       sl_delete_node(next); 
//     }

// #elif defined LOCKFREE
  // result = fraser_remove(set, key);
// #endif
	
  // return result;

  return fraser_remove(set, key);
}


