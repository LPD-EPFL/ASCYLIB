/*   
 *   File: michael.h
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: similar to: Michael, M. M. (2002). High performance dynamic
 *   lock-free hash tables and list-based sets. Proceedings of the Fourteenth Annual
 *    ACM Symposium on Parallel Algorithms and Architectures 
 *   - SPAA ’02
 *   michael.h is part of ASCYLIB
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

#include "linkedlist.h"

/* ################################################################### *
 * MICHAEL's LINKED LIST
 * ################################################################### */

inline int is_marked_ref(long i);
inline long unset_mark(long i);
inline long set_mark(long i);
inline long get_unmarked_ref(long w);
inline long get_marked_ref(long w);

node_t* michael_search(intset_t *set, skey_t key, node_t** left_node);
sval_t michael_find(intset_t *set, skey_t key);
int michael_insert(intset_t *set, skey_t key, sval_t val);
sval_t michael_delete(intset_t *set, skey_t key);
int set_size(intset_t *set);
