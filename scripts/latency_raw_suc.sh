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
tmp_get_suc=data/get_suc.${un}.tmp;
tmp_put_suc=data/put_suc.${un}.tmp;
tmp_rem_suc=data/rem_suc.${un}.tmp;
tmp_get_fal=data/get_fal.${un}.tmp;
tmp_put_fal=data/put_fal.${un}.tmp;
tmp_rem_fal=data/rem_fal.${un}.tmp;

tmp_template="data/pXXX.${un}.tmp";

c=$cores;


i=0;
for p in $progs;
do
    i=$((i+1));
    tmp_p=$(get_tmp $tmp_template $i);
    out=$($run_script ./$p $params -n$c);

    get_suc=$(echo "$out" | awk '/#latency_get_suc/ {$1=""; print}' | tr , '\n');
    put_suc=$(echo "$out" | awk '/#latency_put_suc/ {$1=""; print}' | tr , '\n');
    rem_suc=$(echo "$out" | awk '/#latency_rem_suc/ {$1=""; print}' | tr , '\n');
    get_fal=$(echo "$out" | awk '/#latency_get_fal/ {$1=""; print}' | tr , '\n');
    put_fal=$(echo "$out" | awk '/#latency_put_fal/ {$1=""; print}' | tr , '\n');
    rem_fal=$(echo "$out" | awk '/#latency_rem_fal/ {$1=""; print}' | tr , '\n');

    echo "$get_suc" > $tmp_get_suc;
    echo "$put_suc" > $tmp_put_suc;
    echo "$rem_suc" > $tmp_rem_suc;
    echo "$get_fal" > $tmp_get_fal;
    echo "$put_fal" > $tmp_put_fal;
    echo "$rem_fal" > $tmp_rem_fal;

    pr  -m -t $tmp_get_suc $tmp_get_fal $tmp_put_suc $tmp_put_fal $tmp_rem_suc $tmp_rem_fal | awk 'NF' > $tmp_p;

done;

tmps="";
for i in $(seq 1 1 $progs_num);
do
    tmps="$tmps $(get_tmp $tmp_template $i)";
done;

paste $(echo "$tmps") | column -t;

echo "";

source scripts/unlock_exec;
