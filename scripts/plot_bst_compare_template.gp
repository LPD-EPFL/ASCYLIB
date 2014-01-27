set term pdf dashed font "Times,18"
set output outfile

#set title "Key range: 2M; 90% contains, 5% add, 5% remove"
set xlabel "Number of threads"
set ylabel "Throughput (Ops/s)"
#set key top left
unset key

set style line 1  linetype 1 linecolor rgb "black"   linewidth 3.000 pointtype 12
set style line 2  linetype 2 linecolor rgb "blue"    linewidth 3.000 
set style line 3  linetype 3 linecolor rgb "green"   linewidth 3.000
set style line 4  linetype 5 linecolor rgb "red"     linewidth 3.000
set style line 5  linetype 7 linecolor rgb "magenta" linewidth 3.000 
set style line 6  linetype 10 linecolor rgb "cyan"   linewidth 3.000  

plot \
infile using 1:2 title "Ellen (lf)" with linespoint linestyle 1,\
infile using 1:5 title "Howley (lf)" with linespoint linestyle 2,\
infile using 1:8 title "Bronson Mutex (lb)" with linespoint linestyle 3,\
infile using 1:11 title "Bronson Ticket (lb)" with linespoint linestyle 4,\
infile using 1:14 title "Bronson TAS (lb)" with linespoint linestyle 5,\
infile using 1:17 title "Bronson Spin (lb)" with linespoint linestyle 6





