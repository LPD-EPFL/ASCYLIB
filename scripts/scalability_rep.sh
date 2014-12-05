#!/bin/bash

if [ $# -eq 0 ];
then
    echo "Usage: $0 \"cores\" num_repetitions value_to_keep \"executable1 excutable2 ...\" [params]";
    echo "  where \"cores\" can be the actual thread num to use, such as \"1 10 20\", or"
    echo "  one of the predefined specifications for that platform (e.g., socket -- see "
    echo "  scripts/config)";
    echo "  and value_to_keep can be the min, max, or median";
    exit;
fi;

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


print_n "#       " "%-32s" "$progs" "\n"

print_rep "#cores  " $progs_num "throughput  %%linear scalability " "\n"


printf "%-8d" 1;
thr1="";
for p in $progs;
do
    thr=$($run_script ./$p $params -n1);
    thr1="$thr1 $thr";
    printf "%-12d" $thr;
    printf "%-8.2f" 100.00;
    printf "%-12d" 1;
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
	printf "%-12d" $thr;
	scl=$(echo "$thr/$thr1p" | bc -l);
	linear_p=$(echo "100*(1-(($c-$scl)/$c))" | bc -l);
	printf "%-8.2f" $linear_p;
	printf "%-12.2f" $scl;
    done;     
    echo "";
done;

source scripts/unlock_exec;
