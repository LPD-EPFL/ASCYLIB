#!/bin/bash
plots_folder=plots
data_folder=data
unames="maglite lpd48core lpdxeon2680 parsasrv1.epfl.ch diassrv8 lpdpc4 ol-collab1"
initials="256 2048 8192 65536"
updates="0 20 100"
the_cores="18"
ops="get put rem"
structs="ll ht sl bst"

for n in $unames; do
for s in $structs; do
for i in $initials; do
for u in $updates; do
for c in ${the_cores}; do
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


