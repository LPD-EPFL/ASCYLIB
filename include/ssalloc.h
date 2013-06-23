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

/* #define SSALLOC_USE_MALLOC */

#if defined(__sparc__)
#  define SSALLOC_SIZE (10 * 1024 * 1024)
#elif defined(__tilera__)
#  define SSALLOC_SIZE (100 * 1024 * 1024)
#else
#  define SSALLOC_SIZE (1024 * 1024 * 1024)
#endif

#define SSALLOC_NUM_ALLOCATORS 3

void ssalloc_set(void* mem);
void ssalloc_init();
void ssalloc_offset(size_t size);
void* ssalloc_alloc(unsigned int alloctor, size_t size);
void ssfree_alloc(unsigned int allocator, void* ptr);
void* ssalloc(size_t size);
void ssfree(void* ptr);

#endif
