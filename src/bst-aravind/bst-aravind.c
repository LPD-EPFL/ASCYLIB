#include "bst-aravind.h"

__thread seek_record_t seek_record;

int perform_one_insert_window_operation(thread_data_t* data, seekRecord_t * R, long newKey){
		node_t *newInt ;
		node_t *newLeaf;
		if(data->recycledNodes.empty()){
			node_t * allocedNodeArr =(node_t *)malloc(2*sizeof(node_t));// new pointerNode_t[2];
			newInt = &allocedNodeArr[0];
			newLeaf = &allocedNodeArr[1]; 
		}
		else{
			// reuse memory of previously allocated nodes.
			newInt = data->recycledNodes.back();
			data->recycledNodes.pop_back();
			newLeaf = data->recycledNodes.back();
			data->recycledNodes.pop_back();
		}
		newLeaf->child.AO_val1 = NULL;
		newLeaf->child.AO_val2 = NULL;
		newLeaf->key = newKey;
		node_t * existLeaf = (node_t *)get_addr(R->pL);
		long existKey = R->leafKey;
		if(newKey < existKey){
			// key is to be inserted on lchild
			newInt->key = existKey;
			newInt->child.AO_val1 = create_child_word(newLeaf,0,0);			
			newInt->child.AO_val2 = create_child_word(existLeaf,0,0);
			
		}
		else{
			// key is to be inserted on rchild
			newInt->key = newKey;
			newInt->child.AO_val2 = create_child_word(newLeaf,0,0);			
			newInt->child.AO_val1 = create_child_word(existLeaf,0,0);
		}
		
		// cas to replace window
		AO_t newCasField;
		newCasField = create_child_word(newInt,UNMARK,UNFLAG);
		int result;
		if(R->isLeftL){
			result = atomic_cas_full(&R->parent->child.AO_val1, R->pL, newCasField);
		}
		else{
			result = atomic_cas_full(&R->parent->child.AO_val2, R->pL, newCasField);
		}
		if(result == 1){
			// successfully inserted.
			
			data->numInsert++;
			return 1;
		}
		else{
				// reuse data and pointer nodes
				data->recycledNodes.push_back(newInt);
				data->recycledNodes.push_back(newLeaf);
				return 0;
			}
}

int perform_one_delete_window_operation(thread_data_t* data, seekRecord_t * R, long key){
	 	// mark sibling.
	 	AO_t pS;
	 	if(R->isLeftL){
			// L is the left child of P
			mark_Node(&R->parent->child.AO_val2);
			pS = R->parent->child.AO_val2;
		}
		else{
			mark_Node(&R->parent->child.AO_val1);
			pS = R->parent->child.AO_val1;
		}
	 	AO_t newWord;
		if(is_flagged(pS)){
			newWord = create_child_word((node_t *)get_addr(pS), UNMARK, FLAG);	
		}
		else{
			newWord = create_child_word((node_t *)get_addr(pS), UNMARK, UNFLAG);
		}
		
		int result;
		if(R->isLeftUM){
			result = atomic_cas_full(&R->lum->child.AO_val1, R->lumC, newWord);
		}
		else{
			result = atomic_cas_full(&R->lum->child.AO_val2, R->lumC, newWord);
		}
		return result;	
}

