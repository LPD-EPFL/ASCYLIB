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

HEAD=head;
TAIL=tail;
if [ "$(uname -n)" = ol-collab1 ];
then
    HEAD=/usr/gnu/bin/head
    TAIL=/usr/gnu/bin/tail
fi;


for r in $(seq 1 1 $reps);
do
    res=$($run_script ./$prog $params);
    thr_all=$(echo "$res" | grep "#txs" | ${HEAD} -n1 | cut -d'(' -f2 | cut -d. -f1);
    thr_suc=$(echo "$res" | grep "#txs" | ${TAIL} -n1 | cut -d'(' -f2 | cut -d. -f1);
    thr_cas=$(echo "$res" | awk '/trylock/ { print $7 }');
    if [ -z ${thr_cas} ];
    then
	thr_cas=0;
    fi
       
    echo "$thr_suc $thr_all $thr_cas" >> $tmp;
done;

med_idx=$(echo "1 + $reps/2" | bc);
sort -n $tmp | ${HEAD} -${med_idx} | ${TAIL} -n1;

