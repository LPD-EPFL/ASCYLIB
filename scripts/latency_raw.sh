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

echo "# #Cores: $cores / Params: $params";
print_n "# " "%s  " "$progs" "\n"

un=$(uname -n);
tmp_get=data/get.${un}.tmp;
tmp_put=data/put.${un}.tmp;
tmp_rem=data/rem.${un}.tmp;

tmp_template="data/pXXX.${un}.tmp";

c=$cores;


i=0;
for p in $progs;
do
    i=$((i+1));
    tmp_p=$(get_tmp $tmp_template $i);
    out=$($run_script ./$p $params -n$c);

    get=$(echo "$out" | awk '/#latency_get/ {$1=""; print}' | tr , '\n');
    put=$(echo "$out" | awk '/#latency_put/ {$1=""; print}' | tr , '\n');
    rem=$(echo "$out" | awk '/#latency_rem/ {$1=""; print}' | tr , '\n');
    echo "$get" > $tmp_get;
    echo "$put" > $tmp_put;
    echo "$rem" > $tmp_rem;
    pr  -m -t $tmp_get $tmp_put $tmp_rem | awk 'NF' > $tmp_p;
done;

tmps="";
for i in $(seq 1 1 $progs_num);
do
    tmps="$tmps $(get_tmp $tmp_template $i)";
done;

paste $(echo "$tmps") | column -t;

echo "";

source scripts/unlock_exec;
