#!/bin/sh

suffix="$1";

cd bin;
bins=$(find . -maxdepth 1 -type f);

for b in $bins;
do
    mv $b $b"_${suffix}";
done;

