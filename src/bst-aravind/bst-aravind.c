#include "bst-aravind.h"

__thread seek_record_t* seek_record;
__thread ssmem_allocator_t* alloc;

node_t* initialize_tree(){
    node_t* r;
    node_t* s;
    node_t* inf0;
    node_t* inf1;
    node_t* inf2;
    r = create_node(INF2,0,1);
    s = create_node(INF1,0,1);
    inf0 = create_node(INF0,0,1);
    inf1 = create_node(INF1,0,1);
    inf2 = create_node(INF2,0,1);
    
    asm volatile("" ::: "memory");
    r->left = s;
    r->right = inf2;
    s->right = inf1;
    s->left= inf0;
}

void bst_init_local() {
    seek_record = (seek_record_t*) malloc (sizeof(seek_record_t));
}

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
    new_node->key = k;
    new_node->value = value;
    asm volatile("" ::: "memory");
    return (node_t*) new_node;
}

seekRecord_t * bst_seek(skey_t key, node_t* node_r){
    node_t* node_s = ADDRESS(node_r->left);
    seek_record->ancestor = node_r;
    seek_record->successor = node_s; 
    seek_record->parent = node_s;
    seek_record->leaf = ADDRESS(node_s->left);

    node_t* parent_field = seek_record->parent->left;
    node_t* current_field = seek_record->leaf->left;
    node_t* current = ADDRESS(current_field);


    while (current != NULL) {
        if (!GETTAG(parent_field)) {
            seek_record->ancestor = seek_record->parent;
            seek_record->successor = seek_record->leaf;
        }
        seek_record->parent = seek_record->leaf;
        seek_record->leaf = current;

        parent_field = current_field;
        if (key < current->key) {
            current_field=current->left;
        } else {
            current_field=current->right;
        }
        current=ADDRESS(current_field);
    }
    return seek_record;
}

sval_t bst_search(skey_t key, node_t* node_r) {
   seek(key, node_r);
   if (seek_record->leaf->key == key) {
        return seek_record->leaf->value;
   } else {
        return 0;
   }
}


bool_t bst_insert(skey_t key, sval_t val, node_t* node_r) {
    while (1) {
        seek(key, node_r);
        if (seek_record->leaf->key == key) return FALSE;
        node_t* parent = seek_record->parent;
        node_t* leaf = seek_record->leaf;

        node_t** child_addr;
        if (key < paernt->key) {
           child_addr=&(parent->left); 
        } else {
            child_addr=&(parent->right);
        }
        //TODO check this
        node_t* new_internal=create_node(max(key,node_r->right->key),0); 
        node_t* new_node = create_node(key,val);
        
        node_t* result = CAS_PTR(child_addr, ADDRESS(leaf), ADDRESS(new_internal));
        if (result == ADDRESS(leaf)) {
            return TRUE;
        }
        node_t* chld = *child_addr; 
        if ( (ADDRESS(chld)==leaf) && (GETFLAG(chld) || GETTAG(chld)) ) {
            bst_cleanup(key); 
        }
    }
}

sval_t* bst_remove(skey_t key, node_t* node_r) {
    bool_t injecting = TRUE; 
    node_t* leaf;
    sval_t* val = 0;
    while (1) {
        seek(key, node_t* node_r);
        sval_t val = leaf->value;
        node_t* parent = seek_record->parent;

        node_t** child_addr;
        if (key < parent->key) {
            child_addr = &(parent->left);
        } else {
            child_addr = &(parent->right);
        }

        if (injecting == TRUE) {
            leaf = seek_record->leaf;
            if (leaf->key != key) {
                return 0;
            }
            node_t* lf = ADDRESS(leaf);
            node_t* result = CAS_PTR(child_addr, lf, FLAG(lf));
            if (result == ADDRESS(leaf)) {
                injecting = FALSE;
                bool_t done = bst_cleanup(key);
                if (done == TRUE) {
                    return val;
                }
            } else {
                node_t* chld = *child_addr;
                if ( (ADDRESS(chld) == leaf) && (GETFALG(chld) || GETTAG(chld)) ) {
                    bst_cleanup(key);
                }
            }
        } else {
            if (seek_record->leaf != leaf) {
                return val; 
            } else {
                bool_t done = bst_cleanup(key);
                if (done == TRUE) {
                    return val;
                }
            }
        }
    }
}


bool_t bst_cleanup(skey_t key) {
    node_t* ancestor = seek_record->ancestor;
    node_t* successor = seek_record->successor;
    node_t* parent = seek_record->parent;
    node_t* leaf = seek_record->leaf;

    node_t** succ_addr;
    if (key < ancestor->key) {
        succ_addr = &(ancestor->left);
    } else {
        succ_addr = &(ancestor->right);
    }

    node_t** child_adddr;
    node_t** sibling_addr;
    if (key < parent->key) {
       child_addr = &(parent->left);
       sibling_addr = &(parent->right);
    } else {
       child_addr = &(parent->right);
       sibling_addr = &(parent->left);
    }

    node_t* chld = &(child_addr);
    if (!GETFLAG(chld)) {
        sibling_addr = child_addr;
    }
retry:
    node_t* untagged = *sibling_addr;
    node_t* tagged = TAG(*untagged);
    node_t* res = CAS_PTR(sibling_addr,*sibling_add, tagged);
    if (res != untagged) {
        goto retry;
    }

    node_t* sibl = *sibling_add;
    if ( CAS_PTR(succ_addr, ADDRESS(successor), UNTAG(sibl)) == ADDRESS(successor)) {
#if GC == 1
    ssmem_free(alloc, ADDRESS(leaf));
    ssmem_free(alloc, ADDRESS(successor));
#endif
        return TRUE;
    }
    return FALSE;
}

uint32_t bst_size(node_t* node) {
    if (node == NULL) return 0; 
    if ((node->left == NULL) && (node->right == NULL)) {
       if (node->key < INF0 ) return 1;
    }
    uint32_t l = 0;
    uint32_t r = 0;
    if ( !GETMARK(node->left) && !GETTAG(node->left)) {
        l = bst_size(node->left);
    }
    if ( !GETMARK(node->right) && !GETTAG(node->right)) {
        r = bst_size(node->right);
    }
    return l+r;
}


