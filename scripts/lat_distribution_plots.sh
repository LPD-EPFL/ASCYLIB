#!/bin/bash
source ./scripts/lat_distribution.config
source ./scripts/config

for n in $unames; do
for s in $structs; do
for i in $initials; do
for u in $updates; do
mc=$(max_cores $n)
for c in ${mc}; do
for o in $ops; do
    file1=${data_folder}/${n}_lat_lf_${s}_${i}_${u}_${c}_${o}.csv
    file2=${data_folder}/${n}_lat_lb_${s}_${i}_${u}_${c}_${o}.csv
    out_file=${plots_folder}/${n}_lat_${s}_${i}_${u}_${c}_${o}.pdf
    [ -f ${file1} ] && [ -f ${file2} ] && R -f ./scripts/lat_distribution.r --args ${file1} ${file2} ${out_file} "${s}, ${n}, ${c} cores, i=${i}, u=${u}, ${o}"
done
done
done
done
done
done


