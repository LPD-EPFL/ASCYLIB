load 'gp/style.gp'
set macros
NOYTICS = "set format y ''; unset ylabel"
YTICS = "set ylabel 'Throughput (Mops/s)' offset 2"
PSIZE = "set size 0.5, 0.6"
PSIZE_LARGE = "set size 0.9, 0.6"

set key horiz maxrows 1

set output "eps/fig12_qu.eps"

set terminal postscript color "Helvetica" 24 eps enhanced
set rmargin 0
set lmargin 3
set tmargin 3
set bmargin 2.5

xrange_start   = 1
title_offset   = -0.5
top_row_y      = 0.0
bottom_row_y   = 0.0
graphs_x_offs  = 0.1
plot_size_x    = 2.615
plot_size_y    = 0.67

DIV              =    1e6
FIRST            =    2
OFFSET           =    3
column_select(i) = column(FIRST + (i*OFFSET)) / (DIV);

LINE0 = '"ms-lf"'
LINE1 = '"ms-lb"'
LINE2 = '"optik0"'
LINE3 = '"optik1"'
LINE4 = '"optik2"'
LINE5 = '"optik3"'

ncols="0 1 2 3 4 5"
ncol(i)=word(ncols, i+1);

PLOT0 = '"Decreasing size\n{/*0.8(40% enqueue, 60% dequeue)} "'
PLOT1 = '"Stable size\n{/*0.8(50% enqueue, 50% dequeue)}"'
PLOT2 = '"Increasing size\n{/*0.8(60% enqueue, 40% dequeue)}"'
PLOT3 = '"Stable size\n{/*0.8(on 10 threads)}"'


LDI_FILE0 = '"data/data.qu.ldi.n10.p50.dat"'

# ##########################################################################################
# XEON #####################################################################################
# ##########################################################################################


FILE0 = '"data/data.qu.thr.p40.dat"'
FILE1 = '"data/data.qu.thr.p50.dat"'
FILE2 = '"data/data.qu.thr.p60.dat"'

unset xlabel
set xrange [*:*]
set xlabel "# Threads" offset 0, 0.75 font ",28"
set xtics offset 0,0.4
unset key


set size plot_size_x, plot_size_y
set multiplot layout 5, 2

set size 0.5, 0.6
set origin 0.0 + graphs_x_offs, top_row_y
set title @PLOT0 offset 0.2,title_offset font ",28"
set ylabel 'Throughput (Mops/s)' offset 1,0.5
set yrange [0:*]
plot \
     @FILE0 using 1:(column_select(ncol(0))) title @LINE0 ls 1 with linespoints, \
     "" using 1:(column_select(ncol(1))) title @LINE1 ls 2 with linespoints, \
     "" using 1:(column_select(ncol(2))) title @LINE2 ls 3 with linespoints, \
     "" using 1:(column_select(ncol(3))) title @LINE3 ls 4 with linespoints, \
     "" using 1:(column_select(ncol(4))) title @LINE4 ls 8 with linespoints, \
     "" using 1:(column_select(ncol(5))) title @LINE5 ls 5 with linespoints

set origin 0.5 + graphs_x_offs, top_row_y
@PSIZE
set lmargin 4
@YTICS
set ylabel ""
unset ylabel
set title @PLOT1
plot \
     @FILE1 using 1:(column_select(ncol(0))) title @LINE0 ls 1 with linespoints, \
     "" using 1:(column_select(ncol(1))) title @LINE1 ls 2 with linespoints, \
     "" using 1:(column_select(ncol(2))) title @LINE2 ls 3 with linespoints, \
     "" using 1:(column_select(ncol(3))) title @LINE3 ls 4 with linespoints, \
     "" using 1:(column_select(ncol(4))) title @LINE4 ls 8 with linespoints, \
     "" using 1:(column_select(ncol(5))) title @LINE5 ls 5 with linespoints


