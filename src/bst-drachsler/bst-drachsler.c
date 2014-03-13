#include "bst-drachsler.h"

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
    new_node = (volatile node_t*) ssalloc(CACHE_LINE_SIZE, sizeof(node_t));
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
    new_node->left_height = 0;
    new_node->right_height = 0;
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
    node_t* n = root;
    node_t* child;
    skey_t curr_key;
    while (1) {
        curr_key = n->key;
        if (curr_key == k) {
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
        n = child;
    }
}

sval_t bst_contains(skey_t k, node_t* root) {
    node_t* n = bst_search(k,root);
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

bool_t bst_insert(skey_t k, sval_t v, node_t* root) {
    while(1) {
        node_t* node = bst_search(k, root);
        node_t* p;
        if (node->key >= k) {
            p = node->pred;
        } else {
            p = node;
        }
        LOCK(&(p->succ_lock));
        node_t* s = p->succ;
        if ((k > p->key) && (k <= s->key) && (p->mark == FALSE)) {
            if (s->key == k) {
                UNLOCK(&(p->succ_lock)); 
                return FALSE;
            }
            node_t* new_node = create_node(k,v,0);
            node_t* parent = choose_parent(p, s, node);
            new_node->succ = s;
            new_node->pred = p;
            s->pred = new_node;
            p->succ = new_node;
            UNLOCK(&(p->succ_lock));
            insert_to_tree(parent,new_node,root);
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
        parent->right_height = 1;
    } else {
        parent->left = new_node;
        parent->left_height = 1;
    }
//    if (parent!=root) {
//        bst_rebalance(lock_parent(parent),parent,root);
//    } else {
        UNLOCK(&(parent->tree_lock));
//    }
}


node_t* lock_parent(node_t* node) {
    node_t* p;
    while (1) {
        p = node->parent;
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
        node = bst_search(k, root);
        node_t* p;
        if (node->key >= k) {
            p = node->pred;
        } else {
            p = node;
        }
        LOCK(&(p->succ_lock));
        node_t* s = p->succ;
        if ((k > p->key) && (k <= s->key) && (p->mark == FALSE)) {
            if (s->key > k) {
                UNLOCK(&(p->succ_lock));
                return 0;
            }
            LOCK(&(s->succ_lock));
            bool_t has_two_children = acquire_tree_locks(s);
            s->mark = TRUE;
            node_t* s_succ = s->succ;
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
    while (1) {
        LOCK(&(n->tree_lock));
        lock_parent(n);
        if ((n->right == NULL) || (n->left == NULL)) {
            if (n->right != NULL) {
                if(!TRYLOCK(&(n->right->tree_lock))) {
                    UNLOCK(&(n->tree_lock));
                    UNLOCK(&(n->parent->tree_lock));
                    continue;
                }
            } else if (n->left != NULL) {
                if(!TRYLOCK(&(n->left->tree_lock))) {
                    UNLOCK(&(n->tree_lock));
                    UNLOCK(&(n->parent->tree_lock));
                    continue;
                }
            }
            return FALSE;
        } else {
            node_t* s = n->succ;
            int l=0;
            node_t* parent;
            if (s->parent != n) {
                parent = s->parent;
                if (!TRYLOCK(&(parent->tree_lock))) {
                    UNLOCK(&(n->tree_lock));
                    UNLOCK(&(n->parent->tree_lock));
                    continue;
                }
                l=1;
                if ((parent != s->parent) || (parent->mark==TRUE)) {
                    UNLOCK(&(n->tree_lock));
                    UNLOCK(&(parent->tree_lock));
                    UNLOCK(&(n->parent->tree_lock));
                    continue;
                }
            }
            if (!TRYLOCK(&(s->tree_lock))) {
                UNLOCK(&(n->tree_lock));
                UNLOCK(&(n->parent->tree_lock));
                if (l) { 
                    UNLOCK(&(parent->tree_lock));
                }
                continue;
            }
            if (s->right != NULL) {
                if (!TRYLOCK(&(s->right->tree_lock))) {
                    UNLOCK(&(n->tree_lock));
                    UNLOCK(&(s->tree_lock));
                    if (l) { 
                        UNLOCK(&(parent->tree_lock));
                    }
                    UNLOCK(&(n->parent->tree_lock));
                    continue;
                }
            }
            return TRUE;
        }
    }
}

void remove_from_tree(node_t* n, bool_t has_two_children,node_t* root) {
    node_t* child;
    node_t* parent;
    bool_t violated = FALSE;
    node_t* s;
    //int l=0;
    if (has_two_children == FALSE) { 
        if ( n->right == NULL) {
            child = n->left;
        } else {
            child = n->right;
        }
        parent = n->parent;
        update_child(parent, n, child);
    } else {
        s = n->succ;
        child = s->right;
        parent = s->parent;
        //if (parent != n ) l=1;
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
        uint32_t dif = s->left_height-s->right_height;
        if ((dif>=2) || (dif<= (-2))) {
            violated = TRUE;
        }
        if (parent == n) {
            parent = s;
        } else {
            UNLOCK(&(s->tree_lock));
        }
    }
    if (child) {
       UNLOCK(&(child->tree_lock));
    }

    UNLOCK(&(parent->tree_lock));
    UNLOCK(&(n->parent->tree_lock));
    UNLOCK(&(n->tree_lock));
    /*bst_rebalance(parent,child,root);*/
    /*if (violated) {*/
        /*LOCK(&(s->tree_lock));*/
        /*uint32_t dif = s->left_height-s->right_height;*/
        /*if ((s->mark==FALSE) &&  ((dif>=2) || (dif<= (-2)))) {*/
          /*//  bool_t lr=TRUE;*/
           /*// if (dif>=2) lr=FALSE;*/
            /*bst_rebalance(s, NULL, root); */
        /*} else {*/
            /*UNLOCK(&(s->tree_lock));*/
        /*}*/
    /*}*/
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

bool_t update_height(node_t* ch, node_t* node, bool_t is_left) {
    int32_t new_h;
    if (ch == NULL) {
        new_h = 0;
    } else {
        new_h = max(ch->left_height, ch->right_height) + 1;
    }
    int32_t old_h;
    if (is_left == TRUE) {
        old_h = node->left_height;
    } else {
        old_h = node->right_height;
    }
    if (is_left == TRUE) {
        node->left_height = new_h;
    } else {
        node->right_height =new_h;
    }
    if (old_h == new_h) {
        return FALSE;
    }
    return TRUE;
}

bool_t restart(node_t* node, node_t* parent) {
    if (parent != NULL) {
        UNLOCK(&(parent->tree_lock));
    }
    while (1) {
        UNLOCK(&(node->tree_lock));
        LOCK(&(node->tree_lock));
        if (node->mark == TRUE) {
            UNLOCK(&(node->tree_lock));
            return FALSE;
        }
        int32_t bf = node->left_height - node->right_height;
        node_t* child;
        if (bf >= 2) {
            child = node->left;
        } else { 
            child = node->right;
        }
        if (child == NULL) {
            return TRUE;
        }
        if (TRYLOCK(&(child->tree_lock))) {
            return TRUE;
        }
    }
}

void bst_rotate(node_t* child, node_t* n, node_t* parent, bool_t left_rotation) {
    update_child(parent,n,child);
    n->parent = child;
    if (left_rotation == TRUE) {
        update_child(n,child,child->left);
        child->left = n;
        n->right_height = child->left_height;
        child->left_height = max(n->left_height,n->right_height) + 1;
    } else {
        update_child(n,child,child->right);
        child->right = n;
        n->left_height = child->right_height;
        child->right_height = max(n->left_height,n->right_height) + 1;
    }
}

void bst_rebalance(node_t* node, node_t* child, node_t* root) {
    if (node == root) {
       UNLOCK(&(node->tree_lock));
       if (child != NULL) {
            UNLOCK(&(child->tree_lock));
       }
       return;
    }
    while (node != root) {
        bool_t is_left = FALSE;
        if (node->left == child) {
            is_left = TRUE; 
        }
        node_t* parent = NULL;
        bool_t updated = update_height(child,node,is_left);
        int32_t bf = node->left_height - node->right_height;
        if ((updated == FALSE) && (( bf < 2) && (bf > (-2)))) {
            if (child) {
                UNLOCK(&(child->tree_lock));
            }
            UNLOCK(&(node->tree_lock));
            return;
        }
        while (( bf > 2) || (bf < (-2))) {
            if (((is_left==TRUE) && (bf <= -2)) || ((is_left==FALSE) && (bf  >= 2))) {
                if (child != NULL) {
                    UNLOCK(&(child->tree_lock));
                }
                if (is_left == TRUE) {
                    child = node->right;
                } else {
                    child = node->left;
                }
                if (is_left==TRUE) {
                    is_left = FALSE;
                } else {
                    is_left= TRUE;
                }
                if (!TRYLOCK(&(child->tree_lock))) {
                    if (restart(node,parent)==FALSE) {
                        return;
                    }
                    parent=NULL;
                    bf = node->left_height - node->right_height;
                    if (bf >= 2) {
                        child = node->left;
                    } else { 
                        child = node->right;
                    }
                    if (node->left == child) {
                        is_left = TRUE;
                    } else {
                        is_left = FALSE;
                    }
                    continue; 
                }

            }
            int32_t ch_bf = child->left_height - child->right_height;
            if (((is_left == TRUE) && (ch_bf < 0)) || ((is_left == FALSE) && (ch_bf > 0))) {
                node_t* grand_child;
                if (is_left == TRUE) {
                    grand_child = child->right;
                } else {
                    grand_child = child->left;
                }
                if (!TRYLOCK(&(grand_child->tree_lock))){
                    UNLOCK(&(child->tree_lock)); 
                    if (restart(node,parent)==FALSE) {
                        return;
                    }
                    parent=NULL;
                    bf = node->left_height - node->right_height;
                    if (bf >= 2) {
                        child = node->left;
                    } else { 
                        child = node->right;
                    }
                    if (node->left == child) {
                        is_left = TRUE;
                    } else {
                        is_left = FALSE;
                    }
                    continue; 
                }
                bst_rotate(grand_child,child,node,is_left);
                UNLOCK(&(child->tree_lock));
                child = grand_child;
            }
            if (parent == NULL) {
                parent = lock_parent(node);
            }
            bool_t not_is_left = is_left == TRUE ? FALSE : TRUE;
            bst_rotate(child, node, parent, not_is_left); 
            bf = node->left_height - node->right_height;
            if ((bf >= 2) || (bf <= (-2))) {
                UNLOCK(&(parent->tree_lock));
                parent = child;
                child = NULL;
                if (bf >= 2) {
                    is_left = FALSE;
                } else {
                    is_left = TRUE;
                }
                continue;
            }
            node_t* temp = node;
            node = child;
            child = temp;
            if (node->left == child) {
                is_left = TRUE;
            } else {
                is_left = FALSE;
            }
            bf = node->left_height - node->right_height;
        }
        if (child != NULL) {
            UNLOCK(&(child->tree_lock));
        }
        child = node;
        if (parent!=NULL) {
            node = parent;
        } else {
            node= lock_parent(node);
        }
        parent = NULL;
    }
    if (child != NULL) {
        UNLOCK(&(child->tree_lock));
    }
    UNLOCK(&(node->tree_lock));
}

uint32_t bst_size(node_t* node) {
    if (node==NULL) return 0;
    uint32_t x = 0;
    if ((node->key != MAX_KEY) && (node->key != MIN_KEY)) {
        x = 1;
    }
    return x + bst_size(node->right) + bst_size(node->left);
}

