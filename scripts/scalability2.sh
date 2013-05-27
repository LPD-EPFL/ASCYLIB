#!/bin/bash

cores=$1;
shift;

unm=$(uname -n);

if [ $unm = "parsasrv1.epfl.ch" ];
then
    run_script="./run"
fi;

if [ "$cores" = "all" ];
then
    if [ $unm = "maglite" ];
    then
	cores=$(seq 2 1 64);
    elif [ $unm = "parsasrv1.epfl.ch" ];
    then
	cores=$(seq 2 1 36);
    else
	cores=$(seq 2 1 48);
    fi;
elif [ "$cores" = "socket" ];
then
    if [ $unm = "maglite" ];
    then
	cores=$(seq 8 8 64);
    elif [ $unm = "parsasrv1.epfl.ch" ];
    then
	cores=$(seq 6 6 36);
    else
	cores=$(seq 6 6 48);
    fi;
elif [ "$cores" = "twelve" ];
then
    cores=$(seq 2 1 12);
fi;

prog1=$1;
shift;
prog2=$1;
shift;
params="$@";


echo "#      $prog1                             $prog2"
echo "#cores  throughput  %linear scalability throughput  %linear scalability"

prog=$prog1;

printf "%-8d" 1;
thr1a=$($run_script ./$prog $params -n1 | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);
printf "%-12d" $thr1a;
printf "%-8.2f" 100.00;
printf "%-12d" 1;

prog=$prog2;

thr1b=$($run_script ./$prog $params -n1 | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);
printf "%-12d" $thr1b;
printf "%-8.2f" 100.00;
printf "%-8d\n" 1;

for c in $cores
do
    if [ $c -eq 1 ]
    then
	continue;
    fi;

    printf "%-8d" $c;

    prog=$prog1;
    thr1=$thr1a;

    thr=$($run_script ./$prog $params -n$c | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);
    printf "%-12d" $thr;
    scl=$(echo "$thr/$thr1" | bc -l);
    linear_p=$(echo "100*(1-(($c-$scl)/$c))" | bc -l);
    printf "%-8.2f" $linear_p;
    printf "%-12.2f" $scl;

    prog=$prog2;
    thr1=$thr1b;

    thr=$($run_script ./$prog $params -n$c | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);
    printf "%-12d" $thr;
    scl=$(echo "$thr/$thr1" | bc -l);
    linear_p=$(echo "100*(1-(($c-$scl)/$c))" | bc -l);
    printf "%-8.2f" $linear_p;
    printf "%-8.2f\n" $scl;


done;
