ASCYLIB
=======

ASCYLIB is a concurrent-search data-structure library. It contains over 30 implementantions of linked lists, hash tables, skip lists, and binary search trees (BST). ASCYLIB contains sequential, lock-based, and lock-free implementations for each data structure.

* Website             : http://lpd.epfl.ch/site/ascylib
* Author              : Vasileios Trigonakis <vasileios.trigonakis@epfl.ch>,
                        Tudor David <tudor.david@epfl.ch> 
* Related Publications: ASCYLIB was developed for:
  Asynchronized Concurrency: The Secret to Scaling Concurrent Search Data Structures,
  Tudor David, Rachid Guerraoui, Vasileios Trigonakis (alphabetical order),
  ASPLOS '14

Some of the initial implementations used in ASCYLIB were taken from Synchrobench (https://github.com/gramoli/synchrobench -  V. Gramoli. More than You Ever Wanted to Know about Synchronization. PPoPP 2015.). 

Compilation
-----------

ASCYLIB requires the ssmem memory allocator (https://github.com/LPD-EPFL/ssmem).
We have already compiled and included ssmem in external/lib for x86_64, SPARC, and the Tilera architectures.
Still, if you would like to create your own build of ssmem, take the following steps.
Clone ssmem, do `make libssmem.a` and then copy `libssmem.a` in `ASCYLIB/external/lib` and `ASCYLIB/include/smmem.h` in `CLHT/external/include`.

Additionally, the sspfd profiler library is required (https://github.com/trigonak/sspfd).
We have already compiled and included sspfd in external/lib for x86_64, SPARC, and the Tilera architectures.
Still, if you would like to create your own build of sspfd, take the following steps.
Clone sspfd, do `make` and then copy `libsspfd.a` in `ASCYLIB/external/lib` and `ASCYLIB/include/sspfd.h` in `CLHT/external/include`.

Finally, to measure power on new Intel processors (e.g., Intel Ivy Bridge), the raplread library is required (https://github.com/LPD-EPFL/raplread).
We have already compiled and included raplread in external/lib.
Still, if you would like to create your own build of raplread, take the following steps.
Clone raplread, do `make` and then copy `libraplread.a` in `ASCYLIB/external/lib` and `ASCYLIB/include/sspfd.h` in `CLHT/external/include`.

To build all data structures, you can execute `make all`.
This target builds all lock-free, lock-based, and sequential data structures.

The last to structures, RCU and TBB, are based on external library. 
The RCU-based hash table requires an installation of the URCU library (http://urcu.so/).
The TBB-based hash table requires an installation of Intel's TBB library (https://www.threadingbuildingblocks.org/).

To build all data structures except from those two, you can issue `make`.

ASCYLIB includes a default configuration that uses `gcc` and tries to infer the number of cores and the frequency of the target/build platform. If this configuration is incorrect, you can always create a manual configurations in `common/Makefile.common` and `include/utils.h` (look in these files for examples).

ASCYLIB accepts various compilation parameters. Please refer to the `COMPILE` file.

Tests
-----

Building ASCYLIB generate per-data-structure benchmarks in the `bin` directory.
Issue `./bin/executable -h` for the parameters each of those accepts.

Depending on the compilation flags, these benchmarks can be set to measure throughtput, latency, and/or power-consumption statistics.

