
void f_npv2(void)
{
	//////////////////////////////////////////////////////////////////////////
	///////////DEFINE LOCAL VARIABLES/////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	float**** lm_npv_str;		// Local matrix holding the npv values of all combinations.
	unsigned long int**** lm_ter_str;		// Local matrix tertiary structure holding the unique code of each combination required to sort them.

	unsigned long int** lm_bin_lnk;			// Local matrix holding for each link the binary value of the activities required to finish.
	unsigned short int** lm_lnk_act;			// Local matrix holding all member activities of linked to ptgs.
	bool** lm_lib_act_lst;		// Local matrix holding a list of all liberated activities in each linked to ptg.
	bool** lm_lnk_mem;			// Local matrix link membership. Is activity i a member of link l or not.

	unsigned long int* la_bin_fin;			// Local array binary values of finished activities. Holds the binary equivalent of values finished.
	unsigned long int* la_bin_idl;			// Local array binary code of idle activities. Holds the binary equivalent of values that have not yet been started.
	unsigned long int* la_bin_bus;			// Local array binary code of busy activities. Holds the binary equivalent of values that have been started, but that yet have to finish.
	unsigned long int* la_fnd_cod;			// Local array find code. Holds 16-bits of binary code.
	unsigned short int* la_ptg_act;			// Local array ptg activity. Which activities are a member of the ptg in question.
	unsigned short int* la_ptg_act_inv;		// Local array ptg activity inverse. The inverse relationship of la_ptg_act.
	unsigned short int* la_lnk_nra;			// Local array link number of activities. Holds the number of shared activities between a ptg and the linked ptg.
	unsigned short int* la_lnk_fin;			// Local array link finish. Indicates if a link is established (just a copy of la_lnk_nra).
	unsigned short int* la_act_sta;			// Local array activity status.

	unsigned long int	lv_tot_nrc;				// Local variable total number of combinations.
	unsigned short int	lv_nr32;				// Local variable which holds the number of 32-bits arrays required.
	unsigned short int	lv_nr16;				// Local variable which holds the number of 16-bits arrays required.

	unsigned short int	i;	// Simple counter.
	unsigned short int	j;	// Simple counter.
	unsigned short int	k;	// Simple counter.
	unsigned short int	l;	// Simple counter.
	unsigned short int	m;	// Simple counter.

	unsigned long int	z2;
	unsigned short int	z1;
	bool				z0;

	//////////////////////////////////////////////////////////////////////////
	///////////PROTOTYPE FUNCTIONS////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

//	void f_rid_ptg_act(unsigned short int i, unsigned short int j, unsigned short int *la_ptg_act, unsigned short int *la_ptg_act_inv, unsigned short int *la_lnk_nra, bool **lm_lnk_mem);	// Retrieve and identify ptg activities.
	void f_ter2(unsigned short int i, unsigned short int j, unsigned long int**** lm_ter_str, float**** lm_npv_str);
	double f_dec(unsigned short int i, unsigned short int j, unsigned short int* la_act_sta, unsigned short int* la_ptg_act, unsigned short int* la_ptg_act_inv, unsigned long int* la_bin_fin, unsigned long int* la_bin_idl, unsigned long int* la_bin_bus, bool** lm_lib_act_lst, unsigned short int** lm_lnk_act, unsigned long int** lm_bin_lnk, unsigned long int**** lm_ter_str, float**** lm_npv_str);	// Returns the NPV corresponding to the optimal decision.

	//////////////////////////////////////////////////////////////////////////
	///////////INITIALIZATION/////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	lm_npv_str = new float*** [gv_n];
	lm_ter_str = new unsigned long int*** [gv_n];
	for (i = (gv_n - 2); i > 0; i--)
	{
		lm_npv_str[i] = new float** [ga_ptg[i]];
		lm_ter_str[i] = new unsigned long int** [ga_ptg[i]];
	}

	la_bin_fin = new unsigned long int[gv_bnr];
	la_bin_idl = new unsigned long int[gv_bnr];
	la_bin_bus = new unsigned long int[gv_bnr];
	la_fnd_cod = new unsigned long int[gv_bnr];
	la_ptg_act = new unsigned short int[gv_n];
	la_ptg_act_inv = new unsigned short int[gv_n];

	//////////////////////////////////////////////////////////////////////////
	///////////INITIALIZE FIRST AND LAST NODE/////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	lm_npv_str[(gv_n - 1)] = new float** [1];
	lm_npv_str[(gv_n - 1)][0] = new float* [1];
	lm_npv_str[(gv_n - 1)][0][0] = new float[1];
	lm_npv_str[(gv_n - 1)][0][0][0] = static_cast<float>(ga_rac[(gv_n - 1)]);	// At the end node, the reward is collected.

	lm_ter_str[(gv_n - 1)] = new unsigned long int** [1];
	lm_ter_str[(gv_n - 1)][0] = new unsigned long int* [1];
	lm_ter_str[(gv_n - 1)][0][0] = new unsigned long int[1];
	lm_ter_str[(gv_n - 1)][0][0][0] = 0;	// At the end node, there is only one action and its status is not started. 

	//////////////////////////////////////////////////////////////////////////
	///////////DETERMINE NPV AND LOOKUP STRUCTURE/////////////////////////////
	//////////////////////////////////////////////////////////////////////////


	printf("[%d,%d", gv_n, (gv_n - 1));

	for (i = (gv_n - 2); i > 0; i--)	// Starting from the second highest recursion level, check all recursion levels (except for the first and final node; the latter one is initialized seperately).
	{
		printf(",%d", i);
		for (j = 0; j < ga_ptg[i]; j++)	// Check all ptgs at recursion level i.
		{

			//////////////////////////////////////////////////////////////////////////
			///////////CREATE TERTIARY STRUCTURE FUNCTION/////////////////////////////
			//////////////////////////////////////////////////////////////////////////

			f_ter2(i, j, lm_ter_str, lm_npv_str);		// Construct tertiary structure.

		//////////////////////////////////////////////////////////////////////
		///////INITIALIZATION/////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////

			lm_lnk_mem = new bool* [gm_nio_ptg[i][j][0]];	// Defines if an activity is a member of an outgoing link (there are gm_nio_ptg[i][j][0] outgoing links at this ptg).
			la_lnk_nra = new unsigned short int[gm_nio_ptg[i][j][0]];	// Create number of activities array (for each outgoing link).
			for (k = 0; k < gv_bnr; k++)
			{
				la_bin_fin[k] = 0;	//la_bin_fin we need to evaluate whether a link has been established (to know in which matrix of npvs we will have to look; temporary combination npv matrix (and which) or permanent combination npv matrix).
				la_bin_idl[k] = 0;	//la_bin_idl we need to obtain the number and id of idle activities (to compute the different decisions that can be made at a combination).
				la_bin_bus[k] = 0;	//la_bin_bus we need to obtain the number and id of activities that are busy (to compute costs as well as duration and probability of first finishing activity).
			}

			//////////////////////////////////////////////////////////////////////
			///////FUNCTIONS AND PROGRAM//////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////

				//////////////////////////////////////////////////////////////////
				///FUNCTION F_RID_PTG_ACT/////////////////////////////////////////
				//////////////////////////////////////////////////////////////////

			f_rid_ptg_act(i, j, la_ptg_act, la_ptg_act_inv, la_lnk_nra, lm_lnk_mem);	// Retrieve and identify ptg activities.

			//////////////////////////////////////////////////////////////////
			///OBTAIN LINK ESTABLISHING ACTIVITIES////////////////////////////
			//////////////////////////////////////////////////////////////////

			lm_bin_lnk = new unsigned long int* [gv_bnr];
			for (k = 0; k < gv_bnr; k++)
			{
				lm_bin_lnk[k] = new unsigned long int[gm_nio_ptg[i][j][0]];
				for (l = 0; l < gm_nio_ptg[i][j][0]; l++)	// Check all links.
				{
					lm_bin_lnk[k][l] = ((gm_str_ptg[k][i][j] & gm_str_ptg[k][gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]]) ^ gm_str_ptg[k][i][j]);
				}
			}

			//////////////////////////////////////////////////////////////////
			///IDENTIFY LINK MEMBER ACTIVITIES AND LIBERATED ACTIVITIES///////
			//////////////////////////////////////////////////////////////////

			lm_lnk_act = new unsigned short int* [gm_nio_ptg[i][j][0]];

			for (l = 0; l < gm_nio_ptg[i][j][0]; l++)	// Check all links.
			{
				lm_lnk_act[l] = new unsigned short int[(gv_n + 1)];
				lm_lnk_act[l][0] = 0;
				for (k = 0; k < gv_bnr; k++)
				{
					la_fnd_cod[k] = (gm_str_ptg[k][gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]] & ga_bin_min[16]);	// Binary operation such that only the first 16 bits remain (bitwise & 1111111111111111 (16 ones) and the first 16 bits of the ptg code).
					if (la_fnd_cod[k] > 0)
					{
						la_fnd_cod[k]--;	// Correct for one bit of data too short.
						for (m = 0; m < gm_bin_act_cod[la_fnd_cod[k]][0]; m++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
						{
							lm_lnk_act[l][0]++;	// Increase the number of activities present in this ptg.
							lm_lnk_act[l][lm_lnk_act[l][0]] = ((32 * k) + gm_bin_act_cod[la_fnd_cod[k]][(m + 1)]);	// Record the activity number.
						}
					}
					la_fnd_cod[k] = (gm_str_ptg[k][gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]] >> 16);	// Right shift 16 spaces; as such we only retain the second 16 bits.
					if (la_fnd_cod[k] > 0)
					{
						la_fnd_cod[k]--;	// Correct for one bit of data too short.
						for (m = 0; m < gm_bin_act_cod[la_fnd_cod[k]][0]; m++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
						{
							lm_lnk_act[l][0]++;	// Increase the number of activities present in this ptg.
							lm_lnk_act[l][lm_lnk_act[l][0]] = ((32 * k) + 16 + gm_bin_act_cod[la_fnd_cod[k]][(m + 1)]);	// Record the activity number.
						}
					}
				}
			}

			lm_lib_act_lst = new bool* [gm_nio_ptg[i][j][0]];

			for (l = 0; l < gm_nio_ptg[i][j][0]; l++)	// Check all links.
			{
				lm_lib_act_lst[l] = new bool[lm_lnk_act[l][0]];
				for (m = 0; m < lm_lnk_act[l][0]; m++)
				{
					lv_nr32 = static_cast<int>(floor(static_cast<double>(lm_lnk_act[l][(m + 1)]) / static_cast<double>(32)));
					z2 = ga_bin[static_cast<int>(lm_lnk_act[l][(m + 1)] - (lv_nr32 * 32))];
					if ((gm_str_ptg[lv_nr32][i][j] & z2) == 0)	// The activity is not a member of the current ptg.
					{
						lm_lib_act_lst[l][m] = 1;	// Activity lm_lnk_act[l][m+1] is a liberated activity.
					}
					else
					{
						lm_lib_act_lst[l][m] = 0;	// Activity lm_lnk_act[l][m+1] is not a liberated activity.
					}
				}
			}

			//////////////////////////////////////////////////////////////////
			///INITIALIZATION OF ACTIVITY STATUS//////////////////////////////
			//////////////////////////////////////////////////////////////////

			la_lnk_fin = new unsigned short int[gm_nio_ptg[i][j][0]];
			for (l = 0; l < gm_nio_ptg[i][j][0]; l++)	// Check all links moving from this ptg.
			{
				la_lnk_fin[l] = la_lnk_nra[l];	// As long as la_lnk_fin is larger than 1; there is still a member activity of link l that may be finished (without establishing link l).
			}
			la_act_sta = new unsigned short int[la_ptg_act[0]];	// The dimension of la_act_sta is determined by the number of activities in this ptg.
			m = la_ptg_act[0];
			lv_tot_nrc = 0;	// Initialize the total number of combinations for this ptg.
			lv_nr16 = static_cast<int>(floor(static_cast<double>(lv_tot_nrc) / static_cast<double>(ga_bin_min[16])));
			do
			{
				m--;
				z0 = 0;

				for (l = 0; l < gm_nio_ptg[i][j][0]; l++)	// Check if finishing this activity establishes any of the links.
				{
					if (lm_lnk_mem[l][m] == 1)	// Activity la_ptg_act[m+1] has the potential to establish link l.
					{
						if (la_lnk_fin[l] == 1)	// Finishing the activity would imply that we establish a link (i.e. enter a new ptg).
						{
							z0 = 1;	// Indicate that finishing the activity establishes a link.
						}
					}
				}
				if (z0 == 0)	// We can finish the activity without establishing a link.
				{
					la_act_sta[m] = 2;	// Finish the activity.
					lv_nr32 = static_cast<int>(floor(static_cast<double>(la_ptg_act[(m + 1)]) / static_cast<double>(32)));
					z2 = ga_bin[static_cast<int>(la_ptg_act[(m + 1)] - (lv_nr32 * 32))];
					la_bin_fin[lv_nr32] += z2;
					for (l = 0; l < gm_nio_ptg[i][j][0]; l++)	// Record the impact of finishing activity la_ptg_act[m+1].
					{
						if (lm_lnk_mem[l][m] == 1)	// Activity la_ptg_act[m+1] has the potential to establish link l.
						{
							la_lnk_fin[l]--;
						}
					}
				}
				else	// We cannot finish the activity. Its status is maximized at value 1.
				{
					la_act_sta[m] = 1;
					lv_nr32 = static_cast<int>(floor(static_cast<double>(la_ptg_act[(m + 1)]) / static_cast<double>(32)));
					z2 = ga_bin[static_cast<int>(la_ptg_act[(m + 1)] - (lv_nr32 * 32))];
					la_bin_bus[lv_nr32] += z2;
				}
			} while (m > 0);	// Do until you have arrived at the last position. We end up at m=0; the first position which has the lowest significance (i.e. impact on lm_ter_str).
			z0 = 0;	// Indicates that not all combinations have been found.

			//////////////////////////////////////////////////////////////////
			///SHRINK EXPAND PROCEDURE////////////////////////////////////////
			//////////////////////////////////////////////////////////////////

			do
			{
				if (la_act_sta[m] == 2)	// Unfinishing the last activity will impact la_lnk_fin.
				{
					//--> ENTER NEW FUNCTION TO COMPUTE DECISIONS. (last activity status 2).
					lm_npv_str[i][j][lv_nr16][(lv_tot_nrc - (lv_nr16 * ga_bin_min[16]))] = static_cast<float>(f_dec(i, j, la_act_sta, la_ptg_act, la_ptg_act_inv, la_bin_fin, la_bin_idl, la_bin_bus, lm_lib_act_lst, lm_lnk_act, lm_bin_lnk, lm_ter_str, lm_npv_str));
					for (l = 0; l < gm_nio_ptg[i][j][0]; l++)	// Record the impact of finishing activity la_ptg_act[m+1].
					{
						if (lm_lnk_mem[l][m] == 1)
						{
							la_lnk_fin[l]++;	// Activity la_ptg_act[m+1] is no longer finished. Therefore, an additional member activity may be finished before link l is established.
						}
					}
					la_act_sta[m]--;	// Decrease the status of the last activity. FROM 2 TO 1.
					lv_tot_nrc++;	// Increase the number of combinations at this ptg.
					lv_nr16 = static_cast<int>(floor(static_cast<double>(lv_tot_nrc) / static_cast<double>(ga_bin_min[16])));
					lv_nr32 = static_cast<int>(floor(static_cast<double>(la_ptg_act[(m + 1)]) / static_cast<double>(32)));
					z2 = ga_bin[static_cast<int>(la_ptg_act[(m + 1)] - (lv_nr32 * 32))];
					la_bin_fin[lv_nr32] -= z2;
					la_bin_bus[lv_nr32] += z2;
				}
				//--> ENTER NEW FUNCTION TO COMPUTE DECISIONS. (last activity status 1).
				lm_npv_str[i][j][lv_nr16][(lv_tot_nrc - (lv_nr16 * ga_bin_min[16]))] = static_cast<float>(f_dec(i, j, la_act_sta, la_ptg_act, la_ptg_act_inv, la_bin_fin, la_bin_idl, la_bin_bus, lm_lib_act_lst, lm_lnk_act, lm_bin_lnk, lm_ter_str, lm_npv_str));
				la_act_sta[m]--;
				lv_tot_nrc++;	// Increase the number of combinations at this ptg.
				lv_nr16 = static_cast<int>(floor(static_cast<double>(lv_tot_nrc) / static_cast<double>(ga_bin_min[16])));
				lv_nr32 = static_cast<int>(floor(static_cast<double>(la_ptg_act[(m + 1)]) / static_cast<double>(32)));
				z2 = ga_bin[static_cast<int>(la_ptg_act[(m + 1)] - (lv_nr32 * 32))];
				la_bin_bus[lv_nr32] -= z2;
				la_bin_idl[lv_nr32] += z2;
				//--> ENTER NEW FUNCTION TO COMPUTE DECISIONS. (last activity status 0).
				lm_npv_str[i][j][lv_nr16][(lv_tot_nrc - (lv_nr16 * ga_bin_min[16]))] = static_cast<float>(f_dec(i, j, la_act_sta, la_ptg_act, la_ptg_act_inv, la_bin_fin, la_bin_idl, la_bin_bus, lm_lib_act_lst, lm_lnk_act, lm_bin_lnk, lm_ter_str, lm_npv_str));
				lv_tot_nrc++;	// Increase the number of combinations at this ptg.
				lv_nr16 = static_cast<int>(floor(static_cast<double>(lv_tot_nrc) / static_cast<double>(ga_bin_min[16])));

				while ((la_act_sta[m] == 0) && (m < (la_ptg_act[0] - 1)))	// Do until you arrive at a) an activity which status can further be decreased; b) the first activity.
				{
					m++;	// Increase the position.
				}

				if (m == (la_ptg_act[0] - 1))	// You are at the first activity.
				{
					if (la_act_sta[m] == 0)	// All combinations have been found.
					{
						break;// All combinations have been found.
					}
				}
				if (z0 == 0)	// There are still combinations to be found.
				{
					lv_nr32 = static_cast<int>(floor(static_cast<double>(la_ptg_act[(m + 1)]) / static_cast<double>(32)));
					z2 = ga_bin[static_cast<int>(la_ptg_act[(m + 1)] - (lv_nr32 * 32))];
					if (la_act_sta[m] == 2)	// Unfinishing the activity will impact la_lnk_fin.
					{
						for (l = 0; l < gm_nio_ptg[i][j][0]; l++)	// Record the impact of unfinishing activity la_ptg_act[m+1].
						{
							if (lm_lnk_mem[l][m] == 1)
							{
								la_lnk_fin[l]++;	// Activity la_ptg_act[m+1] is no longer finished. Therefore, an additional member activity may be finished before link l is established.
							}
						}
						// Decrease the status of the activity. FROM 2 TO 1.
						la_bin_fin[lv_nr32] -= z2;
						la_bin_bus[lv_nr32] += z2;
					}
					else	// Activity status equals 1.
					{
						// Decrease the status of the activity. FROM 1 TO 0.
						la_bin_bus[lv_nr32] -= z2;
						la_bin_idl[lv_nr32] += z2;
					}
					la_act_sta[m]--;
					do
					{
						m--;	// Decrease the position. Current position (value 0) will be maximized.
						z1 = 0;	// Indicates if finishing activity la_ptg_act[m+1] would establish an unwanted link.
						for (l = 0; l < gm_nio_ptg[i][j][0]; l++)	// Check all links.
						{
							if (lm_lnk_mem[l][m] == 1)	// Finishing activity la_ptg_act[m+1] has the potential to establish link l.
							{
								if (la_lnk_fin[l] == 1)	// Finishing activity la_ptg_act[m+1] would establish an unwanted link.
								{
									z1 = 1;	// Indicate that the status of activity la_ptg_act[m+1] is not to be maximized.
									la_act_sta[m] = 1;	// Activity la_ptg_act[m+1] is not allowed to finish, so it just starts. FROM 0 TO 1.
									lv_nr32 = static_cast<int>(floor(static_cast<double>(la_ptg_act[(m + 1)]) / static_cast<double>(32)));
									z2 = ga_bin[static_cast<int>(la_ptg_act[(m + 1)] - (lv_nr32 * 32))];
									la_bin_idl[lv_nr32] -= z2;
									la_bin_bus[lv_nr32] += z2;
									break;
								}
							}
						}
						if (z1 == 0)	// Activity la_ptg_act[m+1] is allowed to finish.
						{
							la_act_sta[m] = 2;	// Indicate that activity la_ptg_act[m+1] has finished. FROM 0 TO 2.
							lv_nr32 = static_cast<int>(floor(static_cast<double>(la_ptg_act[(m + 1)]) / static_cast<double>(32)));
							z2 = ga_bin[static_cast<int>(la_ptg_act[(m + 1)] - (lv_nr32 * 32))];
							la_bin_idl[lv_nr32] -= z2;
							la_bin_fin[lv_nr32] += z2;
							for (l = 0; l < gm_nio_ptg[i][j][0]; l++)	// Check all links.
							{
								if (lm_lnk_mem[l][m] == 1)
								{
									la_lnk_fin[l]--;	// Indicate that link l (of which activity la_ptg_act[m+1] is a member activity) is one step closer to becoming established.
								}
							}
						}

					} while (m > 0);	// Do until you arrive at the last activity.
				}
			} while (z0 == 0);	// Repeat reverse shrink-expand procedure until all combinations have been found.

		//////////////////////////////////////////////////////////////////////
		///////DESTRUCTION PHASE PRIOR TO MOVING TO THE NEXT PTG//////////////
		//////////////////////////////////////////////////////////////////////

			for (l = 0; l < gm_nio_ptg[i][j][0]; l++)	// Check all outgoing links.
			{
				if (gm_nio_ptg[gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]][1] == 1)
				{
					lv_nr16 = static_cast<int>(floor(static_cast<double>(gm_ptg_nrc[gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]]) / static_cast<double>(ga_bin_min[16])));
					for (m = 0; m < (lv_nr16 + 1); m++)
					{
						delete lm_npv_str[gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]][m];
						delete lm_ter_str[gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]][m];
					}
					delete lm_npv_str[gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]];
					delete lm_ter_str[gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]];
					gm_nio_ptg[gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]][1]--;
				}
				else
				{
					gm_nio_ptg[gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]][1]--;
				}
			}

			for (k = 0; k < gv_bnr; k++)
			{
				delete lm_bin_lnk[k];
			}
			delete lm_bin_lnk;
			for (l = 0; l < gm_nio_ptg[i][j][0]; l++)
			{
				delete lm_lnk_act[l];
				delete lm_lib_act_lst[l];
				delete lm_lnk_mem[l];
			}
			delete lm_lnk_act;
			delete lm_lib_act_lst;
			delete lm_lnk_mem;
			delete la_lnk_nra;
			delete la_lnk_fin;
			delete la_act_sta;

			//////////////////////////////////////////////////////////////////////
			///////BUG DETECTION//////////////////////////////////////////////////
			//////////////////////////////////////////////////////////////////////

			if (lv_tot_nrc != gm_ptg_nrc[i][j])
			{
				printf("Wrong number of combinations");
				throw "Wrong number of combinations.";
			}
		}
	}

	lv_nr16 = static_cast<int>(floor(static_cast<double>(gm_ptg_nrc[1][0]) / static_cast<double>(ga_bin_min[16])));

	gv_npv = lm_npv_str[1][0][lv_nr16][(gm_ptg_nrc[1][0] - 1 - (lv_nr16 * ga_bin_min[16]))];
	//	gv_npv=lm_npv_str[1][0][0][(gm_ptg_nrc[1][0]-1)];	// Set outcome NPV.
	printf("]");

	//////////////////////////////////////////////////////////////////////////
	///////////DESTRUCTION PHASE//////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	lv_nr16 = static_cast<int>(floor(static_cast<double>(gm_ptg_nrc[gm_dpt_ptg[1][0][0]][gm_lnk_ptg[1][0][0]]) / static_cast<double>(ga_bin_min[16])));
	for (i = (gv_n - 2); i > 0; i--)
	{
		delete lm_npv_str[i];
		delete lm_ter_str[i];
	}
	delete lm_npv_str[(gv_n - 1)];
	delete lm_npv_str;

	delete lm_ter_str[(gv_n - 1)];
	delete lm_ter_str;

	delete la_bin_fin;
	delete la_bin_idl;
	delete la_bin_bus;
	delete la_fnd_cod;
	delete la_ptg_act;
	delete la_ptg_act_inv;
}

