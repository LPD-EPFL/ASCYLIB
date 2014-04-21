#!/bin/bash

file=$1;
field=$2;
lines=$(wc -l $file | awk '// { print $1 }');
keep=$(echo "$lines/2 + 1" | bc);
sort -n $file -k $field | head -${keep} | tail -n1 | sed 's/  */ /g' | cut -d" " -f$field;