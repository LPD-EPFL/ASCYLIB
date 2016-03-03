load 'gp/style.gp'
set macros
NOYTICS = "set format y ''; unset ylabel"
YTICS = "set ylabel 'Throughput (Mops/s)' offset 2"
PSIZE = "set size 0.5, 0.6"

set key horiz maxrows 1

set output "eps/fig11_sl.eps"

set terminal postscript color "Helvetica" 24 eps enhanced
set rmargin 0
set lmargin 3
set tmargin 3
set bmargin 2.5

n_algo = 5

title_offset   = -0.5
top_row_y      = 0.44
bottom_row_y   = 0.0
graphs_x_offs  = 0.1
plot_size_x    = 1.15
plot_size_y    = 0.66

DIV              =    1e6
FIRST            =    2
OFFSET           =    3
column_select(i) = column(FIRST + (i*OFFSET)) / (DIV);

LINE0 = '"fraser"'
LINE1 = '"herlihy"'
LINE2 = '"herl-optik"'
LINE3 = '"optik1"'
LINE4 = '"optik2"'

PLOT0 = '"Large\n{/*0.8(65536 elements, 20% updates)}"'
PLOT1 = '"Medium\n{/*0.8(16384 elements, 20% updates)}"'
PLOT2 = '"Small\n{/*0.8(1024 elements, 20% updates)}"'
PLOT3 = '"Large skewed\n{/*0.8(65536 elements, 20% updates)}"'
PLOT4 = '"Small skewed\n{/*0.8(1024 elements, 20% updates)}"'

unset xlabel
unset key

set size plot_size_x, plot_size_y
set multiplot layout 5, 2

FILE0 = '"data/data.sl.i65536.u40.w0.dat"'
FILE1 = '"data/data.sl.i16384.u40.w0.dat"'
FILE2 = '"data/data.sl.i1024.u40.w0.dat"'
FILE3 = '"data/data.sl.i65536.u40.w2.dat"'
FILE4 = '"data/data.sl.i1024.u40.w2.dat"'

set xlabel "# Threads" offset 0, 0.75 font ",28"

unset title

set lmargin 3
set ylabel 'Throughput (Mops/s)' offset 2,-0.5
set title @PLOT3 offset 0.2,title_offset font ",28"
@PSIZE
set origin 0.0 + graphs_x_offs, bottom_row_y
set ytics 4
plot for [i=1:n_algo] @FILE3 using ($1):(column(i+1) / DIV) ls i with linespoints

set origin 0.53 + graphs_x_offs, bottom_row_y
@PSIZE
@YTICS
set title @PLOT4
set ylabel ""
unset ylabel
set ytics 3
plot for [i=1:n_algo] @FILE4 using ($1):(column(i+1) / DIV) ls i with linespoints

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
set key width -3
set key samplen 2.5
set key at screen 0.568, screen plot_size_y center top

#We need to set an explicit xrange.  Anything will work really.
set xrange [-1:1]
@NOYTICS
set yrange [-1:1]
plot \
     NaN title @LINE0 ls 1 with linespoints, \
     NaN title @LINE1 ls 2 with linespoints, \
     NaN title @LINE2 ls 3 with linespoints, \
     NaN title @LINE3 ls 4 with linespoints, \
     NaN title @LINE4 ls 5 with linespoints

#</null>
unset multiplot  #<--- Necessary for some terminals, but not postscript I don't thin