double f_dec(unsigned short int i, unsigned short int j, unsigned short int* la_act_sta, unsigned short int* la_ptg_act, unsigned short int* la_ptg_act_inv, unsigned long int* la_bin_fin, unsigned long int* la_bin_idl, unsigned long int* la_bin_bus, bool** lm_lib_act_lst, unsigned short int** lm_lnk_act, unsigned long int** lm_bin_lnk, unsigned long int**** lm_ter_str, float**** lm_npv_str)	// Create all decisions given a set of idle activities.
{
	//////////////////////////////////////////////////////////////////////////
	///////////DEFINE LOCAL VARIABLES/////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	unsigned long int* la_fnd_cod;		// Local array find code. Holds 16-bits of binary code.
	unsigned short int* la_idl_act;		// Local array holding the idle activities.
	unsigned short int* la_sub_pos;		// Holds the position.

	double				lv_tmp_npv;			// Holds the temporary npv value.
	double				lv_max_npv;			// Holds the maximum npv value encountered.
	double				lv_costs;			// Local variable costs. Holds the cost of a decision to start a set of activities.
	unsigned long int	lv_tmp_ter;			// Local variable holding the temporary tertiary value.
	unsigned short int	lv_nr32;			// Local variable which holds the number of 32-bits arrays required.

	unsigned short int	k;	// Simple counter.
	unsigned short int	l;	// Simple counter.
	unsigned short int	m;	// Simple counter.
	list<future<bool>> list_parallel_f_fnd;
	unsigned long int	z2;
	bool				z1;

	//////////////////////////////////////////////////////////////////////////
	///////////PROTOTYPE FUNCTIONS////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////
	double f_npv(unsigned short int i, unsigned short int j, unsigned short int* la_act_sta, unsigned short int* la_ptg_act, unsigned short int* la_ptg_act_inv, unsigned long int* la_bin_fin, unsigned long int* la_bin_bus, bool** lm_lib_act_lst, unsigned short int** lm_lnk_act, unsigned long int** lm_bin_lnk, unsigned long int**** lm_ter_str, float**** lm_npv_str);	// Given a decision, computes the npv
	double f_fnd(unsigned short int i, unsigned short int j, unsigned long int lv_tmp_ter, unsigned long gm_ptg_nrc_ij, unsigned long int**** lm_ter_str, float**** lm_npv_str);	// Finds the npv value corresponding to a tertiary value.
	bool parallel_f_fnd(double* max, double add, unsigned short int i, unsigned short int j, unsigned long int lv_tmp_ter, unsigned long gm_ptg_nrc_ij, unsigned long int**** lm_ter_str, float**** lm_npv_str);

	//////////////////////////////////////////////////////////////////////////
	///////////INITIALIZATION/////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	la_fnd_cod = new unsigned long int[gv_bnr];
	la_idl_act = new unsigned short int[(gv_n + 1)];
	la_sub_pos = new unsigned short int[gv_n];

	lv_max_npv = -999999999;

	//////////////////////////////////////////////////////////////////////////
	///////////DO NOT START ADDITIONAL ACTIVITIES/////////////////////////////
	//////////////////////////////////////////////////////////////////////////


	for (k = 0; k < gv_bnr; k++)
	{
		if (la_bin_bus[k] > 0)
		{
			lv_tmp_npv = f_npv(i, j, la_act_sta, la_ptg_act, la_ptg_act_inv, la_bin_fin, la_bin_bus, lm_lib_act_lst, lm_lnk_act, lm_bin_lnk, lm_ter_str, lm_npv_str);
			if (lv_max_npv < lv_tmp_npv)
				lv_max_npv = lv_tmp_npv;	// Maximize npv.
			break;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	///////////START ADDITIONAL ACTIVITIES////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	z1 = 0;
	for (k = 0; k < gv_bnr; k++)
	{
		if (la_bin_idl[k] > 0)
		{
			z1 = 1;
			break;
		}
	}
	if (z1 == 1)	// Start additional activities.
	{

		//////////////////////////////////////////////////////////////////////
		///////RETRIEVE AND IDENTIFY IDLE ACTIVITIES//////////////////////////
		//////////////////////////////////////////////////////////////////////

		la_idl_act[0] = 0;
		for (k = 0; k < gv_bnr; k++)
		{
			// Check first 16 bits.
			la_fnd_cod[k] = (la_bin_idl[k] & ga_bin_min[16]);	// Binary operation such that only the first 16 bits remain (bitwise & 1111111111111111 (16 ones) and the first 16 bits of the ptg code).
			if (la_fnd_cod[k] > 0)
			{
				la_fnd_cod[k]--;	// Correct for one bit of data too short.
				for (l = 0; l < gm_bin_act_cod[la_fnd_cod[k]][0]; l++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
				{
					la_idl_act[0]++;	// Increase the number of activities present in this ptg.
					la_idl_act[la_idl_act[0]] = ((32 * k) + gm_bin_act_cod[la_fnd_cod[k]][(l + 1)]);	// Record the activity number.
				}
			}
			// Check second 16 bits.
			la_fnd_cod[k] = (la_bin_idl[k] >> 16);	// Right shift 16 spaces; as such we only retain the second 16 bits.
			if (la_fnd_cod[k] > 0)
			{
				la_fnd_cod[k]--;	// Correct for one bit of data too short.
				for (l = 0; l < gm_bin_act_cod[la_fnd_cod[k]][0]; l++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
				{
					la_idl_act[0]++;	// Increase the number of activities present in this ptg.
					la_idl_act[la_idl_act[0]] = ((32 * k) + 16 + gm_bin_act_cod[la_fnd_cod[k]][(l + 1)]);	// Record the activity number.
				}
			}
		}

		//////////////////////////////////////////////////////////////////////
		///////INITIALIZATION/////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////

		lv_tmp_ter = 0;
		for (m = 0; m < la_ptg_act[0]; m++)
		{
			if (la_act_sta[m] == 2)
			{
				lv_tmp_ter += static_cast<unsigned long int>(ga_bin3_dbl[m]);
			}
			else if (la_act_sta[m] == 1)
			{
				lv_tmp_ter += static_cast<unsigned long int>(ga_bin3[m]);
			}
		}
		m = 0;	// Indicate position.
		la_sub_pos[m] = 0;	// Indicate value at position.
		lv_nr32 = static_cast<int>(floor(static_cast<double>(la_idl_act[(la_sub_pos[m] + 1)]) / static_cast<double>(32)));
		z2 = ga_bin[static_cast<int>(la_idl_act[(la_sub_pos[m] + 1)] - (lv_nr32 * 32))];
		la_bin_bus[lv_nr32] += z2;
		lv_costs = ga_rac[la_idl_act[(la_sub_pos[m] + 1)]];
		//		if(la_act_sta[la_ptg_act_inv[la_idl_act[(la_sub_pos[m]+1)]]]==2){printf("Error++act_sta1");throw"Error";}
		la_act_sta[la_ptg_act_inv[la_idl_act[(la_sub_pos[m] + 1)]]]++;
		lv_tmp_ter += static_cast<unsigned long int>(ga_bin3[la_ptg_act_inv[la_idl_act[(la_sub_pos[m] + 1)]]]);
		//--> WRITE AWAY DECISION.



		if (multithread)list_parallel_f_fnd.push_back(async(parallel_f_fnd, &lv_max_npv, lv_costs, i, j, lv_tmp_ter, gm_ptg_nrc[i][j], lm_ter_str, lm_npv_str));
		else {
			lv_tmp_npv = (lv_costs + f_fnd(i, j, lv_tmp_ter, gm_ptg_nrc[i][j], lm_ter_str, lm_npv_str));
			if (lv_max_npv < lv_tmp_npv)
			{
				lv_max_npv = lv_tmp_npv;	// Maximize npv.
			}
		}

		//////////////////////////////////////////////////////////////////////
		///////SHRINK EXPAND PROCEDURE////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////

		if (la_idl_act[0] > 1)	// Multiple idle activities are available for starting.
		{
			z1 = 0;	// Indicate that not all decisions have been found.
			do
			{
				do	// Expand.
				{
					//					if(m>gv_n){printf("Error++m1");throw"Error";}
					m++;	// Increase position.
					la_sub_pos[m] = (la_sub_pos[(m - 1)] + 1);	// Set value at the new position.
					lv_nr32 = static_cast<int>(floor(static_cast<double>(la_idl_act[(la_sub_pos[m] + 1)]) / static_cast<double>(32)));
					z2 = ga_bin[static_cast<int>(la_idl_act[(la_sub_pos[m] + 1)] - (lv_nr32 * 32))];
					la_bin_bus[lv_nr32] += z2;
					lv_costs += ga_rac[la_idl_act[(la_sub_pos[m] + 1)]];
					//					if(la_act_sta[la_ptg_act_inv[la_idl_act[(la_sub_pos[m]+1)]]]==2){printf("Error++act_sta1");throw"Error";}
					la_act_sta[la_ptg_act_inv[la_idl_act[(la_sub_pos[m] + 1)]]]++;
					lv_tmp_ter += static_cast<unsigned long int>(ga_bin3[la_ptg_act_inv[la_idl_act[(la_sub_pos[m] + 1)]]]);
					//--> WRITE AWAY DECISION.


					if (multithread)list_parallel_f_fnd.push_back(async(parallel_f_fnd, &lv_max_npv, lv_costs, i, j, lv_tmp_ter, gm_ptg_nrc[i][j], lm_ter_str, lm_npv_str));
					else {
						lv_tmp_npv = (lv_costs + f_fnd(i, j, lv_tmp_ter, gm_ptg_nrc[i][j], lm_ter_str, lm_npv_str));
						if (lv_max_npv < lv_tmp_npv)
						{
							lv_max_npv = lv_tmp_npv;	// Maximize npv.
						}
					}


				} while (la_sub_pos[m] < (la_idl_act[0] - 1));	// Do until the value at the position reaches its maximum.

				do	// Shrink.
				{
					// Remove the last idle activity.
					lv_nr32 = static_cast<int>(floor(static_cast<double>(la_idl_act[(la_sub_pos[m] + 1)]) / static_cast<double>(32)));
					z2 = ga_bin[static_cast<int>(la_idl_act[(la_sub_pos[m] + 1)] - (lv_nr32 * 32))];
					la_bin_bus[lv_nr32] -= z2;
					lv_costs -= ga_rac[la_idl_act[(la_sub_pos[m] + 1)]];
					//					if(la_act_sta[la_ptg_act_inv[la_idl_act[(la_sub_pos[m]+1)]]]==0){printf("Error11");throw"Error";}
					la_act_sta[la_ptg_act_inv[la_idl_act[(la_sub_pos[m] + 1)]]]--;
					lv_tmp_ter -= static_cast<unsigned long int>(ga_bin3[la_ptg_act_inv[la_idl_act[(la_sub_pos[m] + 1)]]]);

					// Remove the second last idle activity.
//					if(m==0){printf("Error12");throw"Error";}
					m--;	// Decrease position.
					lv_nr32 = static_cast<int>(floor(static_cast<double>(la_idl_act[(la_sub_pos[m] + 1)]) / static_cast<double>(32)));
					z2 = ga_bin[static_cast<int>(la_idl_act[(la_sub_pos[m] + 1)] - (lv_nr32 * 32))];
					la_bin_bus[lv_nr32] -= z2;
					lv_costs -= ga_rac[la_idl_act[(la_sub_pos[m] + 1)]];
					//					if(la_act_sta[la_ptg_act_inv[la_idl_act[(la_sub_pos[m]+1)]]]==0){printf("Error13");throw"Error";}
					la_act_sta[la_ptg_act_inv[la_idl_act[(la_sub_pos[m] + 1)]]]--;
					lv_tmp_ter -= static_cast<unsigned long int>(ga_bin3[la_ptg_act_inv[la_idl_act[(la_sub_pos[m] + 1)]]]);

					// Add a new idle activity.
//					if(la_sub_pos[m]>gv_n){printf("Error++subpos1");throw"Error";}
					la_sub_pos[m]++;	// Increase value at the new position.
					lv_nr32 = static_cast<int>(floor(static_cast<double>(la_idl_act[(la_sub_pos[m] + 1)]) / static_cast<double>(32)));
					z2 = ga_bin[static_cast<int>(la_idl_act[(la_sub_pos[m] + 1)] - (lv_nr32 * 32))];
					la_bin_bus[lv_nr32] += z2;
					lv_costs += ga_rac[la_idl_act[(la_sub_pos[m] + 1)]];
					//					if(la_act_sta[la_ptg_act_inv[la_idl_act[(la_sub_pos[m]+1)]]]==2){printf("Error++act_sta1");throw"Error";}
					la_act_sta[la_ptg_act_inv[la_idl_act[(la_sub_pos[m] + 1)]]]++;
					lv_tmp_ter += static_cast<unsigned long int>(ga_bin3[la_ptg_act_inv[la_idl_act[(la_sub_pos[m] + 1)]]]);

					//--> WRITE AWAY DECISION.
					if (multithread)list_parallel_f_fnd.push_back(async(parallel_f_fnd, &lv_max_npv, lv_costs, i, j, lv_tmp_ter, gm_ptg_nrc[i][j], lm_ter_str, lm_npv_str));
					else {
						lv_tmp_npv = (lv_costs + f_fnd(i, j, lv_tmp_ter, gm_ptg_nrc[i][j], lm_ter_str, lm_npv_str));
						if (lv_max_npv < lv_tmp_npv)
						{
							lv_max_npv = lv_tmp_npv;	// Maximize npv.
						}
					}

				} while ((m > 0) && (la_sub_pos[m] == (la_idl_act[0] - 1)));	// Do as long as: a) the first activity has not yet been reached, b) the activity is still at its maximum.

				if (m == 0 && la_sub_pos[m] == (la_idl_act[0] - 1))
				{

					z1 = 1;	// All decisions have been found.
					break;
				}
			} while (z1 == 0);	// Do until all decisions have been found. If all decisions have been found, only the last idle activity is set busy, correct this and exit this function. If only 1 idle action was present, this action is also the last action...
		}

		lv_nr32 = static_cast<int>(floor(static_cast<double>(la_idl_act[(la_sub_pos[m] + 1)]) / static_cast<double>(32)));
		z2 = ga_bin[static_cast<int>(la_idl_act[(la_sub_pos[m] + 1)] - (lv_nr32 * 32))];
		la_bin_bus[lv_nr32] -= z2;
		//		if(la_act_sta[la_ptg_act_inv[la_idl_act[(la_sub_pos[m]+1)]]]==0){printf("Error14");throw"Error";}
		la_act_sta[la_ptg_act_inv[la_idl_act[(la_sub_pos[m] + 1)]]]--;
	}

	//////////////////////////////////////////////////////////////////////////
	///////////DESTRUCTION PHASE//////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	delete la_fnd_cod;
	delete la_idl_act;
	delete la_sub_pos;

	//////////////////////////////////////////////////////////////////////////
	///////////RETURN NPV CORRESPONDING TO THE OPTIMAL DECISION///////////////
	//////////////////////////////////////////////////////////////////////////
	if (multithread)
	{
		list<future<bool>> ::iterator it;
		for (it = list_parallel_f_fnd.begin(); it != list_parallel_f_fnd.end(); ++it)
			it->get();
	}

	return lv_max_npv;
}

double f_npv(unsigned short int i, unsigned short int j, unsigned short int* la_act_sta, unsigned short int* la_ptg_act, unsigned short int* la_ptg_act_inv, unsigned long int* la_bin_fin, unsigned long int* la_bin_bus, bool** lm_lib_act_lst, unsigned short int** lm_lnk_act, unsigned long int** lm_bin_lnk, unsigned long int**** lm_ter_str, float**** lm_npv_str)
{
	//////////////////////////////////////////////////////////////////////////
	///////////DEFINE LOCAL VARIABLES/////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	unsigned long int* la_cur_bin;
	unsigned long int* la_fnd_cod;		// Local array find code. Holds 16-bits of binary code.
	unsigned short int* la_bus_act;		// Local array holding the busy activities.

	double				lv_lambda;			// Holds the total rate.
	double				lv_factor;			// Holds the discounting factor.
	double				lv_prob;			// Holds the probability of an activity finishing first.
	double				lv_tmp_npv;			// Holds the temporary npv value.
	unsigned long int	lv_tmp_ter;			// Holds the temporary tertiary value.
	unsigned short int	lv_nr32;			// Local variable which holds the number of 32-bits arrays required.
	unsigned short int	lv_lnk;				// Local variable which holds the link corresponding to the maximum binary code.

	unsigned short int	a;	// Simple counter.
	unsigned short int	k;	// Simple counter.
	unsigned short int	l;	// Simple counter.
	unsigned short int	m;	// Simple counter.
	unsigned short int	n;	// Simple counter.

	unsigned long int	z3;
	unsigned long int	z2;
	unsigned short int	z1;
	unsigned short int	z0;

	//////////////////////////////////////////////////////////////////////////
	///////////PROTOTYPE FUNCTIONS////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	double f_fnd(unsigned short int i, unsigned short int j, unsigned long int lv_tmp_ter, unsigned long gm_ptg_nrc_ij, unsigned long int**** lm_ter_str, float**** lm_npv_str);	// Finds the npv value corresponding to a tertiary value.

	//////////////////////////////////////////////////////////////////////////
	///////////INITIALIZATION/////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	la_cur_bin = new unsigned long int[gv_bnr];
	la_fnd_cod = new unsigned long int[gv_bnr];
	la_bus_act = new unsigned short int[gv_n];

	//////////////////////////////////////////////////////////////////////////
	///////////RETRIEVE AND IDENTIFY BUSY ACTIVITIES//////////////////////////
	//////////////////////////////////////////////////////////////////////////

	la_bus_act[0] = 0;
	for (k = 0; k < gv_bnr; k++)
	{
		// Check first 16 bits.
		la_fnd_cod[k] = (la_bin_bus[k] & ga_bin_min[16]);	// Binary operation such that only the first 16 bits remain (bitwise & 1111111111111111 (16 ones) and the first 16 bits of the ptg code).
		if (la_fnd_cod[k] > 0)
		{
			la_fnd_cod[k]--;	// Correct for one bit of data too short.
			for (l = 0; l < gm_bin_act_cod[la_fnd_cod[k]][0]; l++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
			{
				la_bus_act[0]++;	// Increase the number of activities present in this ptg.
				la_bus_act[la_bus_act[0]] = ((32 * k) + gm_bin_act_cod[la_fnd_cod[k]][(l + 1)]);	// Record the activity number.
			}
		}
		// Check second 16 bits.
		la_fnd_cod[k] = (la_bin_bus[k] >> 16);	// Right shift 16 spaces; as such we only retain the second 16 bits.
		if (la_fnd_cod[k] > 0)
		{
			la_fnd_cod[k]--;	// Correct for one bit of data too short.
			for (l = 0; l < gm_bin_act_cod[la_fnd_cod[k]][0]; l++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
			{
				la_bus_act[0]++;	// Increase the number of activities present in this ptg.
				la_bus_act[la_bus_act[0]] = ((32 * k) + 16 + gm_bin_act_cod[la_fnd_cod[k]][(l + 1)]);	// Record the activity number.
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	///////////INITIALIZATION/////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	lv_lambda = 0;	// Initialize lv_lambda; the rate of all busy activities.
	for (m = 0; m < la_bus_act[0]; m++)
	{
		lv_lambda += ga_lambda[la_bus_act[m + 1]];
	}
	lv_factor = (lv_lambda / (lv_lambda + gv_rate));	// Compute the discount factor.
	lv_tmp_npv = 0;	// Initialize the temporary npv value.

	//////////////////////////////////////////////////////////////////////////
	///////////EVALUATE EACH BUSY ACTIVITY FINISHING//////////////////////////
	//////////////////////////////////////////////////////////////////////////

	for (m = 0; m < la_bus_act[0]; m++)	// Finish activity la_bus_act[m+1].
	{
		//		if(la_act_sta[la_ptg_act_inv[la_bus_act[m+1]]]==2){printf("Error++act_sta1");throw"Error";}
		la_act_sta[la_ptg_act_inv[la_bus_act[m + 1]]]++;	// Indicate that activity la_bus_act[m+1] has finished.
		lv_prob = (ga_lambda[la_bus_act[m + 1]] / lv_lambda);	// Probability of activity la_bus_act[m+1] ending first.
		lv_tmp_ter = 0;	// Initialize the temporary tertiary value.
		lv_lnk = gm_nio_ptg[i][j][0];
		lv_nr32 = static_cast<int>(floor(static_cast<double>(la_bus_act[m + 1]) / static_cast<double>(32)));
		z2 = ga_bin[static_cast<int>(la_bus_act[m + 1] - (lv_nr32 * 32))];
		la_bin_fin[lv_nr32] += z2;
		for (l = 0; l < gm_nio_ptg[i][j][0]; l++)	// Check all outgoing links and select the link that has the highest binary equivalent.
		{
			z0 = 0;
			for (k = 0; k < gv_bnr; k++)
			{
				// Look at those activities that are in the current UDC but NOT in the target UDC, if these activities are finished a link is established towards the targed UDC.
				// A = gm_str_ptg[k][i][j] is binary code of the current UDC.
				// B = gm_str_ptg[k][gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]] is binary code of the target UDC.
				// A & B holds only the shared activities. (A & B) ^ A holds the activities belonging to A and not to B (i.e. they need to be finished in order to establish a link to B).
				// ((((A & B) ^ A) & la_bin_fin) holds those activities that are unique to A that are finished. 
				// if ((((A & B) ^ A) & la_bin_fin)=(((A & B) ^ A) (i.e. if all actions unique to A have finished) a link is established.

				la_cur_bin[k] = ((gm_str_ptg[k][i][j] & gm_str_ptg[k][gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]]) ^ gm_str_ptg[k][i][j]);
				if ((la_cur_bin[k] & la_bin_fin[k]) == la_cur_bin[k])
				{
					z0++;
				}
			}
			if (z0 == gv_bnr)	// Link l has been established. Now is it the correct link? Finished activities are a subset of other remaining finished activities.
			{
				// For now however first check subset procedure.
				// If the liberated activities of the newly established link B are a subset of the previous link A, link A is superior, otherwise link A should be a subset of B and therefore B should be superior.
				// The liberated activities for link A are given by gm_lnk_lib[k][i][j][A] (binary); those for B are given in gm_lnk_lib[k][i][j][B] (binary).
				// In fact, it is known in advance whether the liberated activities of link A are a subset of those of link B.
				// However, not in all situations, both links are established at the same time => both links are valid.
				// Linked to UDC is gm_str_ptg[k][gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]] = X.
				// Current UDC is gm_str_ptg[k][i][j] = Y.
				// Liberated activities are (X & Y) ^ X (or the shared actions between both UDCs xor the linked UDC as such only the liberated activities remain).

				if (lv_lnk == gm_nio_ptg[i][j][0])	// No link has yet been found.
				{
					lv_lnk = l;
				}
				else	// A link has already been found, determine the best link.
				{
					z1 = 0;
					for (k = 0; k < gv_bnr; k++)
					{
						z3 = ((gm_str_ptg[k][gm_dpt_ptg[i][j][lv_lnk]][gm_lnk_ptg[i][j][lv_lnk]] & gm_str_ptg[k][i][j]) ^ gm_str_ptg[k][gm_dpt_ptg[i][j][lv_lnk]][gm_lnk_ptg[i][j][lv_lnk]]);
						if ((z3 & ((gm_str_ptg[k][gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]] & gm_str_ptg[k][i][j]) ^ gm_str_ptg[k][gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]])) == z3)	// Link lv_lnk is a subset.
						{
							z1++;
						}
						if (z1 == gv_bnr)	// Link lv_lnk is a subset of link l, link l is superior.
						{
							lv_lnk = l;
						}
					}
				}
			}
		}
		la_bin_fin[lv_nr32] -= z2;	// Restore old value of la_bin_fin.
		if (lv_lnk < gm_nio_ptg[i][j][0])	// A link has been established.
		{
			// Compute tertiary value.
			a = 0;
			for (n = 0; n < lm_lnk_act[lv_lnk][0]; n++)
			{
				if (lm_lib_act_lst[lv_lnk][n] == 0)	// Liberated activities do not count. Only take into account non liberated activities.
				{
					// Find in la_ptg_act the first corresponding activity.
					while (lm_lnk_act[lv_lnk][(n + 1)] != la_ptg_act[(a + 1)])	// Do until you have a match.
					{
						a++;
					}
					if (la_act_sta[a] == 2)
					{
						lv_tmp_ter += static_cast<unsigned long int>(ga_bin3_dbl[n]);
					}
					else if (la_act_sta[a] == 1)
					{
						lv_tmp_ter += static_cast<unsigned long int>(ga_bin3[n]);
					}
				}
			}
			lv_tmp_npv += (lv_prob * lv_factor * f_fnd(gm_dpt_ptg[i][j][lv_lnk], gm_lnk_ptg[i][j][lv_lnk], lv_tmp_ter, gm_ptg_nrc[gm_dpt_ptg[i][j][lv_lnk]][gm_lnk_ptg[i][j][lv_lnk]], lm_ter_str, lm_npv_str));	// Find the npv value corresponding to this tertiary value.
		}
		else	// No link has been established.
		{
			// Compute tertiary value.
			for (n = 0; n < la_ptg_act[0]; n++)
			{
				if (la_act_sta[n] == 2)
				{
					lv_tmp_ter += static_cast<unsigned long int>(ga_bin3_dbl[n]);
				}
				else if (la_act_sta[n] == 1)
				{
					lv_tmp_ter += static_cast<unsigned long int>(ga_bin3[n]);
				}
			}
			lv_tmp_npv += (lv_prob * lv_factor * f_fnd(i, j, lv_tmp_ter, gm_ptg_nrc[i][j], lm_ter_str, lm_npv_str));	// Find the npv value corresponding to this tertiary value.
		}

		//		if(la_act_sta[la_ptg_act_inv[la_bus_act[m+1]]]==0){printf("Error15");throw"Error";}
		la_act_sta[la_ptg_act_inv[la_bus_act[m + 1]]]--;	// Indicate that activity la_bus_act[m+1] has become busy once more.
	}

	//////////////////////////////////////////////////////////////////////////
	///////////DESTRUCTION PHASE//////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	delete la_cur_bin;
	delete la_fnd_cod;
	delete la_bus_act;

	//////////////////////////////////////////////////////////////////////////
	///////////RETURN NPV/////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	return lv_tmp_npv;
}

double f_fnd(unsigned short int i, unsigned short int j, unsigned long int lv_tmp_ter, unsigned long gm_ptg_nrc_ij, unsigned long int**** lm_ter_str, float**** lm_npv_str)	// Find the npv value corresponding to the tertiary value. i and j indicate the recursion depth and ptg identification.
{
	//////////////////////////////////////////////////////////////////////////
	///////////DEFINE LOCAL VARIABLES/////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	unsigned short int	lv_nr16_cut;
	unsigned long int	lv_cut;
	unsigned long int	lv_cut_upper;
	unsigned long int	lv_cut_lower;

	unsigned long int	z2;

	//////////////////////////////////////////////////////////////////////////
	///////////INITIALIZATION/////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	// First check in which lv_nr16_cut block the combination falls.
	double ga_bin_min_16 = ga_bin_min[16];
	double floar_lv_nr16_cut = floor(static_cast<double>(gm_ptg_nrc_ij) / static_cast<double>(ga_bin_min_16));
	lv_nr16_cut = static_cast<unsigned short int>(floar_lv_nr16_cut);
	while (lm_ter_str[i][j][lv_nr16_cut][0] < lv_tmp_ter)	// Increase lv_nr16_cut until you arrive at the correct block of the tertiary structure (position 0 holds the highest value).
	{
		lv_nr16_cut--;
	}
	//	if(lv_nr16_cut>=ga_bin_min[16]){printf("lv_nr16_cut too big");throw"Error";}

		// Determine the number of elements in this block of the tertiary structure.
	if (lv_nr16_cut < static_cast<int>(floar_lv_nr16_cut))	// lv_nr16_cut has decreased.
	{
		lv_cut = ga_bin_min_16 - 1;
	}
	else
	{
		lv_nr16_cut = static_cast<int>(floar_lv_nr16_cut);
		//		if(lv_nr16_cut>=ga_bin_min[16]){printf("lv_nr16_cut too big");throw"Error";}
		if (floar_lv_nr16_cut == (static_cast<double>(gm_ptg_nrc_ij) / static_cast<double>(ga_bin_min_16)))	// The remainder equals the full 16 bits.
		{
			if (lv_nr16_cut == 0)
			{
				throw "Error: zero combinations present at this ptg.";
			}
			lv_cut = ga_bin_min_16 - 1;
		}
		else	// The remainder equals some bits.
		{
			lv_cut = gm_ptg_nrc_ij - (lv_nr16_cut * ga_bin_min_16) - 1;
		}
		// Reset the value of lv_nr16_cut.
		lv_nr16_cut = static_cast<unsigned short int>(floar_lv_nr16_cut);
		while (lm_ter_str[i][j][lv_nr16_cut][0] < lv_tmp_ter)	// Increase lv_nr16_cut until you arrive at the correct block of the tertiary structure (position 0 holds the highest value).
		{
			lv_nr16_cut--;
		}
	}
	//	if(lv_nr16_cut>=ga_bin_min[16]){printf("lv_nr16_cut too big");throw"Error";}

		//////////////////////////////////////////////////////////////////////////
		///////////DO EXPONENTIAL SEARCH//////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////

	lv_cut_upper = lv_cut;
	lv_cut_lower = 0;
	while (lv_cut > 1)
	{

		z2 = static_cast<unsigned long int>(lv_cut_lower + (lv_cut % 2 + lv_cut / 2));//z2=static_cast<unsigned long int>(lv_cut_lower+ceil(static_cast<double>(lv_cut)/static_cast<double>(2)));
		if (lv_tmp_ter >= lm_ter_str[i][j][lv_nr16_cut][z2])	// The tertiary value is larger, so it belongs to the first half.
		{
			lv_cut_upper = z2;
		}
		else
		{
			lv_cut_lower = z2;
		}
		lv_cut = lv_cut_upper - lv_cut_lower;
	}
	if (lm_ter_str[i][j][lv_nr16_cut][lv_cut_lower] == lv_tmp_ter)
	{
		lv_cut_upper = lv_cut_lower;
	}
	else if (lm_ter_str[i][j][lv_nr16_cut][lv_cut_upper] != lv_tmp_ter)
	{
		if (lv_cut_upper != lv_cut_lower)
		{
			printf("Error in cut procedure");
			throw "Error in cut procedure";
		}
	}

	//////////////////////////////////////////////////////////////////////////
	///////////RETURN CORRESPONDING NPV VALUE/////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	return lm_npv_str[i][j][lv_nr16_cut][lv_cut_upper];	// Return the corresponding NPV value.
}
std::mutex mtx;
bool parallel_f_fnd(double* max, double add, unsigned short int i, unsigned short int j, unsigned long int lv_tmp_ter, unsigned long gm_ptg_nrc_ij, unsigned long int**** lm_ter_str, float**** lm_npv_str) {
	add += f_fnd(i, j, lv_tmp_ter, gm_ptg_nrc_ij, lm_ter_str, lm_npv_str);
	mtx.lock();
	if (*max < add)
		* max = add;
	mtx.unlock();
	return true;
}
void f_ter2(unsigned short int i, unsigned short int j, unsigned long int**** lm_ter_str, float**** lm_npv_str)		// Construct tertiary structure.
{
	//////////////////////////////////////////////////////////////////////////
	///////////DEFINE LOCAL VARIABLES/////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool** lm_lnk_mem;			// Local matrix link membership. Is activity i a member of link l or not.

	unsigned short int* la_lnk_fin;			// Local array link finish. Indicates if a link is established (just a copy of la_lnk_nra).
	unsigned short int* la_act_sta;			// Local array activity status.
	unsigned short int* la_ptg_act;			// Local array ptg activity. Which activities are a member of the ptg in question.
	unsigned short int* la_ptg_act_inv;		// Local array ptg activity inverse. The inverse relationship of la_ptg_act.
	unsigned short int* la_lnk_nra;			// Local array link number of activities. Holds the number of shared activities between a ptg and the linked ptg.

	unsigned long int	lv_tot_nrc;				// Local variable total number of combinations.
	unsigned long int	lv_tmp_ter;				// Local variable holding a temporary tertiary value.
	unsigned short int	lv_nr16;				// Local variable which holds the number of 16-bits arrays required.

	unsigned short int	l;	// Simple counter.
	unsigned short int	m;	// Simple counter.

	unsigned short int	z1;
	bool				z0;

	//////////////////////////////////////////////////////////////////////////
	///////////PROTOTYPE FUNCTIONS////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

//	void f_rid_ptg_act(unsigned short int i, unsigned short int j, unsigned short int *la_ptg_act, unsigned short int *la_ptg_act_inv, unsigned short int *la_lnk_nra, bool **lm_lnk_mem);	// Retrieve and identify ptg activities.

	//////////////////////////////////////////////////////////////////////////
	///////////INITIALIZATION/////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	lv_nr16 = static_cast<int>(floor(static_cast<double>(gm_ptg_nrc[i][j]) / static_cast<double>(ga_bin_min[16])));
	if ((floor(static_cast<double>(gm_ptg_nrc[i][j]) / static_cast<double>(ga_bin_min[16]))) == (static_cast<double>(gm_ptg_nrc[i][j]) / static_cast<double>(ga_bin_min[16])))	// The remainder equals the full 16 bits.
	{
		if (lv_nr16 == 0)
		{
			throw "Error: zero combinations present at this ptg.";
		}
		lm_npv_str[i][j] = new float* [lv_nr16];
		lm_ter_str[i][j] = new unsigned long int* [lv_nr16];
		do
		{
			lv_nr16--;
			lm_npv_str[i][j][lv_nr16] = new float[ga_bin_min[16]];
			lm_ter_str[i][j][lv_nr16] = new unsigned long int[ga_bin_min[16]];
		} while (lv_nr16 > 0);	// Do as long as there are activities remaining.
	}
	else	// The remainder equals some bits.
	{
		lm_npv_str[i][j] = new float* [(lv_nr16 + 1)];
		lm_ter_str[i][j] = new unsigned long int* [(lv_nr16 + 1)];
		lm_npv_str[i][j][lv_nr16] = new float[gm_ptg_nrc[i][j] - (lv_nr16 * ga_bin_min[16])];
		lm_ter_str[i][j][lv_nr16] = new unsigned long int[gm_ptg_nrc[i][j] - (lv_nr16 * ga_bin_min[16])];
		while (lv_nr16 > 0)
		{
			lv_nr16--;
			lm_npv_str[i][j][lv_nr16] = new float[ga_bin_min[16]];
			lm_ter_str[i][j][lv_nr16] = new unsigned long int[ga_bin_min[16]];
		}
	}

	la_ptg_act = new unsigned short int[gv_n];
	la_ptg_act_inv = new unsigned short int[gv_n];

	lm_lnk_mem = new bool* [gm_nio_ptg[i][j][0]];	// Defines if an activity is a member of an outgoing link (there are gm_nio_ptg[i][j][0] outgoing links at this ptg).
	la_lnk_nra = new unsigned short int[gm_nio_ptg[i][j][0]];	// Create number of activities array (for each outgoing link).

		//////////////////////////////////////////////////////////////////////
		///////FUNCTIONS AND PROGRAM//////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////

			//////////////////////////////////////////////////////////////////
			///FUNCTION F_RID_PTG_ACT/////////////////////////////////////////
			//////////////////////////////////////////////////////////////////

	f_rid_ptg_act(i, j, la_ptg_act, la_ptg_act_inv, la_lnk_nra, lm_lnk_mem);	// Retrieve and identify ptg activities.

	//////////////////////////////////////////////////////////////////
	///INITIALIZATION OF ACTIVITY STATUS//////////////////////////////
	//////////////////////////////////////////////////////////////////

	// Create and initialize la_lnk_fin; which indicates whether additional activities of a given link are allowed to finish (just a copy of la_lnk_nra).
	la_lnk_fin = new unsigned short int[gm_nio_ptg[i][j][0]];
	for (l = 0; l < gm_nio_ptg[i][j][0]; l++)	// Check all links moving from this ptg.
	{
		la_lnk_fin[l] = la_lnk_nra[l];	// As long as la_lnk_fin is larger than 1; there is still a member activity of link l that may be finished (without establishing link l).
	}
	// Create la_act_sta which holds the status of all activities.
	la_act_sta = new unsigned short int[la_ptg_act[0]];	// The dimension of la_act_sta is determined by the number of activities in this ptg.
	// Initialize/reset la_act_sta. We start with the combination in which all activities have their highest possible value.
	m = la_ptg_act[0];
	lv_tot_nrc = 0;	// Initialize the total number of combinations for this ptg.
	lv_nr16 = static_cast<int>(floor(static_cast<double>(lv_tot_nrc) / static_cast<double>(ga_bin_min[16])));
	lm_ter_str[i][j][lv_nr16][(lv_tot_nrc - (lv_nr16 * ga_bin_min[16]))] = 0;	// Initialize the tertiary value for this combination.
	do
	{
		m--;
		z0 = 0;
		for (l = 0; l < gm_nio_ptg[i][j][0]; l++)	// Check if finishing this activity establishes any of the links.
		{
			if (lm_lnk_mem[l][m] == 1)	// Activity la_ptg_act[m+1] has the potential to establish link l.
			{
				if (la_lnk_fin[l] == 1)	// Finishing the activity would imply that we establish a link (i.e. enter a new ptg).
				{
					z0 = 1;	// Indicate that finishing the activity establishes a link.
				}
			}
		}
		if (z0 == 0)	// We can finish the activity without establishing a link.
		{
			la_act_sta[m] = 2;	// Finish the activity.
			lm_ter_str[i][j][lv_nr16][(lv_tot_nrc - (lv_nr16 * ga_bin_min[16]))] += static_cast<unsigned long int>(ga_bin3_dbl[m]);
			for (l = 0; l < gm_nio_ptg[i][j][0]; l++)	// Record the impact of finishing activity la_ptg_act[m+1].
			{
				if (lm_lnk_mem[l][m] == 1)	// Activity la_ptg_act[m+1] has the potential to establish link l.
				{
					la_lnk_fin[l]--;
				}
			}
		}
		else	// We cannot finish the activity. Its status is maximized at value 1.
		{
			la_act_sta[m] = 1;
			lm_ter_str[i][j][lv_nr16][(lv_tot_nrc - (lv_nr16 * ga_bin_min[16]))] += static_cast<unsigned long int>(ga_bin3[m]);
		}
	} while (m > 0);	// Do until you have arrived at the last position. We end up at m=0; the first position which has the lowest significance (i.e. impact on lm_ter_str).
	z0 = 0;	// Indicates that not all combinations have been found.

	//////////////////////////////////////////////////////////////////
	///SHRINK EXPAND PROCEDURE////////////////////////////////////////
	//////////////////////////////////////////////////////////////////

	do
	{
		// You end up at the last activity whose value is maximized.
		if (la_act_sta[m] == 2)	// Unfinishing the last activity will impact la_lnk_fin.
		{
			//--> LAST ACTIVITY STATUS 2.
			for (l = 0; l < gm_nio_ptg[i][j][0]; l++)	// Record the impact of finishing activity la_ptg_act[m+1].
			{
				if (lm_lnk_mem[l][m] == 1)
				{
					la_lnk_fin[l]++;	// Activity la_ptg_act[m+1] is no longer finished. Therefore, an additional member activity may be finished before link l is established.
				}
			}
			la_act_sta[m]--;	// Decrease the status of the last activity. FROM 2 TO 1.
			lv_tmp_ter = lm_ter_str[i][j][lv_nr16][(lv_tot_nrc - (lv_nr16 * ga_bin_min[16]))];
			lv_tot_nrc++;	// Increase the number of combinations at this ptg.
			lv_nr16 = static_cast<int>(floor(static_cast<double>(lv_tot_nrc) / static_cast<double>(ga_bin_min[16])));
			lm_ter_str[i][j][lv_nr16][(lv_tot_nrc - (lv_nr16 * ga_bin_min[16]))] = lv_tmp_ter - static_cast<unsigned long int>(ga_bin3[m]);	// You go from 2 to 1 => ga_bin3 suffices.
		}
		// You end up at a situation in which the value of the last activity equals 1. FROM 1 TO 0.
//--> LAST ACTIVITY STATUS 1.
		la_act_sta[m]--;
		lv_tmp_ter = lm_ter_str[i][j][lv_nr16][(lv_tot_nrc - (lv_nr16 * ga_bin_min[16]))];
		lv_tot_nrc++;	// Increase the number of combinations at this ptg.
		lv_nr16 = static_cast<int>(floor(static_cast<double>(lv_tot_nrc) / static_cast<double>(ga_bin_min[16])));
		lm_ter_str[i][j][lv_nr16][(lv_tot_nrc - (lv_nr16 * ga_bin_min[16]))] = lv_tmp_ter - static_cast<unsigned long int>(ga_bin3[m]);	// You go from 1 to 0 => ga_bin3 suffices.
		// You end up at a situation in which the value of the last activity equals 0.
//--> LAST ACTIVITY STATUS 0.
		lv_tmp_ter = lm_ter_str[i][j][lv_nr16][(lv_tot_nrc - (lv_nr16 * ga_bin_min[16]))];
		lv_tot_nrc++;	// Increase the number of combinations at this ptg.
		lv_nr16 = static_cast<int>(floor(static_cast<double>(lv_tot_nrc) / static_cast<double>(ga_bin_min[16])));

		// You end up at a position which is minimized. Keep increasing position until we arrive at a position at which the value is not yet minimized.
		while ((la_act_sta[m] == 0) && (m < (la_ptg_act[0] - 1)))	// Do until you arrive at a) an activity which status can further be decreased; b) the first activity.
		{
			m++;	// Increase the position.
		}

		if (m == (la_ptg_act[0] - 1))	// You are at the first activity.
		{
			if (la_act_sta[m] == 0)	// All combinations have been found.
			{
				z0 = 1;	// All combinations have been found.
			}
		}
		if (z0 == 0)	// There are still combinations to be found.
		{
			if (la_act_sta[m] == 2)	// Unfinishing the activity will impact la_lnk_fin.
			{
				for (l = 0; l < gm_nio_ptg[i][j][0]; l++)	// Record the impact of unfinishing activity la_ptg_act[m+1].
				{
					if (lm_lnk_mem[l][m] == 1)
					{
						la_lnk_fin[l]++;	// Activity la_ptg_act[m+1] is no longer finished. Therefore, an additional member activity may be finished before link l is established.
					}
				}
				la_act_sta[m]--;	// Decrease the status of the activity. FROM 2 TO 1.
				lm_ter_str[i][j][lv_nr16][(lv_tot_nrc - (lv_nr16 * ga_bin_min[16]))] = lv_tmp_ter;	// Take into account the previous value of lm_ter_str.
				lm_ter_str[i][j][lv_nr16][(lv_tot_nrc - (lv_nr16 * ga_bin_min[16]))] -= static_cast<unsigned long int>(ga_bin3[m]);	// You go from 2 to 1 => ga_bin3 suffices.
			}
			else	// Activity status equals 1.
			{
				la_act_sta[m]--;	// Decrease the status of the activity. FROM 1 TO 0.
				lm_ter_str[i][j][lv_nr16][(lv_tot_nrc - (lv_nr16 * ga_bin_min[16]))] = lv_tmp_ter;	// Take into account the previous value of lm_ter_str.
				lm_ter_str[i][j][lv_nr16][(lv_tot_nrc - (lv_nr16 * ga_bin_min[16]))] -= static_cast<unsigned long int>(ga_bin3[m]);	// You go from 1 to 0 => ga_bin3 suffices.
			}

			// Start resetting; maximizing prior activities.
			do
			{
				m--;	// Decrease the position. Current position (value 0) will be maximized.
				z1 = 0;	// Indicates if finishing activity la_ptg_act[m+1] would establish an unwanted link.
				for (l = 0; l < gm_nio_ptg[i][j][0]; l++)	// Check all links.
				{
					if (lm_lnk_mem[l][m] == 1)	// Finishing activity la_ptg_act[m+1] has the potential to establish link l.
					{
						if (la_lnk_fin[l] == 1)	// Finishing activity la_ptg_act[m+1] would establish an unwanted link.
						{
							z1 = 1;	// Indicate that the status of activity la_ptg_act[m+1] is not to be maximized.
						}
					}
				}
				if (z1 == 0)	// Activity la_ptg_act[m+1] is allowed to finish.
				{
					la_act_sta[m] = 2;	// Indicate that activity la_ptg_act[m+1] has finished. FROM 0 TO 2.
					lm_ter_str[i][j][lv_nr16][(lv_tot_nrc - (lv_nr16 * ga_bin_min[16]))] += static_cast<unsigned long int>(ga_bin3_dbl[m]);	// You go from 0 to 2 => ga_bin3_dbl required.
					for (l = 0; l < gm_nio_ptg[i][j][0]; l++)	// Check all links.
					{
						if (lm_lnk_mem[l][m] == 1)
						{
							la_lnk_fin[l]--;	// Indicate that link l (of which activity la_ptg_act[m+1] is a member activity) is one step closer to becoming established.
						}
					}
				}
				else
				{
					la_act_sta[m] = 1;	// Activity la_ptg_act[m+1] is not allowed to finish, so it just starts. FROM 0 TO 1.
					lm_ter_str[i][j][lv_nr16][(lv_tot_nrc - (lv_nr16 * ga_bin_min[16]))] += static_cast<unsigned long int>(ga_bin3[m]);	// You go from 0 to 1 => ga_bin3 suffices.
				}
			} while (m > 0);	// Do until you arrive at the last activity.
		}
	} while (z0 == 0);	// Repeat reverse shrink-expand procedure until all combinations have been found.

//////////////////////////////////////////////////////////////////////
///////DESTRUCTION PHASE PRIOR TO MOVING TO THE NEXT PTG//////////////
//////////////////////////////////////////////////////////////////////

	for (l = 0; l < gm_nio_ptg[i][j][0]; l++)
	{
		delete lm_lnk_mem[l];
	}
	delete lm_lnk_mem;
	delete la_lnk_nra;
	delete la_lnk_fin;
	delete la_act_sta;

	//////////////////////////////////////////////////////////////////////////
	///////////DESTRUCTION PHASE//////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	delete la_ptg_act;
	delete la_ptg_act_inv;
}