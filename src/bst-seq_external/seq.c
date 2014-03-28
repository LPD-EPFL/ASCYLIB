/*
 * File:
 *   .c
 * Author(s):
 * Description:
 */

#include "seq.h"


sval_t
seq_delete(intset_t* set, skey_t key)
{
  node_t* curr = set->head;
  node_t* pred = curr;

  while (likely(curr != NULL && !curr->leaf))
    {
      pred = curr;
      skey_t curr_key = curr->key;
      if (key < curr_key)
	{
	  curr = curr->left;
	}
      else
	{
	  curr = curr->right;
	}
    }

  if (!curr || curr->key != key)
    {
      return 0;
    }

  if (pred->left == curr)
    {
      pred->left = NULL;
    }
  else
    {
      pred->right = NULL;
    }

  node_delete(curr);

  return 1;
}

sval_t
seq_find(intset_t* set, skey_t key) 
{
  node_t* curr = set->head;

  while (likely(curr != NULL && !curr->leaf))
    {
      skey_t curr_key = curr->key;
      if (key < curr_key)
	{
	  curr = curr->left;
	}
      else
	{
	  curr = curr->right;
	}
    }

  if (curr && curr->key == key)
    {
      return curr->val;
    }  

  return 0;
}

int
seq_insert(intset_t* set, skey_t key, sval_t val) 
{
  node_t* curr = set->head;
  node_t* pred = curr;

  while (likely(curr != NULL && !curr->leaf))
    {
      pred = curr;
      skey_t curr_key = curr->key;
      if (key < curr_key)
	{
	  curr = curr->left;
	}
      else
	{
	  curr = curr->right;
	}
    }

  if (curr && curr->key == key)
    {
      return 0;
    }

  node_t* node = new_node(key, val, NULL, NULL);
  node_t* ins = node;

  if (curr != NULL)
    {
      node_t* rnode;
      if (key < curr->key)
	{
	  rnode = new_node(curr->key, 0, node, curr);
	}
      else
	{
	  rnode = new_node(key, 0, curr, node);
	}
      ins = rnode;
    }

  if ((curr && pred->left == curr) || (key < pred->key))
    {
      pred->left = ins;
    }
  else
    {
      pred->right = ins;
    }

  return 1;
}
