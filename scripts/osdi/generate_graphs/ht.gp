set macros
NOYTICS = "set format y ''; unset ylabel"
YTICS = "set ylabel 'Throughput (Mops/s)'"

set key horiz maxrows 1
#set key box


set output "ht.eps"
set terminal postscript color "Helvetica" 22 eps enhanced
set rmargin 0
set lmargin 3
set tmargin 3
set bmargin 2.5
set xlabel "# Threads" offset 1.5, 0.75
set xrange [0:]
set xtics 16 
#set xtics nomirror scale 2
#set ytics nomirror scale 2
#set style line 12 lc rgb '#808080' lt 2 lw 1
#set grid back ls 12 
#set ytics 10
#set style line 7 lc rgb '#0060ad' lt 1 pt 2 ps 1.2 lw 2 pi 6 
#set style line 2 lc rgb '#dd181f' lt 1 pt 5 ps 1.2 lw 2 pi 8
#set style line 3 lc rgb '#8b1a0e' pt 4 ps 1.2 lt 1 lw 2 pi 7
#set style line 4 lc rgb '#5e9c36' pt 11 ps 1.2 lt 1 lw 2 pi 9
#set style line 5 lc rgb '#663399' lt 1 pt 3 ps 1.2 lw 2 pi 10
#set style line 8 lc rgb '#299fff' lt 1 pt 6 ps 1.2 lw 2 pi 12
#set style line 9 lc rgb '#ff299f' lt 1 pt 9 ps 1.2 lw 2 pi 11
#set style line 6 lc rgb '#cc6600' lt 1 pt 15 ps 1.2 lw 2 pi 13
#set style line 1 lc rgb '#4d2600' lt 1 pt 7 ps 1.2 lw 2 pi 14

set style line 1 lc rgb '#0060ad' lt 1 pt 2 ps 1.6 lw 5 pi 3 
set style line 2 lc rgb '#dd181f' lt 1 pt 5 ps 1.6 lw 5 pi 3
set style line 3 lc rgb '#8b1a0e' lt 2 pt 1 ps 1.6 lw 5 pi 3
set style line 4 lc rgb '#5e9c36' lt 1 pt 6 ps 1.6 lw 5 pi 3
set style line 5 lc rgb '#663399' lt 2 pt 3 ps 1.6 lw 5 pi 3
set style line 6 lc rgb '#cd1eff' lt 1 pt 4 ps 1.6 lw 5 pi 3
set style line 7 lc rgb '#cc6600' lt 2 pt 7 ps 1.6 lw 5 pi 3
set style line 8 lc rgb '#299fff' lt 1 pt 8 ps 1.6 lw 5 pi 3
set style line 9 lc rgb '#ff299f' lt 2 pt 9 ps 1.6 lw 5 pi 3

set style line 12 lc rgb '#808080' lt 2 lw 1
title_offset=-0.7

# solid lines
set style line 10 lt 1 lw 2 lc 1
set style line 20 lt 1 lw 2 lc 2
set style line 30 lt 1 lw 2 lc 3

set size 2.595, 0.61
set multiplot layout 5, 1
set size 0.5, 0.6
set origin 0.05, 0.0
unset key
set xrange [0:49]
set yrange [0:]
set xtics 6 offset 0,0.4
set ytics 40 offset 0.7
set ylabel "Throughput (Mops/s)" offset 2.5
set title "Opteron" offset 0.2,title_offset
@YTICS
plot \
     "common_gp_ht_lpd48core.txt" using 1:($10) title "async" ls 1 with linespoints, \
     "" using 1:($6) title "coupling" ls 9 with linespoints, \
     "" using 1:($5) title "lazy" ls 2 with linespoints, \
     "" using 1:($7) title "pugh" ls 3 with linespoints, \
     "" using 1:($4) title "copy" ls 4 with linespoints, \
     "" using 1:($9) title "urcu" ls 5 with linespoints, \
     "" using 1:($3) title "java" ls 6 with linespoints, \
     "" using 1:($2) title "tbb" ls 8 with linespoints, \
     "" using 1:($8) title "harris" ls 7 with linespoints
