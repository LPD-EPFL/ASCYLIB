#!/bin/bash
datadir=data/
graphdir="scripts/osdi/generate_graphs/"

structs="ll sl ht bst"
for s in $structs; do
#cfile="${graphdir}common_${s}.txt"
#rm ${cfile}
#for f in ${datadir}common_${s}* ;do
#   if [ ! -e ${cfile} ]; then
#    line=$(head -n 1 ${f} ) 
#    echo $line > ${cfile}
#   fi
#   tail -n +2 "$f" >> ${cfile}
#done
efile="${graphdir}extremes_${s}.txt"
rm ${efile}
for f in ${datadir}extremes_${s}* ;do
   if [ ! -e ${efile} ]; then
    line=$(head -n 1 ${f} ) 
    echo $line > ${efile}
   fi
   tail -n +2 "$f" >> ${efile}
done

find ${graphdir} "extremes*.txt" | xargs perl -pi -e 's/parsasrv1.epfl.ch/Tilera/g'
find ${graphdir} "extremes*.txt" | xargs perl -pi -e 's/diassrv8/Xeon80/g'
find ${graphdir} "extremes*.txt" | xargs perl -pi -e 's/lpd48core/Opteron/g'
find ${graphdir} "extremes*.txt" | xargs perl -pi -e 's/lpdxeon2680/Xeon20/g'
find ${graphdir} "extremes*.txt" | xargs perl -pi -e 's/ol-collab1/T4-4/g'


#R -f ${graphdir}general_data.r --args ${graphdir}extremes_${s}.txt ${graphdir}common_${s}.txt ${graphdir}${s}.pdf
R -f ${graphdir}bar_ratio.r --args ${graphdir}extremes_${s}.txt ${graphdir}common_${s}.txt ${graphdir}${s}_bar.pdf
done
