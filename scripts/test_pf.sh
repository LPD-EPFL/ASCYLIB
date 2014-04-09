#!/bin/bash
arr="128 256 512 1024 2048 4096 8192 32768"
echo "=========================================================="
echo "Executing ${1}"
echo "=========================================================="
echo "Initial   Throughput   Prefetches   Prefetches/transaction"
for i in ${arr} 
do 
	r=2*$i; 
	all=$(perf stat -e LLC-prefetches ${1} -n80 -i${i} -r${r} -u10 -d1000 2>&1)
	pref=$(echo "${all}" | grep "LLC-prefetches"| sed -e 's/^[ \t]*//' | cut -d' ' -f1 | sed 's/,//g');
	thr=$(echo "${all}" | grep "Mops" | cut -d' ' -f2);
	pop=$(echo "${pref}/(1000000*${thr})" | bc -l)
	printf "%-9d %-12.3f %-12s %-12.3f\n" ${i} ${thr} ${pref} ${pop}
done
