/*   
 *   File: random.h
 *   Author: Vincent Gramoli <vincent.gramoli@sydney.edu.au>, 
 *  	     Tudor David <tudor.david@epfl.ch>
 *   Description: 
 *   random.h is part of ASCYLIB
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

#ifndef _H_RANDOM_
#define _H_RANDOM_

#include <malloc.h>
#include "measurements.h"
#include "ssalloc.h"

#define LOCAL_RAND

#if defined(LOCAL_RAND)
extern __thread unsigned long* seeds; 
#endif

#define my_random xorshf96

//fast but weak random number generator for the sparc machine
static inline uint32_t
fast_rand() 
{
  return ((getticks()&4294967295UL)>>4);
}


static inline unsigned long* 
seed_rand() 
{
  unsigned long* seeds;
  /* seeds = (unsigned long*) ssalloc_aligned(64, 64); */
  seeds = (unsigned long*) memalign(64, 64);
  seeds[0] = getticks() % 123456789;
  seeds[1] = getticks() % 362436069;
  seeds[2] = getticks() % 521288629;
  return seeds;
}

//Marsaglia's xorshf generator
static inline unsigned long
xorshf96(unsigned long* x, unsigned long* y, unsigned long* z)  //period 2^96-1
{
  unsigned long t;
  (*x) ^= (*x) << 16;
  (*x) ^= (*x) >> 5;
  (*x) ^= (*x) << 1;

  t = *x;
  (*x) = *y;
  (*y) = *z;
  (*z) = t ^ (*x) ^ (*y);

  return *z;
}

static inline long
rand_range(long r) 
{
  /* PF_START(0); */
#if defined(LOCAL_RAND)
  long v = xorshf96(seeds, seeds + 1, seeds + 2) % r;
  v++;
#else
  int m = RAND_MAX;
  long d, v = 0;
	
  do {
    d = (m > r ? r : m);
    v += 1 + (long)(d * ((double)rand()/((double)(m)+1.0)));
    r -= m;
  } while (r > 0);
#endif
  /* PF_STOP(0); */
  return v;
}

/* Re-entrant version of rand_range(r) */
static inline long
rand_range_re(unsigned int *seed, long r) 
{
  /* PF_START(0); */
#if defined(LOCAL_RAND)
  long v = xorshf96(seeds, seeds + 1, seeds + 2) % r;
  v++;
#else
  int m = RAND_MAX;
  long d, v = 0;
	
  do {
    d = (m > r ? r : m);		
    v += 1 + (long)(d * ((double)rand_r(seed)/((double)(m)+1.0)));
    r -= m;
  } while (r > 0);
#endif
  /* PF_STOP(0); */
  return v;
}


#endif
