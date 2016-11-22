/* *********************************************************************************************
 * Note: the article by Persaud et. al had 129 deaths, from years 2006-2010 (spanning, 5 years)
 * around ~ 25 a year.
 * So for 4 million cyclists, we need the probability of crashing x the probability
 * of a fatal head injury to be around ~1 in 150,000
 *
 * So for a 1 in 1,000 odds of crashing, the odds of a fatal head injury are around 1 in 150 etc.
 *
 *
 ********************************************************************************************
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <direct.h>

// Mersenne-Twister, Random Generator taken from http://www.bedaux.net/mtrand/<End
#include "mtrand.h"
static MTRand53 g_random;

static void Zero(void* pointer, int size)
{
	memset(pointer, 0, size);
}

double sqr(double x) { return x*x; }
double min(double x, double y) { return (x<y) ? x : y; }
double max(double x, double y) { return (x>y) ? x : y; }

	
// helper enum for indexing
enum
{
	kNoHelmet = 0,    // in the following structures index 0 means no helmet
	kHelmet = 1,      // index 1 means wearing a helmet
};

// Input profile for a simulation
struct InputProfile
{
	double numberOfCyclists;            // Total number of cyclists for this simulation (as a double for convenience)
	double helmetEffectiveness;         // 0.0 -> 1.0 what fraction of fatal head injuries are prevented by wearing a helmet 
	double helmetWearingFraction;       // The fraction of cyclists wearing a helmet (0.0 -> 1.0)

	double probOfCrashing[2];           // Probability of crashing, for non wearing and helmet wearing cyclist
	double probFatalHeadInjury[2];      // Probability of this crash becoming a fatal head injury (NB: this is the probability *after crashing*)
	double probFatalOtherInjury[2];     // Probability of this crash becoming a fatal body injury (after crashing)

	double coronerAccuracy;             // When deciding whether a death is a "control" or a "case" how accurate is the coroner? 0->1.0, with 1.0 == 100% accurate

	int repeatCount;                   // How many times to repeat this simulation run

	int optimisationReserveCount;      // reserve some memory upfront, as persistent std::vector growth memory allocations are slow
};

struct Death
{
	bool bHelmet;                    // wearing a helmet
	bool bFatalHeadInjury;           // Was there a head injury that would be fatal
	bool bFatalNonHeadInjury;        // Was there a non-head injury that would be fatal
};
	
struct ReportedGroup
{
	double count[2];         // index 0 - no helmet, index 1 - with helmet

	ReportedGroup()
	{
		count[0] = 0.0;
		count[1] = 0.0;
	}
};

// Helper class for calculating the odds ratio
// and storing other sundries from the cases
// and controls groups
class OddsRatio
{
public:
	OddsRatio() { }

	OddsRatio(const ReportedGroup& cases, const ReportedGroup& controls)
	{
		Calculate(cases, controls);
	}
	void Calculate(const ReportedGroup& cases, const ReportedGroup& controls)
	{
		///////////////////////////
		//          cases controls
		// helmet     a      b
		// no-helmet  c      d
		///////////////////////////
		a = cases.count[kHelmet];    b = controls.count[kHelmet];
		c = cases.count[kNoHelmet];  d = controls.count[kNoHelmet];
		oddsRatio = (a/c)/(b/d);
		helmetWearingFractionCases = a/(a+c);
		helmetWearingFractionControls = b/(b+d);
	}

	void Print(FILE* file)
	{
		fprintf(file, "#            cases      controls\n");
		fprintf(file, "#   helmet     %.1f       %.1f\n", a, b);
		fprintf(file, "#   no-helmet  %.1f       %.1f\n", c, d);
		fprintf(file, "#   -------------------------------------------\n");
		fprintf(file, "#   total      %.1f       %.1f\n", a+c, b+d);
		fprintf(file, "#\n");
		fprintf(file, "#   -------------------------------------------\n");
		fprintf(file, "#   ODDS-RATIO   =   %f\n\n", oddsRatio);
		fprintf(file, "#   -------------------------------------------\n");
		fprintf(file, "#   helmetWearingFractionCases    = %.1f %\n\n", helmetWearingFractionCases*100.0);
		fprintf(file, "#   helmetWearingFractionControls = %.1f %\n\n", helmetWearingFractionControls*100.0);
		fprintf(file, "#   -------------------------------------------\n");
		//fflush(file);
	}

	double a,b;
	double c,d;
	double oddsRatio;
	double helmetWearingFractionCases;
	double helmetWearingFractionControls;
};

void PrintProfile(FILE* file, const InputProfile& profile)
{
	fprintf(file, "numberOfCyclists %f\n", profile.numberOfCyclists);
	fprintf(file, "helmetEffectiveness = %f\n", profile.helmetEffectiveness);
	fprintf(file, "helmetWearingFraction = %f\n", profile.helmetWearingFraction);
	for (int i=0; i<2; i++)
	{
		fprintf(file, "probOfCrashing[%d] = %f\n", i, profile.probOfCrashing[i]);
		fprintf(file, "probFatalHeadInjury[%d] = %f\n", i, profile.probFatalHeadInjury[i]);
		fprintf(file, "probFatalOtherInjury[%d] = %f\n", i, profile.probFatalOtherInjury[i]);
	}
	fprintf(file, "coronerAccuracy = %f\n", profile.coronerAccuracy);
	fprintf(file, "repeatCount = %d\n", profile.repeatCount);
};

void PrintResults(const InputProfile& profile, const ReportedGroup& cases, const ReportedGroup& controls)
{
	///////////////////////////
	//          cases controls
	// helmet     a      b
	// no-helmet  c      d
	///////////////////////////
	double a = cases.count[kHelmet],   b = controls.count[kHelmet];
	double c = cases.count[kNoHelmet], d = controls.count[kNoHelmet];

	printf("# =========================================================\n");
	printf("#  RESULTS: Coroner is %.1f accurate in identifying\n", profile.coronerAccuracy*100.0);
	printf("#            cases      controls\n");
	printf("#   helmet     %.1f       %.1f\n", a, b);
	printf("#   no-helmet  %.1f       %.1f\n", c, d);
	printf("#   ---------------------------------\n");
	printf("#   total      %.1f       %.1f\n", a+c, b+d);

	// Proportion of people wearing helmets in the control group
	double controlHelmetWearingRate = b / (b+d);

	// Wearing odds of the control group
	double controlHelmetWearingOdds = b / d;
	
	// Wearing odds of the case group
	double casesHelmetWearingOdds = a / c;

	// Finally the ODDS-RATIO
	double oddsRatio = casesHelmetWearingOdds / controlHelmetWearingOdds;

	printf("\n#  controlHelmetWearingRate = %.1f %\n\n", controlHelmetWearingRate*100.0);

	printf("#  ODDS-RATIO = %f\n\n", oddsRatio);
	
	double f = controlHelmetWearingRate;

	oddsRatio = (a+b)/(c+d) * (1-f)/f;
	printf("#  OVERALL ODDS-RATIO = %f\n\n", oddsRatio);

	fflush(stdout);
}

//=========================================
// Simple Monte-carlo simulation
//=========================================
void Simulate(const InputProfile& profile, ReportedGroup& outputCases, ReportedGroup& outputControls)
{
	// Initialise the cases and control structures	
	Zero(&outputCases, sizeof(ReportedGroup));
	Zero(&outputControls, sizeof(ReportedGroup));
	
	std::vector<Death> deaths;
	deaths.reserve(profile.optimisationReserveCount); // Optimisation, reserve some memory upfront

	for (int repeat=0; repeat<profile.repeatCount; repeat++)
	{
		//=======================================
		// Run the simulation loop
		//=======================================
		for (int i=0; i<(int)profile.numberOfCyclists; i++)
		{
			// For each cyclist - roll the dice (aka the random number generator)
			// Are they wearing a helmet ?
			// 1 - wearing a helmet, 0 - not wearing a helmet
			int helmet = (g_random() < profile.helmetWearingFraction) ? 1 : 0;

			// For each cyclist - roll the dice (aka the random number generator)
			// Is it a crash?
			if (g_random() < profile.probOfCrashing[helmet])
			{
				// What's the probability of this crash leading to a fatal head injury
				double probabilityOfFatalHeadInjury = profile.probFatalHeadInjury[helmet];
				// If helemt wearing that probability is reduced by (1-protection)
				if (helmet)
					probabilityOfFatalHeadInjury *= 1.0 - profile.helmetEffectiveness;

				// Does the crash lead to a fatal HEAD injury?
				bool bFatalHeadInjury = (g_random() < probabilityOfFatalHeadInjury);

				// Does the crash lead to a fatal injury other than the head?
				bool bFatalNonHeadInjury = (g_random() < profile.probFatalOtherInjury[helmet]);

				// If it was a fatality, record the death
				if (bFatalHeadInjury || bFatalNonHeadInjury)
				{
					Death death;
					death.bHelmet = helmet ? true : false;
					death.bFatalHeadInjury = bFatalHeadInjury;
					death.bFatalNonHeadInjury = bFatalNonHeadInjury;
					deaths.push_back(death);
				}
			}
		}

		//===============================================
		// Coroner analyses the deaths
		//===============================================
		for (int i=0; i<(int)deaths.size(); i++)
		{
			Death& death = deaths[i];
			int helmet = death.bHelmet ? kHelmet : kNoHelmet;

			// If the death has a fatal non-head injury occurs it
			// should be classified as a control,
			// regardless of whether there was as head injury as well
			bool bControl = death.bFatalNonHeadInjury;

			// However there might be some bias or error
			// by the coroner in identifying the
			// control if they're not wearing a helmet
			// maybe they're more likely to decide a head injury
			// was the cause rather than another injury
			if (!helmet && (g_random() > profile.coronerAccuracy))
			{
				// The coroner mis-identifies this control
				bControl = false;
			}

			// Equally if they are wearing a helmet 
			// and the other injuries are a borderline case,
			// it might sway a coroner to decide the death is a control
			// rather than a case
			if (helmet && (g_random() > profile.coronerAccuracy))
				bControl = true;

			if (bControl)
			{
				outputControls.count[helmet]++;
			}
			else
			{
				outputCases.count[helmet]++;
			}
		}
	}
}

//=======================================
// MAIN
//=======================================
int main()
{
	const char* baseFilename = "output/result";
	mkdir("output");

	// See initial notes at the top, the approximate odds a cyclist
	// getting a fatal head injury in a year is around:
	const double approximateRiskOfFatalHeadInjury = 1.0/0.35e6;

	/////////////////////////////////////
	// Input profile
	/////////////////////////////////////
	InputProfile profile;
	profile.numberOfCyclists = 4e6;        // around 13.6 million ontarians, ~30% cycle weekly
	profile.helmetEffectiveness = 0.3;     // pick a random number :-/
	profile.helmetWearingFraction = 0.36;  // 2009 survey ~ 36% cyclists wearing a helmet
   	// No idea what the probability of crashing should be so choose something "sensible"
	// and then scale the risk of a subsequent fatal head injury using the overall approximateRiskOfFatalHeadInjury
	profile.probOfCrashing[kNoHelmet] = 1.0/100.0;
	profile.probFatalHeadInjury[kNoHelmet] = approximateRiskOfFatalHeadInjury / profile.probOfCrashing[kNoHelmet];
	// From persaud it looked for every fatal head injury there were around
	profile.probFatalOtherInjury[kNoHelmet] = profile.probFatalHeadInjury[kNoHelmet] * 0.7f;
	// Does helmet wearing population have more chance of crashing
	// and a higher injury risk?

	profile.probOfCrashing[kHelmet] = profile.probOfCrashing[kNoHelmet];
	profile.probFatalHeadInjury[kHelmet] = profile.probFatalHeadInjury[kNoHelmet];
	profile.probFatalOtherInjury[kHelmet] = profile.probFatalOtherInjury[kNoHelmet];

	// Optimisatio so the simulation runs faster	
	double hack = max(profile.probOfCrashing[kNoHelmet], profile.probOfCrashing[kHelmet]);
	profile.numberOfCyclists *= hack;
	profile.probOfCrashing[kHelmet] *= 1.0/hack;
	profile.probOfCrashing[kNoHelmet] *= 1.0/hack;

	// How accurately does the coroner assess and classigy the controls:
	profile.coronerAccuracy = 1.0;
	profile.repeatCount = 5;               // repeat this 5 times, as Persaud et al results were from 2006-2010, spanning 5 years
	profile.optimisationReserveCount = 500;

	//PrintProfile(stderr, profile);
	//exit(0);

	///////////////////////////////////////
	// Run the simulation
	///////////////////////////////////////
	const int numIterations = 20000;
	OddsRatio* results = new OddsRatio[numIterations];

	for (int i=0; i<numIterations; i++)
	{
		ReportedGroup cases;
		ReportedGroup controls;
		Simulate(profile, cases, controls);
		results[i].Calculate(cases, controls);
		//results[i].Print(stderr);
		//if (i>10) exit(0);
		if ((i%100)==0)
		{
			fprintf(stderr, "%d of %d\n", i, numIterations);
			fflush(stderr);
		}
	}

	///////////////////////////////////////
	// Analyse the results
	///////////////////////////////////////
	
	// Mean
	double mean = 0.0;
	for (int i=0; i<numIterations; i++)
	{
		mean += results[i].oddsRatio;
	}
	mean /= (double)numIterations;
	
	// Variance
	double variance = 0.0;
	for (int i=0; i<numIterations; i++)
	{
		double x = results[i].oddsRatio;
		variance += sqr(x - mean);
	}
	variance /= (double)(numIterations-1);

	double standardDeviation = sqrt(variance);

	printf("# # # %f, %f, %f\n", mean, variance, standardDeviation, 2.0*standardDeviation);

	// Create a histogram
	double resolution = max(0.001, min(0.05, standardDeviation/5.0));
	int numBuckets = (int)((3.0+resolution)/resolution);
	double* buckets = new double[numBuckets];
	Zero(buckets, sizeof(double)*numBuckets);
	
	for (int i=0; i<numIterations; i++)
	{
		int bucket = (int)(((results[i].oddsRatio)/resolution)+0.5f);
		if (bucket>=0 && bucket<numBuckets)
		{
			buckets[bucket] += 1.0;
		}
	}

	// Output the histogram
	for (int i=0; i<numBuckets; i++)
	{
		printf("%f %f\n", (double)i*resolution, buckets[i]);
	}
	
	// Clean up
	delete [] results;

	// End
	return 0;
}
