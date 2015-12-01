/*   
 *   File: set-optik.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: A simple OPTIK-based array map
 *   set-optik.c is part of ASCYLIB
 *
 * Copyright (c) 2015 Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>,
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

#include "set-optik.h"

RETRY_STATS_VARS;
LOCK_LOCAL_DATA;

const int version_every = 31;

sval_t
optik_map_contains(map_t* map, skey_t key)
{
  int i;
 restart:
  COMPILER_NO_REORDER(optik_t version = optik_get_version_wait(map->lock););
  for (i = 0; i < map->size; i++)
    {
      if (map->array[i].key == key)
	{
	  sval_t val = map->array[i].val;
	  if (optik_is_same_version(version, *map->lock))
	    {
	      return val;
	    }
	  goto restart;
	}
    }
  return 0;
}

int
optik_map_insert(map_t* map, skey_t key, sval_t val)
{
  NUM_RETRIES();
  optik_t version;
 restart:
  COMPILER_NO_REORDER(version = *map->lock;);
  int free_idx = -1;
  int i;
  for (i = 0; i < map->size; i++)
    {
      skey_t ck = map->array[i].key;
      if (ck == key)
	{
	  return 0;
	}
      else if (ck == 0)
	{
	  free_idx = i;
	}
    }

  if (!optik_trylock_version(map->lock, version))
    {
      DO_PAUSE();
      goto restart;
    }

  int res = 0;
  if (free_idx >= 0)
    {
      map->array[free_idx].key = key;
      map->array[free_idx].val = val;
      res = 1;
    }
  optik_unlock(map->lock);
  return res;
}

sval_t
optik_map_remove(map_t* map, skey_t key)
{
  NUM_RETRIES();
  optik_t version;
 restart:
  COMPILER_NO_REORDER(version = *map->lock;);  
  int i;
  for (i = 0; i < map->size; i++)
    {
      if (map->array[i].key == key)
	{
	  if (!optik_trylock_version(map->lock, version))
	    {
	      DO_PAUSE();
	      goto restart;
	    }
	  sval_t val = map->array[i].val;
	  map->array[i].key = 0;
	  optik_unlock(map->lock);
	  return val;
	}
    }

  return 0;
}
