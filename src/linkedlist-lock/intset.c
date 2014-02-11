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

#include "lazy.h"
#include "utils.h"

int
set_contains_l(intset_l_t *set, skey_t key, int transactional)
{
  int r = 0;
  if (transactional == 2) 
    {
      r = parse_find(set, key);
    }
  else 
    {
      r =  lockc_find(set, key);
    }
  return r;
}

int
set_add_l(intset_l_t *set, skey_t key, sval_t val, int transactional)
{  
  if (transactional == 2) 
    {
      return parse_insert(set, key, val);
    }
  else
    {
      return lockc_insert(set, key, val);
    }
}

int
set_remove_l(intset_l_t *set, skey_t key, int transactional)
{
  if (transactional == 2) 
    {
      return parse_delete(set, key);
    }
  else
    {
      return lockc_delete(set, key);
    }
}
