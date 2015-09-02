/*   
 *   File: lazy.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: A Lazy Concurrent List-Based Set Algorithm,
 *   S. Heller, M. Herlihy, V. Luchangco, M. Moir, W.N. Scherer III, N. Shavit
 *   p.3-16, OPODIS 2005
 *   lazy.c is part of ASCYLIB
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

#include "set-lockb.h"

RETRY_STATS_VARS;
LOCK_LOCAL_DATA;

sval_t
set_contains(intset_l_t* set, skey_t key)
{
  LOCK_A(set->lock);
  int i;
  for (i = 0; i < set->size; i++)
    {
      if (set->array[i].key == key)
	{
	  sval_t val = set->array[i].val;
	  UNLOCK_A(set->lock);
	  return val;
	}
    }

  UNLOCK_A(set->lock);
  return 0;
}

int
set_insert(intset_l_t* set, skey_t key, sval_t val)
{
  LOCK_A(set->lock);
  int free_idx = -1;
  int i;
  for (i = 0; i < set->size; i++)
    {
      skey_t ck = set->array[i].key;
      if (ck == key)
	{
	  UNLOCK_A(set->lock);
	  return 0;
	}
      else if (ck == 0)
	{
	  free_idx = i;
	}
    }

  int res = 0;
  if (free_idx >= 0)
    {
      set->array[free_idx].key = key;
      set->array[free_idx].val = key;
      res = 1;
    }
  UNLOCK_A(set->lock);
  return res;
}

sval_t
set_remove(intset_l_t* set, skey_t key)
{
  LOCK_A(set->lock);
  int i;
  for (i = 0; i < set->size; i++)
    {
      if (set->array[i].key == key)
	{
	  set->array[i].key = 0;
	  sval_t val = set->array[i].val;
	  UNLOCK_A(set->lock);
	  return val;
	}
    }

  UNLOCK_A(set->lock);
  return 0;
}
