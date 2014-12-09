#include "optimistic.h"
#include "utils.h"

RETRY_STATS_VARS;

#include "latency.h"
#if LATENCY_PARSING == 1
__thread size_t lat_parsing_get = 0;
__thread size_t lat_parsing_put = 0;
__thread size_t lat_parsing_rem = 0;
#endif	/* LATENCY_PARSING == 1 */

extern ALIGNED(CACHE_LINE_SIZE) unsigned int levelmax;

#define MAX_BACKOFF 131071
#define HERLIHY_MAX_MAX_LEVEL 64 /* covers up to 2^64 elements */

strval_t optimistic_find(sl_intset_t *set, strkey_t key) {
    PARSE_TRY(); PARSE_START_TS(0);
    strval_t val = { "" };
    sl_node_t* succ = NULL;
    sl_node_t* pred = set->head;
    int lvl;
    for (lvl = levelmax - 1; lvl >= 0; lvl--) {
        succ = pred->next[lvl];
        // while (succ->key < key)
        while (strkey_compare(succ->key, key) < 0) {
            pred = succ;
            succ = succ->next[lvl];
        }

        // if (succ->key == key)	/* at any search level */
        if (strkey_compare(succ->key, key) == 0) {
            val = succ->val;
            break;
        }
    }

    PARSE_END_TS(0, lat_parsing_get++);
    return val;
}

sl_node_t* optimistic_find_node(sl_intset_t *set, strkey_t key) {
    sl_node_t* res = NULL;
    sl_node_t* succ = NULL;
    sl_node_t* pred = set->head;
    int lvl;
    for (lvl = levelmax - 1; lvl >= 0; lvl--) {
        succ = pred->next[lvl];
        // while (succ->key < key)
        while (strkey_compare(succ->key, key) < 0) {
            pred = succ;
            succ = succ->next[lvl];
        }

        // if (succ->key == key)  /* at any search level */
        if (strkey_compare(succ->key, key) == 0) {
            res = succ;
            break;
        }
    }

    return res;
}

sl_node_t*
get_lock(sl_node_t* pred, strkey_t key, int lvl) {
    sl_node_t* succ = pred->next[lvl];
    //while (succ->key < key)
    while (strkey_compare(succ->key, key) < 0) {
        pred = succ;
        succ = succ->next[lvl];
    }

    LOCK(ND_GET_LOCK(pred));
    succ = pred->next[lvl];
    //while (succ->key < key)
    while (strkey_compare(succ->key, key) < 0) {
        UNLOCK(ND_GET_LOCK(pred));
        pred = succ;
        LOCK(ND_GET_LOCK(pred));
        succ = pred->next[lvl];
    }

    return pred;
}

int optimistic_insert(sl_intset_t *set, strkey_t key, strval_t val) {
    PARSE_TRY(); UPDATE_TRY(); PARSE_START_TS(1);
    sl_node_t* update[HERLIHY_MAX_MAX_LEVEL];
    sl_node_t* succ;
    sl_node_t* pred = set->head;
    int lvl;
    for (lvl = levelmax - 1; lvl >= 0; lvl--) {
        succ = pred->next[lvl];
        //while (succ->key < key)

        while (strkey_compare(succ->key, key) < 0) {
            if (strcmp(succ->key.key, key.key) == 0) {
                printf("WARNING");
            }
            pred = succ;
            succ = succ->next[lvl];
        }
        //if (unlikely(succ->key == key))	/* at any search level */
        if (unlikely(strkey_compare(succ->key, key) == 0)) {
            return false;
        }
        update[lvl] = pred;
    } PARSE_END_TS(1, lat_parsing_put++);

    int rand_lvl = get_rand_level(); /* do the rand_lvl outside the CS */

    GL_LOCK(set->lock);
    pred = get_lock(pred, key, 0);
    // if (unlikely(pred->next[0]->key == key))
    if (unlikely(strkey_compare(pred->next[0]->key, key) == 0)) {
        UNLOCK(ND_GET_LOCK(pred));
        GL_UNLOCK(set->lock);
        return false;
    }

    sl_node_t* n = sl_new_simple_node(key, val, rand_lvl, 0);
    LOCK(ND_GET_LOCK(n));

    n->next[0] = pred->next[0]; /* we already hold the lock for lvl 0 */
#ifdef __tile__ 
    MEM_BARRIER;
#endif
    pred->next[0] = n;
    UNLOCK(ND_GET_LOCK(pred));

    for (lvl = 1; lvl < n->toplevel; lvl++) {
        pred = get_lock(update[lvl], key, lvl);
        n->next[lvl] = pred->next[lvl];
#ifdef __tile__
        MEM_BARRIER;
#endif
        pred->next[lvl] = n;
        UNLOCK(ND_GET_LOCK(pred));
    }
    UNLOCK(ND_GET_LOCK(n));
    GL_UNLOCK(set->lock);

    return 1;
}

