#!/bin/bash

for f in data/extremes_ll*
do
    title=$(grep "throughput" ${f} | awk '{$6="ratio"; print}');
    echo "${title}" > data/temp
    th=$(grep high ${f} | grep seq | cut -d' ' -f4); 
    grep high ${f} | awk '{$6=$4/'${th}'; print}' >> data/temp
    th=$(grep low ${f} | grep seq | cut -d' ' -f4); 
    grep low ${f} | awk '{$6=$4/'${th}'; print}' >> data/temp
    cp data/temp ${f}
done
for f in data/extremes_sl*
do
    title=$(grep "throughput" ${f} | awk '{$6="ratio"; print}');
    echo "${title}" > data/temp
    th=$(grep high ${f} | grep seq | cut -d' ' -f4); 
    grep high ${f} | awk '{$6=$4/'${th}'; print}' >> data/temp
    th=$(grep low ${f} | grep seq | cut -d' ' -f4); 
    grep low ${f} | awk '{$6=$4/'${th}'; print}' >> data/temp
    cp data/temp ${f}
done
for f in data/extremes_ht*
do
    title=$(grep "throughput" ${f} | awk '{$6="ratio"; print}');
    echo "${title}" > data/temp
    th=$(grep high ${f} | grep seq | cut -d' ' -f4); 
    grep high ${f} | awk '{$6=$4/'${th}'; print}' >> data/temp
    th=$(grep low ${f} | grep seq | cut -d' ' -f4); 
    grep low ${f} | awk '{$6=$4/'${th}'; print}' >> data/temp
    cp data/temp ${f}
done
for f in data/extremes_bst*
do
    title=$(grep "throughput" ${f} | awk '{$6="ratio"; print}');
    echo "${title}" > data/temp
    thi=$(grep high ${f} | grep "seq-int" | cut -d' ' -f4); 
    the=$(grep high ${f} | grep "seq-ext" | cut -d' ' -f4); 
    th=${thi}
    gt=$(echo $the'>'$thi | bc -l)
    if [ $gt -eq 1 ]; then
        th=${the}
    fi
    grep high ${f} | awk '{$6=$4/'${th}'; print}' >> data/temp
    thi=$(grep low ${f} | grep "seq-int" | cut -d' ' -f4); 
    the=$(grep low ${f} | grep "seq-ext" | cut -d' ' -f4); 
    th=${thi}
    gt=$(echo $the'>'$thi | bc -l)
    if [ $gt -eq 1 ]; then
        th=${the}
    fi
    grep low ${f} | awk '{$6=$4/'${th}'; print}' >> data/temp
    cp data/temp ${f}
done

