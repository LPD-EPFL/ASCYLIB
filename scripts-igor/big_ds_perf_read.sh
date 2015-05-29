#!/bin/sh

SL_SIZES="8192 32768 65536 262144 524288 2097152 4194304 33554432 134217728 268435456"
HT_SIZES="8192 32768 65536 262144 524288 2097152 4194304 33554432 134217728 268435456 1073741824"
SKIPLISTS="lb-sl_pugh lb-sl_herlihy lf-sl_fraser"
HTS="lb-ht_java"

cd ..;
make clean; rm bin/*;
make htjava lfsl_fraser lbsl_pugh lbsl_herlihy_lb INIT_SEQ=1 SKEW9010=1

for sl in $SKIPLISTS;
do
    for size in $SL_SIZES;
    do
        echo $sl - $size;
        ./bin/$sl -n16 -u0 -i$size | tee -a big_perf_read;
    done;
done;

for ht in $HTS;
do
    for size in $HT_SIZES;
    do
        echo $ht - $size;
        ./bin/$ht -n16 -u0 -i$size | tee -a big_perf_read;
    done;
done;

