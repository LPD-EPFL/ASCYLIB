/*   
 *   File: lazy.h
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: A Lazy Concurrent List-Based Set Algorithm,
 *   S. Heller, M. Herlihy, V. Luchangco, M. Moir, W.N. Scherer III, N. Shavit
 *   p.3-16, OPODIS 2005
 *   lazy.h is part of ASCYLIB
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

#ifndef _H_LAZY_
#define _H_LAZY_
#include "linkedlist-lock.h"


#define LAZY_RO_FAIL RO_FAIL


/* linked list accesses */
extern int parse_validate(node_l_t* pred, node_l_t* curr);
sval_t parse_find(intset_l_t* set, skey_t key);
int parse_insert(intset_l_t* set, skey_t key, sval_t val);
sval_t parse_delete(intset_l_t* set, skey_t key);

#endif	/* _H_LAZY_ */
