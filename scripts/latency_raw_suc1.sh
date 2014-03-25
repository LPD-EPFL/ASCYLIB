#!/bin/bash

cores=$1;
shift;

source scripts/lock_exec;
source scripts/config;

prog1="$1";
shift;
params="$@";


echo "# #Cores: $cores / Params: $params";
printf "#%-24s%-24s%-24s%-24s%-24s%-24s%-24s\n" "$prog1" "$prog2" "$prog3" "$prog4" "$prog5" "$prog6" "$prog7";

un=$(uname -n);
tmp_get_suc=data/get_suc.${un}.tmp;
tmp_put_suc=data/put_suc.${un}.tmp;
tmp_rem_suc=data/rem_suc.${un}.tmp;
tmp_get_fal=data/get_fal.${un}.tmp;
tmp_put_fal=data/put_fal.${un}.tmp;
tmp_rem_fal=data/rem_fal.${un}.tmp;
tmp_p1=data/p1.${un}.tmp;
c=$cores;

prog=$prog1;

out=$($run_script ./$prog $params -n$c);

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

pr -m -t $tmp_get_suc $tmp_get_fal $tmp_put_suc $tmp_put_fal $tmp_rem_suc $tmp_rem_fal | awk 'NF' > $tmp_p1;

paste $tmp_p1  $tmp_p2  $tmp_p3  $tmp_p4  $tmp_p5  $tmp_p6  $tmp_p7 | column -t;

echo "";

source scripts/unlock_exec;
