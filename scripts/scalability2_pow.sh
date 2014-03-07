#!/bin/bash

cores=$1;
shift;

source scripts/lock_exec;
source scripts/config;

run_script="sudo $run_script";

prog1=$1;
shift;
prog2=$1;
shift;
params="$@";

printf "#       %-43s%-43s\n" "$prog1" "$prog2";
echo "#cores  throughput  power   eng/op(uj) scalability throughput  power   eng/op(uj) scalability";

prog=$prog1;

printf "%-8d" 1;
res=$($run_script ./$prog $params -n1);
thr1a=$(echo "$res" | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);
pow=$(echo "$res" | awk '/#Total Power Corrected/ { print $5 };' );
eop=$(echo "$res" | awk '/#Energy per Operation/ { print $8 }' | sed 's/)//g');
printf "%-12d" $thr1a;
printf "%-8.3f%-11.6f" $pow $eop;
printf "%-12d" 1;

prog=$prog2;

res=$($run_script ./$prog $params -n1);
thr1b=$(echo "$res" | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);
pow=$(echo "$res" | awk '/#Total Power Corrected/ { print $5 };' );
eop=$(echo "$res" | awk '/#Energy per Operation/ { print $8 }' | sed 's/)//g');
printf "%-12d" $thr1b;
printf "%-8.3f%-11.6f" $pow $eop;
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

    res=$($run_script ./$prog $params -n$c);
    thr=$(echo "$res" | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);
    pow=$(echo "$res" | awk '/#Total Power Corrected/ { print $5 };' );
    eop=$(echo "$res" | awk '/#Energy per Operation/ { print $8 }' | sed 's/)//g');

    printf "%-12d" $thr;
    scl=$(echo "$thr/$thr1" | bc -l);
    linear_p=$(echo "100*(1-(($c-$scl)/$c))" | bc -l);
    printf "%-8.3f%-11.6f" $pow $eop;
    printf "%-12.2f" $scl;

    prog=$prog2;
    thr1=$thr1b;

    res=$($run_script ./$prog $params -n$c);
    thr=$(echo "$res" | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);
    pow=$(echo "$res" | awk '/#Total Power Corrected/ { print $5 };' );
    eop=$(echo "$res" | awk '/#Energy per Operation/ { print $8 }' | sed 's/)//g');
    printf "%-12d" $thr;
    scl=$(echo "$thr/$thr1" | bc -l);
    linear_p=$(echo "100*(1-(($c-$scl)/$c))" | bc -l);
    printf "%-8.3f%-11.6f" $pow $eop;
    printf "%-8.2f\n" $scl;


done;

source scripts/unlock_exec;
