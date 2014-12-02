/*   
 *   File: pugh.h
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: William Pugh. 
 *   Concurrent Maintenance of Skip Lists. Technical report, 1990.
 *   pugh.h is part of ASCYLIB
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

#ifndef _PUGH_H_
#define _PUGH_H_

#include "linkedlist-lock.h"

#define PUGH_RO_FAIL RO_FAIL

/* linked list accesses */
sval_t list_search(intset_l_t* set, skey_t key);
int list_insert(intset_l_t* set, skey_t key, sval_t val);
sval_t list_delete(intset_l_t* set, skey_t key);

#endif	/* _PUGH_H_ */
