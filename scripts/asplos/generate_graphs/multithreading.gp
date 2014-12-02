set xlabel "Number of threads" offset 0,0.45

set terminal postscript color  "Helvetica" 41 eps enhanced

set output "multithreading.eps"

title_offset = -0.5
xsize=1
ysize=0.8
xsize_plot=1.0
xpos_cor=-0.04
xldi_size=xsize_plot+0.19
yldi_size=ysize-0.05

set style line 1 lc rgb '#0060ad' lt 1 pt 2 ps 1.2 lw 2 pi 6 
set style line 2 lc rgb '#dd181f' lt 1 pt 5 ps 1.2 lw 2 pi 6
set style line 3 lc rgb '#8b1a0e' lt 2 pt 1 ps 1.2 lw 2 pi 5
set style line 4 lc rgb '#5e9c36' lt 1 pt 6 ps 1.2 lw 2 pi 5
set style line 5 lc rgb '#663399' lt 2 pt 3 ps 1.2 lw 2 pi 6
set style line 6 lc rgb '#cd1eff' lt 1 pt 4 ps 1.2 lw 2 pi 6
set style line 7 lc rgb '#cc6600' lt 2 pt 7 ps 1.2 lw 2 pi 5
set style line 8 lc rgb '#299fff' lt 1 pt 8 ps 1.2 lw 2 pi 5
set style line 9 lc rgb '#ff299f' lt 2 pt 9 ps 1.2 lw 2 pi 6

point_size=1.75
pi1=9
pi2=10

set style line 11 lc rgb '#0060ad' lt 1 pt 2 ps point_size lw 5 pi pi1
set style line 21 lc rgb '#dd181f' lt 1 pt 5 ps point_size lw 5 pi pi2
set style line 31 lc rgb '#8b1a0e' lt 2 pt 1 ps point_size lw 5 pi pi1
set style line 41 lc rgb '#5e9c36' lt 1 pt 6 ps point_size lw 5 pi pi2
set style line 51 lc rgb '#663399' lt 2 pt 3 ps point_size lw 5 pi pi1
set style line 61 lc rgb '#cd1eff' lt 1 pt 4 ps point_size lw 5 pi pi2
set style line 71 lc rgb '#cc6600' lt 2 pt 7 ps point_size lw 5 pi pi1
set style line 81 lc rgb '#299fff' lt 1 pt 8 ps point_size lw 5 pi pi2
set style line 91 lc rgb '#ff299f' lt 2 pt 9 ps point_size lw 5 pi pi1


# solid lines
set style line 10 lt 1 lw 2 lc 1
set style line 20 lt 1 lw 2 lc 2
set style line 30 lt 1 lw 2 lc 3
set style line 40 lt 1 lw 2 lc 9
# set size 1,0.6
set size xsize, ysize
set multiplot

set xrange[0:201]
set xtics 40
#nomirror - no tics on the right and top
#scale - the size of the tics
# set xtics 6 nomirror scale 2
# set ytics auto nomirror scale 2

#remove top and right borders
# set border 3 back
#add grid
# set grid back ls 12

# set xrange [0:48]
# set yrange [0:150]
# set logscale y 10

#the size of the graph

#move the legend to a custom position (can also be moved to absolute coordinates)
# set key at 11,18
unset key
# set key outside top
#set term tikz standalone color solid size 5in,3in
#set output "test.tex"

#for more details on latex, see http://www.gnuplotting.org/introduction/output-terminals/
#set term epslatex #size 3.5, 2.62 #color colortext
#size can also be in cm: set terminal epslatex size 8.89cm,6.65cm color colortext
#set output "test.eps"

################################################################################
## Throughput
################################################################################

unset title
# set title "Total Throughput" offset 0,title_offset
set ylabel "Throughput\n(Mops/s)" offset 1.5
set origin 0,0

DIV=1			# Mops

FIRST=4
OFFSET=3

column_select(i) = column(FIRST + (i*OFFSET)) / DIV;

plot \
     "multithreading_ht_lpdxeon2680.txt" using 1:($10) title "seq" ls 1 with linespoints, \
     "" using 1:($6) title "coupling" ls 9 with linespoints, \
     "" using 1:($5) title "lazy" ls 2 with linespoints, \
     "" using 1:($7) title "pugh" ls 3 with linespoints, \
     "" using 1:($4) title "copy" ls 4 with linespoints, \
     "" using 1:($9) title "urcu" ls 5 with linespoints, \
     "" using 1:($3) title "lea" ls 6 with linespoints, \
     "" using 1:($2) title "tbb" ls 8 with linespoints, \
     "" using 1:($8) title "harris-opt" ls 7 with linespoints, \
     "" using 1:($11) title "clht-lb" ls 61 with linespoints, \
     "" using 1:($2) title "clht-lf" ls 71 with linespoints

unset multiplot  #<--- Necessary for some terminals, but not postscript I don't thin


