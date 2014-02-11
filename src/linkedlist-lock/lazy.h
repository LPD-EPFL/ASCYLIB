/*
 * File:
 *   lazy.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Lazy linked list implementation of an integer set based on Heller et al. algorithm
 *   "A Lazy Concurrent List-Based Set Algorithm"
 *   S. Heller, M. Herlihy, V. Luchangco, M. Moir, W.N. Scherer III, N. Shavit
 *   p.3-16, OPODIS 2005
 *
 * Copyright (c) 2009-2010.
 *
 * lazy.c is part of Synchrobench
 * 
 * Synchrobench is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _H_LAZY_
#define _H_LAZY_

#include "coupling.h"

/* handling logical deletion flag */ 
extern inline int is_marked_ref(uintptr_t i);
extern inline uintptr_t unset_mark(uintptr_t* i);
extern inline uintptr_t set_mark(uintptr_t* i);
extern inline uintptr_t get_unmarked_ref(uintptr_t w);
extern inline uintptr_t get_marked_ref(uintptr_t w);

/* linked list accesses */
extern int parse_validate(node_l_t* pred, node_l_t* curr);
sval_t parse_find(intset_l_t* set, skey_t key);
int parse_insert(intset_l_t* set, skey_t key, sval_t val);
sval_t parse_delete(intset_l_t* set, skey_t key);

#endif	/* _H_LAZY_ */
