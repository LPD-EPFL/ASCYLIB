/*   
 *   File: linkedlist-lock.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   linkedlist-lock.c is part of ASCYLIB
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

#include "intset.h"
#include "utils.h"

__thread ssmem_allocator_t* alloc;

void
map_init_l(map_t* map, size_t size)
{
  map->size = size;
  /* map->array = (key_val_t*) ssalloc(size * sizeof(key_val_t)); */
  /* assert(map->array != NULL); */
  memset(map->array, 0, size * sizeof(key_val_t));
  optik_init(&map->lock);
}

void map_delete_l(map_t *map)
{
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

void
map_print_l(map_t* map)
{
  printf("[%-10zu]: ", map->lock);
  int i;
  for (i = 0; i < map->size; i++)
    {
      printf("%-5zu ", map->array[i].key);
    }
  printf("\n");
}



      