set origin 1.0 + graphs_x_offs, top_row_y
@PSIZE
@YTICS
set ylabel ""
unset ylabel
set title @PLOT2
plot \
     @FILE2 using 1:(column_select(ncol(0))) title @LINE0 ls 1 with linespoints, \
     "" using 1:(column_select(ncol(1))) title @LINE1 ls 2 with linespoints, \
     "" using 1:(column_select(ncol(2))) title @LINE2 ls 3 with linespoints, \
     "" using 1:(column_select(ncol(3))) title @LINE3 ls 4 with linespoints, \
     "" using 1:(column_select(ncol(4))) title @LINE4 ls 8 with linespoints, \
     "" using 1:(column_select(ncol(5))) title @LINE5 ls 5 with linespoints

set yrange [*:*]

# ##########################################################################################
# LDI ######################################################################################
# ##########################################################################################

set yrange [*:*]
ldi_pos_x=1.59

set origin ldi_pos_x + graphs_x_offs, top_row_y
@PSIZE_LARGE
set title "Stable size\n{/*0.8(on 10 threads)} " offset 0.2,title_offset font ",28"
@YTICS
set ylabel ""
unset ylabel
set style fill solid 0.3 border -1
set style boxplot fraction 1
set style data boxplot
set style boxplot nooutliers

set boxwidth 0.3
set pointsize 0.5
set xrange [xrange_start:*]
set xtics auto
at(x)=x;
cs(x)=column(x);
set xtics ("ms-lf" 1, "ms-lb" 2, "optik0" 3, "optik1" 4, "optik2" 5, "optik3" 6) scale 0.0
# set xtics rotate by -45 # 29 offset -2.2,-1.2 # -45 

DIV=1000
column_left(i)=column(2 + (i-1)*6)/DIV
column_right(i)=column(3 + (i-1)*6)/DIV

set ylabel "Latency distribution\n(Kcycles)" offset 2
unset xlabel
ldi_xoffs=0.20
bnv=6
set ytics offset 0.5
plot for [i=1:bnv] 'data/data.qu.ldi.n10.p50.dat' \
     using (i-ldi_xoffs):(column_left(i)) ls 10 pt 7 ps 0.5 notitle,\
     for [i=1:bnv] '' \
     using (i+ldi_xoffs):(column_right(i)) ls 40 pt 7 ps 0.5 notitle

# ##########################################################################################
# LEGENDS ##################################################################################
# ##########################################################################################

unset origin
unset border
unset tics
unset xlabel
unset label
unset arrow
unset title
unset object

#Now set the size of this plot to something BIG
set size plot_size_x, plot_size_y #however big you need it
set origin 0.0, 1.1

#example key settings
# set key box
#set key horizontal reverse samplen 1 width -4 maxrows 1 maxcols 12 
#set key at screen 0.5,screen 0.25 center top
set key font ",28"
set key spacing 1.5
set key horiz
set key at screen 1.6, screen 0.65 right top

#We need to set an explicit xrange.  Anything will work really.
set xrange [-1:1]
@NOYTICS
set yrange [-1:1]
plot \
     NaN title @LINE0 ls 1 with linespoints, \
     NaN title @LINE1 ls 2 with linespoints, \
     NaN title @LINE2 ls 3 with linespoints, \
     NaN title @LINE3 ls 4 with linespoints, \
     NaN title @LINE4 ls 8 with linespoints, \
     NaN title @LINE5 ls 5 with linespoints


unset title
unset tics
unset xlabel
unset ylabel
set yrange [0:1]
set origin 0.0, 1.1
set size 2,0.2
set key default
set key spacing 1.5
set key font ",28"
# set key box
set key horiz
set key width 0
set key samplen 2
set key at screen 1.95, screen 0.65 left top

plot \
     NaN t "enqueue" ls 10 with boxes, \
     NaN t "dequeue" ls 40 with boxes

#</null>
unset multiplot  #<--- Necessary for some terminals, but not postscript I don't thin

