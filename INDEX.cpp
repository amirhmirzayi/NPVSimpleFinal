//////////////////////////////////////////////////////////////////////////////
//																			//
//	NPV Solver																//
//	Description of the project...											//
//	Author: Sc																//
//	Create date: 8/07/2007													//
//																			//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
///////////////INCLUDES///////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
#include <list>
#include <stdio.h>
#include <stdlib.h>					// Math library.
#include <math.h>					// Another math library.
#include <time.h>					// Keep track of time.
#include <iostream>					// Input output.
#include <fstream>					// Input output.
#include <thread>
#include <future>
#include <mutex>
#include <omp.h>
using namespace std;				// Input output.

//////////////////////////////////////////////////////////////////////////////
///////////////DEFINE GLOBAL VARIABLES////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

unsigned long int*** gm_str_ptg;			// Global matrix holding the final ptg structure.
unsigned short int*** gm_lnk_ptg;			// Global matrix holding the final ptg link structure.
unsigned short int*** gm_dpt_ptg;			// Global matrix holding the final ptg link depth structure.
unsigned short int*** gm_nio_ptg;			// Global matrix holding the number of incoming and outgoing links at each ptg.
unsigned short int*** gm_nio_ptg2;		// Global matrix holding the number of incoming and outgoing links at each ptg (copy).

unsigned long int** gm_ava_bin;			// Global matrix holding the binary code of activities required to be finished prior to a regular action i to be allowed to be executed.
unsigned long int** gm_ava_mrg_bin;		// Global matrix holding the binary code of activities required to be finished prior to a merged action i to be allowed to be executed.
unsigned long int** gm_lib_bin;			// Global matrix holding the binary code of the activities liberated by regular action i.
unsigned long int** gm_reg_bin;			// Global matrix holding the binary code of the activities finished by executing regular action i.
unsigned long int** gm_mrg_bin;			// Global matrix holding the binary code of the activities finished by executing merged action i.

unsigned long int** gm_ptg_nrc;			// Global matrix holding the number of combinations for each ptg.
unsigned short int** gm_bin_act_cod;		// Global matrix binary activity codes. Contains all activities corresponding to a certain binary code of maximum size 2^16.
bool** gm_pre_rel;			// Global matrix precedence relations. Holds the precedence relations.
bool** gm_tra_rel;			// Global matrix transitive relations. Holds the transitive precedence relations.

double* ga_rac;				// Global array reward and cost structure. Holds the reward/cost for each activity.
double* ga_act_dur;			// Global array activity duration. Holds the (exponential) durations of the activities.
double* ga_lambda;			// Global array lambda. Holds the rate of each activity.
unsigned short int* ga_nrs;				// Global array. Holds the number of successors for each activity.
unsigned short int* ga_ptg;				// Global array. Holds the number of ptgs at a given recursion depth.
unsigned short int* ga_batch;				// Global array holding information concerning the batch array.

clock_t				gv_clock;				// Global variable holding the execution time.
clock_t				gv_clock_udc;
clock_t				gv_clock_npv;
double				gv_rate;				// Global variable rate. Holds the discount rate.
double				gv_mem;					// Global variable holding the memory consumption.
double				gv_tot_nrc;				// Global variable holding the total number of combinations.
double				gv_avg_mem;				// Global variable holding the average memory requirement per state.
double				gv_avg_mpa;				// Global variable holding the average number of parallel activities over all UDCs.
double				gv_avg_udc;				// Global variable holding the average number of UDCs per state.
float				gv_npv;					// Global variable holding the outcome npv.
int					gv_max_mem;				// Global variable holding the maximum available memory.
unsigned long int	gv_opt;					// Global variable holding the memory option.
unsigned short int	gv_reg;					// Global variable holding the number of regular actions.
unsigned short int	gv_mrg;					// Global variable holding the number of merged actions.
unsigned short int	gv_double;				// Global variable holding the number of double outgoing links.
unsigned short int	gv_mpa;					// Global variable holding the maximum number of parallel activities.
unsigned short int	gv_bnr;					// Global variable holds the number of 32-bits arrays required to store a system state.
unsigned short int	gv_n;					// Global variable holds the number of activities in the network.
unsigned short int	gv_nr_udc;				// Global variable holding the number of UDCs in memory at any time.
unsigned short int	gv_max_nr_udc;			// Global variable holding the maximum number of UDCs in memory.
bool				gv_batch;				// Global variable holding whether we have a batching assignment.
bool				gv_noprint;				// Global variable indicating if output should be printed.
bool				gv_rangen;				// Global variable indicating if RANGEN instances are used.

