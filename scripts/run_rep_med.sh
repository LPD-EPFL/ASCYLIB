#!/bin/bash

source scripts/config

reps=$1;
shift;
prog="$1";
shift;
params="$@";

un=$(uname -n);
tmp=data/run_rep_med.${un}.tmp
printf "" > $tmp;

for r in $(seq 1 1 $reps);
do
    $run_script ./$prog $params | grep "#txs" | cut -d'(' -f2 | cut -d. -f1 >> $tmp;
done;

med_idx=$(echo "1 + $reps/2" | bc);
sort -n $tmp | head -${med_idx} | tail -n1;

