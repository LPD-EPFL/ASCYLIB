/*   
 *   File: bst-drachsler.c
 *   Author: Tudor David <tudor.david@epfl.ch>
 *   Description: Dana Drachsler, Martin Vechev, and Eran Yahav. 
 *   Practical Concurrent Binary Search Trees via Logical Ordering. PPoPP 2014.
 *   bst-drachsler.c is part of ASCYLIB
 *
 * Copyright (c) 2014 Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>,
 * 	     	      Tudor David <tudor.david@epfl.ch>
 *	      	      Distributed Programming Lab (LPD), EPFL
 *
 * ASCYLIB is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "bst-drachsler.h"

RETRY_STATS_VARS;

__thread ssmem_allocator_t* alloc;

node_t* create_node(skey_t k, sval_t value, int initializing) {
    volatile node_t* new_node;
#if GC == 1
    if (unlikely(initializing)) {
        new_node = (volatile node_t*) ssalloc_aligned(CACHE_LINE_SIZE, sizeof(node_t));
    } else {
        new_node = (volatile node_t*) ssmem_alloc(alloc, sizeof(node_t));
    }
#else 
    new_node = (volatile node_t*) ssalloc(sizeof(node_t));
#endif
    if (new_node == NULL) {
        perror("malloc in bst create node");
        exit(1);
    }
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->parent = NULL;
    new_node->succ = NULL;
    new_node->pred = NULL;
    INIT_LOCK(&(new_node->tree_lock));
    INIT_LOCK(&(new_node->succ_lock));
    new_node->key = k;
    new_node->value = value;
    new_node->mark = FALSE;
    asm volatile("" ::: "memory");
    return (node_t*) new_node;
}

node_t* initialize_tree(){
    node_t* parent = create_node(MIN_KEY, (sval_t) 0, 1); 
    node_t* root = create_node(MAX_KEY, (sval_t) 0, 1);
    root->pred = parent;
    root->succ = parent;
    root->parent = parent;
    parent->right = root;
    parent->succ = root;
    return root;
}

node_t* bst_search(skey_t k, node_t* root) {
  PARSE_TRY();

  node_t* n = root;
  node_t* child;
  skey_t curr_key;
  while (1) {
    curr_key = n->key;
    if (curr_key == k) {
      return n;
    }
    if ( curr_key < k ) {
      child = (node_t*) n->right;
    } else {
      child = (node_t*) n->left;
    }
    if ( child == NULL ) {
      return n;
    }
    n = child;
  }
}

sval_t bst_contains(skey_t k, node_t* root) {
    node_t* n = bst_search(k,root);
    while (n->key > k){
        n = (node_t*) n->pred;
    }
    while (n->key < k){
        n = (node_t*) n->succ;
    }
    if ((n->key == k) && (n->mark == FALSE)) {
        return n->value;
    }
    return 0;
}

bool_t bst_insert(skey_t k, sval_t v, node_t* root) {
    while(1) {
      UPDATE_TRY();
         node_t* node = bst_search(k, root);
        volatile node_t* p;
        if (node->key >= k) {
            p = (node_t*) node->pred;
        } else {
            p = (node_t*) node;
        }

#if DRACHSLER_RO_FAIL == 1
	 node_t* n = node;
	while (n->key > k)
	  {
	    n = (node_t*) n->pred;
	  }
	while (n->key < k)
	  {
	    n = (node_t*) n->succ;
	  }
	if ((n->key == k) && (n->mark == FALSE)) 
	  {
	    return FALSE;
	  }
#endif

        LOCK(&(p->succ_lock));
        volatile node_t* s = (node_t*) p->succ;
        if ((k > p->key) && (k <= s->key) && (p->mark == FALSE)) {
            if (s->key == k) {
                UNLOCK(&(p->succ_lock)); 
                return FALSE;
            }
            node_t* new_node = create_node(k,v,0);
            node_t* parent = choose_parent((node_t*) p, (node_t*) s, node);
            new_node->succ = s;
            new_node->pred = p;
            new_node->parent = parent;
#ifdef __tile__
            MEM_BARRIER;
#endif
            s->pred = new_node;
            p->succ = new_node;
            UNLOCK(&(p->succ_lock));
            insert_to_tree((node_t*) parent,(node_t*) new_node,(node_t*) root);
            return true;
        }
        UNLOCK(&(p->succ_lock));
    }
}

node_t* choose_parent(node_t* p, node_t* s, node_t* first_cand){
    node_t* candidate;
    if ((first_cand == p) || (first_cand == s)) {
        candidate = first_cand;
    } else {
        candidate = p;
    }
    while (1) {
        LOCK(&(candidate->tree_lock));
        if (candidate == p) {
            if (candidate->right == NULL) {
                return candidate;
            }
            UNLOCK(&(candidate->tree_lock));
            candidate = s;
        } else {
            if (candidate->left == NULL) {
                return candidate;
            }
            UNLOCK(&(candidate->tree_lock));
            candidate = p;
        }
    }
}

void insert_to_tree(node_t* parent, node_t* new_node, node_t* root) {
    new_node->parent = parent;
    if (parent->key < new_node->key) {
        parent->right = new_node;
    } else {
        parent->left = new_node;
    }

    UNLOCK(&(parent->tree_lock));
}


node_t* lock_parent(node_t* node) {
    node_t* p;
    while (1) {
        p = (node_t*) node->parent;
        LOCK(&(p->tree_lock));
        if ((node->parent == p) && (p->mark == FALSE)) {
            return p;
        }
        UNLOCK(&(p->tree_lock));
    }
}


sval_t bst_remove(skey_t k, node_t* root) {
    node_t* node;
    while (1) {
      UPDATE_TRY();
        node = bst_search(k, root);
        node_t* p;
        if (node->key >= k) {
            p = (node_t*) node->pred;
        } else {
            p = (node_t*) node;
        }

#if DRACHSLER_RO_FAIL == 1
	node_t* n = node;
	while (n->key > k)
	  {
	    n = (node_t*) n->pred;
	  }
	while (n->key < k)
	  {
	    n = (node_t*) n->succ;
	  }
	if ((n->key != k) && (n->mark == FALSE)) 
	  {
	    return FALSE;
	  }
#endif

        LOCK(&(p->succ_lock));
        node_t* s = (node_t*) p->succ;
        if ((k > p->key) && (k <= s->key) && (p->mark == FALSE)) {
            if (s->key > k) {
                UNLOCK(&(p->succ_lock));
                return 0;
            }
            LOCK(&(s->succ_lock));
            bool_t has_two_children = acquire_tree_locks(s);
            lock_parent(s);
            s->mark = TRUE;
            node_t* s_succ = (node_t*) s->succ;
            s_succ->pred = p;
            p->succ = s_succ;
            UNLOCK(&(s->succ_lock));
            UNLOCK(&(p->succ_lock));
            sval_t v = s->value;
            remove_from_tree(s, has_two_children,root);
            return v; 
        }
        UNLOCK(&(p->succ_lock));
    }
}

bool_t acquire_tree_locks(node_t* n) {
  LOCK_TRY_ONCE_CLEAR();

    while (1) {
        LOCK(&(n->tree_lock));
        node_t* left = (node_t*) n->left;
        node_t* right = (node_t*) n->right;
        //lock_parent(n);
        if ((right == NULL) || (left == NULL)) {
            return FALSE;
        } else {
            node_t* s = (node_t*) n->succ;
            int l=0;
            node_t* parent;
            node_t* sp = (node_t*) s->parent;
            if (sp != n) {
                parent = sp;
                if (!TRYLOCK(&(parent->tree_lock))) {
                    UNLOCK(&(n->tree_lock));
                    //UNLOCK(&(n->parent->tree_lock));
                    continue;
                }
                l=1;
                if ((parent != s->parent) || (parent->mark==TRUE)) {
                    UNLOCK(&(n->tree_lock));
                    UNLOCK(&(parent->tree_lock));
                    //UNLOCK(&(n->parent->tree_lock));
                    continue;
                }
            }
            if (!TRYLOCK(&(s->tree_lock))) {
                UNLOCK(&(n->tree_lock));
                //UNLOCK(&(n->parent->tree_lock));
                if (l) { 
                    UNLOCK(&(parent->tree_lock));
                }
                continue;
            }
            return TRUE;
        }
    }
}

void remove_from_tree(node_t* n, bool_t has_two_children,node_t* root) {
    node_t* child;
    node_t* parent;
    node_t* s;
    //int l=0;
    if (has_two_children == FALSE) { 
        if ( n->right == NULL) {
            child = (node_t*) n->left;
        } else {
            child = (node_t*) n->right;
        }
        parent = (node_t*) n->parent;
        update_child(parent, n, child);
    } else {
        s = (node_t*) n->succ;
        child = (node_t*) s->right;
        parent = (node_t*) s->parent;
        //if (parent != n ) l=1;
        update_child(parent, s, child);
        s->left = n->left;
        s->right = n->right;
        n->left->parent = s;
        if (n->right != NULL) {
            n->right->parent = s;
        }
        update_child((node_t*) n->parent, n, s);
        if (parent == n) {
            parent = s;
        } else {
            UNLOCK(&(s->tree_lock));
        }
        UNLOCK(&(parent->tree_lock));
    }
    UNLOCK(&(n->parent->tree_lock));
    UNLOCK(&(n->tree_lock));

#if GC == 1
    ssmem_free(alloc, n);
#endif
}

void update_child(node_t* parent, node_t* old_ch, node_t* new_ch) {
    if (parent->left == old_ch) {
        parent->left = new_ch;
    } else {
        parent->right = new_ch;
    }
    if (new_ch != NULL) {
        new_ch->parent = parent;
    }
}


uint32_t bst_size(node_t* node) {
    if (node==NULL) return 0;
    uint32_t x = 0;
    if ((node->key != MAX_KEY) && (node->key != MIN_KEY)) {
        x = 1;
    }
    return x + bst_size((node_t*) node->right) + bst_size((node_t*) node->left);
}

