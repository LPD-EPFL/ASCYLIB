/*   
 *   File: alistarh_pugh.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *  	     Egeyar Bagcioglu <egeyar.bagcioglu@epfl.ch>
 *   Description: D. Alistarh, J. Kopinsky, J. Li, N. Shavit. The SprayList:
 *   A Scalable Relaxed Priority Queue. In Proceedings of the 20th ACM SIGPLAN
 *   Symposium on Principles and Practice of Parallel Programming (PPoPP 2015), 2015.
 *   alistarh_pugh.c is part of ASCYLIB
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

#include "alistarh_pugh.h"
#include "utils.h"

RETRY_STATS_VARS;

#include "latency.h"
#if LATENCY_PARSING == 1
__thread size_t lat_parsing_get = 0;
__thread size_t lat_parsing_put = 0;
__thread size_t lat_parsing_rem = 0;
__thread size_t lat_parsing_deleteMin = 0;
__thread size_t lat_parsing_cleaner = 0;
#endif	/* LATENCY_PARSING == 1 */

extern ALIGNED(CACHE_LINE_SIZE) unsigned int levelmax;

#define MAX_BACKOFF 131071
#define HERLIHY_MAX_MAX_LEVEL 64 /* covers up to 2^64 elements */

#define ALISTARH_STARTING_HEIGHT_CONSTANT	1	//K
#define ALISTARH_MAX_JUMP_CONSTANT		1	//J
#define ALISTARH_LEVELS_TO_DESCEND		1	//D

unsigned int num_threads; //p
unsigned int starting_height; //H
unsigned int max_jump_length; //L
unsigned int cleaner_percentage;

sl_node_t* last_dummy_entry;
//KEY_MIN+1 as the value is reserved for dummy entries,
//while KEY_MIN as the key respresents the head of the skiplist!

void
alistarh_init(int _num_threads, sl_intset_t* set, int padding)
{
  num_threads = _num_threads;
  starting_height = floor_log_2(num_threads);
  max_jump_length = floor_log_2(num_threads)+1;
  cleaner_percentage = (99+(num_threads/2))/num_threads;
  cleaner_percentage = cleaner_percentage>1?cleaner_percentage:1;
  
  if (padding)
  {
    int i=1, num_dummies = num_threads*floor_log_2(num_threads)/2;
    for (; i<=num_dummies; i++)
    {
      optimistic_insert(set, KEY_MIN+i, KEY_MIN+1);
    }
    last_dummy_entry = set->head;
    while (last_dummy_entry->next[0]->val==KEY_MIN+1) {
      last_dummy_entry = last_dummy_entry->next[0];
    }
  }
  else
  {
    last_dummy_entry = set->head;
  }
  return;
}

sval_t
optimistic_find(sl_intset_t *set, skey_t key)
{ 
  PARSE_TRY();
  PARSE_START_TS(0);
  sval_t val = 0;
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
	  val = succ->val;
	  break;
	}
    }

  PARSE_END_TS(0, lat_parsing_get++);
  return val;
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
  PARSE_TRY();
  UPDATE_TRY();
  PARSE_START_TS(1);
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
      if (unlikely(succ->key == key))	/* at any search level */
	{
	  return false;
	}
      update[lvl] = pred;
    }
  PARSE_END_TS(1, lat_parsing_put++);

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
#ifdef __tile__ 
      MEM_BARRIER;
#endif
  pred->next[0] = n;
  UNLOCK(ND_GET_LOCK(pred));

  for (lvl = 1; lvl < n->toplevel; lvl++)
    {
      pred = get_lock(update[lvl], key, lvl);
      n->next[lvl] = pred->next[lvl];
#ifdef __tile__
      MEM_BARRIER;
#endif
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
  PARSE_TRY();
  UPDATE_TRY();
  PARSE_START_TS(2);
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
  PARSE_END_TS(2, lat_parsing_rem++);

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
      ssmem_free(alloc, (void*) succ);
#endif

  return succ->val;
}

