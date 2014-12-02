/*   
 *   File: oana_test.c
 *   Author: Balmau Oana <oana.balmau@epfl.ch>, 
 *  	     Zablotchi Igor <igor.zablotchi@epfl.ch>,
 *   Description: 
 *   oana_test.c is part of ASCYLIB
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

	printf("%d\n", bst_add(3, 30, root));
	printf("contains %d %d\n", 30, bst_contains(3, root));
	bst_print(root);

        printf("%d\n", bst_add(2, 20, root));
	printf("contains %d %d\n", 20, bst_contains(2, root));
        bst_print(root);


	printf("%d\n", bst_add(1, 10, root));
  	printf("contains %d %d\n", 10, bst_contains(1, root));
        bst_print(root);


	printf("%d\n", bst_add(4, 40, root));
    printf("contains %d %d\n", 40, bst_contains(4, root));
	bst_print(root);

	printf("%d\n", bst_add(2, 20, root));
    printf("contains %d %d\n", 20, bst_contains(2, root));
	bst_print(root);


	printf("%d\n", bst_add(1, 10, root));
	
	bst_print(root);

	printf("%d\n", bst_remove(1, root));
	bst_print(root);

	printf("%d\n", bst_add(7, 70, root));
	printf("%d\n", bst_add(4, 40, root));
	printf("%d\n", bst_add(8, 80, root));

	bst_print(root);

	printf("contains %d %d\n", 30, bst_contains(3, root));
	printf("%d\n", bst_remove(7, root));

	bst_print(root);
	printf("contains %d %d\n", 70, bst_contains(7, root));

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
