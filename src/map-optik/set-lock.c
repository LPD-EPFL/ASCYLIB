/*   
 *   File: set-lock.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   set-lock.c is part of ASCYLIB
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

#include "intset.h"
#include "utils.h"

__thread ssmem_allocator_t* alloc;

map_t*
map_new_l(size_t size)
{
  map_t* map = ssalloc_aligned(CACHE_LINE_SIZE, sizeof(map_t));
  assert(map != NULL);

  map->size = size;
  map->array = (key_val_t*) ssalloc_aligned(CACHE_LINE_SIZE, size * sizeof(key_val_t));
  assert(map->array != NULL);
  memset(map->array, 0, size * sizeof(key_val_t));

  map->lock = (volatile optik_t*) ssalloc_aligned(CACHE_LINE_SIZE, sizeof(optik_t));
  if (map->lock == NULL)
    {
      perror("malloc");
      exit(1);
    }
  optik_init(map->lock);
  MEM_BARRIER;
  return map;
}

void map_delete_l(map_t *map)
{
  ssfree(map->array);
  ssfree((void*) map->lock);
  ssfree(map);
}

int map_size_l(map_t *map)
{
  int size = 0;
  int i;
  for (i = 0; i < map->size; i++)
    {
      size += (map->array[i].key != 0);
    }
  return size;
}



	
