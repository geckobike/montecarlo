/* *********************************************************************************************
 * A simule poisson test, to check if MTrand is generating suitable random numbers
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
#include "../mtrand.h"
static MTRand53 g_random;

int main(int argc, const char* argv[])
{
	// 512 bombs, distributed over 128 cells
	// average of 4 per cell
	// expect poisson distribution of
	// p(x) = 128 * (4^x) * exp(-4) / fac(x)

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


