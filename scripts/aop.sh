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

echo "# #Cores: $(echo $cores) / Params: $params";
print_n "# " "%s  " "$progs" "\n"

print_rep "#co " $progs_num "#updates  #atomic_op  ratio    " "\n"

un=$(uname -n);
tmp_get=data/get.${un}.tmp;
tmp_put=data/put.${un}.tmp;
tmp_rem=data/rem.${un}.tmp;


for c in 1 $cores
do
    c1=$(($c-1));
    if [ $c1 -eq 0 ];
    then
	c1=1;
    fi;

    printf "%-4d" $c;

    i=0;
    for p in $progs;
    do

	out=$(sudo likwid-perfctr -g MEM_UOP_RETIRED_LOADS_LOCK:PMC0,MEM_UOP_RETIRED_STORES_LOCK:PMC1,MEM_UOP_RETIRED_STORES:PMC2 -c0-${c1} ./$p $params -n$c);
	ins=$(echo "$out" | awk '/insr:/ { print $4 }');
	rem=$(echo "$out" | awk '/rems:/ { print $4 }');
	upd=$(($ins+$rem));
	aop=$(echo "$out" | grep STAT | head -n1 | awk '{printf "%d", $5}');
	ratio=$(echo "$aop/$upd" | bc -l);
	printf "%-9d %-11d %-8.4f " $upd $aop $ratio
    done;
    echo "";
done;

source scripts/unlock_exec;


