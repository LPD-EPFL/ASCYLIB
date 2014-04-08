#!/bin/bash

cores=$1;
shift;

reps=$1;
shift;

source scripts/lock_exec;
source scripts/config;
source scripts/help;

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

progs="$1";
shift;
progs_num=$(echo $progs | wc -w);
params="$@";

progs_short=$(echo $progs | sed -e 's/\.\/bin\///g');
print_n "#       " "%-20s" "$progs_short" "\n"

print_rep "#cores  " $progs_num "throughput          " "\n"


printf "%-8d" 1;
thr1="";
for p in $progs;
do
    thr=$($run_script ./$p $params -n1);
    thr1="$thr1 $thr";
    printf "%-20d" $thr;
done;

echo "";

for c in $cores
do
    if [ $c -eq 1 ]
    then
	continue;
    fi;

    printf "%-8d" $c;

    i=0;
    for p in $progs;
    do
	i=$(($i+1));
	thr1p=$(get_n $i "$thr1");
	thr=$($run_script ./$p $params -n$c);
	printf "%-20d" $thr;
    done;     
    echo "";
done;

source scripts/unlock_exec;
