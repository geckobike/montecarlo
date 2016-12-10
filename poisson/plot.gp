# factorial 
fac(x) = (int(x)==0) ? 1.0 : int(x) * fac(int(x)-1.0)
poisson(x, lam) = lam**x * exp(-lam) / fac(x)

set samples 10
plot "plot.txt" u 1:2 w lp lc black lw 3 pt 6, 128*poisson(x,4) w l lw 4 lc rgb "#a0a0a0"


