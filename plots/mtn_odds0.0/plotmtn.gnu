# Output to png?
set terminal pngcairo
#size 900,600 font ',14'
set output 'output.png'

#set object rectangle from screen 0,0 to screen 0.5,1 behind fillcolor rgb "#c0c0cd" fillstyle solid noborder

# Draw vertical mean at OddsRatio of 0.8
set arrow from 1.0, graph 0 to 1.0, graph 1 nohead front fillcolor rgb "#808080"
set arrow from 0.719, graph 0 to 0.719, graph 1 nohead front fillcolor rgb "#808080"

set logscale x
set xtics (0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0)
set title "Monte Carlo Simulation - Demonstrating Over-Estimation of Odds Ratio"

#set label 1 at 0.5, graph -0.5
#set label 1 "Superscripts and subscripts:" tc lt 3 front

plot [0.3:1.6][0:1000]\
"mtn_odds0.0_arms2_crash2_w0.3.txt" w filledcurve lt 1 lc rgb "#e0e0e0" title "",\
"mtn_odds0.0_arms2_crash2_w0.3.txt" u 1:2 w lp lt rgb "#808080" lw 3 pt 6 title "Mountain Bikes Included",\
"mtn_odds0.0_arms0_crash0_w0.3.txt" u 1:2 w lp lt rgb "#000000" pt 6 lw 3 title "No Mountain Bikes"

#   ----------------------------------------------------------------
#    Monte Carlo simulation of Odd Ratio Cycling Helmet Experiment.
#   ----------------------------------------------------------------
#
#   General Settings:
#    2,000,000 cyclists
#    Number of times simulation repeated 10,000
#    Helmet effectiveness *** 0 % *** ( Odds Ratio = 1.0 )
#
#   Settings for "Normal Cyclists": 
#    Probability of wearing a helmet 0.30
#    Probability of cyclist having a "serious" crash 0.02
#    Probability of crash leading to a serious head injury 0.02
#    Probability of crash leading to a limb injury 0.02
#
#   Settings for Mountain Bikers:
#    Same as above but,
#    100 % wearing helmets
#    0.1 of cyclists are mountain bikers
#    2x more likely to crash
#    2x more likely to have an arm injury
#
#   Correct Odds Ratio should be 1.0
#   The measured Odds Ratio when mountain bikers are included is
#   significantly reduced to 0.71.
#   i.e. helmet effectiveness has been OVERESTIMATED
#
# A typical Result:
#  where limb injuries have been used for controls
#
#            cases      controls                                                                    
#   helmet     351.0       554.0                                                                    
#   no-helmet  526.0       532.0                                                                    
#   -------------------------------------------                                                     
#   total      877.0       1086.0                                                                   
#                                                                                                   
#   -------------------------------------------                                                     
#   ODDS-RATIO   =   0.640801                                                                       
#
#            cases      controls                                                                    
#   helmet     365.0       563.0                                                                    
#   no-helmet  467.0       510.0                                                                    
#   -------------------------------------------                                                     
#   total      832.0       1073.0                                                                   
#                                                                                                   
#   -------------------------------------------                                                     
#   ODDS-RATIO   =   0.708007   
#
# Zeegers email:
#   e-mail: th.zeegers@xs4all.nl                                                                    
