#include "bst-drachsler.h"

node_t* search(skey_t k, node_t* root) {
    node_t* n = root;
    node_t* child;
    skey_t curr_key;
    while (1) {
        curr_key = n->key;
        if (curr_key = k) {
            return n;
        }
        if ( curr_key < k ) {
            child = n->right;
        } else {
            child = n->left;
        }
        if ( child == NULL ) {
            return n;
        }
    }
}

sval_t contains(skey_t k, node_t* root) {
    node_t* n = search(k,root);
    while (n->key > k){
        n=n->pred;
    }
    while (n->key < k){
        n=n->succ;
    }
    //TODO: can I use a mark bit instead of a separate field?
    if ((n->key == k) && (n->mark == FALSE)) {
        return n->value;
    }
    return 0;
}

bool_t insert(skey_t k, sval_t v) {
    node_t* node = search(k);
    node_t* p;
    if (node->key > k) {
        p = node->pred;
    } else {
        p = node;
    }
    LOCK(p->succ_lock);
    node_t* s = p->succ;
    if ((k > p->key) && (k <= s->key) && (p->mark == FALSE)) {
        if (s->key == k) {
           UNLOCK(p->succ_lock); 
           return FALSE;
        }
        new_node = create_node(k,v);
        node_t* parent = choose_parent(p, s, node);
        new_node->succ = s;
        new_node->pred = p;
        s->pred = new_node;
        p->succ = new_node;
        UNLOCK(p->succ_lock);
        insert_to_tree(parent,new_node);
        return true;
    }
    UNLOCK(p->succ_lock);
}

node_t* choose parent(node_t* p, node_t* s, node_t* first_cand){
    node_t* candidate;
    if ((first_cand == p) || (first_cand == s)) {
        candidate = first_cand;
    } else {
        candidate = p;
    }
    while (1) {
        LOCK(candidate->tree_lock);
        if (candidate == p) {
            if (candidate->right == NULL) {
                return candidate;
            }
            UNLOCK(candidate->tree_lock);
            candidate = s;
        } else {
            if (candidate->left == NULL) {
                return candidate;
            }
            UNLOCK(candidate->tree_lock);
            candidate = p;
        }
    }
}
