/*   
 *   File: ssalloc.c
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   ssalloc.c is part of ASCYLIB
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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <malloc.h>

#include "ssalloc.h"
#include "measurements.h"

#define SSMEM_CACHE_LINE_SIZE 64

#if !defined(SSALLOC_USE_MALLOC)
static __thread uintptr_t ssalloc_app_mem[SSALLOC_NUM_ALLOCATORS];
static __thread size_t alloc_next[SSALLOC_NUM_ALLOCATORS] = {0};
static __thread void* ssalloc_free_list[SSALLOC_NUM_ALLOCATORS][256] = {{0}};
static __thread uint8_t ssalloc_free_cur[SSALLOC_NUM_ALLOCATORS] = {0};
static __thread uint8_t ssalloc_free_num[SSALLOC_NUM_ALLOCATORS] = {0};
#endif 

void
ssalloc_set(void* mem)
{
#if !defined(SSALLOC_USE_MALLOC)
  ssalloc_app_mem[0] = (uintptr_t) mem;
#endif
}

void
ssalloc_init()
{
#if !defined(SSALLOC_USE_MALLOC)
  int i;
  for (i = 0; i < SSALLOC_NUM_ALLOCATORS; i++)
    {
      ssalloc_app_mem[i] = (uintptr_t) memalign(SSMEM_CACHE_LINE_SIZE, SSALLOC_SIZE);
      assert((void*) ssalloc_app_mem[i] != NULL);
    }
#endif
}

void
ssalloc_offset(size_t size)
{
#if !defined(SSALLOC_USE_MALLOC)
  ssalloc_app_mem[0] += size;
#endif
}

void*
ssalloc_alloc(unsigned int allocator, size_t size)
{
  void* ret = NULL;

#if defined(SSALLOC_USE_MALLOC)
  ret = (void*) malloc(size);
#else
  if (ssalloc_free_num[allocator] > 2)
    {
      uint8_t spot = ssalloc_free_cur[allocator] - ssalloc_free_num[allocator];
      ret = ssalloc_free_list[allocator][spot];
      ssalloc_free_num[allocator]--;
    }
  else
    {
      ret = (void*) (ssalloc_app_mem[allocator] + alloc_next[allocator]);
      alloc_next[allocator] += size;
      if (alloc_next[allocator] > SSALLOC_SIZE)
	{
	  fprintf(stderr, "*** warning: allocator %2d : out of bounds alloc\n", allocator);
	}
    }
#endif
  return ret;
}

void*
ssalloc(size_t size)
{
  return ssalloc_alloc(0, size);
}

void*
ssalloc_aligned_alloc(unsigned int allocator, size_t alignement, size_t size)
{
  void* ret = NULL;

#if defined(SSALLOC_USE_MALLOC)
  ret = (void*) memalign(alignement, size);
#else
  ret = (void*) (ssalloc_app_mem[allocator] + alloc_next[allocator]);
  uintptr_t retu = (uintptr_t) ret;
  if ((retu & (alignement - 1)) != 0)
    {
      size_t offset = alignement - (retu & (alignement - 1));
      retu += offset;
      alloc_next[allocator] += offset;
      ret = (void*) retu;
    }

  alloc_next[allocator] += size;

  assert((((uintptr_t) ret) & (alignement-1)) == 0);
  if (alloc_next[allocator] > SSALLOC_SIZE)
    {
      fprintf(stderr, "*** warning: allocator %2d : out of bounds alloc\n", allocator);
    }
#endif
  return ret;
}

void*
ssalloc_aligned(size_t alignment, size_t size)
{
  return ssalloc_aligned_alloc(0, alignment, size);
}

void
ssfree_alloc(unsigned int allocator, void* ptr)
{
#if defined(SSALLOC_USE_MALLOC)
  free(ptr);
#else
  ssalloc_free_num[allocator]++;
  /* PRINT("free %3d (num_free after: %3d)", ssalloc_free_cur, ssalloc_free_num); */
  ssalloc_free_list[allocator][ssalloc_free_cur[allocator]++] = ptr;
#endif
}


void
ssfree(void* ptr)
{
  ssfree_alloc(0, ptr);
}
