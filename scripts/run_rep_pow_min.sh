#!/bin/bash

source scripts/config;

reps=$1;
shift;
prog="$1";
shift;
params="$@";

un=$(uname -n);
tmp=/tmp/run_rep_pow_max.${un}.tmp
printf '' > $tmp;

for r in $(seq 1 1 $reps);
do
    out=$($run_script ./$prog $params);
    thr=$(echo "$out" | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);
    pow=$(echo "$out" | awk '/#Total Power Corrected/ { print $5 };' );
    ppw=$(echo "$thr/$pow" | bc -l);
    eop=$(echo "$out" | awk '/#Energy per Operation/ { print $8 }' | sed 's/)//g');

    printf "%d %.2f %f %f \n" $thr $ppw $pow $eop >> $tmp;
done;

sort -n $tmp | grep -v "-" | tail -n1

