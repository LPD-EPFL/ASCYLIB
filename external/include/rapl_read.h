/*   
 *   File: rapl_read.h
 *   Author: Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *   Description: 
 *   rapl_read.h is part of ASCYLIB
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>
 *	      	      Distributed Programming Lab (LPD), EPFL
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

#ifndef _RAPL_READ_H_
#define _RAPL_READ_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <inttypes.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

/* #include "platform_defs.h" */

/* #define RAPL_READ_ENABLE 1 */

/*********************************************************************************/
/* interface */
/*********************************************************************************/
#if RAPL_READ_ENABLE != 1

/* initialize the library */
#define RR_INIT(core)
/* initialize the library for all sockets. Called, for example, from a main thread. */
#define RR_INIT_ALL()
/* start ALL measurements. Only the responsible (for each socket) core actually does
   measurements. */
#define RR_START()			
/* stop ALL measurements and update statistics. Only the responsible (for each socket) 
   core actually does measurements. */
#define RR_STOP()				
/* start some measurements (i.e., package, pp0, and dram). Only the responsible 
   (for each socket) core actually does measurements. */
#define RR_START_SIMPLE()			
/* stop some measurements (i.e., package, pp0, and dram) and update the statistics. 
   Only the responsible  (for each socket) core actually does  measurements. */
#define RR_STOP_SIMPLE()			
/* start some measurements (i.e., package, pp0, and dram), w/o checking if the core
   is the responsible for taking the measurements. To be used, for instance, when
   a main thread takes the measurements for a socket. */
#define RR_START_UNPROTECTED()			
/* stop some measurements (i.e., package, pp0, and dram) and update the statistics,
   w/o checking if the core is the responsible for taking the measurements. */
#define RR_STOP_UNPROTECTED()			
/* start some measurements (i.e., package, pp0, and dram), w/o checking if the core
   is the responsible for taking the measurements. Takes measurements for all 
   sockets, not only the one where the threads runs. */
#define RR_START_UNPROTECTED_ALL()			
/* stop some measurements (i.e., package, pp0, and dram) and update the statistics,
   w/o checking if the core is the responsible for taking the measurements. */
#define RR_STOP_UNPROTECTED_ALL()			
/* print the current statistics with `detailed` level of details (only the responsible
   core for printing)*/
#define RR_PRINT(detailed)			
/* print the current statistics with `detailed` level of details */
#define RR_PRINT_UNPROTECTED(detailed)			
/* print the current statistics with `detailed` level of details for node nd, or 
   for all nodes if nd == RR_NODE_ALL*/
#define RR_PRINT_UNPROTECTED_NODE(nd, detailed)			
/* terminate the rapl_read library */
#define RR_TERM()				
/* generate stats and store them in s (rapl_stats_t*) */
#define RR_STATS(s)				

#else  /* RAPL_READ_ENABLE *********************************************************/

#define RR_INIT(core)				\
  if (rapl_read_init(core) < 0)			\
    {						\
      printf("[RAPL] Could not initialize\n");	\
    }						

#define RR_INIT_ALL()				\
  if (rapl_read_init_all() < 0)			\
    {						\
      printf("[RAPL] Could not initialize\n");	\
    }						

#define RR_START()				\
  rapl_read_start()

#define RR_STOP()				\
  rapl_read_stop()

#define RR_START_SIMPLE()			\
  rapl_read_start_pack_pp0();

#define RR_STOP_SIMPLE()			\
  rapl_read_stop_pack_pp0();

#define RR_START_UNPROTECTED()			\
  rapl_read_start_pack_pp0_unprotected();

#define RR_STOP_UNPROTECTED()			\
  rapl_read_stop_pack_pp0_unprotected();

#define RR_START_UNPROTECTED_ALL()		\
  rapl_read_start_pack_pp0_unprotected_all();

#define RR_STOP_UNPROTECTED_ALL()		\
  rapl_read_stop_pack_pp0_unprotected_all();

#define RR_PRINT(detailed)			\
  rapl_read_print_all_sockets(detailed, 1)

#define RR_PRINT_UNPROTECTED(ds)		\
  rapl_read_print_all_sockets(ds, 0)

#define RR_PRINT_UNPROTECTED_NODE(nd, ds)	\
  rapl_read_print_sockets(nd, ds, 0)

#define RR_TERM()				\
  rapl_read_term()

#define RR_STATS(s)				\
  rapl_read_stats(s)

#endif	/* RAPL_READ_ENABLE ***********************************************************/

#define RAPL_PRINT_NOT     -1L
#define RAPL_PRINT_POW     0L
#define RAPL_PRINT_ENE     1L
#define RAPL_PRINT_BEF_AFT 2L
#define RAPL_PRINT_ALL     3L

