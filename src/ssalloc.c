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

#include "ssalloc.h"
#include "measurements.h"

static __thread void* ssalloc_app_mem;
static __thread size_t alloc_next = 0;
static __thread void* ssalloc_free_list[256] = {0};
static __thread uint8_t ssalloc_free_cur = 0;
static __thread uint8_t ssalloc_free_num = 0;

void
ssalloc_set(void* mem)
{
  ssalloc_app_mem = mem;
}

void
ssalloc_init()
{
  ssalloc_app_mem = (void*) malloc(SSALLOC_SIZE);
  assert(ssalloc_app_mem != NULL);
}

void
ssalloc_offset(size_t size)
{
  ssalloc_app_mem += size;
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
ssalloc(size_t size)
{
  PF_START(1);

  void* ret = NULL;
  if (ssalloc_free_num > 2)
    {
      uint8_t spot = ssalloc_free_cur - ssalloc_free_num;
      ret = ssalloc_free_list[spot];
      ssalloc_free_num--;
    }
  else
    {
      ret = ssalloc_app_mem + alloc_next;
      alloc_next += size;
      if (alloc_next > SSALLOC_SIZE)
	{
	  printf("*** warning: out of bounds alloc");
	}
    }

  /* PRINT("[lib] allocated %p [offs: %lu]", ret, ssalloc_app_addr_offs(ret)); */
  PF_STOP(1);

  return ret;
}

//--------------------------------------------------------------------------------------
// FUNCTION: ssfree
//--------------------------------------------------------------------------------------
// Deallocate memory in off-chip shared memory. Also collective, see ssmalloc
//--------------------------------------------------------------------------------------
// pointer to data to be freed
void
ssfree(void* ptr)
{
  ssalloc_free_num++;
  /* PRINT("free %3d (num_free after: %3d)", ssalloc_free_cur, ssalloc_free_num); */
  ssalloc_free_list[ssalloc_free_cur++] = ptr;
}
