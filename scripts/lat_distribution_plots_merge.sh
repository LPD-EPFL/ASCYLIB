#!/bin/bash
source scripts/lat_distribution.config

if [ $# -ge 1 ];
then
    unames="$@";
    echo "**Creating plots for: $unames";
fi;

plots_folder=./plots
cd $plots_folder;

#per size
for unm in $unames
do
    any_pdf=0;
    for structure in $structs;
    do
    for op in $ops; do
    
    ap=0;
	pdfs=$(ls -1 ${unm}_lat_${structure}_*_${op}.pdf | sort -V);
	if [ "$pdfs" != "" ];
	then
	    any_pdf=1;
        ap=1;
	    all_sep="${unm}_${structure}_lat_${op}_all_sep.pdf";
	    pdftk $pdfs cat output $all_sep;
	    pdfnup --a4paper --nup 4x3 --outfile "${unm}_${structure}_lat_${op}_all.pdf" $all_sep;
	fi;
    if [ $ap -eq 1 ]; then
	    all_ops="${unm}_${structure}_lat_*_all.pdf";
	    pdftk ${all_ops} cat output ${unm}_${structure}_lat_all.pdf;
    fi;	
    done;
    done;

    if [ $any_pdf -eq 1 ];
    then
    all=$(ls -1 ${unm}_*_lat_all.pdf | grep -v "heatmap");
	pdftk $all cat output ${unm}_lat_all.pdf
    fi;

done

echo "**Created  plots for: $unames";
