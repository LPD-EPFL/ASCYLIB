/*
 * File:
 *   .c
 * Author(s):
 * Description:
 */

#include "bst_tk.h"

RETRY_STATS_VARS;

sval_t
bst_tk_delete(intset_t* set, skey_t key)
{
  node_t* curr;
  node_t* pred = NULL;
  node_t* ppred = NULL;
  uint32_t curr_ver = 0, pred_ver = 0, ppred_ver = 0, left = 0, pleft = 0;

 retry:
  PARSE_TRY();
  UPDATE_TRY();

  curr = set->head;

  do
    {
      COMPILER_NO_REORDER(curr_ver = curr->lock.version;);

      ppred = pred;
      COMPILER_NO_REORDER(ppred_ver = pred_ver;);
      pleft = left;

      pred = curr;
      COMPILER_NO_REORDER(pred_ver = curr_ver;);

      if (key < curr->key)
	{
	  left = 1;
	  curr = (node_t*) curr->left;
	}
      else
	{
	  left = 0;
	  curr = (node_t*) curr->right;
	}
    }
  while(likely(!curr->leaf));


  if (curr->key != key)
    {
      return 0;
    }


  if (unlikely(!tl_trylock_version(&ppred->lock, ppred_ver)))
    {
      goto retry;
    }

  if (unlikely(!tl_trylock_version(&pred->lock, pred_ver)))
    {
      tl_unlock(&ppred->lock);
      goto retry;
    }


  if (pleft)
    {
      if (left)
	{
	  ppred->left = pred->right;
	}
      else
	{
	  ppred->left = pred->left;
	}
    }
  else
    {
      if (left)
	{
	  ppred->right = pred->right;
	}
      else
	{
	  ppred->right = pred->left;
	}
    }


  tl_unlock(&ppred->lock);


#if GC == 1
  ssmem_free(alloc, curr);
  ssmem_free(alloc, pred);
#endif

  return curr->val;

}

sval_t
bst_tk_find(intset_t* set, skey_t key) 
{
  PARSE_TRY();

  node_t* curr = set->head;

  while (likely(!curr->leaf))
    {
      if (key < curr->key)
	{
	  curr = (node_t*) curr->left;
	}
      else
	{
	  curr = (node_t*) curr->right;
	}
    }

  if (curr->key == key)
    {
      return curr->val;
    }  

  return 0;
}

int
bst_tk_insert(intset_t* set, skey_t key, sval_t val) 
{
  node_t* curr;
  node_t* pred = NULL;
  uint32_t curr_ver = 0, pred_ver = 0, left = 0;

 retry:
  PARSE_TRY();
  UPDATE_TRY();

  curr = set->head;

  do
    {
      COMPILER_NO_REORDER(curr_ver = curr->lock.version;);

      pred = curr;
      COMPILER_NO_REORDER(pred_ver = curr_ver;);

      if (key < curr->key)
	{
	  left = 1;
	  curr = (node_t*) curr->left;
	}
      else
	{
	  left = 0;
	  curr = (node_t*) curr->right;
	}
    }
  while(likely(!curr->leaf));


  if (curr->key == key)
    {
      return 0;
    }

  if (unlikely(!tl_trylock_version(&pred->lock, pred_ver)))
    {
      goto retry;
    }

  node_t* nn = new_node(key, val, NULL, NULL, 0);
  node_t* nr;
  
  if (key < curr->key)
    {
      nr = new_node(curr->key, 0, nn, curr, 0);
    }
  else
    {
      nr = new_node(key, 0, curr, nn, 0);
    }

  if (left)
    {
      pred->left = nr;
    }
  else
    {
      pred->right = nr;
    }

  tl_unlock(&pred->lock);

  return 1;
}
