import math;

# children
child_controls = 590 + 1043.0;
child_heads = 94 + 127.0;
adult_controls = 1293 + 1468 + 893 + 819.0;
adult_heads = 119 + 112 + 81 + 106.0;

print "========= From paper ==================";
print "child, heads/controls in paper", child_heads, child_controls
print "adults, heads/controls in paper", adult_heads, adult_controls
print "========= From paper ==================";
print "children no helmet:", (323+588), " helmet:", (361+582)
print "adults   no helmet:", (324+231+116+75), " helmet:", (1088+1349+858+849)

total_child = child_controls + child_heads;
total_adults = adult_controls + adult_heads;

total_controls = [0, 0];
total_heads = [0, 0];


## #          count, head, control, helmet
## #profile1 = [1900, 0.155, 0.0, 0.51];  # children
## #profile2 = [4890, 0.195, 0.0, 0.76];  # adults
## 
## # # THESE DONT WORK!!!!
## # profile1 = [total_child, 0.163, 0.0, 0.51,  0.46];   # children
## # profile2 = [total_adults, 0.145, 0.0, 0.76, 0.46];  # adults
## 
## profile1 = [total_child, 0.12, 0.0, 0.51,  0.9];   # children
## profile2 = [total_adults, 0.145, 0.0, 0.76, 0.46];  # adults
## 
## def Construct(profile):
## 	HOR = profile[4];
## 	n = profile[0];
## 	w = profile[3];
## 	nheads = profile[1] * n;
## 	controlfrac = 1.0 - profile[1];
## 	ncontrols = controlfrac * n;
## 
## 	nc = [ ncontrols * (1-w), ncontrols * w];
## 	headinj = [ nheads * (1-w), nheads * w ];
## 
## 	# Some injuries are saved
## 	saved = headinj[1] * (1-HOR);
## 	headinj[1] -= saved;
## 
## 	nc[1] += saved;
## 
## 	global total_heads, total_controls;
## 	total_heads[0] += headinj[0];
## 	total_heads[1] += headinj[1];
## 	total_controls[0] += nc[0];
## 	total_controls[1] += nc[1];
## 
## 	print "heads/controls", headinj, nc;
## 	print "total heads/controls", (headinj[0]+headinj[1]), (nc[0]+nc[1]), "ratio heads/controls", (headinj[0]+headinj[1]) / (nc[0]+nc[1])
## 	print "check OR", (headinj[1]*nc[0])/(nc[1]*headinj[0]);
## 
## 
## 
## print "Children:"
## Construct(profile1);
## print "Adults:"
## Construct(profile2);
## 
## print ""
## print "FINAL heads/controls", total_heads, total_controls

class MakeStruct:
	def __init__(self, **kwds):
		self.__dict__.update(kwds)

# Damn, their results work
childrenProfile = MakeStruct(count=total_child,  headRate=0.1650, wearingFrac=0.508, OR = 0.41);

adultProfile    = MakeStruct(count=total_adults, headRate=0.1610, wearingFrac=0.849, OR = 0.41);

childrenProfile = MakeStruct(count=total_child,  headRate=0.0190, wearingFrac=0.508, OR = 1.0);
adultProfile    = MakeStruct(count=total_adults, headRate=0.0855, wearingFrac=0.847, OR = 1.0);


total_controls = [0, 0];
total_heads = [0, 0];

def Create2x2Table(profile, msg):
	# Require:
	#	               cases controls
	#	   with helmet   a     b
	#	without helmet	 c     d
	#
	#	OR = (a*d)/(c*b)
	#
	#	without = count * (1 - wearingFrac) = c + d
	# 	c = without * headRate
	#	d = without - c
	
	cplusd = profile.count * (1.0 - profile.wearingFrac);
	c = cplusd * profile.headRate;
	d = cplusd - c;
	aOverB = profile.OR * (c/d);
	aplusb = profile.count * profile.wearingFrac;
	b = aplusb/(1.0 + aOverB);
	a = aplusb - b;

	print "---------------------------------------------------"
	#print msg, profile.OR, (a*d)/(b*c);
	print msg, "a,b,c,d", "%.1f" %a, "%.1f" % b, "%.1f" % c,"%.1f" % d
	print msg, "no %.1f" % (c+d), "with %.2f" % (a+b)
	print msg, "heads & controls %.1f" % (a+c), " %1.f" % (b+d)
	return MakeStruct(a=a,b=b,c=c,d=d);

tc = Create2x2Table(childrenProfile, "children:")
ta = Create2x2Table(adultProfile, "adults:")

print "---------------------------------------------"
print "Addition Combined table:"
print "%.1f" % (ta.a+tc.a), "%.1f" % (ta.b+tc.b)
print "%.1f" % (ta.c+tc.c), "%.1f" % (ta.d+tc.d)
print "OR = %.2f" % (((ta.a+tc.a)/(ta.b+tc.b))/((ta.c+tc.c)/(ta.d+tc.d)))

print "---------------------------------------------"
print "In Bambach's paper the combined table is:"
print 372, 4715, "|", (372+4715)
print 267, 1391, "|", (267+1391)
print " --  ---"
print (372+267), (4715+1391)


heads = ta.a + ta.c + tc.a + tc.c
controls = ta.b + ta.d + tc.b + tc.d
print "---------------------------------------------"
print "Add together to give total heads, controls"
print heads, controls