set origin 0.55, 0.0
unset key
set lmargin 4
set size 0.5, 0.6
set xrange [0:41]
set yrange [0:]
set xtics 10
set ytics 100
@YTICS
set ylabel ""
unset ylabel
set title "Xeon20"
plot \
     "common_gp_ht_lpdxeon2680.txt" using 1:($10) title "async" ls 1 with linespoints, \
     "" using 1:($6) title "coupling" ls 9 with linespoints, \
     "" using 1:($5) title "lazy" ls 2 with linespoints, \
     "" using 1:($7) title "pugh" ls 3 with linespoints, \
     "" using 1:($4) title "copy" ls 4 with linespoints, \
     "" using 1:($9) title "urcu" ls 5 with linespoints, \
     "" using 1:($3) title "java" ls 6 with linespoints, \
     "" using 1:($2) title "tbb" ls 8 with linespoints, \
     "" using 1:($8) title "harris" ls 7 with linespoints
set origin 1.05, 0.0
set size 0.5, 0.6
set xrange [0:81]
set yrange [0:]
set xtics 20
set ytics 100
@YTICS
set ylabel ""
unset ylabel
set title "Xeon40"
plot \
     "common_gp_ht_diassrv8.txt" using 1:($10) title "async" ls 1 with linespoints, \
     "" using 1:($6) title "coupling" ls 9 with linespoints, \
     "" using 1:($5) title "lazy" ls 2 with linespoints, \
     "" using 1:($7) title "pugh" ls 3 with linespoints, \
     "" using 1:($4) title "copy" ls 4 with linespoints, \
     "" using 1:($9) title "urcu" ls 5 with linespoints, \
     "" using 1:($3) title "java" ls 6 with linespoints, \
     "" using 1:($2) title "tbb" ls 8 with linespoints, \
     "" using 1:($8) title "harris" ls 7 with linespoints
set origin 1.55, 0.0
set size 0.5, 0.6
set title "Tilera"
set xrange [0:35]
set yrange [0:]
set xtics 6
set ytics 50
@YTICS
set ylabel ""
unset ylabel
#set key bottom right
plot \
     "common_gp_ht_parsasrv1.epfl.ch.txt" using 1:($8) title "async" ls 1 with linespoints, \
     "" using 1:($5) title "coupling" ls 9 with linespoints, \
     "" using 1:($4) title "lazy" ls 2 with linespoints, \
     "" using 1:($6) title "pugh" ls 3 with linespoints, \
     "" using 1:($3) title "copy" ls 4 with linespoints, \
     "" using 1:($2) title "java" ls 6 with linespoints, \
     "" using 1:($7) title "harris" ls 7 with linespoints
set origin 2.05, 0.0
set size 0.5, 0.6
set title "T4-4"
set xrange [0:257]
set xtics 64
set ytics 200
set yrange [0:]
@YTICS
#set key bottom right
set ylabel ""
unset ylabel
plot \
     "common_gp_ht_ol-collab1.txt" using 1:($8) title "async" ls 1 with linespoints, \
     "" using 1:($5) title "coupling" ls 9 with linespoints, \
     "" using 1:($4) title "lazy" ls 2 with linespoints, \
     "" using 1:($6) title "pugh" ls 3 with linespoints, \
     "" using 1:($3) title "copy" ls 4 with linespoints, \
     "" using 1:($2) title "java" ls 6 with linespoints, \
     "" using 1:($7) title "harris" ls 7 with linespoints
unset origin
unset border
unset tics
unset xlabel
unset label
unset arrow
unset title
unset object

#Now set the size of this plot to something BIG
set size 2.595,0.61 #however big you need it
set origin 0.0, 0.0

#example key settings
set key box 
#set key horizontal reverse samplen 1 width -4 maxrows 1 maxcols 12 
#set key at screen 0.5,screen 0.25 center top
set key above width -3 vertical maxrows 1

set key at screen 1.3, screen 0.585 center top
#We need to set an explicit xrange.  Anything will work really.
set xrange [-1:1]
@NOYTICS
set yrange [-1:1]
plot \
     NaN title "async" ls 1 with linespoints, \
     NaN title "coupling" ls 9 with linespoints, \
     NaN title "lazy" ls 2 with linespoints, \
     NaN title "pugh" ls 3 with linespoints, \
     NaN title "copy" ls 4 with linespoints, \
     NaN title "urcu" ls 5 with linespoints, \
     NaN title "java" ls 6 with linespoints, \
     NaN title "tbb" ls 8 with linespoints, \
     NaN title "harris" ls 7 with linespoints
#</null>
unset multiplot  #<--- Necessary for some terminals, but not postscript I don't thin
