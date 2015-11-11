/*   
 *   File: ssmp_x86.h
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: definitions specific to x86 architectures
 *   ssmp_x86.h is part of ssmp
 *
 * The MIT License (MIT)
 *
 * Copyright (C) 2013  Vasileios Trigonakis
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef _SSMP_X86_H_
#define _SSMP_X86_H_

#define SSMP_CHUNK_SIZE 8192
#define SSMP_WAIT_TIME  66

#if !defined(PREFETCH) 
#  define PREFETCHW(x) asm volatile("prefetchw %0" :: "m" (*(unsigned long*)x)) /* write */
#  define PREFETCH(x) asm volatile("prefetch %0" :: "m" (*(unsigned long*)x)) /* read */
#  define PREFETCHNTA(x) asm volatile("prefetchnta %0" :: "m" (*(unsigned long*)x)) /* non-temporal */
#  define PREFETCHT0(x) asm volatile("prefetcht0 %0" :: "m" (*(unsigned long*)x)) /* all levels */
#  define PREFETCHT1(x) asm volatile("prefetcht1 %0" :: "m" (*(unsigned long*)x)) /* all but L1 */
#  define PREFETCHT2(x) asm volatile("prefetcht2 %0" :: "m" (*(unsigned long*)x)) /* all but L1 & L2 */
#endif	/* PREFETCH */

typedef struct ALIGNED(SSMP_CACHE_LINE_SIZE) ssmp_msg 
{
  int32_t w0;
  int32_t w1;
  int32_t w2;
  int32_t w3;
  int32_t w4;
  int32_t w5;
  int32_t w6;
  int32_t w7;
  int32_t f[6];
  union 
  {
    SSMP_FLAG_TYPE state;
    volatile uint32_t sender;
    volatile uint64_t pad;
  };
} ssmp_msg_t;

typedef struct ALIGNED(SSMP_CACHE_LINE_SIZE) ssmp_buf
{
  char data[SSMP_CACHE_LINE_SIZE - 1];
  union 
  {
    SSMP_FLAG_TYPE state;
    volatile uint8_t sender;
  };
} ssmp_buf_t;


/* barrier type */
typedef struct ALIGNED(SSMP_CACHE_LINE_SIZE)
{
  volatile uint64_t participants; /* the participants of a barrier can be given
				     either by this, as bits (0 -> no, 1 ->participate */
  int (*color)(int); /* or as a color function: if the function return 0 -> no participant, 
			1 -> participant. The color function has priority over the lluint participants*/
  volatile uint32_t ticket;
  volatile uint32_t cleared;
} ssmp_barrier_t;

/*********************************************************************************
  memory stuff
*********************************************************************************/

#  if defined(__SSE__)
#    include <xmmintrin.h>
#  else
#    define _mm_lfence() asm volatile ("lfence" : :)
#    define _mm_sfence() asm volatile ("sfence" : :)
#    define _mm_mfence() asm volatile ("mfence" : :)
#    define _mm_pause()  asm volatile ("rep; nop" : : )
#    define _mm_clflush(__A)  asm volatile("clflush %0" : "+m" (*(volatile char*)__A))
#  endif

#define PAUSE _mm_pause()

#endif	/* _SSMP_X86_H_ */
