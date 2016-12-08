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
#include <algorithm>
#include <direct.h>

// Mersenne-Twister, Random Generator taken from http://www.bedaux.net/mtrand/<End
#include "mtrand.h"
static MTRand53 g_random2;
double g_random()
{
	return g_random2();
	//const int N = 2048;
	//double r[N];
	//static int n = N;
	//if (n>=N)
	//{
	//	n=0;
	//	for (int i=0; i<N; i++)
	//		r[i] = g_random2();
	//}
	//return r[n++];
}

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
	
	double probOfDrunk;
	double probOfDrunkCrash;
	double probOfDrunkHead;
	double probOfDrunkBody;

	double coronerAccuracy;             // When deciding whether a death is a "control" or a "case" how accurate is the coroner? 0->1.0, with 1.0 == 100% accurate
	
	int numIterations;                  // How many times should the simulation be repeated
	int seed;                           // [optional] seed for the Mersenne Twister

	int optimisationReserveCount;       // reserve some memory upfront, as persistent std::vector growth memory allocations are slow

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
	fprintf(file, "\nprofile:\n");
	fprintf(file, "  numberOfCyclists         = %d\n", (int)profile.numberOfCyclists);
	fprintf(file, "  helmetEffectiveness      = %f\n", profile.helmetEffectiveness);
	fprintf(file, "  helmetWearingFraction    = %f\n", profile.helmetWearingFraction);
	for (int i=0; i<2; i++)
	{
		fprintf(file, "  probOfCrashing[%d]        = %f\n", i, profile.probOfCrashing[i]);
		fprintf(file, "  probFatalHeadInjury[%d]   = %f\n", i, profile.probFatalHeadInjury[i]);
		fprintf(file, "  probFatalOtherInjury[%d]  = %f\n", i, profile.probFatalOtherInjury[i]);
	}
	fprintf(file, "  coronerAccuracy          = %f\n", profile.coronerAccuracy);
	fprintf(file, "  numIterations            = %d\n", profile.numIterations);
	fprintf(file, "  seed                     = %x\n", profile.seed);
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

	printf("# # =========================================================\n");
	printf("# #  RESULTS: Coroner is %.1f accurate in identifying\n", profile.coronerAccuracy*100.0);
	printf("# #            cases      controls\n");
	printf("# #   helmet     %.1f       %.1f\n", a, b);
	printf("# #   no-helmet  %.1f       %.1f\n", c, d);
	printf("# #   ---------------------------------\n");
	printf("# #   total      %.1f       %.1f\n", a+c, b+d);

	// Proportion of people wearing helmets in the control group
	double controlHelmetWearingRate = b / (b+d);

	// Wearing odds of the control group
	double controlHelmetWearingOdds = b / d;
	
	// Wearing odds of the case group
	double casesHelmetWearingOdds = a / c;

	// Finally the ODDS-RATIO
	double oddsRatio = casesHelmetWearingOdds / controlHelmetWearingOdds;

	printf("\n# #  controlHelmetWearingRate = %.1f %\n\n", controlHelmetWearingRate*100.0);

	printf("# #  ODDS-RATIO = %f\n\n", oddsRatio);
	
	double f = controlHelmetWearingRate;

	oddsRatio = (a+b)/(c+d) * (1-f)/f;
	printf("# # OVERALL ODDS-RATIO = %f\n\n", oddsRatio);

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

	//=======================================
	// Run the simulation loop
	//=======================================
	for (int i=0; i<(int)profile.numberOfCyclists; i++)
	{
		int drunk = (g_random() < profile.probOfDrunk) ? 1 : 0;

		// For each cyclist - roll the dice (aka the random number generator)
		// Are they wearing a helmet ?
		// 1 - wearing a helmet, 0 - not wearing a helmet
		int helmet = 0;

		if (!drunk)
			helmet = (g_random() < profile.helmetWearingFraction) ? 1 : 0;
		else
			helmet = 1; // Let pretend drunks are mountain bikers for this experiment
		
		double probOfCrashing = profile.probOfCrashing[helmet];
		if (drunk)
			probOfCrashing *= profile.probOfDrunkCrash;

		// For each cyclist - roll the dice (aka the random number generator)
		// Is it a crash?
		if (g_random() < probOfCrashing)
		{
			// What's the probability of this crash leading to a fatal head injury
			double probabilityOfFatalHeadInjury = profile.probFatalHeadInjury[helmet];

			if (drunk)
				probabilityOfFatalHeadInjury *= profile.probOfDrunkHead;

			// If helemt wearing that probability is reduced by (1-protection)
			if (helmet)
				probabilityOfFatalHeadInjury *= 1.0 - profile.helmetEffectiveness;

			// Does the crash lead to a fatal HEAD injury?
			bool bFatalHeadInjury = (g_random() < probabilityOfFatalHeadInjury);

			// Does the crash lead to a fatal injury other than the head?
			double probOFFatalOther = profile.probFatalOtherInjury[helmet];
			if (drunk)
				probOFFatalOther *= profile.probOfDrunkBody;
			bool bFatalNonHeadInjury = (g_random() < (probOFFatalOther));

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
	const int numRepeats = 15;
	for (int repeat=0; repeat<numRepeats; repeat++)
	{
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
	outputCases.count[0] /= (float)numRepeats;
	outputCases.count[1] /= (float)numRepeats;
	outputControls.count[0] /= (float)numRepeats;
	outputControls.count[1] /= (float)numRepeats;
}

static void Bail(FILE* file, const char* msg)
{
	fprintf(file, "ERROR, bailing %s", msg);
	exit(0);
}

class SettingsStringsParser
{
public:
	SettingsStringsParser(const char* rawInputStringConst)
	{
		// Need to copy the input str as we cant modify the original
		// NB: add two null-terminators at the end for easier processing
		int len = strlen(rawInputStringConst);
		if (len==0)
			Bail(stderr, "Bad input settings");

		rawInputString = new char[len+1];
		strcpy(rawInputString, rawInputStringConst);
		rawInputString[len] = 0;

		// Now chop it up into sections, split by the 'commas'
		char* p = rawInputString;
		while (true)
		{
			inputStrings.push_back(p);
			// Find the [next] comma
			while (*p && *p!=',') ++p;
			if (*p==0)
				break;
			*p = 0;
			++p;
		}
	}

	~SettingsStringsParser()
	{
		delete [] rawInputString;
	}

	bool GetOption(const char* option, const char* &value)
	{
		int len = strlen(option);
		for (int i=0; i<(int)inputStrings.size(); i++)
		{
			if (strncmp(option, inputStrings[i], len)==0)
			{
				if ((inputStrings[i])[len] == '=')
					value = &((inputStrings[i])[len+1]);
				return true;
			}
		}
		return false;
	}

	bool GetOption(const char* option, double& value)
	{
		const char* v;
		if (GetOption(option, v))
		{
			value = atof(v);
			return true;
		}
		return false;
	}
	
	bool GetOption(const char* option, float& value)
	{
		double d;
		if (GetOption(option, d))
		{
			value = d;
			return true;
		}
		return false;
	}
	
	bool GetOption(const char* option, int& value)
	{
		double d;
		if (GetOption(option, d))
		{
			value = d;
			return true;
		}
		return false;
	}

	void DebugPrint()
	{
		for (int i=0; i<(int)inputStrings.size(); i++)
		{
			fprintf(stderr, "option: %s\n", inputStrings[i]);
		}
	}

public:
	char* rawInputString;
	std::vector<char*> inputStrings;
};

static void ProcessSettings(const char* rawInputStringConst, InputProfile& profile)
{
	Zero(&profile, sizeof(profile));
	
	SettingsStringsParser parser(rawInputStringConst);
	parser.GetOption("numberOfCyclists", profile.numberOfCyclists);
	parser.GetOption("helmetEffectiveness", profile.helmetEffectiveness);
	parser.GetOption("helmetWearingFraction", profile.helmetWearingFraction);
	parser.GetOption("probOfCrashing[0]", profile.probOfCrashing[0]);
	parser.GetOption("probOfCrashing[1]", profile.probOfCrashing[1]);
	parser.GetOption("probFatalHeadInjury[0]", profile.probFatalHeadInjury[0]);
	parser.GetOption("probFatalHeadInjury[1]", profile.probFatalHeadInjury[1]);
	parser.GetOption("probFatalOtherInjury[0]", profile.probFatalOtherInjury[0]);
	parser.GetOption("probFatalOtherInjury[1]", profile.probFatalOtherInjury[1]);
	parser.GetOption("coronerAccuracy", profile.coronerAccuracy);
	parser.GetOption("numIterations", profile.numIterations);
	parser.GetOption("seed", profile.seed);
	
	parser.GetOption("probOfDrunk", profile.probOfDrunk);
	parser.GetOption("probOfDrunkCrash", profile.probOfDrunkCrash);
	parser.GetOption("probOfDrunkHead", profile.probOfDrunkHead);
	parser.GetOption("probOfDrunkBody", profile.probOfDrunkBody);

	PrintProfile(stderr, profile);

	//fprintf(out, "       helmetEffectiveness\n"); 
	//fprintf(out, "       helmetWearingFraction\n"); 
	//fprintf(out, "       probOfCrashing[]             nb: probOfCrashing[0] no helmet, probOfCrashing[1] with helmet\n");
	//fprintf(out, "       probFatalHeadInjury[]\n");
	//fprintf(out, "       probFatalOtherInjury[]\n");
	//fprintf(out, "       coronerAccuracy              0 -> 1.0\n");
	//fprintf(out, "       numIterations\n");
	//fprintf(out, "       seed {optional}              seed value for the Mersenne Twister generator\n");

	//fprintf(out, "\nExample:\n");
	//fprintf(out, "    --settings=numberOfCyclists=10e6,helmetEffectiveness=0.2,helmetWearingFraction=0.35\n");
	//fprintf(out, "      NB: settings are case sensitive\n");
}

static const char* ArgStringComp(const char* arg, const char* str)
{
	int len = strlen(str);
	if (strncmp(arg, str, len)==0 && arg[len]=='=')
	{
		return &arg[len+1];
	}
	return NULL;
}

//=======================================
//
//=======================================
void ProcessArgs(int argc, const char* argv[], InputProfile& profile)
{
	FILE* out = stderr;
	bool bPrintUsage = (argc==1);
	const char* argValue = "";

	for (int i=1; i<argc; i++)
	{
		if ((argValue = ArgStringComp(argv[i], "-h")))
		{
			bPrintUsage = true;
			break;
		}
		
		if ((argValue = ArgStringComp(argv[i], "--settings")))
		{
			ProcessSettings(argValue, profile);
			continue;
		}
	
		// If we get here an argument wasnt processed, bail
		char badArg[128];
		strncpy(badArg, argv[i], 127);
		badArg[127] = 0;
		fprintf(out, "Argument error!\n");
		fprintf(out, ">   %s\n\n", badArg);
		Bail(out, "...bailing");
	}

	if (bPrintUsage)
	{
		fprintf(out, "Usage:\n");
		fprintf(out, "  -h,  this help msg\n");
		fprintf(out, "  --settings, --settings=\"comma separated list of settings\"\n");

		fprintf(out, "\nList of Settings:\n");

		fprintf(out, "       numberOfCyclists\n");
		fprintf(out, "       helmetEffectiveness\n"); 
		fprintf(out, "       helmetWearingFraction\n"); 
		fprintf(out, "       probOfCrashing[]             nb: probOfCrashing[0] no helmet, probOfCrashing[1] with helmet\n");
		fprintf(out, "       probFatalHeadInjury[]\n");
		fprintf(out, "       probFatalOtherInjury[]\n");
		fprintf(out, "       coronerAccuracy              0 -> 1.0\n");
		fprintf(out, "       numIterations\n");
		fprintf(out, "       seed {optional}              seed value for the Mersenne Twister generator\n");

		fprintf(out, "\nExample:\n");
		fprintf(out, "    --settings=numberOfCyclists=10e6,helmetEffectiveness=0.2,helmetWearingFraction=0.35\n");
		fprintf(out, "      NB: settings are case sensitive\n");
		Bail(out, "\n");
	}
}

//=======================================
// MAIN
//=======================================
int main(int argc, const char* argv[])
{
	InputProfile profile;
	Zero(&profile, sizeof(profile));
	ProcessArgs(argc, argv, profile);
	if (profile.numIterations<=0)
		Bail(stderr, "Nothing to do!");

	// Optimisation so the simulation runs faster
	double hack = max(profile.probOfCrashing[kNoHelmet], profile.probOfCrashing[kHelmet]);
	hack = max(hack, profile.probOfCrashing[kNoHelmet] * profile.probOfDrunkCrash);
	hack *= 3.0;
	profile.numberOfCyclists *= hack;
	profile.probOfCrashing[kHelmet] *= 1.0/hack;
	profile.probOfCrashing[kNoHelmet] *= 1.0/hack;

	///////////////////////////////////////
	// Run the simulation
	///////////////////////////////////////
	const int numIterations = profile.numIterations;
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
			results[i].Print(stderr);
			fprintf(stderr, "%.1f %% completed\n", ((double)(i+100)/(double)numIterations)*100.0);
			fflush(stderr);
		}
	}

	if (numIterations>1)
	{
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
		variance /= (double)((numIterations-1));

		double standardDeviation = sqrt(variance);

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

		// Sort results to create ERF
		std::vector<float> sortedResults;
		sortedResults.reserve(numIterations);
		for (int i=0; i<numIterations; i++)
		{
			sortedResults.push_back(results[i].oddsRatio);
		}
		std::sort(sortedResults.begin(), sortedResults.end());

		// Careful!
		float start95 = 0.025 * (float)numIterations;
		float mid = 0.5 * (float)numIterations;
		float end95 = (1.0-0.025) * (float)numIterations;
		
		if ((int)start95>=0 && (int)end95<(int)sortedResults.size())
		{
			printf("# # # 95%% intervals are at %f - %f\n", sortedResults[(int)start95], sortedResults[(int)end95]);
			printf("# # # mid = %f\n", sortedResults[mid]);
		}
		printf("# # # mean: %f, var: %f, 2sd: %f\n", mean, variance, standardDeviation, 2.0*standardDeviation);
	}
	
	// Clean up
	delete [] results;

	// End
	return 0;
}

#if 0
int PoissonTest(int argc, const char* argv[])
{
	float hist[10] = {0};

	for (int repeat=0; repeat<1000; repeat++)
	{
		int bombed[128] = {0};
		for (int i=0; i<(int)512; i++)
		{
			int index = (int)(g_random()*127.9999);
			bombed[index]++;
		}
		for (int i=0; i<128; i++)
		{
			if (bombed[i]<10)
			{
				hist[bombed[i]] += 1.0;
			}
		}
	}

	for (int i=0; i<10; i++)
	{
		hist[i] /= 1000.0;
		printf("%f %f\n\n", (float)i, hist[i]);
	}


	return 0;
}
#endif


