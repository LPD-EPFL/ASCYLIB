#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>


#include "bst_howley.h"
#include "measurements.h"
#include "utils.h"
#include "ssalloc.h"

int main(int argc, char* const argv[]) {

	ssalloc_init();
	node_t* root = bst_initialize();
	printf("Initialized tree\n");
	int i;
	bool_t added;

	for ( i = 1; i < 10; i++){

		added = bst_add(i);
		printf("Added %d? %d\n", i, added==TRUE);
	}

	printf("Root right node: %d", root->right->key);
	
	for ( i = 1; i < 10; i++){

		bool_t found = bst_contains(i);
		printf("Contains %d? %d\n", i, found==FOUND);
	}

	for ( i = 1; i < 5; i++){

		bool_t found = bst_remove(i);
		printf("Removed %d? %d\n", i, found==TRUE);
	}

	for ( i = 1; i < 10; i++){

		bool_t found = bst_contains(i);
		printf("Contains %d? %d\n", i, found==FOUND);
	}
	

	return 0;
}