#!/bin/bash

if [ $# -eq 0 ];
then
    echo "Usage: $0 \"num_cores\" \"executable1 excutable2 ...\" [params]";
    exit;
fi;

nc=$1;
shift;

source scripts/lock_exec;
source scripts/config;
source scripts/help;

progs="$1";
shift;
progs_num=$(echo $progs | wc -w);
params="$@";


echo "# number of cores: $nc";

print_n "#" "%-48s" "$progs" "\n"

# print_rep "#cores  " $progs_num "throughput  %%linear scalability " "\n"

for p in $progs;
do
    res=$(./$p $params -n$nc | awk '/ECDF-/ {$1=""; print i++ $0;}');
    for ((i = 0; i < 6; i++))
    do
	line=$(echo "$res" | awk "/^"$i"/ { \$1=\"\"; \$2=\"\"; \$8=\"\"; print; }");
	rows[$i]=${rows[$i]}" "$line;
    done
done;

# col0=( ${rows[0]} );
# nc=${#col0[@]};
# pstr="";
# for ((l = 0; l < $nc; l++))
# do
#     pstr="%-8d $pstr";
# done;
# pstr="$pstr \n";
# for ((i = 0; i < 6; i++))
# do
#     printf " $pstr" ${rows[$i]};
# done
# echo "";

prog_num=$(echo "$progs" | wc -w);

val_num=5;
col_num=6;

for ((r = 0; r < $val_num; r++))
do
    printf " ";
    for ((p = 0; p < $prog_num; p++))
    do
	for ((c = 0; c < $col_num; c++))
	do
	    row=( ${rows[$c]} );
	    keep=$((($p * $val_num) + $r));
	    printf "%-7d " ${row[$keep]};
	done
    done
    echo
done 

source scripts/unlock_exec;
