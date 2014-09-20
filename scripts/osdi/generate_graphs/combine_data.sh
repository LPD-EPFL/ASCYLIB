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
done

#platform names
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/parsasrv1.epfl.ch/Tilera/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/diassrv8/Xeon40/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/lpd48core/Opteron/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/lpdxeon2680/Xeon20/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/ol-collab1/T4-4/g'
#linkedlist
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/ll-seq/async/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/ll-lazy/lazy/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/ll-pugh/pugh/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/ll-copy/copy/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/ll-coupling/coupling/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/ll-harris-opt/harris-opt/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/ll-harris/harris/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/ll-michael/michael/g'
#hash table
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/ht-seq/async/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/ht-coupling/coupling/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/ht-harris/harris/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/ht-lazy/lazy/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/ht-lea/java/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/ht-pugh/pugh/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/ht-copy/copy/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/ht-rcu/urcu/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/ht-tbb/tbb/g'
#skip-list
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/sl-seq/async/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/sl-lf-fraser/fraser/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/sl-lf-herlihy/fraser-opt/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/sl-herlihy/herlihy/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/sl-pugh/pugh/g'
#bst
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/bst-seq-ext/async-ext/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/bst-seq-int/async-int/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/bst-bronson/bronson/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/bst-drachsler/drachsler/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/bst-ellen/ellen/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/bst-howley/howley/g'
find ${graphdir} -name "extremes*.txt" | xargs perl -pi -e 's/bst-natarajan/natarajan/g'

#generate the graphs
R -f ${graphdir}bar_ll.r --args ${graphdir}extremes_ll.txt ${graphdir}common_ll.txt ${graphdir}ll_bar.pdf
R -f ${graphdir}bar_sl.r --args ${graphdir}extremes_sl.txt ${graphdir}common_sl.txt ${graphdir}sl_bar.pdf
R -f ${graphdir}bar_ht.r --args ${graphdir}extremes_ht.txt ${graphdir}common_ht.txt ${graphdir}ht_bar.pdf
R -f ${graphdir}bar_bst.r --args ${graphdir}extremes_bst.txt ${graphdir}common_bst.txt ${graphdir}bst_bar.pdf
#for s in $structs; do
#R -f ${graphdir}general_data.r --args ${graphdir}extremes_${s}.txt ${graphdir}common_${s}.txt ${graphdir}${s}.pdf
#R -f ${graphdir}bar2.r --args ${graphdir}extremes_${s}.txt ${graphdir}common_${s}.txt ${graphdir}${s}_bar.pdf
#done
