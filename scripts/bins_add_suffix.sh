#!/bin/bash

if [ $# -gt 0 ];
then
    suffix=$1;
    echo  "Using suffix: $suffix";

    for f in $(ls bin/* | grep "lb-..$");
    do
	echo "Renaming: $f to $f"_$suffix;
	mv $f $f"_"$suffix;
    done;

else
    echo "Please provide the suffix"
fi;
