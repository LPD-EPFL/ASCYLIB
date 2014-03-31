#!/bin/bash

source scripts/config;

reps=$1;
shift;
prog="$1";
shift;
params="$@";

un=$(uname -n);
tmp=data/run_rep_max.${un}.tmp
printf "" > $tmp;

for r in $(seq 1 1 $reps);
do
    $run_script ./$prog $params | grep "#txs" | cut -d'(' -f2 | cut -d. -f1 >> $tmp;
done;

TAIL=tail;
if [ "$(uname -n)" = ol-collab1 ];
then
    TAIL=/usr/gnu/bin/tail
fi;

sort -n $tmp | ${TAIL} -n1;

