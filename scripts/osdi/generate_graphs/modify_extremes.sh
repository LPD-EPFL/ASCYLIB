#!/bin/bash

for f in data/extremes*
do
    th=$(grep high ${f} | grep seq | cut -d' ' -f4); 
    grep high ${f} | awk '{$4=$4/'${th}'; print}' > data/temp
    th=$(grep low ${f} | grep seq | cut -d' ' -f4); 
    grep low ${f} | awk '{$4=$4/'${th}'; print}' >> data/temp
    cp data/temp ${f}
done

