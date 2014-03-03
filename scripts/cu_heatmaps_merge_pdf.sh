#!/bin/bash

#param1: list of platoform(s), e.g., "maglite lpdpc4"
#param2: folder of plots to merge, e.g., "archive/something/plots"

structures="ll ht sl bst";
unames="maglite lpd48core lpdxeon2680 parsasrv1.epfl.ch diassrv8 lpdpc4 ol-collab1"
if [ $# -ge 1 ];
then
    unames="$1";
    echo "**Creating plots for: $unames";
fi;

plots_folder=./plots

if [ $# -ge 2 ];
then
    plots_folder="$2";
    echo "**Merging plots in: $plots_folder";
fi;

cd $plots_folder;

inits="256 1024 2048 8192 65536"


#per size
for unm in $unames
do
    any_pdf=0;
    any_pdf_scal=0;
    for structure in $structures;
    do
    #pdfs=$(ls -1 ${unm}_${structure}_heatmap_uc_*.pdf | sort -V);
    # might confuse these with some latency heatmaps
    pdfs=$(ls -1 ${unm}_${structure}_heatmap_uc_*.pdf | grep -v "lat" | sort -V);
	pdfs_scal=$(ls -1 ${unm}_${structure}_heatmap_scal_*.pdf | sort -V);
	if [ "$pdfs" != "" ];
	then
	    any_pdf=1;
	    all_sep="${unm}_${structure}_heatmap_all_sep.pdf";
	    pdftk $pdfs cat output $all_sep;
	    pdfnup --a4paper --nup 3x2 --outfile "${unm}_${structure}_heatmap_all.pdf" $all_sep;
	fi;
	if [ "$pdfs_scal" != "" ];
	then
	    any_pdf_scal=1;
	    all_sep_scal="${unm}_${structure}_heatmap_all_sep_scal.pdf";
	    pdftk ${pdfs_scal} cat output ${all_sep_scal};
	    pdfnup --a4paper --nup 3x2 --outfile "${unm}_${structure}_heatmap_all_scal.pdf" ${all_sep_scal};
	fi;

    done;

    if [ $any_pdf -eq 1 ];
    then
	all=$(ls ${unm}_*_heatmap_all.pdf);
	echo "** merging: "$all;
	pdftk $all cat output ${unm}_heatmap_all.pdf
    fi;
    if [ $any_pdf_scal -eq 1 ];
    then
	all_scal=$(ls ${unm}_*_heatmap_all_scal.pdf);
	echo "** merging: "$all_scal;
	pdftk ${all_scal} cat output ${unm}_heatmap_all_scal.pdf
    fi;

done

echo "**Created  plots for: $unames";
