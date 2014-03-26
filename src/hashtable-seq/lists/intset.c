/*
 * File:
 * Author(s):
 */

#include "intset.h"

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
