#!/bin/sh

for i in `seq 1 50`; 
do
	./run bin/lf-bst-howley -n20 -u100 -r8 | grep "size" | perl -nle 'print $1 ne $2 if /(\d+) .* (\d+)/';
done;
