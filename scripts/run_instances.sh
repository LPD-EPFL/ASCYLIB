#!/bin/bash

tmp=/tmp/is.tmp;

n_instances=$1;
shift;
what="$@";

echo -n "" > $tmp;

for i in $(seq 1 1 $n_instances);
do
#    printf "#%-3d $what\n" $i;
    ./$what | awk '/#Energy per Operation/ { print $8 }' | sed 's/)//g' | tee -a $tmp &
done;

echo "# started "$n_instances" instances of "$what;

t=0;
while [ ! ${t[0]} -eq $n_instances ];
do
    sleep 1;
    t=($(wc -l $tmp));
done;

awk '{s+=$1; i++; } END { print s/i }' $tmp;

