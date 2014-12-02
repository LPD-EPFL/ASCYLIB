/*   
 *   File: bst_ellen.c
 *   Author: Tudor David <tudor.david@epfl.ch>
 *   Description: non-blocking binary search tree
 *      based on "Non-blocking Binary Search Trees"
 *      F. Ellen et al., PODC 2010
 *   bst_ellen.c is part of ASCYLIB
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

#include "bst_ellen.h"

RETRY_STATS_VARS;

__thread search_result_t * last_result;
__thread ssmem_allocator_t* alloc;

node_t* create_node(skey_t key, sval_t value, bool_t is_leaf, int initializing) {
    volatile node_t * new_node;
#if GC == 1
    if (unlikely(initializing)) {
        new_node = (volatile node_t*) ssalloc(sizeof(node_t));
    } else {
        new_node = (volatile node_t*) ssmem_alloc(alloc, sizeof(node_t));
    }
#else
    new_node = (volatile node_t*) ssalloc(sizeof(node_t));
#endif
    if (new_node==NULL) {
        perror("malloc in bst create node");
        exit(1);
    }
    new_node->leaf =is_leaf;
    new_node->key=key;
    new_node->value=value;
    new_node->update=NULL;
    new_node->right=NULL;
    new_node->left=NULL;

    asm volatile("" ::: "memory");
#ifdef __tile__
    MEM_BARRIER;
#endif
    return (node_t*) new_node;
}

node_t* bst_initialize(){
    node_t* root;
    node_t* i1;
    node_t* i2;
    root = create_node(INF2,0,FALSE,1);
    i1 = create_node(INF1,0,TRUE,1);
    i2 = create_node(INF2,0,TRUE,1);
  
    root->left = i1;
    root->right = i2;
    
    return root;
}

void bst_init_local(){
    last_result = (search_result_t*) malloc(sizeof(search_result_t));
}

search_result_t* bst_search(skey_t key, node_t* root) {
  PARSE_TRY();

    volatile search_result_t res;

    res.l = root;
    while (!(res.l->leaf)) {
        res.gp = res.p;
        res.p = res.l;
        res.gpupdate = res.pupdate;
        res.pupdate = res.p->update;
        if (key < res.l->key) {
	        res.l = (node_t*) res.p->left;
        } else {
            res.l = (node_t*) res.p->right;
        }
    
    }
    last_result->gp=res.gp;
    last_result->p=res.p;
    last_result->l=res.l;
    last_result->gpupdate=res.gpupdate;
    last_result->pupdate=res.pupdate;
    return last_result;
}


node_t* bst_find(skey_t key, node_t* root) {
    search_result_t * result = bst_search(key,root);
    if (result->l->key == key) {
      return (node_t*) result->l;
    }
    return NULL;
}

info_t* create_iinfo_t(node_t* p, node_t* ni, node_t* l){
    volatile info_t * new_info;
#if GC == 1
    new_info = (volatile info_t*) ssmem_alloc(alloc,sizeof(info_t));
#else
    new_info = (volatile info_t*) ssalloc(sizeof(info_t));
#endif
    if (new_info==NULL) {
        perror("malloc in bst create node");
        exit(1);
    }

    new_info->iinfo.p = p; 
    new_info->iinfo.new_internal = ni; 
    new_info->iinfo.l = l; 
    MEM_BARRIER;
    return (info_t*) new_info;
}

info_t* create_dinfo_t(node_t* gp, node_t* p, node_t* l, update_t u){
    volatile info_t * new_info;
#if GC == 1
    new_info = (volatile info_t*) ssmem_alloc(alloc,sizeof(info_t));
#else
    new_info = (volatile info_t*) ssalloc(sizeof(info_t));
#endif
    if (new_info==NULL) {
        perror("malloc in bst create node");
        exit(1);
    }

    new_info->dinfo.gp = gp; 
    new_info->dinfo.p = p; 
    new_info->dinfo.l = l; 
    new_info->dinfo.pupdate = u; 
    MEM_BARRIER;
    return (info_t*) new_info;
}


bool_t bst_insert(skey_t key, sval_t value,  node_t* root) {
    node_t * new_internal;
    node_t *new_sibling;

    node_t * new_node = NULL; 
   
    update_t result;

    info_t* op;
    search_result_t* search_result;

    while(1) {
      UPDATE_TRY();
        search_result = bst_search(key,root);
        if (search_result->l->key == key) {
#if GC == 1
            if (new_node!=NULL) {
                ssmem_free(alloc,new_node);
            }
#endif
            return FALSE;
        }
        if (GETFLAG(search_result->pupdate) != STATE_CLEAN) {
            bst_help(search_result->pupdate);
        } else {
            if (new_node==NULL){
                new_node = create_node(key, value, TRUE, 0); 
            }
            new_sibling = create_node(search_result->l->key, search_result->l->value, TRUE, 0);
            new_internal = create_node(max(key,search_result->l->key), 0, FALSE, 0);

           if (new_node->key < new_sibling->key) {
                new_internal->left = new_node;
                new_internal->right = new_sibling;
            } else {
                new_internal->left = new_sibling;
                new_internal->right = new_node;
            }
            op = create_iinfo_t(search_result->p, new_internal, search_result->l);
            result = CAS_PTR(&(search_result->p->update),search_result->pupdate,FLAG(op,STATE_IFLAG));
            if (result == search_result->pupdate) {
                bst_help_insert(op);
#if GC == 1
                if (UNFLAG(result)!=0) {
                    ssmem_free(alloc, (void*) UNFLAG(search_result->pupdate));
                }
#endif
                return TRUE;
            } else {
                bst_help(result);
#if GC == 1
                ssmem_free(alloc,new_sibling);
                ssmem_free(alloc,new_internal);
                ssmem_free(alloc,op);
#endif
            }
        }
    }
}

void bst_help_insert(info_t * op) {
  CLEANUP_TRY();

    int i = bst_cas_child(op->iinfo.p,op->iinfo.l,op->iinfo.new_internal);
    //iinfo_t* cl = (iinfo_t*) UNFLAG(op);
    void* UNUSED dummy = CAS_PTR(&(op->iinfo.p->update),FLAG(op,STATE_IFLAG),FLAG(op,STATE_CLEAN));
#if GC == 1
   if (i){
     info_t* uop= (info_t*) UNFLAG(op);
        ssmem_free(alloc,uop->iinfo.l);
    }
#endif
}

sval_t bst_delete(skey_t key, node_t* root) {
    update_t result;
    info_t* op;
    sval_t found_value; 

    search_result_t* search_result;

    while (1) {
      UPDATE_TRY();
        search_result = bst_search(key,root); 
        if (search_result->l->key!=key) {
            return 0;
        }
        found_value=search_result->l->value;
        if (GETFLAG(search_result->gpupdate)!=STATE_CLEAN) {
            bst_help(search_result->gpupdate);
        } else if (GETFLAG(search_result->pupdate)!=STATE_CLEAN){
            bst_help(search_result->pupdate);
        } else {
            op = create_dinfo_t(search_result->gp, search_result->p, search_result->l,search_result->pupdate);
            result = CAS_PTR(&(search_result->gp->update),search_result->gpupdate,FLAG(op,STATE_DFLAG));
            if (result == search_result->gpupdate) {
                if (bst_help_delete(op)==TRUE) {
#if GC == 1
		  ssmem_free(alloc,(void*) UNFLAG(search_result->gpupdate));
#endif
                    return found_value;
                }
            } else {
                bst_help(result);
#if GC == 1
                ssmem_free(alloc,op);
#endif
            }
        }
    }
}

bool_t bst_help_delete(info_t* op) {
  CLEANUP_TRY();

   update_t result; 
    result = CAS_PTR(&(op->dinfo.p->update), op->dinfo.pupdate, FLAG(op,STATE_MARK));
    if ((result == op->dinfo.pupdate) || (result == ((info_t*)FLAG(op,STATE_MARK)))) {
#if GC == 1
      if ((result == op->dinfo.pupdate) && ((void*) UNFLAG(result)!=NULL)) {
	  ssmem_free(alloc,(void*) UNFLAG(result));
        }
#endif
        bst_help_marked(op);
        return TRUE;
    } else {
        bst_help(result);
        void* UNUSED dummy = CAS_PTR(&(op->dinfo.gp->update), FLAG(op,STATE_DFLAG), FLAG(op,STATE_CLEAN));
        return FALSE;
    }
}


void bst_help_marked(info_t* op) {
  CLEANUP_TRY();

    node_t* other;
    if (op->dinfo.p->right == op->dinfo.l) {
        other = (node_t*) op->dinfo.p->left;
    } else {
        other = (node_t*) op->dinfo.p->right; 
    }
    int i = bst_cas_child(op->dinfo.gp,op->dinfo.p,other);
    void* UNUSED dummy = CAS_PTR(&(op->dinfo.gp->update), FLAG(op,STATE_DFLAG),FLAG(op,STATE_CLEAN));

#if GC == 1
    if (i){
      info_t* opu= (info_t*) UNFLAG(op);
        ssmem_free(alloc,opu->dinfo.l);
        ssmem_free(alloc,opu->dinfo.p);
    }
#endif
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

int bst_cas_child(node_t* parent, node_t* old, node_t* new){
    if (new->key < parent->key) {
      if (CAS_PTR(&(parent->left),old,new) == old) return 1;
      return 0;
    } else {
      if (CAS_PTR(&(parent->right),old,new) == old) return 1;
      return 0;
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
            bst_print((node_t*) node->left);
            bst_print((node_t*) node->right);
        } else {
            fprintf(stderr, "leaf\n");
        }
}

size_t bst_size_rec(node_t* node){
        if (node->leaf==FALSE) {
            return (bst_size_rec((node_t*) node->right) + bst_size_rec((node_t*) node->left));
        } else {
            return 1;
        }
}

size_t bst_size(node_t* node){
    return bst_size_rec(node)-2;
}