double				ga_bin3[] = { 1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683, 59049, 177147, 531441, 1594323, 4782969, 14348907, 43046721, 129140163, 387420489, 1162261467, 3486784401, 10460353203, 31381059609, 94143178827, 282429536481, 847288609443, 2541865828329, 7625597484987, 22876792454961, 68630377364883, 205891132094649, 617673396283947 };
double				ga_bin3_dbl[] = { 2, 6, 18, 54, 162, 486, 1458, 4374, 13122, 39366, 118098, 354294, 1062882, 3188646, 9565938, 28697814, 86093442, 258280326, 774840978, 2324522934, 6973568802, 20920706406, 62762119218, 188286357654, 564859072962, 1694577218886, 5083731656658, 15251194969974, 45753584909922, 137260754729766, 411782264189298 };
unsigned long int	ga_bin[] = { 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216, 33554432, 67108864, 134217728, 268435456, 536870912, 1073741824, 2147483648 };
unsigned long int	ga_bin_min[] = { 0, 1, 3, 7, 15, 31, 63, 127, 255, 511, 1023, 2047, 4095, 8191, 16383, 32767, 65535, 131071, 262143, 524287, 1048575, 2097151, 4194303, 8388607, 16777215, 33554431, 67108863, 134217727, 268435455, 536870911, 1073741823, 2147483647, 4294967295 };

bool multithread =false;
//////////////////////////////////////////////////////////////////////////////
///////////////PROTOTYPE FUNCTIONS////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void	f_bin(void);	// Get gm_bin_act_cod; which translates binary codes into activities.
void	f_inp(void);	// Get the input.
void	f_act(void);	// Get the actions to be performed at a given state.
void	f_udc(void);	// Get the udcs and their linkage.
void	f_npv(void);	// Get the NPVs.
void	f_kil(void);	// Destroy data structures no longer required before advancing to a next entry in the loop.

void main()
{
	
	// Initialize counter.
	unsigned short int	i;

	// Initialize batch and no-print possibility.
	gv_noprint = 1;
	gv_batch = 0;
	gv_rangen = 0;
	ga_batch = new unsigned short int[2];

	// Get gm_bin_act_cod.
	f_bin();

	// Get the maximum amount of available memory.
	printf("Please indicate the maximum of system memory (in MB) available for processing:");
	printf("\n");
	//scanf_s("%d", &gv_max_mem);
	gv_max_mem = 500;
	printf("\n");

	// Run the program.
	do
	{
		printf("\n");
		// Get input.
		f_inp();
		printf("INP[OK]---ACT");
		gv_clock = clock();	// Start the clock.
		// Get actions.
		gv_clock_udc = clock();	// Start the clock.
		f_act();
		printf("[OK]---PTG");
		// Get ptgs.
		f_udc();
		gv_clock_udc = clock() - gv_clock_udc;	// Stop the clock.
		printf("[OK]\n");
		printf("NPV");
		// Get NPVs and solution.
		f_npv();
		// Destruction phase.
		f_kil();
	} while (gv_n > 0);

	for (i = 0; i < ga_bin_min[16]; i++)
	{
		delete gm_bin_act_cod[i];
	}
	delete gm_bin_act_cod;

	delete ga_batch;
	return;
}

//////////////////////////////////////////////////////////////////////////////
///////////////INCLUDE FUNCTIONS//////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#include "F_BIN.CPP"
#include "F_INP.CPP"
#include "F_ACT.CPP"
#include "F_UDC.CPP"
#include "F_NPV.CPP"
#include "F_NPV2.CPP"
#include "F_KIL.CPP"