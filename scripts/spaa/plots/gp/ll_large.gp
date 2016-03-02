load 'gp/style.gp'
set macros
NOYTICS = "set format y ''; unset ylabel"

set key horiz maxrows 1

set output "eps/ll_large.eps"

set terminal postscript color "Helvetica" 32 eps enhanced
set rmargin 0
set lmargin 3
set tmargin 3
set bmargin 2.5

n_algo = 5

title_offset   = -0.5
top_row_y      = 0.7
bottom_row_y   = 0.0

graphs_x_offs  = 0.18
right_graph_x  = 0.85

plot_size_x    = 1.9
plot_size_y    = 1.7
fig_size_x     = 0.8
fig_size_y     = 0.9

DIV              =    1e6
FIRST            =    2
OFFSET           =    3


LINE0 = '"GL"'
LINE1 = '"GL-SP_{1}"'
LINE2 = '"GL-SP_{2,5,6}"'
LINE3 = '"FG-SP"'
LINE4 = '"LAZY"'
LINE5 = '"LAZY-1"'

PLOT0 = '"Small List\n{/*0.8(128 elements, 20% updates)}"'
PLOT1 = '"Medium\n{/*0.8(256 elements, 20% updates)}"'
PLOT2 = '"Small\n{/*0.8(512 elements, 20% updates)}"'
PLOT3 = '"Large skewed\n{/*0.8(1024 elements, 20% updates)}"'
PLOT4 = '"Large List\n{/*0.8(2048 elements, 20% updates)}"'

# font "Helvetica Bold"
set label 1 "Opteron" at screen 0.033, screen 0.28 rotate by 90 font ',40' textcolor rgb "red"
set label 2 "Xeon"    at screen 0.033, screen 1.04 rotate by 90 font ',40' textcolor rgb "red"


# ##########################################################################################
# XEON #####################################################################################
# ##########################################################################################

FILE0 = '"data/lpdxeon2680.ll.i128.u20.w0.dat"'
FILE1 = '"data/lpdxeon2680.ll.i256.u20.w0.dat"'
FILE2 = '"data/lpdxeon2680.ll.i512.u20.w0.dat"'
FILE3 = '"data/lpdxeon2680.ll.i1024.u20.w0.dat"'
FILE4 = '"data/lpdxeon2680.ll.i2048.u20.w0.dat"'

unset xlabel
set xrange [0:41]
set xtics ( 1, 10, 20, 30, 40 ) offset 0,0.2
unset key


set size plot_size_x, plot_size_y
set multiplot layout 5, 2

set size fig_size_x, fig_size_y
set origin 0.0 + graphs_x_offs, top_row_y
set title @PLOT0 offset 0.2,title_offset font ",34"
set ylabel 'Throughput (Mops/s)' offset 1
plot for [i=1:n_algo] @FILE0 using ($1):(column(i+1) / DIV) ls i with linespoints


set origin right_graph_x + graphs_x_offs, top_row_y
set lmargin 4
set size fig_size_x+0.032, fig_size_y
set ylabel ""
unset ylabel
set title @PLOT4
# set ytics 1
plot for [i=1:n_algo] @FILE4 using ($1):(column(i+1) / DIV) ls i with linespoints

# ##########################################################################################
# OPTERON ##################################################################################
# ##########################################################################################

FILE0 = '"data/lpd48core.ll.i128.u20.w0.dat"'
FILE1 = '"data/lpd48core.ll.i256.u20.w0.dat"'
FILE2 = '"data/lpd48core.ll.i512.u20.w0.dat"'
FILE3 = '"data/lpd48core.ll.i1024.u20.w0.dat"'
FILE4 = '"data/lpd48core.ll.i2048.u20.w0.dat"'

set xlabel "# Threads" offset 0, 0.6 font ",34"
set xrange [0:49]
set xtics ( 1, 12, 24, 36, 48 ) offset 0,0.2

unset title

set lmargin 3
set origin 0.0 + graphs_x_offs, bottom_row_y
set size fig_size_x, fig_size_y
# set title @PLOT0 offset 0.2,title_offset
set ylabel 'Throughput (Mops/s)' offset 1
set ytics 3
plot for [i=1:n_algo] @FILE0 using ($1):(column(i+1) / DIV) ls i with linespoints

set origin right_graph_x + graphs_x_offs, bottom_row_y
set lmargin 4
set size fig_size_x+0.032, fig_size_y
set ylabel ""
unset ylabel
set ytics 1
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
set origin 0.0, plot_size_y

#example key settings
# set key box 
#set key horizontal reverse samplen 1 width -4 maxrows 1 maxcols 12 
#set key at screen 0.5,screen 0.25 center top
set key font ",32"
set key spacing 1.5
set key horiz
set key at screen 0.9, screen plot_size_y center top

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
