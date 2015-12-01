/*   
 *   File: intset.c
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   intmap.c is part of ASCYLIB
 *
 * Copyright (c) 2015 Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>,
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

#include "set-optik.h"
#include "utils.h"

int
map_contains_l(map_t* map, skey_t key)
{
  return optik_map_contains(map, key);
}

int
map_add_l(map_t* map, skey_t key, sval_t val)
{  
  return optik_map_insert(map, key, val);
}

sval_t
map_remove_l(map_t* map, skey_t key)
{
  return optik_map_remove(map, key);
}
