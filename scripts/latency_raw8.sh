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


echo "# #Cores: $cores / Params: $params";
printf "#%s  %s  %s  %s  %s  %s  %s  %s  \n" "$prog1" "$prog2" "$prog3" "$prog4" "$prog5" "$prog6" "$prog7" "$prog8";


un=$(uname -n);
tmp_get=data/get.${un}.tmp;
tmp_put=data/put.${un}.tmp;
tmp_rem=data/rem.${un}.tmp;
tmp_p1=data/p1.${un}.tmp;
tmp_p2=data/p2.${un}.tmp;
tmp_p3=data/p3.${un}.tmp;
tmp_p4=data/p4.${un}.tmp;
tmp_p5=data/p5.${un}.tmp;
tmp_p6=data/p6.${un}.tmp;
tmp_p7=data/p7.${un}.tmp;
tmp_p8=data/p8.${un}.tmp;
c=$cores;

prog=$prog1;

out=$($run_script ./$prog $params -n$c);

get=$(echo "$out" | awk '/#latency_get/ {$1=""; print}' | tr , '\n');
put=$(echo "$out" | awk '/#latency_put/ {$1=""; print}' | tr , '\n');
rem=$(echo "$out" | awk '/#latency_rem/ {$1=""; print}' | tr , '\n');

echo "$get" > $tmp_get;
echo "$put" > $tmp_put;
echo "$rem" > $tmp_rem;

pr  -m -t $tmp_get $tmp_put $tmp_rem | awk 'NF' > $tmp_p1;

prog=$prog2;

out=$($run_script ./$prog $params -n$c);

get=$(echo "$out" | awk '/#latency_get/ {$1=""; print}' | tr , '\n');
put=$(echo "$out" | awk '/#latency_put/ {$1=""; print}' | tr , '\n');
rem=$(echo "$out" | awk '/#latency_rem/ {$1=""; print}' | tr , '\n');

echo "$get" > $tmp_get;
echo "$put" > $tmp_put;
echo "$rem" > $tmp_rem;

pr  -m -t $tmp_get $tmp_put $tmp_rem | awk 'NF' > $tmp_p2;

prog=$prog3;

out=$($run_script ./$prog $params -n$c);

get=$(echo "$out" | awk '/#latency_get/ {$1=""; print}' | tr , '\n');
put=$(echo "$out" | awk '/#latency_put/ {$1=""; print}' | tr , '\n');
rem=$(echo "$out" | awk '/#latency_rem/ {$1=""; print}' | tr , '\n');

echo "$get" > $tmp_get;
echo "$put" > $tmp_put;
echo "$rem" > $tmp_rem;

pr  -m -t $tmp_get $tmp_put $tmp_rem | awk 'NF' > $tmp_p3;

prog=$prog4;

out=$($run_script ./$prog $params -n$c);

get=$(echo "$out" | awk '/#latency_get/ {$1=""; print}' | tr , '\n');
put=$(echo "$out" | awk '/#latency_put/ {$1=""; print}' | tr , '\n');
rem=$(echo "$out" | awk '/#latency_rem/ {$1=""; print}' | tr , '\n');

echo "$get" > $tmp_get;
echo "$put" > $tmp_put;
echo "$rem" > $tmp_rem;

pr  -m -t $tmp_get $tmp_put $tmp_rem | awk 'NF' > $tmp_p4;

prog=$prog5;

out=$($run_script ./$prog $params -n$c);

get=$(echo "$out" | awk '/#latency_get/ {$1=""; print}' | tr , '\n');
put=$(echo "$out" | awk '/#latency_put/ {$1=""; print}' | tr , '\n');
rem=$(echo "$out" | awk '/#latency_rem/ {$1=""; print}' | tr , '\n');

echo "$get" > $tmp_get;
echo "$put" > $tmp_put;
echo "$rem" > $tmp_rem;

pr  -m -t $tmp_get $tmp_put $tmp_rem | awk 'NF' > $tmp_p5;

prog=$prog6;

out=$($run_script ./$prog $params -n$c);

get=$(echo "$out" | awk '/#latency_get/ {$1=""; print}' | tr , '\n');
put=$(echo "$out" | awk '/#latency_put/ {$1=""; print}' | tr , '\n');
rem=$(echo "$out" | awk '/#latency_rem/ {$1=""; print}' | tr , '\n');

echo "$get" > $tmp_get;
echo "$put" > $tmp_put;
echo "$rem" > $tmp_rem;

pr  -m -t $tmp_get $tmp_put $tmp_rem | awk 'NF' > $tmp_p6;

prog=$prog7;

out=$($run_script ./$prog $params -n$c);

get=$(echo "$out" | awk '/#latency_get/ {$1=""; print}' | tr , '\n');
put=$(echo "$out" | awk '/#latency_put/ {$1=""; print}' | tr , '\n');
rem=$(echo "$out" | awk '/#latency_rem/ {$1=""; print}' | tr , '\n');

echo "$get" > $tmp_get;
echo "$put" > $tmp_put;
echo "$rem" > $tmp_rem;

pr  -m -t $tmp_get $tmp_put $tmp_rem | awk 'NF' > $tmp_p7;

prog=$prog8;

out=$($run_script ./$prog $params -n$c);

get=$(echo "$out" | awk '/#latency_get/ {$1=""; print}' | tr , '\n');
put=$(echo "$out" | awk '/#latency_put/ {$1=""; print}' | tr , '\n');
rem=$(echo "$out" | awk '/#latency_rem/ {$1=""; print}' | tr , '\n');

echo "$get" > $tmp_get;
echo "$put" > $tmp_put;
echo "$rem" > $tmp_rem;

pr  -m -t $tmp_get $tmp_put $tmp_rem | awk 'NF' > $tmp_p8;


paste $tmp_p1  $tmp_p2  $tmp_p3  $tmp_p4  $tmp_p5  $tmp_p6  $tmp_p7  $tmp_p8 | column -t;

echo "";

source scripts/unlock_exec;
