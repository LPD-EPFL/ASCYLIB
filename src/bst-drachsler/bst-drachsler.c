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

node_t* choose_parent(node_t* p, node_t* s, node_t* first_cand){
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

void insert_to_tree(node_t* parent, node_t* new_node) {
    new_node->parent = parent;
    if (parent->key < new_node->key) {
        parent->right = new_node;
        parent->right_height = 1;
    } else {
        parent->left = new_node;
        parent->left_height = 1;
    }
    //TODO: rebalance(lock_parent(parent)),parent);
}


node_t* lock_parent(node_t* node) {
    node_t* p;
    while (1) {
        p = node->parent;
        LOCK(p->tree_lock);
        if ((node->parent == p) && (p->mark == FALSE)) {
            return p;
        }
        UNLOCK(p->tree_lock);
    }
}


sval_t remove(skey_t k, node_t* root) {
    node_t* node;
    while (1) {
        node = search(k, root);
        node_t* p;
        if (node->key > k) {
            p = node->pred;
        } else {
            p = node;
        }
        LOCK(p->succ_lock);
        node_t* s = p->succ;
        if ((k > p->key) && (k <= s->key) && (p->mark == FALSE)) {
            if (s->key > k) {
                UNLOCK(p->succ_lock);
                return 0;
            }
            LOCK(s->succ_lock);
            bool_t has_two_children = acquire_tree_locks(s);
            s->mark = TRUE;
            node_t* s_succ = s->succ;
            s_succ->pred = p;
            p->succ = s_succ;
            UNLOCK(s->succ_lock);
            UNLOCK(p->succ_lock);
            sval_t v = s->val;
            remove_from_tree(s, has_two_children);
            return v; 
        }
        UNLOCK(p->succ_lock);
    }
}

bool_t acquire_tree_locks(node_t* n) {
    while (1) {
        lock(n->tree_lock);
        lock_parent(n);
        if ((n->right == NULL) || (n->left == NULL)) {
            if (n->right != NULL) {
                if(!TRYLOCK(n->right->tree_lock)) {
                    UNLOCK(n->tree_lock);
                    UNLOCK(n->parent->tree_lock);
                    continue;
                }
            } else if (n->left != NULL) {
                if(!TRYLOCK(n->left->tree_lock)) {
                    UNLOCK(n->tree_lock);
                    UNLOCK(n->parent->tree_lock);
                    continue;
                }
            }
            return FALSE;
        } else {
            node_t s = n->succ;
            if (s->parent != n) {
                parent = s->parent;
                if (!TRYLOCK(parent->tree_lock)) {
                    UNLOCK(n->tree_lock);
                    UNLOCK(n->parent->tree_lock);
                    continue;
                }
                if ((parent != s->parent) || (parent->mark==TRUE)) {
                    UNLOCK(n->tree_lock);
                    UNLOCK(n->parent->tree_lock);
                    continue;
                }
            }
            if (!TRYLOCK(s->tree_lock)) {
                UNLOCK(n->tree_lock);
                UNLOCK(n->parent->tree_lock);
                continue;
            }
            if (s->right == NULL) {
                if (!TRYLOCK(s->right->tree_lock)) {
                    UNLOCK(n->tree_lock);
                    UNLOCK(n->parent->tree_lock);
                    continue;
                }
            }
            return TRUE;
        }
    }
}

void remove_from_tree(node_t* n, bool_t has_two_children) {
    node_t* child;
    node_t* parent;
    if (has_two_children == FALSE) { 
        if ( n->right == NULL) {
            child = n->left;
        } else {
            child = n->right;
        }
        parent = n->parent;
        update_child(parent, n, child);
    } else {
        node_t* s = n->succ;
        child = s->right;
        parent = s->parent;
        update_child(parent, s, child);
        s->left = n->left;
        s->right = n->right;
        s->left_height = n->left_height;
        s->right_height = n->right_height;
        n->left->parent = s;
        if (n->right != NULL) {
            n->right->parent = s;
        }
        update_child(n->parent, n, s);
        if (parent == n) {
            parent = s;
        } else {
            UNLOCK(s->tree_lock);
        }
        UNLOCK(n->parent->tree_lock);
    }
    UNLOCK(n->tree_lock);
    //TODO: rbalance(parent,child);
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
