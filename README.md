ASCYLIB
=======

ASCYLIB is a concurrent-search data-structure library. It contains over 30 implementations of linked lists, hash tables, skip lists, and binary search trees (BST). ASCYLIB contains sequential, lock-based, and lock-free implementations for each data structure.

ASCYLIB works on x86, SPARC, and Tilera architectures and contains tests to evaluate the throughput, latency, latency distribution, and energy efficiency of the included data structures.

* Website             : http://lpd.epfl.ch/site/ascylib
* Author              : Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>,
                        Tudor David <tudor.david@epfl.ch> 
* Related Publications: ASCYLIB was developed for:
  *Asynchronized Concurrency: The Secret to Scaling Concurrent Search Data Structures*,
  Tudor David, Rachid Guerraoui, Vasileios Trigonakis (alphabetical order),
  ASPLOS '15

BST-TK is a new lock-based BST, introduced in ASCYLIB. 
Additionally, CLHT is a new hash hash table, introduced in ASCYLIB. We provide lock-free and lock-based variants of CLHT as a separate repository (https://github.com/LPD-EPFL/CLHT).
Details of the algorithms and a proofs/sketches of correctness can be found in the following technical report: https://infoscience.epfl.ch/record/203822


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

ASCYLIB includes a default configuration that uses `gcc` and tries to infer the number of cores and the frequency of the target/build platform. If this configuration is incorrect, you can always create a manual configurations in `common/Makefile.common` and `include/utils.h` (look in these files for examples).

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

