#ifndef _PUGH_H_
#define _PUGH_H_

#include "linkedlist-lock.h"

#define PUGH_RO_FAIL RO_FAIL

/* linked list accesses */
sval_t list_search(intset_l_t* set, skey_t key);
int list_insert(intset_l_t* set, skey_t key, sval_t val);
sval_t list_delete(intset_l_t* set, skey_t key);

#endif	/* _PUGH_H_ */
