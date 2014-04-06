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
    run_script="./scripts/run_rep_lat_parse_max.sh $reps";
    echo "# Result from $reps repetitions: max";
    shift;

elif [ "$result_type" = "min" ];
then
    run_script="./scripts/run_rep_lat_parse_min.sh $reps";
    echo "# Result from $reps repetitions: min";
    shift;
elif [ "$result_type" = "median" ];
then
    run_script="./scripts/run_rep_lat_parse_med.sh $reps";
    echo "# Result from $reps repetitions: median";
    shift;
else
    run_script="./scripts/run_rep_lat_parse_max.sh $reps";
    echo "# Result from $reps repetitions: max (default). Available: min, max, median";
fi;

progs="$1";
shift;
progs_num=$(echo $progs | wc -w);
params="$@";

print_n "#   " "%-30s" "$progs" "\n"

print_rep "#co " $progs_num "search    insert    remove    " "\n"

d=0;
for c in 1 $cores
do
    if [ $c -eq 1 ];
    then
	if [ $d -eq 1 ];
	then
	    continue;
	fi;
    fi;
    d=1;

    printf "%-4d" $c;
    i=0;
    for p in $progs;
    do
	i=$(($i+1));
	thr=$($run_script ./$p $params -n$c);
	printf "%-10d%-10d%-10d" $thr;
    done;     
    echo "";
done;

source scripts/unlock_exec;
