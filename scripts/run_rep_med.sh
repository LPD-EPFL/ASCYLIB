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
    timeout 20 $run_script ./$prog $params | grep "#txs" | cut -d'(' -f2 | cut -d. -f1 >> $tmp;
done;

HEAD=head;
TAIL=tail;
if [ "$(uname -n)" = ol-collab1 ];
then
    HEAD=/usr/gnu/bin/head
    TAIL=/usr/gnu/bin/tail
fi;

med_idx=$(echo "1 + $reps/2" | bc);
sort -n $tmp | ${HEAD} -${med_idx} | ${TAIL} -n1;

