#!/bin/bash

ds=$1;
shift;
c0=$1;
shift;
c1=$1;
shift;
grep_out=( $1 );
gl=${#grep_out[@]};

echo ">> Doing throughut stats for $ds";
if [ $gl -gt 0 ];
then
    echo ">> without ${grep_out[@]}";
fi;
echo ">> Comparing columns $c0 vs. $c1";
files=$(ls *.${ds}.*w* | grep -v ldi);
for g in ${grep_out[@]};
do
    files=$(echo "$files" | grep -v $g);
done;


tot=0;
i=0;

for f in $files;
do
    r=$(awk "/^[0-9]/ {i++; s+=\$$c1/\$$c0; } END {print s/i}" $f);
    printf ">%-30s : %f\n" $f $r;
    tot=$(echo "$tot+$r" | bc -l);
    i=$(($i+1));
done;

echo "> avg: " $(echo "$tot/$i" | bc -l);
