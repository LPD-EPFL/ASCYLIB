/*
 *  linkedlist.h
 *  
 *
 *  Created by Vincent Gramoli on 1/12/09.
 *  Copyright 2009 __MyCompanyName__. All rights reserved.
 *
 */

#include "harris.h"

sval_t set_contains(intset_t *set, skey_t key);
int set_add(intset_t *set, skey_t key, skey_t val);
sval_t set_remove(intset_t *set, skey_t key);

