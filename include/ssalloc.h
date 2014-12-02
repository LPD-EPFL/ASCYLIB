/*   
 *   File: ssalloc.h
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   ssalloc.h is part of ASCYLIB
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

#ifndef SSALLOC_H
#define SSALLOC_H

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sched.h>
#include <inttypes.h>
#include <string.h>

#if GC == 1			/* don't even allocate ssalloc if we have ssmem */
#  define SSALLOC_USE_MALLOC
#endif

#define SSALLOC_NUM_ALLOCATORS 2

#if defined(__sparc__)
#  define SSALLOC_SIZE (128LL * 1024 * 1024)
#elif defined(__tile__)
#  define SSALLOC_SIZE (100 * 1024 * 1024)
#elif defined(LAPTOP) | defined(IGORLAPTOPLINUX) | defined(OANALAPTOPLINUX)
#  define SSALLOC_SIZE (100 * 1024 * 1024)
#else
#  define SSALLOC_SIZE (1024 * 1024 * 1024)
#endif


void ssalloc_set(void* mem);
void ssalloc_init();
void ssalloc_offset(size_t size);
void* ssalloc_alloc(unsigned int allocator, size_t size);
void* ssalloc_aligned_alloc(unsigned int allocator, size_t alignment, size_t size);
void ssfree_alloc(unsigned int allocator, void* ptr);
void* ssalloc(size_t size);
void* ssalloc_aligned(size_t alignment, size_t size);

void ssfree(void* ptr);

#endif
