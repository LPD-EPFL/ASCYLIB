#include "optimistic.h"
#include "utils.h"

unsigned int levelmax;

#define MAX_BACKOFF 131071
#define HERLIHY_MAX_MAX_LEVEL 64 /* covers up to 2^64 elements */

sval_t
optimistic_find(sl_intset_t *set, skey_t key)
{ 
  sl_node_t* succ = NULL;
  sl_node_t* pred = set->head;
  int lvl;
  for (lvl = levelmax - 1; lvl >= 0; lvl--)
    {
      succ = pred->next[lvl];
      while (succ->key < key)
	{
	  pred = succ;
	  succ = succ->next[lvl]; 
	}

      if (succ->key == key)	/* at any search level */
	{
	  return succ->val;
	}
    }

  return false;
}

sl_node_t*
get_lock(sl_node_t* pred, skey_t key, int lvl)
{
  sl_node_t* succ = pred->next[lvl];
  while (succ->key < key)
    {
      pred = succ;
      succ = succ->next[lvl];
    }

  LOCK(ND_GET_LOCK(pred));
  succ = pred->next[lvl];
  while (succ->key < key)
    {
      UNLOCK(ND_GET_LOCK(pred));
      pred = succ;
      LOCK(ND_GET_LOCK(pred));
      succ = pred->next[lvl];
    }

  return pred;
}

int
optimistic_insert(sl_intset_t *set, skey_t key, sval_t val)
{
  sl_node_t* update[HERLIHY_MAX_MAX_LEVEL];
  sl_node_t* succ;
  sl_node_t* pred = set->head;
  int lvl;
  for (lvl = levelmax - 1; lvl >= 0; lvl--)
    {
      succ = pred->next[lvl];
      while (succ->key < key)
	{
	  pred = succ;
	  succ = succ->next[lvl];
	}
      if (succ->key == key)	/* at any search level */
	{
	  return false;
	}
      update[lvl] = pred;
    }

  int rand_lvl = get_rand_level(); /* do the rand_lvl outside the CS */

  GL_LOCK(set->lock);
  pred = get_lock(pred, key, 0);
  if (unlikely(pred->next[0]->key == key))
    {
      UNLOCK(ND_GET_LOCK(pred));
      GL_UNLOCK(set->lock);
      return false;
    }

  sl_node_t* n = sl_new_simple_node(key, val, rand_lvl, 0);
  LOCK(ND_GET_LOCK(n));

  n->next[0] = pred->next[0];	/* we already hold the lock for lvl 0 */
  pred->next[0] = n;
  UNLOCK(ND_GET_LOCK(pred));

  for (lvl = 1; lvl < n->toplevel; lvl++)
    {
      pred = get_lock(update[lvl], key, lvl);
      n->next[lvl] = pred->next[lvl];
      pred->next[lvl] = n;
      UNLOCK(ND_GET_LOCK(pred));
    }  
  UNLOCK(ND_GET_LOCK(n));
  GL_UNLOCK(set->lock);

  return 1;
}

sval_t
optimistic_delete(sl_intset_t *set, skey_t key)
{
  sl_node_t* update[HERLIHY_MAX_MAX_LEVEL];
  sl_node_t* succ = NULL;
  sl_node_t* pred = set->head;
  int lvl;
  for (lvl = levelmax - 1; lvl >= 0; lvl--)
    {
      succ = pred->next[lvl];
      while (succ->key < key)
	{
	  pred = succ;
	  succ = succ->next[lvl];
	}
      update[lvl] = pred;
    }

  GL_LOCK(set->lock);

  succ = pred;
  int is_garbage;
  do
    {
      succ = succ->next[0];
      if (succ->key > key)
	{
	  GL_UNLOCK(set->lock);
	  return false;
	}

      LOCK(ND_GET_LOCK(succ));
      is_garbage = (succ->key > succ->next[0]->key);
      if (is_garbage || succ->key != key)
	{
	  UNLOCK(ND_GET_LOCK(succ));
	}
      else
	{
	  break;
	}
    }
  while(true);

  for (lvl = succ->toplevel - 1; lvl >= 0; lvl--)
    {
      pred = get_lock(update[lvl], key, lvl);
      pred->next[lvl] = succ->next[lvl];
      succ->next[lvl] = pred;	/* pointer reversal! :-) */
      UNLOCK(ND_GET_LOCK(pred));
    }  

  UNLOCK(ND_GET_LOCK(succ));
  GL_UNLOCK(set->lock);
#if GC == 1
      ssmem_free(alloc, succ);
#endif

  return succ->val;
}
