ASCYLIB + OPTIK
===============

ASCYLIB (with OPTIK) is a concurrent data-structure library. It contains over 40 implementations of linked lists, hash tables, skip lists, binary search trees (BSTs), queues, priority queues, and stacks. ASCYLIB contains sequential, lock-based, and lock-free implementations for each data structure.

ASCYLIB works on x86, SPARC, and Tilera architectures and contains tests to evaluate the throughput, latency, latency distribution, and energy efficiency of the included data structures.

OPTIK is a new design pattern for easily implementing fast and scalable concurrent data structures. We have merged several concurrent data structures developed with OPTIK in ASCYLIB. More details can be found here: http://lpd.epfl.ch/site/optik.

* Website             : http://lpd.epfl.ch/site/ascylib - http://lpd.epfl.ch/site/optik
* Authors             : Vasileios Trigonakis <github@trigonakis.com>,
                        Tudor David <repos@tudordavid.com> 
* Related Publications:
  * [*Optimistic Concurrency with OPTIK*](https://dl.acm.org/citation.cfm?id=2851146),  
    Rachid Guerraoui, Vasileios Trigonakis (alphabetical order),  
  PPoPP 2016
  * [*Asynchronized Concurrency: The Secret to Scaling Concurrent Search Data Structures*](https://dl.acm.org/citation.cfm?id=2694359),  
  Tudor David, Rachid Guerraoui, Vasileios Trigonakis (alphabetical order),  
  ASPLOS 2015


Algorithms
----------

The following table contains the algorithms (and various implementations of some algorithms) included in ASCYLIB:

| # |    Name   |  Progress  |  Year | Referece |
|:-:|-----------|:-----:|:-----:|:-----:|
|| **Array Maps** ||||
|1| [Java's CopyOnWrite array map](./src/linkedlist-copy/) |	lock-based | 2004 | [[ORACLE+04]](#ORACLE+04) |
|2| [Global-lock array map](./src/map-lock/) |	lock-based | 2016 | [[GT+16]](#GT+16) |
|3| [OPTIK global-lock array map](./src/map-optik/) |	lock-based | 2016 | [[GT+16]](#GT+16) |
|| **Linked lists** ||||
|4| [Sequential linked list](./src/linkedlist-seq/) |	sequential | | |
|5| [Hand-over-hand locking linked list](./src/linkedlist-coupling/) |	lock-based | | [[HS+12]](#HS+12) |
|6| [Pugh's linked list](./src/linkedlist-pugh/) |	lock-based | 1990 | [[P+90]](#P+90) |
|7| [Harris linked list](./src/linkedlist-harris/) |	lock-free | 2001 | [[H+01]](#H+01) |
|8| [Michael linked list](./src/linkedlist-michael/) |	lock-free | 2002 | [[M+02]](#M+02) |
|9| [Lazy linked list](./src/linkedlist-lazy/) |	lock-based | 2006 | [[HHL+06]](#HHL+06) |
|10| [Harris linked list with ASCY](./src/linkedlist-harris_opt/) |	lock-free | 2015 | [[DGT+15]](#DGT+15) |
|11| [Global-lock linked list with wait-free search](./src/linkedlist-gl_opt/) |	lock-based | 2016 | [[GT+16]](#GT+16) |
|12| [OPTIK global-lock linked list](./src/linkedlist-optik_gl/) |	lock-based | 2016 | [[GT+16]](#GT+16) |
|13| [OPTIK fine-grained linked list](./src/linkedlist-optik/) |	lock-based | 2016 | [[GT+16]](#GT+16) |
|14| [OPTIK fine-grained linked list with cache](./src/linkedlist-optik_cache/) |	lock-based | 2016 | [[GT+16]](#GT+16) |
|| **Hash Tables** ||||
|15| [Sequential hash table](./src/hashtable-seq/) |	sequential | | |
|16| [Sequential hash table - less pointer indirections ](./src/hashtable-seq2/) |	sequential | | |
|17| [Hash table using hand-over-hand-locking lists](./src/hashtable-coupling/) |	lock-based | | [[HS+12]](#HS+12) |
|18| [Hash table using Pugh's list](./src/hashtable-pugh/) |	lock-based | 1990 | [[P+90]](#P+90) |
|19| [Hash table using Harris' list](./src/hashtable-harris/) |	lock-free | 2001 | [[H+01]](#H+01) |
|20| [Java's ConcurrentHashMap](./src/hashtable-java/) |	lock-based | 2003 | [[L+03]](#L+03) |
|21| [Hash table using Java's CopyOnWrite array map](./src/hashtable-copy/) |	lock-based | 2004 | [[ORACLE+04]](#ORACLE+04) |
|22| [Intel's TBB hash table](./src/hashtable-tbb/) |	lock-based | 2006 | [[INTEL+06]](#INTEL+06) |
|23| [Hash table using lazy list](./src/hashtable-lazy/) |	lock-based | 2006 | [[HHL+06]](#HHL+06) |
|24| [URCU hash table](./src/hashtable-rcu/) |	lock-free | 2012 | [[DMS+12]](#DMS+12) |
|25| [Java's ConcurrentHashMap with OPTIK](./src/hashtable-java_optik/) |	lock-based | 2016 | [[GT+16]](#GT+16) |
|26| [Hash table using fine-grained OPTIK list](./src/hashtable-optik0/) |	lock-based | 2016 | [[GT+16]](#GT+16) |
|27| [Hash table using global-lock OPTIK list](./src/hashtable-optik1/) |	lock-based | 2016 | [[GT+16]](#GT+16) |
|28| [Hash table using OPTIK array map](./src/hashtable-map_optik/) |	lock-based | 2016 | [[GT+16]](#GT+16) |
|| **Skip Lists** ||||
|29| [Sequential skip list](./src/skiplist-seq/) |	sequential | | |
|30| [Pugh skip list](./src/skiplist-pugh/) |	lock-based | 1990 | [[P+90]](#P+90) |
|31| [Fraser skip list](./src/skiplist-fraser/) |	lock-free | 2003 | [[F+03]](#F+03) |
|32| [Herlihy et al. skip list](./src/skiplist-herlihy_lb/) |	lock-based | 2007 | [[HLL+07]](#HLL+07) |
|33| [Fraser skip list with Herlihy's optimization](./src/skiplist-herlihy_lf/) |	lock-free | 2011 | [[HLS+11]](#HLS+11) |
|34| [Herlihy's skip list with OPTIK](./src/skiplist-optik2/) |	lock-based | 2016 | [[GT+16]](#GT+16) |
|35| [OPTIK skip list using trylocks (*default OPTIK skip list*)](./src/skiplist-optik1/) |	lock-based | 2016 | [[GT+16]](#GT+16) |
|36| [OPTIK skip list lock-version](./src/skiplist-optik/) |	lock-based | 2016 | [[GT+16]](#GT+16) |
|| **Binary Search Trees (BSTs)** ||||
|37| [Sequential external binary search tree](./src/bst-seq_external/) |	sequential | | |
|38| [Sequential internal binary search tree](./src/bst-seq_internal/) |	sequential | | |
|39| [Bronson et al. binary search tree](./src/bst-bronson/) |	lock-based | 2010 | [[BCH+10]](#BCH+10) |
|40| [Ellen et al. binary search tree](./src/bst-ellen/) |	lock-free | 2010 | [[EFR+10]](#EFR+10) |
|41| [Howley and Jones binary search tree](./src/bst-howley/) |	lock-free | 2012 | [[HJ+12]](#HJ+12) |
|42| [Natarajan and Mittal binary search tree](./src/bst-aravind/) |	lock-free | 2014 | [[NM+14]](#NM+14) |
|43| [Drachsler et al. binary search tree](./src/bst-drachsler/) |	lock-based | 2014 | [[DVY+14]](#DVY+14) |
|44| [BST-TK binary search tree](./src/bst-tk/) |	lock-based | 2015 | [[DGT+15]](#DGT+15) |
|| **Queues** ||||
|45| [Michael and Scott (MS) lock-based queue](./src/queue-ms_lb/) |	lock-based | 1996 | [[MS+96]](#MS+96) |
|46| [Michael and Scott (MS) lock-free queue](./src/queue-ms_lf/) |	lock-free | 1996 | [[MS+96]](#MS+96) |
|47| [Michael and Scott (MS) hybrid queue](./src/queue-ms_hybrid/) |	lock-based | 1996 | [[MS+96]](#MS+96) |
|48| [MS queue with OPTIK lock-version](./src/queue-optik0/) |	lock-based | 2016 | [[GT+16]](#GT+16) |
|49| [MS queue with OPTIK trylock-version](./src/queue-optik1/) |	lock-based | 2016 | [[GT+16]](#GT+16) |
|50| [MS queue with OPTIK trylock-version](./src/queue-optik2/) |	lock-based | 2016 | [[GT+16]](#GT+16) |
|51| [MS queue with OPTIK and victim queue](./src/queue-optik3/) |	lock-based | 2016 | [[GT+16]](#GT+16) |
|| **Priority Queues** ||||
|52| [Lotan and Shavit priority queue](./src/priorityqueue-lotanshavit_lf/) |	lock-free | 2000 | [[LS+00]](#LS+00) |
|53| [Alistarh et al. priority queue based on Fraser's skip list](./src/priorityqueue-alistarh/) |	lock-based | 2015 | [[AKL+15]](#AKL+15) |
|54| [Alistarh et al. priority queue based on Herlihy's skip list](./src/priorityqueue-alistarh-herlihyBased/) |	lock-based | 2015 | [[AKL+15]](#AKL+15) |
|55| [Alistarh et al. priority queue based on Pugh's skip list](./src/priorityqueue-alistarh-pughBased/) |	lock-based | 2015 | [[AKL+15]](#AKL+15) |
|| **Stacks** ||||
|56| [Global-lock stack](./src/stack-lock/) |	lock-based | | |
|57| [Treiber stack](./src/stack-treiber/) |	lock-free | 1986 | [[T+86]](#T+86) |
|58| [Treiber stack with OPTIK trylocks](./src/stack-optik/) |	lock-based | 2016 | [[GT+16]](#GT+16) |

References
----------

* <a name="AKL+15">**[AKL+15]**</a>
D. Alistarh, J. Kopinsky, J. Li, N. Shavit. The SprayList:
*A Scalable Relaxed Priority Queue*. 
PPoPP '15.
* <a name="BCH+10">**[BCH+10]**</a>
N. G. Bronson, J. Casper, H. Chafi, and K. Olukotun.
*A Practical Concurrent Binary Search Tree*.
PPoPP '10.
* <a name="DGT+15">**[DGT+15]**</a>
T. David, R. Guerraoui, and V. Trigonakis.
*Asynchronized Concurrency: The Secret to Scaling Concurrent Search Data Structures*.
ASPLOS '15.
* <a name="DMS+12">**[DMS+12]**</a>
M. Desnoyers, P. E. McKenney, A. S. Stern, M. R. Dagenais, and J. Walpole.
*User-level implementations of read-copy update*.
PDS '12.
* <a name="DVY+14">**[DVY+14]**</a>
D. Drachsler, M. Vechev, and E. Yahav.
*Practical Concurrent Binary Search Trees via Logical Ordering*.
PPoPP '14.
* <a name="EFR+10">**[EFR+10]**</a>
F. Ellen, P. Fatourou, E. Ruppert, and F. van Breugel.
*Non-blocking Binary Search Trees*.
PODC '10.
* <a name="F+03">**[F+03]**</a>
K. Fraser.
*Practical Lock-Freedom*.
PhD thesis, University of Cambridge, 2004.
* <a name="GT+16">**[GT+16]**</a>
R. Guerraoui, and V. Trigonakis.
*Optimistic Concurrency with OPTIK*.
PPoPP '16.
* <a name="H+01">**[H+01]**</a>
T. Harris.
*A Pragmatic Implementation of Non-blocking Linked Lists*.
DISC '01.
* <a name="HHL+06">**[HHL+06]**</a>
S. Heller, M. Herlihy, V. Luchangco, M. Moir, W. N. Scherer, and N. Shavit.
*A Lazy Concurrent List-Based Set Algorithm*.
OPODIS '05.
* <a name="HS+12">**[HS+12]**</a>
M. Herlihy and N. Shavit.
*The Art of Multiprocessor Programming, Revised First Edition*. 2012.
* <a name="HLL+07">**[HLL+07]**</a>
M. Herlihy, Y. Lev, V. Luchangco, and N. Shavit.
*A Simple Optimistic Skiplist Algorithm*.
SIROCCO '07.
* <a name="HLS+11">**[HLS+11]**</a>
M. Herlihy, Y. Lev, and N. Shavit.
*Concurrent lock-free skiplist with wait-free contains operator*, May 3 2011.
US Patent 7,937,378.
* <a name="HJ+12">**[HJ+12]**</a>
S. V. Howley and J. Jones. 
*A non-blocking internal binary search tree*. 
SPAA '12.
* <a name="INTEL+06">**[INTEL+06]**</a>
Intel.
*Intel Thread Building Blocks*.
https://www.threadingbuildingblocks.org.
* <a name="L+03">**[L+03]**</a>
D. Lea.
*Overview of Package util.concurrent Release 1.3.4*.
http://gee.cs.oswego.edu/dl/classes/EDU/oswego/cs/dl/util/concurrent/intro.html, 2003.
* <a name="LS+00">**[LS+00]**</a>
I. Lotan and N. Shavit. 
*Skiplist-based concurrent priority queues*.
IPDPS '00.
* <a name="M+02">**[M+02]**</a>
M. M. Michael.
*High Performance Dynamic Lock-free Hash tables and List-based Sets*.
SPAA '02.
* <a name="MS+96">**[MS+96]**</a>
M. M. Michael and M. L. Scott.
*Simple, Fast, and Practical Non-blocking and Blocking Concurrent Queue Algorithms*.
PODC '96.
* <a name="NM+14">**[NM+14]**</a>
A. Natarajan and N. Mittal.
*Fast Concurrent Lock-free Binary Search Trees*.
PPoPP '14.
* <a name="ORACLE+04">**[ORACLE+04]**</a>
Oracle.
*Java CopyOnWriteArrayList*.
http://docs.oracle.com/javase/7/docs/api/java/util/concurrent/CopyOnWriteArrayList.html.
* <a name="P+90">**[P+90]**</a>
W. Pugh.
*Concurrent Maintenance of Skip Lists*.
Technical report, 1990.
* <a name="T+86">**[T+86]**</a>
R. Treiber.
*Systems Programming: Coping with Parallelism*.
Technical report, 1986.

New Algorithms
--------------

BST-TK is a new lock-based BST, introduced in ASCYLIB. 
Additionally, CLHT is a new hash hash table, introduced in ASCYLIB. We provide lock-free and lock-based variants of CLHT as a separate repository (https://github.com/LPD-EPFL/CLHT).
Details of the algorithms and proofs/sketches of correctness can be found in the following technical report: https://infoscience.epfl.ch/record/203822

We have developed the following algorithms using OPTIK:
  1. A simple array map (in `src/hashtable-map_optik`).  
  We use this map in a hash table (in `src/hashtable-optik0`);
  2. An optimistic global-lock-based linked list (in `src/linkedlist-optik_gl`).  
  We use this list in a hash table (in `src/hashtable-optik1`);
  3. A fine-grained linked list (in `src/linkedlist-optik`).  
  We use this list in a hash table (in `src/hashtable-optik0`);
  4. A skip list algorithm (in `src/skiplist-optik1`).   
  We also provide a variant of the same algorithm (in `src/skiplist-optik`).

Additionally, we have optimized existing algorithms using OPTIK:
  1. Java's ConcurrentHashMap algorithm (in`src/hashtable-java_optik`);
  2. Herlihy's optimistic skip list (in `src/skiplist-optik2`);
  3. The classic Michael-Scott queues:
    * lock-based `push`, `pop` optimized with `optik_lock_version_backoff` (in `src/queue-optik0`)
    * lock-based `push`, `pop` optimized with `optik_trylock_version` (in `src/queue-optik1`)
    * lock-free `push`, `pop` optimized with `optik_trylock_version` (in `src/queue-optik2`)

Finally, we have introduced two optimization techniques inspired by OPTIK:
  1. Node caching for optimizing lists (in `src/linkedlist-optik_cache`);
  2. Victim queue for optimizing `push` in queues (in `src/queue-optik3`).

Compilation
-----------

ASCYLIB requires the ssmem memory allocator (https://github.com/LPD-EPFL/ssmem).
We have already compiled and included ssmem in external/lib for x86_64, SPARC, and the Tilera architectures.
Still, if you would like to create your own build of ssmem, take the following steps.
Clone ssmem, do `make libssmem.a` and then copy `libssmem.a` in `ASCYLIB/external/lib` and `smmem.h` in `ASCYLIB/external/include`.

Additionally, the sspfd profiler library is required (https://github.com/trigonak/sspfd).
We have already compiled and included sspfd in external/lib for x86_64, SPARC, and the Tilera architectures.
Still, if you would like to create your own build of sspfd, take the following steps.
Clone sspfd, do `make` and then copy `libsspfd.a` in `ASCYLIB/external/lib` and `sspfd.h` in `ASCYLIB/external/include`.

Finally, to measure power on new Intel processors (e.g., Intel Ivy Bridge), the raplread library is required (https://github.com/LPD-EPFL/raplread).
We have already compiled and included raplread in external/lib.
Still, if you would like to create your own build of raplread, take the following steps.
Clone raplread, do `make` and then copy `libraplread.a` in `ASCYLIB/external/lib` and `sspfd.h` in `ASCYLIB/external/include`.

To build all data structures, you can execute `make all`.
This target builds all lock-free, lock-based, and sequential data structures.

The last two structures, RCU and TBB, are based on external libraries. 
The RCU-based hash table requires an installation of the URCU library (http://urcu.so/).
You need to set the `URCU_PATH` in `common/Makefile.common` to point to the folder of your local URCU installation, or alternatively, you can install URCU globally.
The TBB-based hash table requires an installation of Intel's TBB library (https://www.threadingbuildingblocks.org/). You need to set the `TBB_LIBS` and the `TBB_INCLUDES` variables in `common/Makefile.common`, or alternatively, you can install TBB globally.

To build all data structures except from those two, you can issue `make`.

ASCYLIB includes a default configuration that uses `gcc` and tries to infer the number of cores and the frequency of the target/build platform. If this configuration is incorrect, you can always create a manual configurations in `common/Makefile.common` and `include/utils.h` (look in these files for examples). If you do not care about pinning threads to cores, these settings do not matter. You can compile with `make SET_CPU=0 ...` to disable thread pinning.

ASCYLIB accepts various compilation parameters. Please refer to the `COMPILE` file.

Tests
-----

Building ASCYLIB generate per-data-structure benchmarks in the `bin` directory.
Issue `./bin/executable -h` for the parameters each of those accepts.

Depending on the compilation flags, these benchmarks can be set to measure throughtput, latency, and/or power-consumption statistics.

Scripts
-------

ASCYLIB includes tons of usefull scripts (in the `scripts` folders). Some particularly useful ones are:
* `scalability.sh` and `scalability_rep.h`: run the given list of executable on the given (list of) number of threads, with the given parameters, and report throughput and scalability over single-threaded execution.
* scripts in `apslos/` directory: they were used to create the plots for the ASPLOS '15 paper. In particular, `apslos/run_scy.sh` accepts configuration files (see `asplos/config`) so it can be configured to execute almost any per-data-structure scenario.
* scripts in `ppopp/` directory: they were used to create the plots for the PPoPP '16 paper. In particular, `ppopp/run_and_plot.sh` can run and plot graphs for all the tests in the paper.

Thanks
------

Some of the initial implementations used in ASCYLIB were taken from Synchrobench (https://github.com/gramoli/synchrobench -  V. Gramoli. More than You Ever Wanted to Know about Synchronization. PPoPP 2015.). 
