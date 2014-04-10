#!/bin/bash
cores=$1;
shift;

source scripts/lock_exec;
source scripts/config;
source scripts/help;

progs="$1";
shift;
progs_num=$(echo $progs | wc -w);
params="$@";

print_n "#   " "%-30s" "$progs" "\n"

print_rep "#co " $progs_num "throughput parsing  update   cleanup  queue    lock/upd  " "\n"

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
	out=$($run_script ./$p $params -n$c);
	thr=$(echo "$out" | grep "#txs" | cut -d'(' -f2 | cut -d. -f1);
	par=$(echo "$out" | awk '/#parse_all/ { print $3 }');
	upd=$(echo "$out" | awk '/#update_all/ { print $3 }');
	cln=$(echo "$out" | awk '/#cleanup_all/ { print $3 }');
	que=$(echo "$out" | awk '/#lock_all/ { print $3 }');
	lpu=$(echo "$out" | awk '/#lock_all/ { print $4 }');
	printf "%-10d %f %f %f %f %f  " $thr $par $upd $cln $que $lpu;
    done;     
    echo "";
done;

source scripts/unlock_exec;
