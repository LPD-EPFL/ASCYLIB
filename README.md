ASCYLIB + OPTIK
===============

ASCYLIB (with OPTIK) is a concurrent data-structure library. It contains over 40 implementations of linked lists, hash tables, skip lists, binary search trees (BSTs), queues, priority queues, and stacks. ASCYLIB contains sequential, lock-based, and lock-free implementations for each data structure.

ASCYLIB works on x86, SPARC, and Tilera architectures and contains tests to evaluate the throughput, latency, latency distribution, and energy efficiency of the included data structures.

OPTIK is a new design pattern for easily implementing fast and scalable concurrent data structures. We have merged several concurrent data structures developed with OPTIK in ASCYLIB. More details can be found here: http://lpd.epfl.ch/site/optik.

* Website             : http://lpd.epfl.ch/site/ascylib - http://lpd.epfl.ch/site/optik
* Authors             : Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>,
                        Tudor David <tudor.david@epfl.ch> 
* Related Publications:
  * *Optimistic Concurrency with OPTIK*,  
    Rachid Guerraoui, Vasileios Trigonakis (alphabetical order),  
  PPoPP '16 *(to appear)*
  * *Asynchronized Concurrency: The Secret to Scaling Concurrent Search Data Structures*,  
  Tudor David, Rachid Guerraoui, Vasileios Trigonakis (alphabetical order),  
  ASPLOS '15

BST-TK is a new lock-based BST, introduced in ASCYLIB. 
Additionally, CLHT is a new hash hash table, introduced in ASCYLIB. We provide lock-free and lock-based variants of CLHT as a separate repository (https://github.com/LPD-EPFL/CLHT).
Details of the algorithms and a proofs/sketches of correctness can be found in the following technical report: https://infoscience.epfl.ch/record/203822

We have developed the following algorithms using OPTIK:
  1. A simple array map (in `src/hashtable-map_optik`).  
  We use this map in a hash table (in `src/hashtable-optik0`);
  2. An optimistic global-lock-based linked list (in `src/linkedlist-optik_gl`).  
  We use this list in a hash table (in `src/hashtable-optik1`);
  3. A fine-grained linked list (in `src/linkedlist-optik`).  
  We use this list in a hash table (in `src/hashtable-optik0`);
  4. A skip list algorithm (in `src/skiplist-optik1`).   
  We also provide a variant of the same algorithm (in `src/skiplist-optik1`).

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


Some of the initial implementations used in ASCYLIB were taken from Synchrobench (https://github.com/gramoli/synchrobench -  V. Gramoli. More than You Ever Wanted to Know about Synchronization. PPoPP 2015.). 

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

