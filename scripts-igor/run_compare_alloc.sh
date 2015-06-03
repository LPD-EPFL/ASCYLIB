#!/bin/bash

cores=$1;
shift;

target=$1;
shift;

rm bin/*

#first, compile and run with vanilla ssmem
cd ../ssmem;
make clean; make TIGHT_ALLOC=0;
cp libssmem.a ../ascylib/external/lib/libssmem_x86_64.a

cd ../ascylib
make $target > /dev/null

echo "########## VANILLA SSMEM ############"
bin=`ls bin`;
LD_PRELOAD="" ./scripts/scalability_rep.sh "1 2 4 8" 5 median ./bin/$bin -u100

#second, compile and run with modified ssmem
cd ../ssmem;
make clean; make TIGHT_ALLOC=1;
cp libssmem.a ../ascylib/external/lib/libssmem_x86_64.a

cd ../ascylib;
make $target > /dev/null

echo "########## MODIFIED SSMEM + MALLOC ############"
bin=`ls bin`;
LD_PRELOAD="" ./scripts/scalability_rep.sh "1 2 4 8" 5 median ./bin/$bin -u100

echo "########## MODIFIED SSMEM + TCMALLOC ############"
bin=`ls bin`;
LD_PRELOAD="/home/zablotch/lib/libtcmalloc.so" ./scripts/scalability_rep.sh "1 2 4 8" 5 median ./bin/$bin -u100

echo "########## MODIFIED SSMEM + JEMALLOC ############"
bin=`ls bin`;
LD_PRELOAD="/home/zablotch/local/lib/libjemalloc.so.1" ./scripts/scalability_rep.sh "1 2 4 8" 5 median ./bin/$bin -u100