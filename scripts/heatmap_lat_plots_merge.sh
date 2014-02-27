#!/bin/bash

structures="ll ht sl bst";
unames="maglite lpd48core lpdxeon2680 parsasrv1.epfl.ch diassrv8 lpdpc4 ol-collab1"
if [ $# -ge 1 ];
then
    unames="$@";
    echo "**Creating plots for: $unames";
fi;
inits="256 1024 2048 8192 65536"
ops="get put rem"


plots_folder=./plots
cd $plots_folder;

#per size
for unm in $unames
do
    any_pdf=0;
    for structure in $structures;
    do
    for op in $ops; do
    
    ap=0;
	pdfs=$(ls -1 ${unm}_${structure}_heatmap_uc_lat_${op}*.pdf | sort -V);
	if [ "$pdfs" != "" ];
	then
	    any_pdf=1;
        ap=1;
	    all_sep="${unm}_${structure}_heatmap_lat_${op}_all_sep.pdf";
	    pdftk $pdfs cat output $all_sep;
	    pdfnup --a4paper --nup 3x2 --outfile "${unm}_${structure}_heatmap_lat_${op}_all.pdf" $all_sep;
	fi;
    if [ $ap -eq 1 ]; then
	    all_ops="${unm}_${structure}_heatmap_lat_*_all.pdf";
	    pdftk ${all_ops} cat output ${unm}_${structure}_heatmap_lat_all.pdf;
    fi;	
    done;
    done;

    if [ $any_pdf -eq 1 ];
    then
	all=$(ls ${unm}_*_heatmap_lat_all.pdf);
	pdftk $all cat output ${unm}_heatmap_lat_all.pdf
    fi;

done

echo "**Created  plots for: $unames";
