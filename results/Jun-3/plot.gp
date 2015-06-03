set term pdf dashed linewidth 4
set key below right

set xlabel "Time (s)"
set ylabel "Throughput (ops/s)" 


infile="./htjava+slfraser"
set output infile.".pdf"


set title "Java HT - Updates = 100%"

plot    infile i 0 using 1:2 title "ssmem" with linespoint ls 2, \
        infile i 1 using 1:2 title "malloc" with linespoint ls 3, \
        infile i 2 using 1:2 title "tcmalloc" with linespoint ls 4, \
        infile i 3 using 1:2 title "jemalloc" with linespoint ls 5


set title "Fraser SL - Updates = 100%"                                

plot    infile i 4 using 1:2 title "ssmem" with linespoint ls 2, \
        infile i 5 using 1:2 title "malloc" with linespoint ls 3, \
        infile i 6 using 1:2 title "tcmalloc" with linespoint ls 4, \
        infile i 7 using 1:2 title "jemalloc" with linespoint ls 5


unset output