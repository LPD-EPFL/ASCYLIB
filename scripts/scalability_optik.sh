#!/bin/bash

if [ $# -eq 0 ];
then
    echo "Usage: $0 \"cores\" num_repetitions \"executable1 excutable2 ...\" [params]";
    echo "  where \"cores\" can be the actual thread num to use, such as \"1 10 20\", or"
    echo "  one of the predefined specifications for that platform (e.g., socket -- see "
    echo "  scripts/config)";
    exit;
fi;

cores=$1;
shift;

reps=$1;
shift;

source scripts/lock_exec;
source scripts/config;
source scripts/help;

run_script="./scripts/run_rep_med_optik.sh $reps";

progs="$1";
shift;
progs_num=$(echo $progs | wc -w);
params="$@";


print_n "#       " "%-34s" "$progs" "\n"

print_rep "#cores  " $progs_num "thrgh-suc thrgh-all thrgh-cas cas/sc " "\n"


printf "%-8d" 1;
thr1="";
for p in $progs;
do
    thr=$($run_script ./$p $params -n1);
    printf "%-9d %-9d %-9d " $thr;
    printf "%-7.2f" 1;
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
	printf "%-9d %-9d %-9d " $thr;
	thra=( $thr );
	ratio=$(echo "${thra[2]} / ${thra[0]}" | bc -l);
	printf "%-7.2f" $ratio;
    done;     
    echo "";
done;

source scripts/unlock_exec;
