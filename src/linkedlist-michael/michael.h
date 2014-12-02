/*
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
