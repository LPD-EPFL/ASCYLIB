#!/bin/bash

cores=$1;
shift;

unm=$(uname -n);

if [ "$cores" = "all" ];
then
    if [ $unm = "maglite" ];
    then
	cores=$(seq 2 1 64);
    else
	cores=$(seq 2 1 48);
    fi;
elif [ "$cores" = "socket" ];
then
    if [ $unm = "maglite" ];
    then
	cores=$(seq 8 8 64);
    else
	cores=$(seq 6 6 48);
    fi;
fi;

prog=$1;
shift;
params="$@";


echo "#cores  throughput  %linear scalability"

printf "%-8d" 1;
thr1=$(./$prog $params -n1 | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);
printf "%-12d" $thr1;
printf "%-8.2f" 100.00;
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
    scl=$(echo "$thr/$thr1" | bc -l);
    linear_p=$(echo "100*(1-(($c-$scl)/$c))" | bc -l);
    printf "%-8.2f" $linear_p;
    printf "%-8.2f\n" $scl;

done;
