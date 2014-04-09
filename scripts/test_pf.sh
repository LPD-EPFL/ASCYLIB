#!/bin/bash
arr="128 256 512 1024 2048 4096 8192 32768"
echo "Executing ${1}"
echo "Initial Throughput Prefetches"
for i in ${arr} 
do 
	r=2*$i; 
	all=$(perf stat -e LLC-prefetches ${1} -n80 -i${i} -r${r} -u10 2>&1)
	pref=$(echo "${all}" | grep "LLC-prefetches"| sed -e 's/^[ \t]*//' | cut -d' ' -f1);
	thr=$(echo "${all}" | grep "Mops" | cut -d' ' -f2);
	printf "%-7d %-10.3f %-10s\n" ${i} ${thr} ${pref}
done