seekRecord_t * bst_seek(skey_t key, node_t* node_r, node_t* node_s){
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

sval_t search(skey_t key, node_t* node_r, node_t* node_s) {
   seek(key, node_r, node_s);
   if (seek_record->leaf->key == key) {
        return seek_record->leaf->value;
   } else {
        return 0;
   }
}


bool_t bst_insert(skey_t key, sval_t val, node_t* node_r, node_t* node_s) {
    while (1) {
        seek(key, node_r, node_s);
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

sval_t* bst_remove(skey_t key, node_t* node_r, node_t* node_s) {
    mode_t mode = INJECTION; 
    node_t* leaf;
    sval_t* val = 0;
    while (1) {
        seek(key, node_t* node_r, node_t* node_s);
        sval_t val = leaf->value;
        node_t* parent = seek_record->parent;

        node_t** child_addr;
        if (key < parent->key) {
            child_addr = &(parent->left);
        } else {
            child_addr = &(parent->right);
        }

        if (mode == INJECTION) {
            leaf = seek_record->leaf;
            if (leaf->key != key) {
                return 0;
            }
            node_t* lf = ADDRESS(leaf);
            node_t* result = CAS_PTR(child_addr, lf, FLAG(lf));
            if (result == ADDRESS(leaf)) {
                mode = CLEANUP;
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
    node_t* unflagged = *sibling_addr;
    node_t* flagged = FLAG(*unflagged);
    node_t* res = CAS_PTR(sibling_addr,*sibling_add, flagged);
    if (res != unflagged) {
        goto retry;
    }

    node_t* sibl = *sibling_add;
    if ( CAS_PTR(succ_addr, ADDRESS(successor), UNTAG(sibl)) == ADDRESS(successor)) {
#if GC == 1
        //free ADDRESS(leaf)
        //free ADDRESS(successor)
#endif
        return TRUE;
    }
    return FALSE;
}


seekRecord_t * insseek(thread_data_t * data, long key, int op){

    node_t * gpar = NULL; // last node (ancestor of parent on access path) whose child pointer field is unmarked
    node_t * par = data->rootOfTree;
    node_t * leaf;
    node_t * leafchild;

    AO_t parentPointerWord = NULL; // contents in gpar
    AO_t leafPointerWord = par->child.AO_val1; // contents in par. Tree has two imaginary keys \inf_{1} and \inf_{2} which are larger than all other keys. 
    AO_t leafchildPointerWord; // contents in leaf

    bool isparLC = false; // is par the left child of gpar
    bool isleafLC = true; // is leaf the left child of par
    bool isleafchildLC; // is leafchild the left child of leaf

    leaf = (node_t *)get_addr(leafPointerWord);
    if(key < leaf->key){
        leafchildPointerWord = leaf->child.AO_val1;
        isleafchildLC = true;
    }
    else{
        leafchildPointerWord = leaf->child.AO_val2;
        isleafchildLC = false;
    }
    leafchild = (node_t *)get_addr(leafchildPointerWord);

    while(leafchild != NULL){
        if(!is_marked(leafPointerWord)){
            gpar = par;
            parentPointerWord = leafPointerWord;
            isparLC = isleafLC;
        }
        par = leaf;
        leafPointerWord = leafchildPointerWord;
        isleafLC = isleafchildLC;
        leaf = leafchild;
        if(key < leaf->key){
            leafchildPointerWord = leaf->child.AO_val1;
            isleafchildLC = true;
        }
        else{
            leafchildPointerWord = leaf->child.AO_val2;
            isleafchildLC = false;
        }	
        leafchild = (node_t *)get_addr(leafchildPointerWord);
    }
    if(key == leaf->key){
        // key matches that being inserted	
        return NULL;
    }
    seekRecord_t * R = data->sr;
    R->leafKey = leaf->key;
    R->parent = par;
    R->pL = leafPointerWord;
    R->isLeftL = isleafLC;
    R->lum = gpar;
    R->lumC = parentPointerWord;	
    R->isLeftUM = isparLC;
    return R;
}

seekRecord_t * delseek(thread_data_t * data, long key, int op){
    node_t * gpar = NULL; // last node (ancestor of parent on access path) whose child pointer field is unmarked
    node_t * par = data->rootOfTree;
    node_t * leaf;
    node_t * leafchild;

    AO_t parentPointerWord = NULL; // contents in gpar
    AO_t leafPointerWord = par->child.AO_val1; // contents in par. Tree has two imaginary keys \inf_{1} and \inf_{2} which are larger than all other keys. 
    AO_t leafchildPointerWord; // contents in leaf

    bool isparLC = false; // is par the left child of gpar
    bool isleafLC = true; // is leaf the left child of par
    bool isleafchildLC; // is leafchild the left child of leaf

    leaf = (node_t *)get_addr(leafPointerWord);
    if(key < leaf->key){
        leafchildPointerWord = leaf->child.AO_val1;
        isleafchildLC = true;

    }
    else{
        leafchildPointerWord = leaf->child.AO_val2;
        isleafchildLC = false;
    }

    leafchild = (node_t *)get_addr(leafchildPointerWord);
    while(leafchild != NULL){
        if(!is_marked(leafPointerWord)){
            gpar = par;
            parentPointerWord = leafPointerWord;
            isparLC = isleafLC;
        }
        par = leaf;
        leafPointerWord = leafchildPointerWord;
        isleafLC = isleafchildLC;

        leaf = leafchild;

        if(key < leaf->key){
            leafchildPointerWord = leaf->child.AO_val1;
            isleafchildLC = true;
        }
        else{
            leafchildPointerWord = leaf->child.AO_val2;
            isleafchildLC = false;
        }	

        leafchild = (node_t *)get_addr(leafchildPointerWord);
    }
    // op = DEL
    if(key != leaf->key){
        // key is not found in the tree.
        return NULL;
    }
    seekRecord_t * R = data->sr;
    R->leafKey = leaf->key;
    R->parent = par;
    R->pL = leafPointerWord;
    R->isLeftL = isleafLC;
    R->lum = gpar;
    R->lumC = parentPointerWord;	
    R->isLeftUM = isparLC;
    return R;
}

seekRecord_t * secondary_seek(thread_data_t * data, skey_t key, seekRecord_t * sr){
    node_t * flaggedLeaf = (node_t *)get_addr(sr->pL);
    node_t * gpar = NULL; // last node (ancestor of parent on access path) whose child pointer field is unmarked
    node_t * par = data->rootOfTree;
    node_t * leaf;
    node_t * leafchild;
    AO_t parentPointerWord = NULL; // contents in gpar
    AO_t leafPointerWord = par->child.AO_val1; // contents in par. Tree has two imaginary keys \inf_{1} and \inf_{2} which are larger than all other keys. 
    AO_t leafchildPointerWord; // contents in leaf

    bool isparLC = false; // is par the left child of gpar
    bool isleafLC = true; // is leaf the left child of par
    bool isleafchildLC; // is leafchild the left child of leaf
    leaf = (node_t *)get_addr(leafPointerWord);
    if(key < leaf->key){
        leafchildPointerWord = leaf->child.AO_val1;
        isleafchildLC = true;
    }
    else{
        leafchildPointerWord = leaf->child.AO_val2;
        isleafchildLC = false;
    }

    leafchild = (node_t *)get_addr(leafchildPointerWord);
    while(leafchild != NULL){
        if(!is_marked(leafPointerWord)){
            gpar = par;
            parentPointerWord = leafPointerWord;
            isparLC = isleafLC;
        }
        par = leaf;
        leafPointerWord = leafchildPointerWord;
        isleafLC = isleafchildLC;
        leaf = leafchild;
        if(key < leaf->key){
            leafchildPointerWord = leaf->child.AO_val1;
            isleafchildLC = true;
        }
        else{
            leafchildPointerWord = leaf->child.AO_val2;
            isleafchildLC = false;
        }	
        leafchild = (node_t *)get_addr(leafchildPointerWord);
    }

    if( !is_flagged(leafPointerWord) || (leaf != flaggedLeaf) ){
        // operation has been completed by another process.
        return NULL;		
    }
    seekRecord_t * R = data->ssr;
    R->leafKey = leaf->key;
    R->parent = par;
    R->pL = leafPointerWord;
    R->isLeftL = isleafLC;
    R->lum = gpar;
    R->lumC = parentPointerWord;	
    R->isLeftUM = isparLC;
    return R;
}

sval_t search(thread_data_t * data, skey_t key){
    node_t * cur = (node_t *)get_addr(data->rootOfTree->child.AO_val1);
    long lastKey;	
    while(cur != NULL){
        lastKey = cur->key;
        cur = (key < lastKey? (node_t *)get_addr(cur->child.AO_val1): (node_t *)get_addr(cur->child.AO_val2));
    }
    return (key == lastKey);
}

int help_conflicting_operation (thread_data_t * data, seekRecord_t * R){
    if(is_flagged(R->pL)){
        // leaf node is flagged for deletion by another process.
        //1. mark sibling of leaf node for deletion and then read its contents.
        AO_t pS;
        if(R->isLeftL){
            // L is the left child of P
            mark_Node(&R->parent->child.AO_val2);
            pS = R->parent->child.AO_val2;
        }
        else{
            mark_Node(&R->parent->child.AO_val1);
            pS = R->parent->child.AO_val1;
        }
        // 2. Execute cas on the last unmarked node to remove the 
        // if pS is flagged, propagate it. 
        AO_t newWord;
        if(is_flagged(pS)){
            newWord = create_child_word((node_t *)get_addr(pS), UNMARK, FLAG);	
        }
        else{
            newWord = create_child_word((node_t *)get_addr(pS), UNMARK, UNFLAG);
        }
        int result;
        if(R->isLeftUM){
            result = atomic_cas_full(&R->lum->child.AO_val1, R->lumC, newWord);
        }
        else{
            result = atomic_cas_full(&R->lum->child.AO_val2, R->lumC, newWord);
        }
        return result; 
    }
    else{
        // leaf node is marked for deletion by another process.
        // Note that leaf is not flagged, as it will be taken care of in the above case.
        AO_t newWord;
        if(is_flagged(R->pL)){
            newWord = create_child_word((node_t *)get_addr(R->pL), UNMARK, FLAG);
        }
        else{
            newWord = create_child_word((node_t *)get_addr(R->pL), UNMARK, UNFLAG);
        }
        int result;
        if(R->isLeftUM){
            result = atomic_cas_full(&R->lum->child.AO_val1, R->lumC, newWord);
        }
        else{
            result = atomic_cas_full(&R->lum->child.AO_val2, R->lumC, newWord);
        }
        return result; 
    }	

}

int inject(thread_data_t * data, seekRecord_t * R, int op){
    // pL is free
    //1. Flag L
    AO_t newWord = create_child_word((node_t *)get_addr(R->pL),UNMARK,FLAG);
    int result; 
    if(R->isLeftL){
        result = atomic_cas_full(&R->parent->child.AO_val1, R->pL, newWord);
    }
    else{
        result = atomic_cas_full(&R->parent->child.AO_val2, R->pL, newWord);
    }
    return result;
}

bool_t insert(thread_data_t * data, skey_t key, sval_t value){
    int injectResult;
    int fasttry = 0;	
    while(true){
        seekRecord_t * R = insseek(data, key, INS);
        fasttry++;
        if(R == NULL){
            if(fasttry == 1){
                return false;
            }
            else{
                return true;
            }	
        }
        if(!is_free(R->pL)){
            help_conflicting_operation(data, R);
            continue;
        }
        // key not present in the tree. Insert		
        injectResult = perform_one_insert_window_operation(data, R, key);
        if(injectResult == 1){
            // Operation injected and executed
            return true;
        }
    }

    // execute insert window operation.	
} 

sval_t delete_node(thread_data_t * data, skey_t key){
    int injectResult;
    while(true){
        seekRecord_t * R = delseek(data, key, DEL);
        if(R == NULL){
            return false;
        }
        // key is present in the tree. Inject operation into the tree
        if(!is_free(R->pL)){
            help_conflicting_operation(data, R);
            continue;
        }

        injectResult = inject(data, R, DEL);
        if(injectResult == 1){
            // Operation injected 
            data->numActualDelete++;
            int res = perform_one_delete_window_operation(data, R, key);
            if(res == 1){
                // operation successfully executed.
                return true;
            }
            else{
                // window transaction could not be executed.
                // perform secondary seek.
                while(true){
                    R = secondary_seek(data, key, R);
                    if(R == NULL){
                        // flagged leaf not found. Operation has been executed by some other process.
                        return false;
                    }
                    res = perform_one_delete_window_operation(data, R, key);
                    if(res == 1){
                        return true;
                    }
                }
            }
        }
        // otherwise, operation was not injected. Restart.
    }
}

