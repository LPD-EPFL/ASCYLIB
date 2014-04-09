#!/bin/bash
arr="128 256 512 1024 2048 4096 8192 32768"
echo "${1}"
for i in ${arr} 
do 
	echo $i;
	 r=2*$i; 
	perf stat -e LLC-prefetches ${1} -n80 -i${i} -r${r} -u10 2>&1 | grep "LLC-prefetches"
done
