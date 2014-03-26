#!/bin/bash

cores=$1;
shift;

source scripts/lock_exec;
source scripts/config;

prog1="$1";
shift;
prog2="$1";
shift;
prog3="$1";
shift;
prog4="$1";
shift;
prog5="$1";
shift;
prog6="$1";
shift;
prog7="$1";
shift;
prog8="$1";
shift;
params="$@";

printf "#   %-60s%-60s%-60s%-60s%-60s%-60s%-60s%-60s\n" "$prog1" "$prog2" "$prog3" "$prog4" "$prog5" "$prog6" "$prog7" "$prog8";

echo "#co get_s  get_f put_s  put_f rem_s  rem_f get_s  get_f put_s  put_f rem_s  rem_f get_s  get_f put_s  put_f rem_s  rem_f get_s  get_f put_s  put_f rem_s  rem_f get_s  get_f put_s  put_f rem_s  rem_f get_s  get_f put_s  put_f rem_s  rem_f get_s  get_f put_s  put_f rem_s  rem_f get_s  get_f put_s  put_f rem_s  rem_f";

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

    prog=$prog2;

    thr=$($run_script ./$prog $params -n$c | grep "#thread" -A1 | tail -n1 | awk '{$1=""; print}');
    printf "%-10d%-10d%-10d%-10d%-10d%-10d" $thr;

    prog=$prog3;

    thr=$($run_script ./$prog $params -n$c | grep "#thread" -A1 | tail -n1 | awk '{$1=""; print}');
    printf "%-10d%-10d%-10d%-10d%-10d%-10d" $thr;

    prog=$prog4;

    thr=$($run_script ./$prog $params -n$c | grep "#thread" -A1 | tail -n1 | awk '{$1=""; print}');
    printf "%-10d%-10d%-10d%-10d%-10d%-10d" $thr;

    prog=$prog5;

    thr=$($run_script ./$prog $params -n$c | grep "#thread" -A1 | tail -n1 | awk '{$1=""; print}');
    printf "%-10d%-10d%-10d%-10d%-10d%-10d" $thr;

    prog=$prog6;

    thr=$($run_script ./$prog $params -n$c | grep "#thread" -A1 | tail -n1 | awk '{$1=""; print}');
    printf "%-10d%-10d%-10d%-10d%-10d%-10d" $thr;

    prog=$prog7;

    thr=$($run_script ./$prog $params -n$c | grep "#thread" -A1 | tail -n1 | awk '{$1=""; print}');
    printf "%-10d%-10d%-10d%-10d%-10d%-10d" $thr;

    prog=$prog8;

    thr=$($run_script ./$prog $params -n$c | grep "#thread" -A1 | tail -n1 | awk '{$1=""; print}');
    printf "%-10d%-10d%-10d%-10d%-10d%-10d" $thr;


    echo "";
done;

source scripts/unlock_exec;
