/*
 * File:
 *   intset.c
 * Author(s):
 */

#include "bst-seq.h"

sval_t set_contains(intset_t *set, skey_t key);
int set_add(intset_t *set, skey_t key, sval_t val);
sval_t set_remove(intset_t *set, skey_t key);
