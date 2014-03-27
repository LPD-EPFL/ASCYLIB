#!/bin/bash

source scripts/config;

reps=$1;
shift;
prog="$1";
shift;
params="$@";

un=$(uname -n);
tmp=data/run_rep_lat_max.${un}.tmp
printf "" > $tmp;

for r in $(seq 1 1 $reps);
do
    out=$($run_script ./$prog $params);
    lat=$(echo "$out" | grep "#thread" -A1 | tail -n1 | awk '{$1=""; print}');
    thr=$(echo "$out" | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);
    printf "%-10d%-10d%-10d%-10d%-10d%-10d%-10d\n" $thr $lat >> $tmp;
done;

med_idx=$(echo "1 + $reps/2" | bc);
sort -n $tmp | head -${med_idx} | tail -n1 | awk '{$1=""; print}';

