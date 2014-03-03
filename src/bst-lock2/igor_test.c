
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#include "bst_bronson_java.h"
#include "measurements.h"
#include "utils.h"
#include "ssalloc.h"
#include "lock_if.h"

int main(int argc, char const *argv[])
{

	ssalloc_init();

	node_t* root = bst_initialize();

	printf("node size: %d\n", sizeof(node_t));
	printf("lock size: %d\n", sizeof(ptlock_t));

	bst_add(3, root);
	bst_add(2, root);
	bst_add(1, root);
	// bst_print(root);

	// bst_remove(1, root);
	// bst_print(root);

	// bst_add(7, root);
	// bst_add(4, root);
	// bst_add(8, root);
	// bst_add(0, root);
	// bst_print(root);

	// printf("contains %d %d\n", 3, bst_contains(3, root));
	// bst_remove(7,root);
	// bst_print(root);
	// printf("contains %d %d\n", 7, bst_contains(7, root));

	// bst_remove(3,root);
	// bst_print(root);
	// printf("contains %d %d\n", 3, bst_contains(3, root));

	// bst_add(1, root);
	// bst_add(2, root);
	// bst_add(3, root);
	// bst_add(4, root);
	// bst_add(5, root);
	// bst_add(6, root);
	// bst_add(7, root);
	// bst_add(8, root);
	//  bst_print(root);

	// bst_remove(7, root);
	// bst_print(root);

	
	return 0;
}