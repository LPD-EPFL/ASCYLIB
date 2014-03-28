/*
 * File:
 *   .c
 * Author(s):
 * Description:
 */

#include "seq.h"


node_t** 
search(node_t** root, skey_t key) 
{
  node_t** node = root;
  while (unlikely(*node != NULL))
    {
      sval_t node_key = (*node)->key;
      if (key < node_key)
	{
	  node = &(*node)->left;
	}
      else if (key > node_key)
	{
	  node = &(*node)->right;
	}
      else
	{
	  break;
	}
    }
  return node;
}


void
seq_delete_node(node_t** node)
{
  node_t* old_node = *node;
  if ((*node)->left == NULL) 
    {
      *node = (*node)->right;
      node_delete(old_node);
    } 
  else if ((*node)->right == NULL) 
    {
      *node = (*node)->left;
      node_delete(old_node);
    } 
  else 
    {
      /* delete node with two children */
      node_t* rchild = (*node)->right;
      node_t* rchildparent = *node;
      while(rchild->left != NULL)
	{
	  rchildparent=rchild;
	  rchild = rchild->left;
	}

      (*node)->key = rchild->key;
      if(rchildparent == *node)
	{
	  (*node)->right = rchild->right;
	}
      else
	{
	  rchildparent->left = rchild->right;
	}

      node_delete(rchild);
    }
}


sval_t
seq_delete(intset_t* set, skey_t key)
{
  node_t** node = search(&set->head, key);
  if (*node == NULL)
    {
      return 0;
    }
  
  seq_delete_node(node);

  return 1;
}

sval_t
seq_find(intset_t* set, skey_t key) 
{
  node_t* node = set->head;
  while (unlikely(node != NULL))
    {
      sval_t node_key = (node)->key;
      if (key < node_key)
	{
	  node = node->left;
	}
      else if (key > node_key)
	{
	  node = node->right;
	}
      else
	{
	  break;
	}
    }
  return (node != NULL);
}

int
seq_insert(intset_t* set, skey_t key, sval_t val) 
{
  node_t** node = search(&set->head, key);
  if (*node == NULL) 
    {
      *node = new_node(key, val, 0);
      return 1;
    }

  return 0;
}
