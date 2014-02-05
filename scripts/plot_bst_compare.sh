#!/bin/sh

initials="1024 8192 131072 1048576"; #"8 64 512 1024 2048 8192";
updates="0 1 10 100";
COUNT='0'
rm -rf ./data/temp$1
mkdir ./data/temp$1
csplit -z -f "./data/temp${1}/temp" -b '%d.dat' ./data/$1.dat /##/ {*}

rm -rf ./data/pdfs$1
mkdir ./data/pdfs$1

for i in $initials
do
    for u in $updates
    do
	gnuplot -e "outfile='./data/pdfs${1}/out_${i}_${u}.pdf'; infile='./data/temp${1}/temp${COUNT}.dat'; mytitle='Initial ${i} Updates ${u}'" ./scripts/plot_bst_compare_template.gp
	COUNT=`expr $COUNT + 1`	
    done;
done

rm -rf ./data/temp$1
