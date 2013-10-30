set terminal pdf
set output outfile

set title mytitle
set xlabel "Number of threads"
set ylabel "Throughput (Ops/s)"

plot \
infile using 1:2 title "Ellen (lf)" with linespoint linestyle 1,\
infile using 1:5 title "Howley (lf)" with linespoint linestyle 2,\
infile using 1:8 title "Bronson (lb)" with linespoint linestyle 3



