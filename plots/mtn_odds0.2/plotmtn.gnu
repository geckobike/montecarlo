# Output to png?
set terminal pngcairo
#size 900,600 font ',14'
set output 'output.png'

#set object rectangle from screen 0,0 to screen 0.5,1 behind fillcolor rgb "#c0c0cd" fillstyle solid noborder

# Draw vertical mean at OddsRatio of 0.8
set arrow from 0.8, graph 0 to 0.8, graph 1 nohead front fillcolor rgb "#808080"
set arrow from 0.556, graph 0 to 0.556, graph 1 nohead front fillcolor rgb "#808080"

set logscale x
set xtics (0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0)
set title "Monte Carlo Simulation - Demonstrating Over-Estimation of Odds Ratio"

#set label 1 at 0.5, graph -0.5
#set label 1 "Superscripts and subscripts:" tc lt 3 front

plot [0.15:2.2]\
"mtn_odds0.2_arms2_crash2_w0.3.txt" w filledcurve lt 1 lc rgb "#e0e0e0" title "",\
"mtn_odds0.2_arms2_crash2_w0.3.txt" u 1:2 w lp lt rgb "#808080" lw 3 pt 6 title "Mountain Bikes Included",\
"mtn_odds0.2_arms0_crash0_w0.3.txt" u 1:2 w lp lt rgb "#000000" pt 6 lw 3 title "No Mountain Bikes"

#   ----------------------------------------------------------------
#    Monte Carlo simulation of Odd Ratio Cycling Helmet Experiment.
#   ----------------------------------------------------------------
#
#   General Settings:
#    1,000,000 cyclists
#    Number of times simulation repeated 10,000
#    Helmet effectiveness 20% ( Odds Ratio = 0.8 )
#
#   Settings for "Normal Cyclists": 
#    Probability of wearing a helmet 0.30
#    Probability of cyclist having a "serious" crash 0.01
#    Probability of crash leading to a serious head injury 0.01
#    Probability of crash leading to a limb injury 0.01
#
#   Settings for Mountain Bikers:
#    Same as above but,
#    100 % wearing helmets
#    0.1 of cyclists are mountain bikers
#    2x more likely to crash
#    2x more likely to have an arm injury
#
#   Correct Odds Ratio should be 0.8
#   The measured Odds Ratio when mountain bikers are included is
#   significantly reduced to 0.55.
#   i.e. helmet effectiveness has been OVERESTIMATED
#
