#!/bin/bash

cores=$1;
shift;

source scripts/lock_exec;
source scripts/config;

prog1="$1";
shift;
params="$@";

printf "#   %-60s\n" "$prog1" 

echo "#co get_s  get_f put_s  put_f rem_s";

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
    prog=$prog1;

    thr=$($run_script ./$prog $params -n$c | grep "#thread" -A1 | tail -n1 | awk '{$1=""; print}');
    printf "%-10d%-10d%-10d%-10d%-10d%-10d" $thr;

    echo "";
done;

source scripts/unlock_exec;