#define RR_NODE_ALL      -1

#define MSR_RAPL_POWER_UNIT		0x606

/*
 * Platform specific RAPL Domains.
 * Note that PP1 RAPL Domain is supported on 062A only
 * And DRAM RAPL Domain is supported on 062D only
 */
/* Package RAPL Domain */
#define MSR_PKG_RAPL_POWER_LIMIT	0x610
#define MSR_PKG_ENERGY_STATUS		0x611
#define MSR_PKG_PERF_STATUS		0x613
#define MSR_PKG_POWER_INFO		0x614

/* PP0 RAPL Domain */
#define MSR_PP0_POWER_LIMIT		0x638
#define MSR_PP0_ENERGY_STATUS		0x639
#define MSR_PP0_POLICY			0x63A
#define MSR_PP0_PERF_STATUS		0x63B

/* PP1 RAPL Domain, may reflect to uncore devices */
#define MSR_PP1_POWER_LIMIT		0x640
#define MSR_PP1_ENERGY_STATUS		0x641
#define MSR_PP1_POLICY			0x642

/* DRAM RAPL Domain */
#define MSR_DRAM_POWER_LIMIT		0x618
#define MSR_DRAM_ENERGY_STATUS		0x619
#define MSR_DRAM_PERF_STATUS		0x61B
#define MSR_DRAM_POWER_INFO		0x61C

/* RAPL UNIT BITMASK */
#define POWER_UNIT_OFFSET	0
#define POWER_UNIT_MASK		0x0F

#define ENERGY_UNIT_OFFSET	0x08
#define ENERGY_UNIT_MASK	0x1F00

#define TIME_UNIT_OFFSET	0x10
#define TIME_UNIT_MASK		0xF000

#define CPU_SANDYBRIDGE		42
#define CPU_SANDYBRIDGE_EP	45
#define CPU_IVYBRIDGE		58
#define CPU_IVYBRIDGE_EP	62
#define CPU_HASWELL		60

int open_msr(int core);
long long int read_msr(int fd, int which);
int detect_cpu(void);

int rapl_read_init(int core);
int rapl_read_init_all();
void rapl_read_start();
void rapl_read_stop();
void rapl_read_start_pack_pp0();
void rapl_read_stop_pack_pp0();
void rapl_read_start_pack_pp0_unprotected();
void rapl_read_stop_pack_pp0_unprotected();
void rapl_read_start_pack_pp0_unprotected_all();
void rapl_read_stop_pack_pp0_unprotected_all();
void rapl_read_term();
void rapl_read_print(int detailed);
void rapl_read_print_all_sockets(int detailed, int is_protected);
void rapl_read_print_sockets(int socket, int detailed, int is_protected);

typedef struct rapl_stats
{
  double duration[NUMBER_OF_SOCKETS + 1];
  double energy_package[NUMBER_OF_SOCKETS + 1];
  double energy_pp0[NUMBER_OF_SOCKETS + 1];
  double energy_pp1[NUMBER_OF_SOCKETS + 1];
  double energy_rest[NUMBER_OF_SOCKETS + 1];
  double energy_dram[NUMBER_OF_SOCKETS + 1];
  double energy_total[NUMBER_OF_SOCKETS + 1];
  double power_package[NUMBER_OF_SOCKETS + 1];
  double power_pp0[NUMBER_OF_SOCKETS + 1];
  double power_pp1[NUMBER_OF_SOCKETS + 1];
  double power_rest[NUMBER_OF_SOCKETS + 1];
  double power_dram[NUMBER_OF_SOCKETS + 1];
  double power_total[NUMBER_OF_SOCKETS + 1];
} rapl_stats_t;

void rapl_read_stats(rapl_stats_t* s);

typedef uint64_t rapl_read_ticks;

#if defined(__i386__)
  static inline rapl_read_ticks rapl_read_getticks(void) {
    rapl_read_ticks ret;

    __asm__ __volatile__("rdtsc" : "=A" (ret));
    return ret;
  }
#elif defined(__x86_64__)
  static inline rapl_read_ticks rapl_read_getticks(void)
  {
    unsigned hi, lo;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
  }
#elif defined(__sparc__)
  static inline rapl_read_ticks rapl_read_getticks(){
    rapl_read_ticks ret;
    __asm__ __volatile__ ("rd %%tick, %0" : "=r" (ret) : "0" (ret)); 
    return ret;
  }
#elif defined(__tile__)
  static inline rapl_read_ticks rapl_read_getticks(){
    return get_cycle_count();
  }

#endif

#ifdef __cplusplus
}
#endif

#endif	/* _RAPL_READ_H_ */
