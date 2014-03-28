/*
 * File:
 *   coupling.h
 * Author(s):
 */

#include "bst-seq.h"

sval_t seq_delete(intset_t* set, skey_t key);
sval_t seq_find(intset_t* set, skey_t key);
int seq_insert(intset_t* set, skey_t key, sval_t val);
