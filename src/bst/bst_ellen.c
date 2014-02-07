#include "bst_ellen.h"

__thread search_result_t * last_result;

node_t* bst_initialize(){
    node_t* root;
    node_t* i1;
    node_t* i2;
    root = (node_t*) ssalloc(64);
    i1 = (node_t*) ssalloc(64);
    i2 = (node_t*) ssalloc(64);

    root->key=INF2;
    root->leaf=FALSE;
    i1->key=INF1;
    i1->leaf=TRUE;
    i2->key=INF2;
    i2->leaf=TRUE;
    root->update = NULL;
    i1->update=NULL;
    i2->update=NULL;
    
    root->left = i1;
    root->right = i2;
    
    return root;
}

void bst_init_local(){
    last_result = (search_result_t*) malloc(sizeof(search_result_t));
}

search_result_t* bst_search(bst_key_t key, node_t* root) {
    search_result_t * result = last_result;

    result->l = root;
    while (!(result->l->leaf)) {
        result->gp = result->p;
        result->p = result->l;
        result->gpupdate = result->pupdate;
        result->pupdate = result->p->update;
        if (key < result->l->key) {
            result->l = result->p->left;
        } else {
            result->l = result->p->right;
        }
    }
    return result;
}


node_t* bst_find(bst_key_t key, node_t* root) {
    search_result_t * result = bst_search(key,root);
    if (result->l->key == key) {
        return result->l;
    }
    return NULL;
}

bool_t bst_insert(bst_key_t key, node_t* root) {
    node_t * new_internal;
    node_t *new_sibling;

    node_t * new_node = (node_t*) ssalloc(sizeof(node_t));
    new_node->leaf = TRUE;
    new_node->key=key;
    new_node->update=NULL;
    
    update_t result;

    info_t* op;
    search_result_t* search_result;

    while(1) {
        search_result = bst_search(key,root);
        if (search_result->l->key == key) {
            return FALSE;
        }
        if (GETFLAG(search_result->pupdate) != STATE_CLEAN) {
            bst_help(search_result->pupdate);
        } else {
            new_sibling = (node_t*) ssalloc(sizeof(node_t));
            new_sibling->leaf = TRUE;
            new_sibling->key = search_result->l->key;
            new_sibling->update=NULL;

            new_internal = (node_t*) ssalloc(sizeof(node_t));
            new_internal->leaf = FALSE; 
            new_internal->update = NULL;
            new_internal->key = max(key, search_result->l->key); 
            if (new_node->key < new_sibling->key) {
                new_internal->left = new_node;
                new_internal->right = new_sibling;
            } else {
                new_internal->left = new_sibling;
                new_internal->right = new_node;
            }
            op = (info_t*) ssalloc_alloc(1,sizeof(info_t));
            op->iinfo.p = search_result->p;
            op->iinfo.new_internal = new_internal;
            op->iinfo.l =  search_result->l;
            MEM_BARRIER;
            result = CAS_PTR(&(search_result->p->update),search_result->pupdate,FLAG(op,STATE_IFLAG));
            if (result == search_result->pupdate) {
                bst_help_insert(op);
                return TRUE;
            } else {
                bst_help(result);
            }
        }
    }
}

void bst_help_insert(info_t * op) {
    bst_cas_child(op->iinfo.p,op->iinfo.l,op->iinfo.new_internal);
    //iinfo_t* cl = (iinfo_t*) UNFLAG(op);
    CAS_PTR(&(op->iinfo.p->update),FLAG(op,STATE_IFLAG),FLAG(op,STATE_CLEAN));
}

bool_t bst_delete(bst_key_t key, node_t* root) {
    update_t result;
    info_t* op;

    search_result_t* search_result;

    while (1) {
        search_result = bst_search(key,root); 
        if (search_result->l->key!=key) {
            return FALSE;
        }
        if (GETFLAG(search_result->gpupdate)!=STATE_CLEAN) {
            bst_help(search_result->gpupdate);
        } else if (GETFLAG(search_result->pupdate)!=STATE_CLEAN){
            bst_help(search_result->pupdate);
        } else {
            op = (info_t*) ssalloc_alloc(1,sizeof(info_t));
            op->dinfo.gp = search_result->gp;
            op->dinfo.p = search_result->p;
            op->dinfo.l = search_result->l;
            op->dinfo.pupdate = search_result->pupdate;
            MEM_BARRIER;
            result = CAS_PTR(&(search_result->gp->update),search_result->gpupdate,FLAG(op,STATE_DFLAG));
            if (result == search_result->gpupdate) {
                if (bst_help_delete(op)==TRUE) {
                    return TRUE;
                }
            } else {
                bst_help(result);
            }
        }
    }
}

bool_t bst_help_delete(info_t* op) {
   update_t result; 
    result = CAS_PTR(&(op->dinfo.p->update), op->dinfo.pupdate, FLAG(op,STATE_MARK));
    if ((result == op->dinfo.pupdate) || (result == ((info_t*)FLAG(op,STATE_MARK)))) {
        bst_help_marked(op);
        return TRUE;
    } else {
        bst_help(result);
        CAS_PTR(&(op->dinfo.gp->update), FLAG(op,STATE_DFLAG), FLAG(op,STATE_CLEAN));
        return FALSE;
    }
}


void bst_help_marked(info_t* op) {
    node_t* other;
    if (op->dinfo.p->right == op->dinfo.l) {
        other = op->dinfo.p->left;
    } else {
        other = op->dinfo.p->right; 
    }
    bst_cas_child(op->dinfo.gp,op->dinfo.p,other);
    CAS_PTR(&(op->dinfo.gp->update), FLAG(op,STATE_DFLAG),FLAG(op,STATE_CLEAN));
}

void bst_help(update_t u){
    if (GETFLAG(u) == STATE_IFLAG) {
        bst_help_insert((info_t*) UNFLAG(u));
    } else if (GETFLAG(u) == STATE_MARK) {
        bst_help_marked((info_t*) UNFLAG(u));
    } else if (GETFLAG(u) == STATE_DFLAG) {
       bst_help_delete((info_t*) UNFLAG(u)); 
    }
}

void bst_cas_child(node_t* parent, node_t* old, node_t* new){
    if (new->key < parent->key) {
        CAS_PTR(&(parent->left),old,new);
    } else {
        CAS_PTR(&(parent->right),old,new);
    }
}

void bst_print(node_t* node){
        fprintf(stderr, "key: %lu; ",node->key);
        if (node->update!=NULL) {
            fprintf(stderr, "update state: %lu; ", GETFLAG(node->update));
        } else {
            fprintf(stderr, "no update; ");
        }
        if (node->leaf==FALSE) {
            fprintf(stderr, "internal; left child %lu; right child %lu\n",node->left->key,node->right->key);
            bst_print(node->left);
            bst_print(node->right);
        } else {
            fprintf(stderr, "leaf\n");
        }
}

size_t bst_size_rec(node_t* node){
        if (node->leaf==FALSE) {
            return (bst_size_rec(node->right) + bst_size_rec(node->left));
        } else {
            return 1;
        }
}

size_t bst_size(node_t* node){
    return bst_size_rec(node)-2;
}
