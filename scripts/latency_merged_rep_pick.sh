#!/bin/bash
cores=$1;
shift;

pick=$1;
shift;
echo "## Pick column $pick";

reps=$1;
shift;

source scripts/lock_exec;
source scripts/config;
source scripts/help;

result_type=$1;

if [ "$result_type" = "max" ];
then
    run_script="./scripts/run_rep_lat_max.sh $reps";
    echo "# Result from $reps repetitions: max";
    shift;

elif [ "$result_type" = "min" ];
then
    run_script="./scripts/run_rep_lat_min.sh $reps";
    echo "# Result from $reps repetitions: min";
    shift;
elif [ "$result_type" = "median" ];
then
    run_script="./scripts/run_rep_lat_med.sh $reps";
    echo "# Result from $reps repetitions: median";
    shift;
else
    run_script="./scripts/run_rep_lat_max.sh $reps";
    echo "# Result from $reps repetitions: max (default). Available: min, max, median";
fi;

progs="$1";
shift;
progs_num=$(echo $progs | wc -w);
params="$@";

progs_short=$(echo $progs | sed -e 's/\.\/bin\///g' -e 's/\/bin\///g');

print_n "#   " "%-19s " "$progs_short" "\n"

str="get,put,rem";
str_pick=$(echo $str | cut -d, -f$pick);
print_rep "#co " $progs_num "$str_pick                 " "\n"

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
	g1=$(echo $thr | awk '{ print $1 };');
	g2=$(echo $thr | awk '{ print $2 };');
	p1=$(echo $thr | awk '{ print $3 };');
	p2=$(echo $thr | awk '{ print $4 };');
	r1=$(echo $thr | awk '{ print $5 };');
	r2=$(echo $thr | awk '{ print $6 };');

	g=$(echo "($g1+$g2)/2" | bc);
	p=$(echo "($p1+$p2)/2" | bc);
	r=$(echo "($r1+$r2)/2" | bc);
	all=$(printf "%d,%d,%d" $g $p $r);
	printf "%-20d" $(echo $all | cut -d, -f$pick);


    done;     
    echo "";
done;

source scripts/unlock_exec;
