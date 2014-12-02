/*   
 *   File: copy_on_write.h
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: Similar to Java's CopyOnWriteArrayList.
 *   http://docs.oracle.com/javase/7/docs/api/java/util/concurrent/CopyOnWriteArrayList.html
 *   copy_on_write.h is part of ASCYLIB
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

#ifndef _COPY_ON_WRITE_H_
#define _COPY_ON_WRITE_H_

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>

#include <atomic_ops.h>
#include "lock_if.h"

#include "common.h"
#include "utils.h"
#include "measurements.h"
#include "ssalloc.h"
#include "ssmem.h"

#define DEFAULT_ALTERNATE		0
#define DEFAULT_EFFECTIVE		1

#define CPY_ON_WRITE_READ_ONLY_FAIL     RO_FAIL
#define CPY_ON_WRITE_USE_MEM_RELEAS     1

static volatile int stop;
extern __thread ssmem_allocator_t* alloc;

typedef volatile struct kv
{
  skey_t key;
  sval_t val;
} kv_t;


typedef ALIGNED(CACHE_LINE_SIZE) struct array_ll
{
  size_t size;
  kv_t* kvs;
} array_ll_t;


typedef ALIGNED(CACHE_LINE_SIZE) struct copy_on_write
{
   union
   {
     struct
     {
       volatile ptlock_t* lock;
       volatile array_ll_t* array;
     };
     uint8_t padding[CACHE_LINE_SIZE];
   };
} copy_on_write_t;

copy_on_write_t* copy_on_write_new();
size_t copy_on_write_size(copy_on_write_t* set);
sval_t cpy_search(copy_on_write_t* set, skey_t key);
int cpy_insert(copy_on_write_t* set, skey_t key, sval_t val);
sval_t cpy_delete(copy_on_write_t* set, skey_t key);

#endif	/* _COPY_ON_WRITE_H_ */
