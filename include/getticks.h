/*   
 *   File: getticks.h
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   getticks.h is part of ASCYLIB
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

#ifndef _H_GETTICKS_
#define _H_GETTICKS_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
typedef uint64_t ticks;

#if defined(__i386__)
static inline ticks 
getticks(void) 
{
  ticks ret;

  __asm__ __volatile__("rdtsc" : "=A" (ret));
  return ret;
}
#elif defined(__x86_64__)
static inline ticks
 getticks(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}
#elif defined(__sparc__)
static inline ticks 
getticks()
{
  ticks ret = 0;
  __asm__ __volatile__ ("rd %%tick, %0" : "=r" (ret) : "0" (ret)); 
  return ret;
}
#elif defined(__tile__)
#  include <arch/cycle.h>
static inline ticks
getticks()
{
  return get_cycle_count();
}
#endif

#ifdef __cplusplus
}
#endif

#endif
