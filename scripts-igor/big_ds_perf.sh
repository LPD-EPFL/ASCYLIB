#!/bin/sh

SL_SIZES="8192 32768 65536 262144 528288 2097152 4194304 33554432 134217728 268435456"
HT_SIZES="8192 32768 65536 262144 528288 2097152 4194304 33554432 134217728 268435456 1073741824 2147483648"
SKIPLISTS="lb-sl_pugh lb_sl_herlihy lf-sl_fraser"
HTS="lb-ht_java"

for sl in $SKIPLISTS;
do
    for size in $SL_SIZES;
    do
        echo $sl - $size
        ../bin/$sl -n16 -u100 -i$size
    done;
done;

for ht in $HTS;
do
    for size in $HT_SIZES;
    do
        echo $ht - $size
        ./bin/$ht -n16 -u100 -i$size;
    done;
done;