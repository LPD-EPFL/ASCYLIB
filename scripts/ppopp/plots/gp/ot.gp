load 'gp/style.gp'
set macros
NOYTICS = "set format y ''; unset ylabel"
YTICS = "set ylabel 'Throughput (Mops/s)' offset 2"
PSIZE = "set size 0.5, 0.6"
PSIZE_LARGE = "set size 0.9, 0.6"

set key horiz maxrows 1

set output "eps/optik_test.eps"

set terminal postscript color "Helvetica" 24 eps enhanced
set rmargin 0
set lmargin 3
set tmargin 3
set bmargin 2.5

set xlabel "# Threads" offset 0, 0.75 font ",28"

xrange_start   = 1
title_offset   = -0.5
top_row_y      = 0.0
bottom_row_y   = 0.0
graphs_x_offs  = 0.03
plot_size_x    = 1.119
plot_size_y    = 0.57

DIV              =    1e6
FIRST            =    2
OFFSET           =    4
column_select(i) = column(FIRST + (i*OFFSET)) / (DIV);

LINE0 = '"ttas"'
LINE1 = '"optik-ticket"'
LINE2 = '"optik-versioned"'

ncols="0 1 2 3 4 6"
ncol(i)=word(ncols, i+1);

# PLOT0 = '"Small map\n{/*0.8(4 elements, 20% updates)} "'
# PLOT1 = '"Large map\n{/*0.8(1024 elements, 20% updates)} "'

# ##########################################################################################
# XEON #####################################################################################
# ##########################################################################################

FILE0 = '"data/lpdxeon2680.ot.i1.u0.dat"'
FILE1 = FILE0

set xrange [xrange_start:61]
set xtics ( xrange_start, 10, 20, 30, 40, 50, 60 ) offset 0,0.4
unset key


set size plot_size_x, plot_size_y
set multiplot layout 5, 2

set size 0.5, 0.6
set origin 0.0 + graphs_x_offs, top_row_y
set ylabel 'Throughput (Mops/s)' offset 1,-0.5
set yrange [*:8]
set ytics 2
plot \
     @FILE0 using 1:(column_select(ncol(0))) title @LINE0 ls 1 with linespoints, \
     "" using 1:(column_select(ncol(1))) title @LINE1 ls 2 with linespoints, \
     "" using 1:(column_select(ncol(2))) title @LINE2 ls 3 with linespoints


DIV              =    1
FIRST            =    5
OFFSET           =    4
column_select(i) = column(FIRST + (i*OFFSET)) / (DIV);

set origin 0.5 + 3*graphs_x_offs, top_row_y
@PSIZE
set lmargin 4
@YTICS
set ylabel '# CAS per validation' offset 2,-0.5
set yrange [*:*]
set ytics 12 offset 0.5
# set title @PLOT1
plot \
     @FILE1 using 1:(column_select(ncol(0))) title @LINE0 ls 1 with linespoints, \
     "" using 1:(column_select(ncol(1))) title @LINE1 ls 2 with linespoints, \
     "" using 1:(column_select(ncol(2))) title @LINE2 ls 3 with linespoints


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
set size 2*plot_size_x, plot_size_y #however big you need it
set origin 0.0, 1.1

#example key settings
# set key box 
#set key horizontal reverse samplen 1 width -4 maxrows 1 maxcols 12 
#set key at screen 0.5,screen 0.25 center top
set key font ",28"
set key spacing 1.5
set key horiz
set key at screen 0.47, screen 0.57 center top

#We need to set an explicit xrange.  Anything will work really.
set xrange [-1:1]
@NOYTICS
set yrange [-1:1]
plot \
     NaN title @LINE0 ls 1 with linespoints, \
     NaN title @LINE1 ls 2 with linespoints, \
     NaN title @LINE2 ls 3 with linespoints

#</null>
unset multiplot  #<--- Necessary for some terminals, but not postscript I don't thin

