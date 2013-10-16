

typedef uint32_t bst_height_t; 
typedef uint32_t bst_version_t;
typedef uint32_t bst_key_t;
typedef ALIGNED(64) struct node_t node_t;

// TODO define constants (Fig 3 on page 3)

struct node_t {
	// TODO should this be volatile?
	volatile bst_height_t height;
	volatile bst_version_t version;
	bst_key_t key;
	volatile node_t* parent;
	volatile node_t* left;
	volatile node_t* right;	

	// how to include a lock? Also, lock or pointer to lock?
// #if defined(LL_GLOBAL_LOCK)
//   	volatile ptlock_t lock;
// #endif
};

