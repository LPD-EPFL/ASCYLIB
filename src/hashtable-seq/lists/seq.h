/*
 * File:
 *   coupling.h
 * Author(s):
 * Description:
 *   Hand-over-hand lock-based linked list implementation of an integer set
 *
 * Copyright (c) 2009-2010.
 *
 * coupling.h is part of HIDDEN
 * 
 * HIDDEN is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "linkedlist-seq.h"

sval_t seq_delete(intset_t* set, skey_t key);
sval_t seq_find(intset_t* set, skey_t key);
int seq_insert(intset_t* set, skey_t key, sval_t val);