strval_t optimistic_delete(sl_intset_t *set, strkey_t key) {
    PARSE_TRY(); UPDATE_TRY(); PARSE_START_TS(2);
    sl_node_t* update[HERLIHY_MAX_MAX_LEVEL];
    sl_node_t* succ = NULL;
    sl_node_t* pred = set->head;
    int lvl;
    for (lvl = levelmax - 1; lvl >= 0; lvl--) {
        succ = pred->next[lvl];
        //while (succ->key < key)
        while (strkey_compare(succ->key, key) < 0) {
            pred = succ;
            succ = succ->next[lvl];
        }
        update[lvl] = pred;
    } PARSE_END_TS(2, lat_parsing_rem++);

    GL_LOCK(set->lock);

    succ = pred;
    int is_garbage;
    do {
        succ = succ->next[0];
        //if (succ->key > key)
        if (strkey_compare(succ->key, key) > 0) {
            GL_UNLOCK(set->lock);
            strval_t val_0 = { "" };
            return val_0;
            //return false;
        }

        LOCK(ND_GET_LOCK(succ));
        //is_garbage = (succ->key > succ->next[0]->key);
        is_garbage = (strkey_compare(succ->key, succ->next[0]->key) > 0);

        //if (is_garbage || succ->key != key)
        if (is_garbage || strkey_compare(succ->key, key) != 0) {
            UNLOCK(ND_GET_LOCK(succ));
        } else {
            break;
        }
    } while (true);

    for (lvl = succ->toplevel - 1; lvl >= 0; lvl--) {
        pred = get_lock(update[lvl], key, lvl);
        pred->next[lvl] = succ->next[lvl];
        succ->next[lvl] = pred; /* pointer reversal! :-) */
        UNLOCK(ND_GET_LOCK(pred));
    }

    UNLOCK(ND_GET_LOCK(succ));
    GL_UNLOCK(set->lock);
#if GC == 1
    ssmem_free(alloc, (void*) succ);
#endif

    return succ->val;
}

int merge(sl_intset_t *set, strkey_t key, strval_t val,
        int (*merge_operator)(strkey_t, strval_t, strval_t, strval_t*)) {

  strval_t existing_val, new_val;
  strval_t void_val = {""};
  sl_node_t *succ, *pred;
  int lvl;

retry:
    //get val associated to key
    existing_val =  void_val;
    new_val = void_val ;
    succ = NULL;
    pred = set->head;
    for (lvl = levelmax - 1; lvl >= 0; lvl--) {
        succ = pred->next[lvl];
        while (strkey_compare(succ->key, key) < 0) {
            pred = succ;
            succ = succ->next[lvl];
        }

        if (strkey_compare(succ->key, key) == 0) {
            //save succ val
            existing_val = succ->val;
            //lock and check
            LOCK(ND_GET_LOCK(succ));
            sl_node_t* check_node = optimistic_find_node(set, key);
            if (check_node == NULL || check_node != succ || strval_compare(check_node->val, existing_val) != 0) {
              UNLOCK(ND_GET_LOCK(succ));
              goto retry;
            }

            int status = merge_operator(key, existing_val, val, &new_val);
            //update val
            succ->val = new_val;

            //unlock everything
            UNLOCK(ND_GET_LOCK(succ));
            return status;
        }
    }

    merge_operator(key, void_val, val, &new_val);
    //update val
    if( optimistic_insert(set, key, new_val) == false){
      goto retry;
    }
    else {
      return true;
    }
}

strval_t* multiget(sl_intset_t *set, strkey_t* keys, size_t num_keys){
  return NULL;
}