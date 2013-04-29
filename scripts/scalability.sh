#!/bin/bash

cores=$1;
shift;

if [ "$cores" = "all" ];
then
    cores=$(seq 2 1 48);
elif [ "$cores" = "socket" ];
then
    cores=$(seq 6 6 48);
fi;

prog=$1;
shift;
params="$@";


echo "#cores  throughput  linear  actual"

printf "%-8d" 1;
thr1=$(./$prog $params -n1 | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);
printf "%-12d" $thr1;
printf "%-8d" 1;
printf "%-8d\n" 1;

for c in $cores
do
    if [ $c -eq 1 ]
    then
	continue;
    fi;

    printf "%-8d" $c;
    thr=$(./$prog $params -n$c | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);
    printf "%-12d" $thr;
    printf "%-8d" $c;
    printf "%-8.2f\n" $(echo "$thr/$thr1" | bc -l);

done;
