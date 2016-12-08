#set object rectangle from screen 0,0 to screen 0.5,1 behind fillcolor rgb "#c0c0cd" fillstyle solid noborder

# Draw mean of 0.6 and 0.364 lines
set arrow from 0.346, graph 0 to 0.346, graph 1 nohead front fillcolor rgb "#808080"
set arrow from 0.6, graph 0 to 0.6, graph 1 nohead front fillcolor rgb "#808080"

set logscale x
set xtics (0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0)
set title "Monte Carlo Simulation - Demonstrating Over-Estimation of Odds Ratio"

plot [0.05:3]\
"mtn_arms1.0.txt" u 1:2 w lp title "arms1.0",\
"mtn_arms1.5.txt" u 1:2 w lp title "arms1.5",\
"mtn_arms1.5_wearing0.25.txt" u 1:2 w lp,\
"mtn_arms1.5_wearing0.75.txt" u 1:2 w lp,\
"mtn_arms2_crash2_w0.3.txt" u 1:2 w lp,\
"mtn.txt" u 1:2 w filledcurve lt 1 lc rgb "#e0e0e0" title "",\
"mtn.txt" u 1:2 w lp lt rgb "#808080" lw 3 pt 6 title "Mountain Bikes Included arms2.0",\
"same.txt" w lp lt rgb "#000000" pt 6 lw 3 title "No Mountain Bikes"

#pause -1


# ----------------------------------------------------------------
#  Monte Carlo simulation of Odd Ratio Cycling Helmet Experiment.
# ----------------------------------------------------------------
#
# General Settings:
#  1,000,000 cyclists
#  Number of times simulation repeated 10,000
#  Helmet effectiveness 40% ( Odds Ratio = 0.6 )
#
# Settings for "Normal Cyclists": 
#  Probability of wearing a helmet 0.35
#  Probability of cyclist having a "serious" crash 0.01
#  Probability of crash leading to a serious head injury 0.01
#  Probability of crash leading to a limb injury 0.01
#
# Settings for Mountain Bikers:
#  Same as above but,
#  100 % wearing helmets
#  0.1 of cyclists are mountain bikers
#  10x more likely to crash
#  2x more likely to have an arm injury
#



