/*
 * File:
 *   intset.c
 * Author(s):
 *   Vincent Gramoli <vincent.gramoli@epfl.ch>
 * Description:
 *   Linked list integer set operations
 *
 * Copyright (c) 2009-2010.
 *
 * intset.c is part of Synchrobench
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

#include "bst-seq.h"
#include "seq.h"

sval_t
set_contains(intset_t* set, skey_t key)
{
  return seq_find(set, key);
}

int
set_add(intset_t* set, skey_t key, sval_t val)
{  
  return seq_insert(set, key, val);
}

sval_t
set_remove(intset_t* set, skey_t key)
{
  return seq_delete(set, key);
}
