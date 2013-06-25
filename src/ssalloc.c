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

#if !defined(SSALLOC_USE_MALLOC)
static __thread void* ssalloc_app_mem[SSALLOC_NUM_ALLOCATORS];
static __thread size_t alloc_next[SSALLOC_NUM_ALLOCATORS] = {0};
static __thread void* ssalloc_free_list[SSALLOC_NUM_ALLOCATORS][256] = {{0}};
static __thread uint8_t ssalloc_free_cur[SSALLOC_NUM_ALLOCATORS] = {0};
static __thread uint8_t ssalloc_free_num[SSALLOC_NUM_ALLOCATORS] = {0};
#endif 

void
ssalloc_set(void* mem)
{
#if !defined(SSALLOC_USE_MALLOC)
  ssalloc_app_mem[0] = mem;
#endif
}

void
ssalloc_init()
{
#if !defined(SSALLOC_USE_MALLOC)
  int i;
  for (i = 0; i < SSALLOC_NUM_ALLOCATORS; i++)
    {
      ssalloc_app_mem[i] = (void*) memalign(64, SSALLOC_SIZE);
      assert(ssalloc_app_mem[i] != NULL);
    }
#endif
}

void
ssalloc_align()
{
#if !defined(SSALLOC_USE_MALLOC)
  int i;
  for (i = 0; i < SSALLOC_NUM_ALLOCATORS; i++)
    {
      while (alloc_next[i] % 64)
	{
	  alloc_next[i]++;
	}
    }
#endif
}

void
ssalloc_align_alloc(unsigned int allocator)
{
#if !defined(SSALLOC_USE_MALLOC)
  while (alloc_next[allocator] % 64)
    {
      alloc_next[allocator]++;
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

//--------------------------------------------------------------------------------------
// FUNCTION: ssmalloc
//--------------------------------------------------------------------------------------
// Allocate memory in off-chip shared memory. This is a collective call that should be
// issued by all participating cores if consistent results are required. All cores will
// allocate space that is exactly overlapping. Alternatively, determine the beginning of
// the off-chip shared memory on all cores and subsequently let just one core do all the
// allocating and freeing. It can then pass offsets to other cores who need to know what
// shared memory regions were involved.
//--------------------------------------------------------------------------------------
 // requested space
void*
ssalloc_alloc(unsigned int allocator, size_t size)
{
  /* PF_START(1); */
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
      ret = ssalloc_app_mem[allocator] + alloc_next[allocator];
      alloc_next[allocator] += size;
      if (alloc_next[allocator] > SSALLOC_SIZE)
	{
	  fprintf(stderr, "*** warning: allocator %2d : out of bounds alloc\n", allocator);
	}
    }
#endif
  /* PRINT("[lib] allocated %p [offs: %lu]", ret, ssalloc_app_addr_offs(ret)); */
  /* PF_STOP(1); */
  return ret;
}

void*
ssalloc(size_t size)
{
  return ssalloc_alloc(0, size);
}

//--------------------------------------------------------------------------------------
// FUNCTION: ssfree
//--------------------------------------------------------------------------------------
// Deallocate memory in shared memory. Also collective, see ssmalloc
//--------------------------------------------------------------------------------------
// pointer to data to be freed
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