sval_t
alistarh_deleteMin(sl_intset_t *set)
{
  skey_t key;
  sval_t result;
  sl_node_t *next, *node;
  sl_node_t *update[HERLIHY_MAX_MAX_LEVEL];
  sl_node_t *succ, *pred;
  int i, level, continue_flag;
  
 retry:
  if (unlikely(rand_range(100) <= cleaner_percentage))
  { //become cleaner
	result = 0;
    PARSE_START_TS(4);
    node = last_dummy_entry->next[0];
    while(node->next[0]!=NULL && result==0)
    {
      if (unlikely(node->key > node->next[0]->key))
      {
        node = node->next[0];
        continue;
      }

      succ = NULL;
      pred = set->head;
      key = node->key;
      continue_flag = 0;

      for (level = levelmax - 1; level >= 0; level--)
      {
        succ = pred->next[level];
        while (succ->key < key)
	    {
	      pred = succ;
	      succ = succ->next[level];
	    }
        update[level] = pred;
      }
      PARSE_END_TS(3, lat_parsing_deleteMin++);

      GL_LOCK(set->lock);

      succ = pred;
      int is_garbage;
      do
      {
        succ = succ->next[0];
        if (succ->key > key)
	    {
	      GL_UNLOCK(set->lock);
	      continue_flag = 1;
	      break;
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
      } while(true);

      if (continue_flag)
      {
        node = node->next[0];
        continue;
      }

      for (level = succ->toplevel - 1; level >= 0; level--)
      {
        pred = get_lock(update[level], key, level);
        pred->next[level] = succ->next[level];
        succ->next[level] = pred;	// pointer reversal! :-)
        UNLOCK(ND_GET_LOCK(pred));
      }  
 
      result = node->val;

      UNLOCK(ND_GET_LOCK(succ));
      GL_UNLOCK(set->lock);
#if GC == 1
      ssmem_free(alloc, (void*) succ);
#endif
    }
    PARSE_END_TS(4, lat_parsing_cleaner++);

    return result;
  }
  else //spray & mark as deleted
  {
    UPDATE_TRY();
  
    PARSE_START_TS(3);
    result = 0;
    node = set->head;
    next = NULL;
    for(level = starting_height; level>=0; level-=ALISTARH_LEVELS_TO_DESCEND)
    {
      i = (int)rand_range(max_jump_length);
      for (; i>0; i--)
      {
        next = node->next[level];
        if (next==NULL || next->next[0]==NULL)
          break;
        node = next;
      }
    }
  
    if (unlikely(node == set->head))
      goto retry;
    
    if (unlikely(node->val == KEY_MIN+1))
      goto retry;

    if (unlikely(node->key > node->next[0]->key))
      goto retry;

    succ = NULL;
    pred = set->head;
    key = node->key;

    for (level = levelmax - 1; level >= 0; level--)
    {
      succ = pred->next[level];
      while (succ->key < key)
	{
	  pred = succ;
	  succ = succ->next[level];
	}
      update[level] = pred;
    }
    PARSE_END_TS(3, lat_parsing_deleteMin++);

    GL_LOCK(set->lock);

    succ = pred;
    int is_garbage;
    do
    {
      succ = succ->next[0];
      if (succ->key > key)
	  {
	    GL_UNLOCK(set->lock);
	    goto retry;
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

    for (level = succ->toplevel - 1; level >= 0; level--)
    {
      pred = get_lock(update[level], key, level);
      pred->next[level] = succ->next[level];
      succ->next[level] = pred;	// pointer reversal! :-)
      UNLOCK(ND_GET_LOCK(pred));
    }
    
    result = node->val;

    UNLOCK(ND_GET_LOCK(succ));
    GL_UNLOCK(set->lock);
#if GC == 1
    ssmem_free(alloc, (void*) succ);
#endif

    return result;
  }
}

skey_t
alistarh_spray(sl_intset_t *set)
{
  sl_node_t *next, *node;
  int i, level;
  
 retry:
  UPDATE_TRY();
  PARSE_START_TS(3);
  node = set->head;
  next = NULL;
  for(level = starting_height; level>=0; level-=ALISTARH_LEVELS_TO_DESCEND)
  {
    i = (int)rand_range(max_jump_length);
    for (; i>0; i--)
    {
      next = node->next[level];
      if (next==NULL || next->next[0]==NULL)
        break;
      node = next;
    }
  }
  PARSE_END_TS(3, lat_parsing_deleteMin++);
  if (unlikely(node == set->head))
    goto retry;

  if (unlikely(node->val == KEY_MIN+1))
    goto retry;

  return node->key;
}
