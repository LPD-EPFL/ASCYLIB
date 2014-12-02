/*   
 *   File: intset.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   intset.c is part of ASCYLIB
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

#include "bst.h"
#include "bst_tk.h"

sval_t
set_contains(intset_t* set, skey_t key)
{
  return bst_tk_find(set, key);
}

int
set_add(intset_t* set, skey_t key, sval_t val)
{  
  return bst_tk_insert(set, key, val);
}

sval_t
set_remove(intset_t* set, skey_t key)
{
  return bst_tk_delete(set, key);
}
