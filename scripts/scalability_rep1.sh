#!/bin/bash

cores=$1;
shift;

reps=$1;
shift;

source scripts/lock_exec;
source scripts/config;

result_type=$1;

if [ "$result_type" = "max" ];
then
    run_script="./scripts/run_rep_max.sh $reps";
    echo "# Result from $reps repetitions: max";
    shift;

elif [ "$result_type" = "min" ];
then
    run_script="./scripts/run_rep_min.sh $reps";
    echo "# Result from $reps repetitions: min";
    shift;
elif [ "$result_type" = "median" ];
then
    run_script="./scripts/run_rep_med.sh $reps";
    echo "# Result from $reps repetitions: median";
    shift;
else
    run_script="./scripts/run_rep_max.sh $reps";
    echo "# Result from $reps repetitions: max (default). Available: min, max, median";
fi;

prog=$1;
shift;
params="$@";


echo "#cores  throughput  %linear scalability"

printf "%-8d" 1;
thr1=$($run_script ./$prog $params -n1);
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
    thr=$($run_script ./$prog $params -n$c);
    printf "%-12d" $thr;
    scl=$(echo "$thr/$thr1" | bc -l);
    linear_p=$(echo "100*(1-(($c-$scl)/$c))" | bc -l);
    printf "%-8.2f" $linear_p;
    printf "%-8.2f\n" $scl;

done;

source scripts/unlock_exec;
