#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "ssalloc.h"
#include "lock_if.h"

#define UNLINKED 1
#define GROWING 2
#define GROW_COUNT_INCR 8		
#define GROW_COUNT_MASK 2040 //11111111000 (ff << 3)
#define SHRINKING 4
#define SHRINK_COUNT_INCR 2048 //100000000000 (1 << 11)
#define IGNORE_GROW ~(GROWING | GROW_COUNT_MASK) // ~(GROWING | GROW_COUNT_MASK)

// TODO maybe change value of RETRY to 1?
// TODO maybe make retry a pointer to a dummy value (bst_value_t)
#define RETRY (bst_value_t*)3

#define TRUE 1
#define FALSE 0

// Spin time for bst_wait_until_not_changing
#define SPIN_COUNT 100

typedef uint32_t bst_height_t; 
typedef uint64_t bst_version_t;
typedef uint32_t bst_key_t;

typedef uint32_t bst_value_t;

typedef ALIGNED(64) struct node_t node_t;
typedef uint8_t bool_t;


// TODO define constants (Fig 3 on page 3)

struct node_t {
	// TODO should this be volatile?
	volatile bst_height_t height;
	volatile bst_version_t version;
	bst_key_t key;
	bst_value_t* value;
	volatile node_t* parent;
	volatile node_t* left;
	volatile node_t* right;
	volatile ptlock_t lock;	
};

node_t* bst_initialize();
bst_value_t* bst_get(bst_key_t k);
bst_value_t* bst_attempt_get(bst_key_t k, node_t* node, bool_t is_right, bst_version_t node_v);
bst_value_t* bst_put(bst_key_t k, bst_value_t* v);
bst_value_t* bst_attempt_put(bst_key_t k, bst_value_t* v, node_t* node, bool_t is_right, bst_version_t node_v);
bst_value_t* bst_attempt_insert(bst_key_t k, bst_value_t* v, node_t* node, bool_t is_right, bst_version_t node_v);
bst_value_t* bst_attempt_update(node_t* node, bst_value_t* v);
bst_value_t* bst_remove(bst_key_t k);
// TODO what is the signature of bst_attempt_remove
bst_value_t* bst_attempt_remove(bst_key_t k, node_t* rootHolder, bool_t is_right, bst_version_t node_v);
bool_t can_unlink(node_t* node);
bst_value_t* bst_attempt_remove_node(node_t* par, node_t* n);
void bst_fix_height_and_rebalance(node_t* par);
void bst_wait_until_not_changing(node_t* n);

static inline node_t* CHILD(node_t* parent, bool_t is_right) {
	return is_right ? parent->right : parent->left;
}