#!/bin/bash

structures="ll ht sl bst";
unames="maglite lpd48core lpdxeon2680 parsasrv1.epfl.ch diassrv8"
inits="256 1024 2048 8129 8192 65536"


plots_folder=./plots
cd $plots_folder;

#per size
for unm in $unames
do
    any_pdf=0;
    for structure in $structures;
    do
	pdfs=$(ls -1 ${unm}_${structure}_heatmap_uc_*.pdf | sort -V);
	if [ "$pdfs" != "" ];
	then
	    any_pdf=1;
	    all_sep="${unm}_${structure}_heatmap_all_sep.pdf";
	    pdftk $pdfs cat output $all_sep;
	    pdfnup --a4paper --nup 3x2 --outfile "${unm}_${structure}_heatmap_all.pdf" $all_sep;
	fi;
    done;

    if [ $any_pdf -eq 1 ];
    then
	all=$(ls ${unm}_*_heatmap_all.pdf);
	pdftk $all cat output ${unm}_heatmap_all.pdf
    fi;
done

    # for initial in ${inits}
    # do

    # done
