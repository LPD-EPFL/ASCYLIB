#!/bin/bash

out_file="data/lf_socket_short_"$(date | awk '// {print $2"_"$3}')".dat";
echo "Output file: $out_file";
printf "" > $out_file;

initials="8 64 512 1024 2048 8192";
updates="0 1 10 100";

for i in $initials
do
    for u in $updates
    do
	r=$((2*$i));	
	settings="-i$i -r$r -u$u";
	echo "## $settings" | tee -a $out_file;
	./scripts/scalability4.sh socket ./bin/lf-ll ./bin/lf-sl ./bin/lf-ht_harris ./bin/lf-bst_ellen $settings -l4 | tee -a $out_file;
    done;
done;
