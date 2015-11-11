/*   
 *   File: optik.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
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

#include "linkedlist-optik.h"

RETRY_STATS_VARS;

#define CACHE_TYPE       0	/* 0 - validation with optik versions */
                                /* 1 - uses any non-deleted element */


#if CACHE_TYPE == 0

typedef struct node_cache
{
  skey_t key;
  node_l_t* node;
  optik_t version;
} node_cache_t;

__thread node_cache_t node_last = {0, 0, OPTIK_INIT};

sval_t optik_find_pes(intset_l_t *set, skey_t key);


static inline int
optik_cache_validate(intset_l_t* set, skey_t key) 
{
  if (unlikely(!node_last.key))
    {
      //      printf("--> pessimistic run\n");
      return optik_find_pes(set, key);
    }

  return (key > node_last.key &&
	  optik_is_same_version(node_last.node->lock, node_last.version));
}

static inline int
optik_cache_validate_plus(intset_l_t* set, skey_t key) 
{
  if (unlikely(!node_last.key))
    {
      //      printf("--> pessimistic run\n");
      return optik_find_pes(set, key);
    }

  return (key >= node_last.key &&
	  optik_is_same_version(node_last.node->lock, node_last.version));
}

static inline void
optik_cache_and_unlock(node_l_t* pred)
{
  node_last.key = pred->key;
  node_last.node = pred;
  node_last.version = optik_unlockv(&pred->lock);
}

sval_t
optik_find_pes(intset_l_t *set, skey_t key)
{
  node_l_t *curr, *pred;
  optik_t pred_ver = OPTIK_INIT;
	
 restart:
  PARSE_TRY();
  curr = set->head;

  do
    {
      COMPILER_NO_REORDER(optik_t curr_ver = curr->lock;);
	  
      pred = curr;
      pred_ver = curr_ver;

      curr = curr->next;
    }
  while (likely(curr->key < key));

  if ((!optik_trylock_version(&pred->lock, pred_ver)))
    {
      goto restart;
    }

  int res = (curr->key == key);

  optik_cache_and_unlock(pred);
  return res;
}

#elif CACHE_TYPE == 1

typedef struct node_cache
{
  node_l_t* node;
} node_cache_t;

__thread node_cache_t node_last = {0};

static inline int
optik_cache_validate(intset_l_t* set, skey_t key) 
{
  return (node_last.node &&
	  !optik_is_deleted(node_last.node->lock) &&
	  key > node_last.node->key);
}

static inline int
optik_cache_validate_plus(intset_l_t* set, skey_t key) 
{
  return (node_last.node &&
	  !optik_is_deleted(node_last.node->lock) &&
	  key >= node_last.node->key);
}

static inline void
optik_cache_and_unlock(node_l_t* pred)
{
  node_last.node = pred;
  optik_unlock(&pred->lock);
}

#endif	/* CACHE_TYPE */


sval_t
optik_find(intset_l_t *set, skey_t key)
{
  PARSE_TRY();
  node_l_t* curr;
  if (optik_cache_validate_plus(set, key))
    {
      NODE_CACHE_HIT();
      /* printf("++> optik_find(%zu) cache start @%zu\n", key, node_last.key); */
      curr = node_last.node;
    }
  else
    {
      curr = set->head;
    }

  while (likely(curr->key < key))
    {
      curr = curr->next;
    }

#if CACHE_TYPE == 1
  node_last.node = curr;
#endif

  sval_t res = 0;
  if (curr->key == key)
    {
      res = curr->val;
    }
  
  return res;
}

/* static int __dc = 0, __r = 0; */

int
optik_insert(intset_l_t *set, skey_t key, sval_t val)
{
  node_l_t *curr, *pred;
  optik_t pred_ver = OPTIK_INIT;
	
 restart:
  PARSE_TRY();

  if (optik_cache_validate(set, key))
    {
      NODE_CACHE_HIT();
      curr = node_last.node;
    }
  else
    {
      curr = set->head;
    }

  do
    {
      COMPILER_NO_REORDER(optik_t curr_ver = curr->lock;);
	  
      pred = curr;
      pred_ver = curr_ver;

      curr = curr->next;
    }
  while (likely(curr->key < key));

  UPDATE_TRY();

  if (curr->key == key)
    {
      return false;
    }

  node_l_t* newnode = new_node_l(key, val, curr, 0);


  if ((!optik_trylock_version(&pred->lock, pred_ver)))
    {
      node_delete_l(newnode);
      goto restart;
    }

#ifdef __tile__
  MEM_BARRIER;
#endif
  pred->next = newnode;
  optik_cache_and_unlock(pred);

  return true;
}

sval_t
optik_delete(intset_l_t *set, skey_t key)
{
  node_l_t *pred, *curr;
  optik_t pred_ver = OPTIK_INIT, curr_ver = OPTIK_INIT;

 restart:
  PARSE_TRY();

  if (optik_cache_validate(set, key))
    {
      NODE_CACHE_HIT();
#if CACHE_TYPE == 0
      curr_ver = node_last.version;
#elif CACHE_TYPE == 1
      curr_ver = node_last.node->lock;
#endif
      curr = node_last.node;
    }
  else
    {
      curr = set->head;
      curr_ver = curr->lock;
    }

  do
    {
      //      PREFETCH(curr->next);
      pred = curr;
      pred_ver = curr_ver;

      curr = curr->next;
      curr_ver = curr->lock;
    }
  while (likely(curr->key < key));

  UPDATE_TRY();

  if (curr->key != key)
    {
      return false;
    }

  if (unlikely(!optik_trylock_version(&pred->lock, pred_ver)))
    {
      goto restart;
    }

  if (unlikely(!optik_trylock_version(&curr->lock, curr_ver)))
    {
      optik_revert(&pred->lock);
      goto restart;
    }

  pred->next = curr->next;
  optik_cache_and_unlock(pred);  

  sval_t result = curr->val;
  node_delete_l(curr);
  return result;
}
