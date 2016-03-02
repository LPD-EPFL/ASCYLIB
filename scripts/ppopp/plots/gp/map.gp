load 'gp/style.gp'
set macros
NOYTICS = "set format y ''; unset ylabel"
YTICS = "set ylabel 'Throughput (Mops/s)' offset 2"
PSIZE = "set size 0.5, 0.6"
PSIZE_LARGE = "set size 0.9, 0.6"

set key horiz maxrows 1

set output "eps/map.eps"

set terminal postscript color "Helvetica" 24 eps enhanced
set rmargin 0
set lmargin 3
set tmargin 3
set bmargin 2.5

xrange_start   = 1
title_offset   = 0
top_row_y      = 0.48
bottom_row_y   = -0.066
graphs_x_offs  = 0.1
plot_size_x    = 1.119
plot_size_y    = 1.08

DIV              =    1e6
FIRST            =    2
OFFSET           =    3
column_select(i) = column(FIRST + (i*OFFSET)) / (DIV);

LINE0 = '"mcs"'
LINE1 = '"optik"'

ncols="0 1 2 3 4 6"
ncol(i)=word(ncols, i+1);


PLOT0 = '"Small map\n{/*0.8(4 elements, 10% updates)} "'
PLOT1 = '"Large map\n{/*0.8(1024 elements, 10% updates)} "'
PLOT3 = '"{/*0.8(on 10 threads)}"'
PLOT4 = '"{/*0.8(on 10 threads)}"'

# font "Helvetica Bold"
# set label 1 "Opteron" at screen 0.018, screen 0.18 rotate by 90 font ',30' textcolor rgb "red"
# set label 2 "Xeon"    at screen 0.018, screen 0.66 rotate by 90 font ',30' textcolor rgb "red"

LDI_FILE0 = '"data/lpdxeon2680.map.ldi.n10.i4.u20.dat"'
LDI_FILE1 = '"data/lpdxeon2680.map.ldi.n10.i1024.u20.dat"'

# ##########################################################################################
# XEON #####################################################################################
# ##########################################################################################


FILE0 = '"data/lpdxeon2680.map.i4.u20.dat"'
FILE1 = '"data/lpdxeon2680.map.i1024.u20.dat"'

set xlabel "# Threads" offset 0, 0.9 font ",28"
set xrange [xrange_start:61]
set xtics ( xrange_start, 10, 20, 30, 40, 50, 60 ) offset 0,0.4
unset key


set size plot_size_x, plot_size_y
set multiplot layout 5, 2

set size 0.5, 0.6
set origin 0.0 + graphs_x_offs, top_row_y
set title @PLOT0 offset 0.2,title_offset font ",28"
set ylabel 'Throughput (Mops/s)' offset 0.5,0
set ytics 20
plot \
     @FILE0 using 1:(column_select(ncol(0))) title @LINE0 ls 1 with linespoints, \
     "" using 1:(column_select(ncol(1))) title @LINE1 ls 2 with linespoints

set origin 0.5 + graphs_x_offs, top_row_y
@PSIZE
set lmargin 4
@YTICS
set ylabel ""
unset ylabel
set title @PLOT1
set ytics 1 offset 0.5
# set key inside top right font ",28"
plot \
     @FILE1 using 1:(column_select(ncol(0))) title @LINE0 ls 1 with linespoints, \
     "" using 1:(column_select(ncol(1))) title @LINE1 ls 2 with linespoints


# ##########################################################################################
# LDI ######################################################################################
# ##########################################################################################

ldi_pos_x=1.59

set origin 0.0 + graphs_x_offs - 0.018, bottom_row_y
set size 0.5 + 0.018, 0.6
# @PSIZE_LARGE
unset title
# set title @PLOT3 offset 0.2,title_offset-0.3 font ",28"
@YTICS
set ylabel ""
unset ylabel
set style fill solid 0.3 border -1
set style boxplot fraction 1
set style data boxplot
set style boxplot nooutliers

set boxwidth 0.8
set pointsize 0.5
set xrange [*:*]
set xtics auto
at(x)=x;
cs(x)=column(x);
set xtics ("mcs" 3.5, "optik" 10.5 ) scale 0.0
# set xtics rotate by -45 # 29 offset -2.2,-1.2 # -45 

DIV=1000
column_left(i)=column(2 + (i-1)*6)/DIV
column_right(i)=column(3 + (i-1)*6)/DIV
column_keep_left(i)=column(i)/DIV
column_keep_right(i)=column(6+i)/DIV


perc="05% 25% 50% 75% 95%"
nperc(i)=word(perc, i+1);

do for [v=15:35:5] {
set arrow v from 3.2,v to 2.2,v
set label v nperc((v-15)/5) at 3.3,v font ',21'
}

set obj 4 rect at 3,25 size 5,28 lw 2

set ylabel "Latency distribution\n(Kcycles)" offset 2
unset xlabel
bnv=6
set ytics 10
plot for [i=1:bnv] @LDI_FILE0 \
     using (i):(column_keep_left(i)) ls (i*10) pt 7 ps 0.5 notitle,\
     for [i=1:bnv] '' \
     using (i+7):(column_keep_right(i)) ls (i*10) pt 7 ps 0.5 notitle,\
     "data/ldi_example" using (1.5):(column_keep_left(1)) ls 10 pt 7 ps 0.5 notitle


do for [v=15:35:5] {
unset arrow v
unset label v
}
unset obj 4

set origin 0.5 + graphs_x_offs, bottom_row_y
set size 0.5, 0.6

# unset title
# set ylabel "Latency distribution\n(Kcycles)" offset 2
# unset xlabel
# unset ylabel
# set title @PLOT4 offset 0.2,title_offset-0.3 font ",28"
unset xlabel
unset ylabel
bnv=6
set ytics 100 offset 0.7
plot for [i=1:bnv] @LDI_FILE1 \
     using (i):(column_keep_left(i)) ls (i*10) pt 7 ps 0.5 notitle,\
     for [i=1:bnv] '' \
     using (i+7):(column_keep_right(i)) ls (i*10) pt 7 ps 0.5 notitle

# set title @PLOT4 offset 0.2,title_offset font ",28"
# plot for [i=1:bnv] @LDI_FILE2 \
#      using (i-ldi_xoffs):(column_left(i)) ls 10 pt 7 ps 0.5 notitle,\
#      for [i=1:bnv] '' \
#      using (i+ldi_xoffs):(column_right(i)) ls 40 pt 7 ps 0.5 notitle

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
set key samplen 3
set key width -1
set key at screen 0.6, screen 0.93 right top

# We need to set an explicit xrange.  Anything will work really.
set xrange [-1:1]
@NOYTICS
set yrange [-1:1]
plot \
     NaN title @LINE0 ls 1 with linespoints, \
     NaN title @LINE1 ls 2 with linespoints

unset title
unset tics
unset xlabel
unset ylabel
set yrange [0:1]
set origin 0.0, 1.1
set size 2,0.2
set key default
set key spacing 1.5
set key font ",25"
# set key box
set key horiz
set key width -2.4
set key samplen 1
set key at screen 0.07, screen 0.487 left top

plot \
     NaN t "srch-suc" ls 10 with boxes, \
     NaN t "insr-suc" ls 20 with boxes, \
     NaN t "delt-suc" ls 30 with boxes, \
     NaN t "srch-fal" ls 40 with boxes, \
     NaN t "insr-fal" ls 50 with boxes, \
     NaN t "delt-fal" ls 60 with boxes

#</null>
unset multiplot  #<--- Necessary for some terminals, but not postscript I don't thin

