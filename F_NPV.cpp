void f_npv(void)
{
	unsigned long int f_mem(void);	// Returns the chosen npv-computation option in function of the corresponding memory requirements.
	void f_npv2(void);	// Executes the second npv-computation option (store a minimum amount of data).
	void f_store(void);	// Stores the results.
	double f_en(void);

	gv_clock_npv = clock();	// Start the clock.
	if(static_cast<unsigned short int>(f_mem())==1)	// Computation option 1 is selected.
	{
		f_npv2();
	}
	else		// No valid computation option was returned.
	{
		printf("\n");printf("Error, no valid option.");
	}
	f_store();
}

void f_store(void)
{
	//////////////////////////////////////////////////////////////////
	///////////STORE BATCH OUTPUT/////////////////////////////////////
	//////////////////////////////////////////////////////////////////

	clock_t				lv_clock;	// Holds the clock ticks used for computation of this particular instance.
	float				lv_time;	// Holds the time used for computation of this particular instance.
	float				lv_time_npv;	// Holds the time used for computation of this particular instance.
	float				lv_time_udc;	// Holds the time used for computation of this particular instance.
	float				lv_time2;	// Holds the duration of the enumeration.
	float				lv_tot_mem;	// Total memory requirement.
	float				lv_max_mem;	// Holds the maximum effective memory consumption.
	float				lv_npv;		// Holds the npv if computed using enumeration.
	unsigned long int	lv_tot_nrp;	// Total number of ptgs.
	unsigned long int	lv_tot_nrc;	// Total number of combinations.
	unsigned long int	lv_tot_nro;	// Total number of outgoing combinations.
		
	unsigned short int	i;
	unsigned short int	j;

	double f_en(void);
	gv_clock_npv=clock()-gv_clock_npv;	// Stop the clock.
	lv_clock=clock()-gv_clock;	// Stop the clock.
	lv_time=static_cast<float>(lv_clock/static_cast<double>(CLOCKS_PER_SEC));
	lv_time_npv=static_cast<float>(gv_clock_npv/static_cast<double>(CLOCKS_PER_SEC));
	lv_time_udc=static_cast<float>(gv_clock_udc/static_cast<double>(CLOCKS_PER_SEC));
	lv_tot_nrp=0;
	lv_tot_nrc=0;
	lv_tot_nro=0;
	for(i=0;i<gv_n;i++)
	{
		lv_tot_nrp+=ga_ptg[i];
		for(j=0;j<ga_ptg[i];j++)
		{
			lv_tot_nrc+=gm_ptg_nrc[i][j];
			lv_tot_nro+=gm_nio_ptg[i][j][0];
		}
	}
	gv_avg_mem=gv_avg_mem*static_cast<double>(8);
	gv_avg_mem=(gv_avg_mem/static_cast<double>(lv_tot_nrc));
	gv_avg_mem=(gv_avg_mem/static_cast<double>(ga_bin[20]));
//	gv_avg_mem=(static_cast<double>(gv_avg_mem*8)/static_cast<double>(lv_tot_nrc*ga_bin[20]));	// Compute average memory requirement per state.
	gv_avg_mpa=(static_cast<double>(gv_avg_mpa)/static_cast<double>(lv_tot_nrp));	// Compute the average number of parallel activities per UDC.
	gv_avg_udc=(static_cast<double>(gv_avg_udc)/static_cast<double>(lv_tot_nrc));	// Compute the average number of udcs per state.

	if(ga_batch[1]==1)	// Print Header.
	{
		ofstream Results("Results.csv", ios::trunc);
		Results << "N , INSTANCE , # UDC , # COMBINATIONS , # OUTGOING LINKS , MAX # PARALLEL ACTIVITIES , AVG # PARALLEL ACTIVITIES , # DOUBLE LINKS , NPV , MAX MEMORY (ABSOLUTE) , MAX MEMORY (1) , MAX MEMORY (2) , AVG MEMORY , MAX # UDC IN MEMORY , AVG # UDC IN MEMORY , UDC CPU TIME , NPV CPU TIME , NPV (ENU) , CPU TIME (ENU) \n";
	}

	lv_max_mem=static_cast<float>(static_cast<double>(gv_mem*8)/static_cast<double>(ga_bin[20]));
	lv_tot_mem=static_cast<float>(static_cast<double>(lv_tot_nrc*8)/static_cast<double>(ga_bin[20]));
	gv_mem=static_cast<float>(static_cast<double>(gv_mem*8)/static_cast<double>(ga_bin[20]));

	if(ga_batch[0]>10)	// No Full Enumeration.
	{
		if(gv_opt==0)	// Not Feasible.
		{
			printf("\n");
			printf("[%d][%d]---PTGs[%u]---COMBOs[%u]---LINKS[%u]---DOUBLES[%u]---MPA[%u]---APA[%.2f]\n",ga_batch[0],ga_batch[1],lv_tot_nrp,lv_tot_nrc,lv_tot_nro,gv_double,gv_mpa,gv_avg_mpa);
			printf("[%d][%d]---MEM(MB)[%.2f]---MEM(MB)(EFF)[%.2f]---MEM(MB)(AVG)[%.2f]---MPTG[%u]---APTG[%.2f]\n",ga_batch[0],ga_batch[1],lv_tot_mem,lv_max_mem,gv_avg_mem,gv_max_nr_udc,gv_avg_udc);
			printf("\n");
			ofstream Results("Results.csv", ios::app);
			Results << ga_batch[0] << ", " << ga_batch[1] << ", " << lv_tot_nrp << ", " << lv_tot_nrc << ", " << lv_tot_nro << ", " << gv_mpa << ", " << gv_avg_mpa << ", " << gv_double << ", " << "NA" << ", " << lv_tot_mem << ", " << gv_mem << ", " << lv_max_mem << " , " << gv_avg_mem << ", " << gv_max_nr_udc << ", " << gv_avg_udc << ", " << lv_time_udc << ", " << "NA" << ", " << "NA" << ", " << "NA" << "\n";
		}
		else	// Feasible.
		{
			printf("\n");
			printf("[%d][%d]---PTGs[%u]---COMBOs[%u]---LINKS[%u]---DOUBLES[%u]---MPA[%u]---APA[%.2f]\n",ga_batch[0],ga_batch[1],lv_tot_nrp,lv_tot_nrc,lv_tot_nro,gv_double,gv_mpa,gv_avg_mpa);
			printf("[%d][%d]---NPV[%.2f]---MEM(MB)[%.2f]---MEM(MB)(EFF)[%.2f]---MEM(MB)(AVG)[%.2f]---T_UDC(sec)[%.2f]---T_NPV(sec)[%.2f]---MPTG[%u]---APTG[%.2f]\n",ga_batch[0],ga_batch[1],gv_npv,lv_tot_mem,lv_max_mem,gv_avg_mem,lv_time_udc,lv_time_npv,gv_max_nr_udc,gv_avg_udc);
			printf("\n");
			ofstream Results("Results.csv", ios::app);
			Results << ga_batch[0] << ", " << ga_batch[1] << ", " << lv_tot_nrp << ", " << lv_tot_nrc << ", " << lv_tot_nro << ", " << gv_mpa << ", " << gv_avg_mpa << ", " << gv_double << ", " << gv_npv << ", " << lv_tot_mem << ", " << gv_mem << ", " << lv_max_mem << " , " << gv_avg_mem << ", " << gv_max_nr_udc << ", " << gv_avg_udc << ", " << lv_time_udc << ", " << lv_time_npv << ", " << "NA" << ", " << "NA" << "\n";
		}
	}
	else	// Full Enumeration.
	{
		lv_clock=clock();	// Start the clock.
		lv_npv=0;
//		lv_npv=static_cast<float>(f_en());
		lv_clock=clock()-lv_clock;	// Stop the clock.
		lv_time2=static_cast<float>(lv_clock/static_cast<double>(CLOCKS_PER_SEC));
		if(gv_opt==0)	// Not Feasible.
		{
			printf("\n");
			printf("[%d][%d]---PTGs[%u]---COMBOs[%u]---LINKS[%u]---DOUBLES[%u]---MPA[%u]---APA[%.2f]\n",ga_batch[0],ga_batch[1],lv_tot_nrp,lv_tot_nrc,lv_tot_nro,gv_double,gv_mpa,gv_avg_mpa);
			printf("[%d][%d]---MEM(MB)[%.2f]---MEM(MB)(EFF)[%.2f]---MEM(MB)(AVG)[%.2f]---MPTG[%u]---APTG[%.2f]\n",ga_batch[0],ga_batch[1],lv_tot_mem,lv_max_mem,gv_avg_mem,gv_max_nr_udc,gv_avg_udc);
			printf("[%d][%d]---ENU[%.2f]---TIME(sec)[%.2f]",ga_batch[0],ga_batch[1],lv_npv,lv_time2);
			printf("\n");
			ofstream Results("Results.csv", ios::app);
			Results << ga_batch[0] << ", " << ga_batch[1] << ", " << lv_tot_nrp << ", " << lv_tot_nrc << ", " << lv_tot_nro << ", " << gv_mpa << ", " << gv_avg_mpa << ", " << gv_double << ", " << "NA" << ", " << lv_tot_mem << ", " << gv_mem << ", " << lv_max_mem << " , " << gv_avg_mem << ", " << gv_max_nr_udc << ", " << gv_avg_udc << ", " << lv_time_udc << ", " << "NA" << ", " << lv_npv << ", " << lv_time2 << "\n";
		}
		else	// Feasible.
		{
			printf("\n");
			printf("[%d][%d]---PTGs[%u]---COMBOs[%u]---LINKS[%u]---DOUBLES[%u]---MPA[%u]---APA[%.2f]\n",ga_batch[0],ga_batch[1],lv_tot_nrp,lv_tot_nrc,lv_tot_nro,gv_double,gv_mpa,gv_avg_mpa);
			printf("[%d][%d]---NPV[%.2f]---MEM(MB)[%.2f]---MEM(MB)(EFF)[%.2f]---MEM(MB)(AVG)[%.2f]---T_UDC(sec)[%.2f]---T_NPV(sec)[%.2f]---MPTG[%u]---APTG[%.2f]\n",ga_batch[0],ga_batch[1],gv_npv,lv_tot_mem,lv_max_mem,gv_avg_mem,lv_time_udc,lv_time_npv,gv_max_nr_udc,gv_avg_udc);
			printf("[%d][%d]---ENU[%.2f]---TIME(sec)[%.2f]",ga_batch[0],ga_batch[1],lv_npv,lv_time2);
			printf("\n");
			ofstream Results("Results.csv", ios::app);
			Results << ga_batch[0] << ", " << ga_batch[1] << ", " << lv_tot_nrp << ", " << lv_tot_nrc << ", " << lv_tot_nro << ", " << gv_mpa << ", " << gv_avg_mpa << ", " << gv_double << ", " << gv_npv << ", " << lv_tot_mem << ", " << gv_mem << ", " << lv_max_mem << " , " << gv_avg_mem << ", " << gv_max_nr_udc << ", " << gv_avg_udc << ", " << lv_time_udc << ", " << lv_time_npv << ", " << lv_npv << ", " << lv_time2 << "\n";
		}
	}
}

unsigned long int f_mem(void)
{
	//////////////////////////////////////////////////////////////////////////
	///////////DEFINE LOCAL VARIABLES/////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	bool				** lm_lnk_mem;			// Local matrix link membership. Is activity i a member of link l or not.

	unsigned short int	* la_ptg_act;			// Local array ptg activity. Which activities are a member of the ptg in question.
	unsigned short int	* la_ptg_act_inv;		// Local array ptg activity inverse. The inverse relationship of la_ptg_act.
	unsigned short int	* la_lnk_nra;			// Local array link number of activities. Holds the number of shared activities between a ptg and the linked ptg.
	unsigned short int	* la_lnk_fin;			// Local array link finish. Indicates if a link is established (just a copy of la_lnk_nra).
	unsigned short int	* la_act_sta;			// Local array activity status.

	double				lv_mem;					// Local variable holding the current memory consumption.

	unsigned short int	i;	// Simple counter.
	unsigned short int	j;	// Simple counter.
	unsigned short int	l;	// Simple counter.
	unsigned short int	m;	// Simple counter.

	unsigned short int	z1;
	bool				z0;

	//////////////////////////////////////////////////////////////////////////
	///////////PROTOTYPE FUNCTIONS////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	void f_rid_ptg_act(unsigned short int i, unsigned short int j, unsigned short int *la_ptg_act, unsigned short int *la_ptg_act_inv, unsigned short int *la_lnk_nra, bool **lm_lnk_mem);	// Retrieve and identify ptg activities.

	//////////////////////////////////////////////////////////////////////////
	///////////INITIALIZATION/////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	gm_ptg_nrc = new unsigned long int * [gv_n];
	gm_ptg_nrc[(gv_n-1)] = new unsigned long int [1];
	gm_ptg_nrc[(gv_n-1)][0]=1;	// At the final node, there is only one combination.
	gm_ptg_nrc[0] = new unsigned long int [1];
	gm_ptg_nrc[0][0]=0;	// At the first node, there are no combinations.

	la_ptg_act = new unsigned short int	[gv_n];
	la_ptg_act_inv = new unsigned short int [gv_n];

	gv_avg_mem=0;
	gv_avg_mpa=0;
	gv_avg_udc=0;
	gv_tot_nrc=0;
	lv_mem=0;
	gv_mem=0;
	gv_mpa=0;
	gv_nr_udc=0;
	gv_max_nr_udc=0;

	for(i=(gv_n-2);i>0;i--)
	{
		gm_ptg_nrc[i] = new unsigned long int [ga_ptg[i]];
	}

	//////////////////////////////////////////////////////////////////////////
	///////////DETERMINE MEMORY REQUIREMENT///////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	for(i=(gv_n-2);i>0;i--)	// Starting from the second highest recursion level, check all recursion levels (except for the first and final node; the latter one is initialized seperately).
	{

		//////////////////////////////////////////////////////////////////////
		///////INITIALIZATION/////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////

		for(j=0;j<ga_ptg[i];j++)	// Check all ptgs at recursion level i.
		{

		//////////////////////////////////////////////////////////////////////
		///////INITIALIZATION/////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////

			gm_ptg_nrc[i][j]=0;

			lm_lnk_mem = new bool * [gm_nio_ptg[i][j][0]];	// Defines if an activity is a member of an outgoing link (there are gm_nio_ptg[i][j][0] outgoing links at this ptg).
			la_lnk_nra = new unsigned short int [gm_nio_ptg[i][j][0]];	// Create number of activities array (for each outgoing link).

		//////////////////////////////////////////////////////////////////////
		///////FUNCTIONS AND PROGRAM//////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////

			//////////////////////////////////////////////////////////////////
			///FUNCTION F_RID_PTG_ACT/////////////////////////////////////////
			//////////////////////////////////////////////////////////////////

			f_rid_ptg_act(i,j,la_ptg_act,la_ptg_act_inv,la_lnk_nra,lm_lnk_mem);	// Retrieve and identify ptg activities.

			//////////////////////////////////////////////////////////////////
			///INITIALIZATION OF ACTIVITY STATUS//////////////////////////////
			//////////////////////////////////////////////////////////////////

			// Create and initialize la_lnk_fin; which indicates whether additional activities of a given link are allowed to finish (just a copy of la_lnk_nra).
			la_lnk_fin = new unsigned short int [gm_nio_ptg[i][j][0]];
			for(l=0;l<gm_nio_ptg[i][j][0];l++)	// Check all links moving from this ptg.
			{
				la_lnk_fin[l]=la_lnk_nra[l];	// As long as la_lnk_fin is larger than 1; there is still a member activity of link l that may be finished (without establishing link l).
			}
			// Create la_act_sta which holds the status of all activities.
			la_act_sta = new unsigned short int [la_ptg_act[0]];	// The dimension of la_act_sta is determined by the number of activities in this ptg.
			// Initialize/reset la_act_sta. We start with the combination in which all activities have their highest possible value.
			for(m=0;m<la_ptg_act[0];m++)	// Check all activities in this ptg.
			{
				z0=0;
				for(l=0;l<gm_nio_ptg[i][j][0];l++)	// Check if finishing this activity establishes any of the links.
				{
					if(lm_lnk_mem[l][m]==1)	// Activity la_ptg_act[m+1] has the potential to establish link l.
					{
						if(la_lnk_fin[l]==1)	// Finishing the activity would imply that we establish a link (i.e. enter a new ptg).
						{
							z0=1;	// Indicate that finishing the activity establishes a link.
						}
					}
				}
				if(z0==0)	// We can finish the activity without establishing a link.
				{
					la_act_sta[m]=2;	// Finish the activity.
					for(l=0;l<gm_nio_ptg[i][j][0];l++)	// Record the impact of finishing activity la_ptg_act[m+1].
					{
						if(lm_lnk_mem[l][m]==1)	// Activity la_ptg_act[m+1] has the potential to establish link l.
						{
							la_lnk_fin[l]--;
						}
					}
				}
				else	// We cannot finish the activity. Its status is maximized at value 1.
				{
					la_act_sta[m]=1;
				}
			}
			m=la_ptg_act[0]-1;	// Indicate that m should start at the last position (with the last activity).
			z0=0;	// Indicates that not all combinations have been found.

			//////////////////////////////////////////////////////////////////
			///SHRINK EXPAND PROCEDURE////////////////////////////////////////
			//////////////////////////////////////////////////////////////////

			// Reverse shrink-expand procedure. Decrease the value at a position until the minimum is reached. If the minimum is reached; increase the position. If the value at the new position is minimal as well, further increase the position. Eventually you end up at a position at which the value may be decreased. Decrease the value and reset all previous activities (reset implying to maximize activity status).
			do
			{
				// You end up at the last activity whose value is maximized.
				if(la_act_sta[m]==2)	// Unfinishing the last activity will impact la_lnk_fin.
				{
					for(l=0;l<gm_nio_ptg[i][j][0];l++)	// Record the impact of finishing activity la_ptg_act[m+1].
					{
						if(lm_lnk_mem[l][m]==1)
						{
							la_lnk_fin[l]++;	// Activity la_ptg_act[m+1] is no longer finished. Therefore, an additional member activity may be finished before link l is established.
						}
					}
					la_act_sta[m]--;	// Decrease the status of the last activity.
					gm_ptg_nrc[i][j]++;	// 1 combination (where last activity status equals 2).
					if(gm_ptg_nrc[i][j]>=ga_bin_min[32])
					{
						printf("too much combinations");
						throw "too much combinations";
					}
				}
				// You end up at a situation in which the value of the last activity equals 1.
				la_act_sta[m]--;

				// You end up at a situation in which the value of the last activity equals 0.
				if(gm_ptg_nrc[i][j]>=ga_bin_min[32])
				{
					printf("too much combinations");
					throw "too much combinations";
				}
				gm_ptg_nrc[i][j]+=2;	// 2 combinations (where last activity status equals 1 and 0 respectively)

				// You end up at a position which is minimized. Keep increasing position until we arrive at a position at which the value is not yet minimized.
				while((la_act_sta[m]==0) && (m>0))	// Do until you arrive at a) an activity which status can further be decreased; b) the first activity.
				{
					m--;	// Decrease the position.
				}

				// You end up at a position which may be decreased or at the first position.
				if(m==0)
				{
					if(la_act_sta[m]==0)	// All combinations have been found; write away cycle class and sequence data for all activities (the last cycle).
					{
						z0=1;	// All combinations have been found.
					}
				}
				if(z0==0)	// There are still combinations to be found.
				{
					if(la_act_sta[m]==2)	// Unfinishing the activity will impact la_lnk_fin.
					{
						for(l=0;l<gm_nio_ptg[i][j][0];l++)	// Record the impact of unfinishing activity la_ptg_act[m+1].
						{
							if(lm_lnk_mem[l][m]==1)
							{
								la_lnk_fin[l]++;	// Activity la_ptg_act[m+1] is no longer finished. Therefore, an additional member activity may be finished before link l is established.
							}
						}
					}
					la_act_sta[m]--;	// Decrease the status of the activity.
					// Start resetting; maximizing prior activities.
					do
					{
						m++;	// Increase the position. Minimal position (value 0) will be maximized.
						z1=0;	// Indicates if finishing activity la_ptg_act[m+1] would establish an unwanted link.
						for(l=0;l<gm_nio_ptg[i][j][0];l++)	// Check all links.
						{
							if(lm_lnk_mem[l][m]==1)	// Finishing activity la_ptg_act[m+1] has the potential to establish link l.
							{
								if(la_lnk_fin[l]==1)	// Finishing activity la_ptg_act[m+1] would establish an unwanted link.
								{
									z1=1;	// Indicate that the status of activity la_ptg_act[m+1] is not to be maximized.
								}
							}
						}
						if(z1==0)	// Activity la_ptg_act[m+1] is allowed to finish.
						{
							la_act_sta[m]=2;	// Indicate that activity la_ptg_act[m+1] has finished.
							for(l=0;l<gm_nio_ptg[i][j][0];l++)	// Check all links.
							{
								if(lm_lnk_mem[l][m]==1)
								{
									la_lnk_fin[l]--;	// Indicate that link l (of which activity la_ptg_act[m+1] is a member activity) is one step closer to becoming established.
								}
							}
						}
						else
						{
							la_act_sta[m]=1;	// Activity la_ptg_act[m+1] is not allowed to finish, so it just starts.
						}
					}while(m<la_ptg_act[0]-1);	// Do until you arrive at the last activity.
				}
			}while(z0==0);	// Repeat reverse shrink-expand procedure until all combinations have been found.
			gv_tot_nrc+=gm_ptg_nrc[i][j];	// Keep track of the total number of combinations.
			// Record the maximum number of parallel activities.
			if(la_ptg_act[0]>gv_mpa)
			{
				gv_mpa=la_ptg_act[0];
			}
			// Record average number of parallel activities per udc.
			gv_avg_mpa+=la_ptg_act[0];

		//////////////////////////////////////////////////////////////////////
		///////DESTRUCTION PHASE PRIOR TO MOVING TO THE NEXT PTG//////////////
		//////////////////////////////////////////////////////////////////////
			
			lv_mem+=gm_ptg_nrc[i][j];	// Increase memory requirement.
			gv_nr_udc++;

			for(l=0;l<gm_nio_ptg2[i][j][0];l++)	// Check all outgoing links.
			{
				if(gm_nio_ptg2[gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]][1]==1)	// Is the incoming link at its minimum?
				{
					lv_mem-=gm_ptg_nrc[gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]];
					gv_nr_udc--;
				}
				else	// The incoming link is not yet at its minimum. Decrease gm_nio_ptg2. Memory is still required.
				{
					gm_nio_ptg2[gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]][1]--;
				}
			}

			gv_avg_mem+=(gm_ptg_nrc[i][j]*lv_mem);	// Indicate that gm_ptg_nrc[i][j] combinations (i.e. states) require lv_mem memory requirements.
			gv_avg_udc+=(gm_ptg_nrc[i][j]*gv_nr_udc);	// Indicate that gm_ptg_nrc[i][j] combinations (i.e. states) require lv_mem udcs in memory.

			if(gv_nr_udc>gv_max_nr_udc)
			{
				gv_max_nr_udc=gv_nr_udc;
			}

			if(lv_mem>gv_mem)
			{
				gv_mem=lv_mem;	// Increase global memory requirement.
			}

			for(l=0;l<gm_nio_ptg[i][j][0];l++)
			{
				delete lm_lnk_mem[l];
			}
			delete lm_lnk_mem;
			delete la_lnk_nra;
			delete la_lnk_fin;
			delete la_act_sta;

			 //DELETE THIS IF YOU WANT ALL DATA!!!
			//if(static_cast<float>(static_cast<double>(8*gv_mem)/static_cast<double>(ga_bin[20]))>gv_max_mem)
			//{
			//	gv_opt=0;
			//	j=ga_ptg[i];	// EXIT LOOP.
			//	i=1;
			//}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	///////////DESTRUCTION PHASE//////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	for(i=0;i<gv_n;i++)
	{
		for(j=0;j<ga_ptg[i];j++)
		{
			delete gm_nio_ptg2[i][j];
		}
		delete gm_nio_ptg2[i];
	}
	delete gm_nio_ptg2;

	delete la_ptg_act;
	delete la_ptg_act_inv;

	//////////////////////////////////////////////////////////////////////////
	///////////DEFINE NPV COMPUTATION OPTION//////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	if(static_cast<float>(static_cast<double>(8*gv_mem)/static_cast<double>(ga_bin[20]))>gv_max_mem)
	{
		gv_opt=0;
	}
	else
	{
		//if(static_cast<float>(static_cast<double>(8*gv_mem)/static_cast<double>(ga_bin[20]))>=1700)	// ONLY DO THIS IF MORE THAN 1700 MEM IS AVAILABLE
		//{
			gv_opt=1;
		//}
		//else
		//{
		//	gv_opt=0;
		//}
	}

	return gv_opt;	// Return the chosen option to the main program.
}

void f_rid_ptg_act(unsigned short int i, unsigned short int j, unsigned short int *la_ptg_act, unsigned short int *la_ptg_act_inv, unsigned short int *la_lnk_nra, bool **lm_lnk_mem)
{
	//////////////////////////////////////////////////////////////////////////
	///////////DEFINE LOCAL VARIABLES/////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	unsigned long int	* la_fnd_cod;			// Local array find code. Holds 16-bits of binary code.

	unsigned short int	lv_nr32;				// Local variable number of arrays of 32-bit size required to store npv values.

	unsigned short int	k;	// Simple counter.
	unsigned short int	l;	// Simple counter.
	unsigned short int	m;	// Simple counter.

	//////////////////////////////////////////////////////////////////////////
	///////////INITIALIZATION/////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	la_fnd_cod = new unsigned long int [gv_bnr];

	//////////////////////////////////////////////////////////////////////////
	///////////RETRIEVE AND IDENTIFY PTG ACTIVITIES///////////////////////////
	//////////////////////////////////////////////////////////////////////////

	// Find out how many activities are comprised in the ptg. Remark that the activities to be recorded in la_ptg_act are stored in topological order.
	la_ptg_act[0]=0;	// Indicates the number of activities in the ptg.
	for(k=0;k<gv_bnr;k++)
	{
		// Check first 16 bits.
		la_fnd_cod[k]=(gm_str_ptg[k][i][j] & ga_bin_min[16]);	// Binary operation such that only the first 16 bits remain (bitwise & 1111111111111111 (16 ones) and the first 16 bits of the ptg code).
		if(la_fnd_cod[k]>0)
		{
			la_fnd_cod[k]--;	// Correct for one bit of data too short.
			for(l=0;l<gm_bin_act_cod[la_fnd_cod[k]][0];l++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
			{
				la_ptg_act[0]++;	// Increase the number of activities present in this ptg.
				la_ptg_act[la_ptg_act[0]]=((32*k)+gm_bin_act_cod[la_fnd_cod[k]][(l+1)]);	// Record the activity number.
				la_ptg_act_inv[la_ptg_act[la_ptg_act[0]]]=la_ptg_act[0]-1;	// Record the position of activity la_ptg_act[la_ptg_act[0]] in la_ptg_act.
			}
		}
		// Check second 16 bits.
		la_fnd_cod[k]=(gm_str_ptg[k][i][j] >> 16);	// Right shift 16 spaces; as such we only retain the second 16 bits.
		if(la_fnd_cod[k]>0)
		{
			la_fnd_cod[k]--;	// Correct for one bit of data too short.
			for(l=0;l<gm_bin_act_cod[la_fnd_cod[k]][0];l++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
			{
				la_ptg_act[0]++;	// Increase the number of activities present in this ptg.
				la_ptg_act[la_ptg_act[0]]=((32*k)+16+gm_bin_act_cod[la_fnd_cod[k]][(l+1)]);	// Record the activity number.
				la_ptg_act_inv[la_ptg_act[la_ptg_act[0]]]=la_ptg_act[0]-1;	// Record the position of activity la_ptg_act[la_ptg_act[0]] in la_ptg_act.
			}
		}
	}
	// While all activities have been recorded; lets create and intialize lm_lnk_mem. In addition also create and initialize la_lnk_nra.
	for(l=0;l<gm_nio_ptg[i][j][0];l++)	// Check all links moving out this ptg.
	{
		lm_lnk_mem[l] = new bool [la_ptg_act[0]];	// Do not store all activities, only store activities that belong to this ptg (other activities cannot be a member of a link moving from this ptg anyway).
		la_lnk_nra[l]=0;	// Initialize la_lnk_nra (which holds how many activities belong to this link).
		for(m=0;m<la_ptg_act[0];m++)	// Check all activities in this ptg. m indicates the position while la_ptg_act[m+1] indicates the real activity number
		{
			// Test if activity la_ptg_act[m+1] is still present in the ptg linked to; if it is, it is not a part of link l. If it is not present => it has to be finished for link l to be established; hence it is a member of link l.
			lv_nr32=static_cast<int>(floor(static_cast<double>(la_ptg_act[(m+1)])/static_cast<double>(32)));
			if((ga_bin[static_cast<int>(la_ptg_act[(m+1)]-(lv_nr32*32))] & gm_str_ptg[lv_nr32][gm_dpt_ptg[i][j][l]][gm_lnk_ptg[i][j][l]])==0)
			{
				lm_lnk_mem[l][m]=1;	// Indicate that activity la_ptg_act[m+1] is a member of link l, it is not a shared activity, finishing this activity might establish a link.
				la_lnk_nra[l]++;	// Indicate that for this link we found a shared activity la_ptg_act[m+1]. For a link to be established, lnk_nra has to equal zero.
			}
			else
			{
				lm_lnk_mem[l][m]=0;	// Indicate that for this link la_ptg_act[m+1] is a shared activity, it cannot establish the link.
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	///////////DESTRUCTION PHASE//////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	delete la_fnd_cod;
}

double f_en(void)
{
	unsigned long int	** lm_act_fin;	// Holds binary code of predecessors for eacht activity. Determines whether an activity is allowed to start.
	unsigned long int	* la_bin_fin;
	unsigned long int	* la_bin_bus;
	unsigned long int	* la_bin_idl;
	double				lv_npv;
	unsigned short int	lv_nr32;
	unsigned short int	i;
	unsigned short int	j;
	unsigned short int	k;
	unsigned long int	z2;
	unsigned short int	z0;

	double f_en_bus(unsigned long int *la_bin_fin, unsigned long int *la_bin_bus, unsigned long int *la_bin_idl, unsigned long int **lm_act_fin);
	double f_en_dec(unsigned long int *la_bin_fin, unsigned long int *la_bin_bus, unsigned long int *la_bin_idl, unsigned long int **lm_act_fin);

	lm_act_fin = new unsigned long int * [gv_bnr];
	for(k=0;k<gv_bnr;k++)
	{
		lm_act_fin[k] = new unsigned long int [gv_n];
		for(i=0;i<gv_n;i++)
		{
			lm_act_fin[k][i]=0;
		}
	}
	for(i=0;i<gv_n;i++)
	{
		// check all other actions to see if they are predecessors.
		for(j=(i+1);j<gv_n;j++)
		{
			if(gm_tra_rel[i][j]==1)	// it is a predecessor. i is a predecessor of j.
			{
				lv_nr32=static_cast<int>(floor(static_cast<double>(i)/static_cast<double>(32)));z2=ga_bin[static_cast<int>(i-(lv_nr32*32))];
				lm_act_fin[lv_nr32][j]+=z2;
			}
		}
	}
	la_bin_fin = new unsigned long int [gv_bnr];
	la_bin_bus = new unsigned long int [gv_bnr];
	la_bin_idl = new unsigned long int [gv_bnr];
	for(k=0;k<gv_bnr;k++)
	{
		la_bin_fin[k]=0;
		la_bin_bus[k]=0;
		la_bin_idl[k]=0;
	}

	lv_npv=0;

	// Activity 1 (dummy) finishes, what is the npv resulting from the optimal decision to be taken next?
	la_bin_fin[0]+=ga_bin[0];
	// Check all other activities to see if they are allowed to start. Activity i should not be finished nor busy nor idle (available).
	for(i=0;i<gv_n;i++)
	{	lv_nr32=static_cast<int>(floor(static_cast<double>(i)/static_cast<double>(32)));
		if((ga_bin[(i-(lv_nr32*32))] & la_bin_fin[lv_nr32])==0)	// Activity i is not finished.
		{	if((ga_bin[(i-(lv_nr32*32))] & la_bin_bus[lv_nr32])==0)	// Activity i is not busy.
			{	if((ga_bin[(i-(lv_nr32*32))] & la_bin_idl[lv_nr32])==0)	// Activity i is not idle.
				{	z0=0;
					for(k=0;k<gv_bnr;k++)
					{	if((lm_act_fin[k][i] & la_bin_fin[k])==lm_act_fin[k][i])	// Predecessors might be finished.
						{
							z0++;
						}
					}
					if(z0==gv_bnr)	// We have a new idle activity.
					{	z2=ga_bin[static_cast<int>(i-(lv_nr32*32))];la_bin_idl[lv_nr32]+=z2;
	}	}	}	}	}
	lv_npv+=f_en_dec(la_bin_fin,la_bin_bus,la_bin_idl,lm_act_fin);

	for(k=0;k<gv_bnr;k++)
	{
		delete lm_act_fin[k];
	}
	delete lm_act_fin;
	delete la_bin_fin;
	delete la_bin_bus;
	delete la_bin_idl;

	return lv_npv;
}

double f_en_bus(unsigned long int *la_bin_fin, unsigned long int *la_bin_bus, unsigned long int *la_bin_idl, unsigned long int **lm_act_fin)	//COMING FROM DECISION STATE
{
	unsigned long int	* la_fnd_cod;
	unsigned long int	* la_tmp_cod;
	unsigned short int	* la_act_bus;
	double				lv_tmp_npv;
	double				lv_lambda;
	unsigned short int lv_nr32;
	unsigned short int	i;
	unsigned short int	k;
	unsigned short int	l;
	unsigned short int	m;
	unsigned long int	z2;
	unsigned short int	z0;

	double f_en_dec(unsigned long int *la_bin_fin, unsigned long int *la_bin_bus, unsigned long int *la_bin_idl, unsigned long int **lm_act_fin);

	la_fnd_cod = new unsigned long int [gv_bnr];
	la_tmp_cod = new unsigned long int [gv_bnr];
	la_act_bus = new unsigned short int [(gv_n+1)];

	lv_lambda=0;
	// Get set of busy activities = after you took a decision you end up in this situation. Evaluate the npv resulting from this decision.
	la_act_bus[0]=0;
	for(k=0;k<gv_bnr;k++)
	{	la_fnd_cod[k]=(la_bin_bus[k] & ga_bin_min[16]);
		if(la_fnd_cod[k]>0)
		{	la_fnd_cod[k]--;
			for(l=0;l<gm_bin_act_cod[la_fnd_cod[k]][0];l++)
			{	la_act_bus[0]++;la_act_bus[la_act_bus[0]]=((32*k)+gm_bin_act_cod[la_fnd_cod[k]][(l+1)]);lv_lambda+=ga_lambda[la_act_bus[la_act_bus[0]]];
		}	}
		la_fnd_cod[k]=(la_bin_bus[k] >> 16);
		if(la_fnd_cod[k]>0)
		{	la_fnd_cod[k]--;
			for(l=0;l<gm_bin_act_cod[la_fnd_cod[k]][0];l++)
			{	la_act_bus[0]++;la_act_bus[la_act_bus[0]]=((32*k)+16+gm_bin_act_cod[la_fnd_cod[k]][(l+1)]);lv_lambda+=ga_lambda[la_act_bus[la_act_bus[0]]];
	}	}	}

	// Have each busy activity finish and obtain the npv corresponding to the decision state in which you end up.
	lv_tmp_npv=0;
	for(m=0;m<la_act_bus[0];m++)	// Activity m finishes.
	{
		for(k=0;k<gv_bnr;k++){la_tmp_cod[k]=0;}
		lv_nr32=static_cast<int>(floor(static_cast<double>(la_act_bus[(m+1)])/static_cast<double>(32)));z2=ga_bin[static_cast<int>(la_act_bus[(m+1)]-(lv_nr32*32))];la_bin_fin[lv_nr32]+=z2;la_bin_bus[lv_nr32]-=z2;
		// Check all other activities to see if they are allowed to start. Activity i should not be finished nor busy nor idle (available).
		for(i=0;i<gv_n;i++)
		{	lv_nr32=static_cast<int>(floor(static_cast<double>(i)/static_cast<double>(32)));
			if((ga_bin[(i-(lv_nr32*32))] & la_bin_fin[lv_nr32])==0)	// Activity i is not finished.
			{	if((ga_bin[(i-(lv_nr32*32))] & la_bin_bus[lv_nr32])==0)	// Activity i is not busy.
				{	if((ga_bin[(i-(lv_nr32*32))] & la_bin_idl[lv_nr32])==0)	// Activity i is not idle.
					{	z0=0;
						for(k=0;k<gv_bnr;k++)
						{	if((lm_act_fin[k][i] & la_bin_fin[k])==lm_act_fin[k][i])	// Predecessors might be finished.
							{
								z0++;
							}
						}
						if(z0==gv_bnr)	// We have a new idle activity.
						{	z2=ga_bin[static_cast<int>(i-(lv_nr32*32))];la_bin_idl[lv_nr32]+=z2;la_tmp_cod[lv_nr32]+=z2;
	}	}	}	}	}
		lv_tmp_npv+=((static_cast<double>(lv_lambda)/static_cast<double>(lv_lambda+gv_rate))*(static_cast<double>(ga_lambda[la_act_bus[(m+1)]])/static_cast<double>(lv_lambda))*f_en_dec(la_bin_fin,la_bin_bus,la_bin_idl,lm_act_fin));
		lv_nr32=static_cast<int>(floor(static_cast<double>(la_act_bus[(m+1)])/static_cast<double>(32)));z2=ga_bin[static_cast<int>(la_act_bus[(m+1)]-(lv_nr32*32))];la_bin_fin[lv_nr32]-=z2;la_bin_bus[lv_nr32]+=z2;
		for(k=0;k<gv_bnr;k++)
		{
			la_bin_idl[k]-=la_tmp_cod[k];
		}
	}

	delete la_act_bus;
	delete la_fnd_cod;
	delete la_tmp_cod;

	// Return npv value.
	return lv_tmp_npv;
}

double f_en_dec(unsigned long int *la_bin_fin, unsigned long int *la_bin_bus, unsigned long int *la_bin_idl, unsigned long int **lm_act_fin)	//COMING FROM BUSY STATE
{
	unsigned long int	* la_fnd_cod;
	unsigned short int	* la_act_idl;
	unsigned short int	* la_sub_pos;
	unsigned short int	lv_nr32;
	double				lv_costs;
	double				lv_tmp_npv;
	double				lv_max_npv;
	unsigned short int	k;
	unsigned short int	l;
	unsigned short int	m;
	unsigned long int	z2;
	bool				z1;

	double f_en_bus(unsigned long int *la_bin_fin, unsigned long int *la_bin_bus, unsigned long int *la_bin_idl, unsigned long int **lm_act_fin);

	la_fnd_cod = new unsigned long int [gv_bnr];
	la_act_idl = new unsigned short int [(gv_n+1)];
	la_sub_pos = new unsigned short int [gv_n];

	lv_max_npv=-999999999;
	z1=0;
	for(k=0;k<gv_bnr;k++){if(la_bin_bus[k]>0){z1=1;}}
	if(z1==1)
	{
		lv_tmp_npv=(f_en_bus(la_bin_fin,la_bin_bus,la_bin_idl,lm_act_fin));if(lv_tmp_npv>lv_max_npv){lv_max_npv=lv_tmp_npv;}	// ENTER BUSY STATE. DO NOT START.
	}
	// Get set of idle activities. After an activity finishes, you end up in this situation.
	la_act_idl[0]=0;
	for(k=0;k<gv_bnr;k++)
	{	la_fnd_cod[k]=(la_bin_idl[k] & ga_bin_min[16]);
		if(la_fnd_cod[k]>0)
		{	la_fnd_cod[k]--;
			for(l=0;l<gm_bin_act_cod[la_fnd_cod[k]][0];l++)
			{	la_act_idl[0]++;la_act_idl[la_act_idl[0]]=((32*k)+gm_bin_act_cod[la_fnd_cod[k]][(l+1)]);
		}	}
		la_fnd_cod[k]=(la_bin_idl[k] >> 16);
		if(la_fnd_cod[k]>0)
		{	la_fnd_cod[k]--;
			for(l=0;l<gm_bin_act_cod[la_fnd_cod[k]][0];l++)
			{	la_act_idl[0]++;la_act_idl[la_act_idl[0]]=((32*k)+16+gm_bin_act_cod[la_fnd_cod[k]][(l+1)]);
	}	}	}

	// Create all possible decisions (shrink expand) and end up in a busy state.
	if(la_act_idl[0]>0)
	{	if(la_act_idl[1]==(gv_n-1))
		{	lv_tmp_npv=ga_rac[la_act_idl[1]];if(lv_tmp_npv>lv_max_npv){lv_max_npv=lv_tmp_npv;}
		}
		else
		{	m=0;la_sub_pos[m]=0;
			lv_nr32=static_cast<int>(floor(static_cast<double>(la_act_idl[(la_sub_pos[m]+1)])/static_cast<double>(32)));z2=ga_bin[static_cast<int>(la_act_idl[(la_sub_pos[m]+1)]-(lv_nr32*32))];la_bin_bus[lv_nr32]+=z2;la_bin_idl[lv_nr32]-=z2;lv_costs=ga_rac[la_act_idl[(la_sub_pos[m]+1)]];
			lv_tmp_npv=(lv_costs+f_en_bus(la_bin_fin,la_bin_bus,la_bin_idl,lm_act_fin));if(lv_tmp_npv>lv_max_npv){lv_max_npv=lv_tmp_npv;}	// ENTER BUSY STATE.
			if(la_act_idl[0]>1)	// Multiple activities are available, create all subsets.
			{	z1=0;
				do
				{	do	// Expand. Add new activity.
					{	m++;	// Increase position.
						la_sub_pos[m]=(la_sub_pos[(m-1)]+1);	// Set value at the new position.
						lv_nr32=static_cast<int>(floor(static_cast<double>(la_act_idl[(la_sub_pos[m]+1)])/static_cast<double>(32)));z2=ga_bin[static_cast<int>(la_act_idl[(la_sub_pos[m]+1)]-(lv_nr32*32))];la_bin_bus[lv_nr32]+=z2;la_bin_idl[lv_nr32]-=z2;lv_costs+=ga_rac[la_act_idl[(la_sub_pos[m]+1)]];
						lv_tmp_npv=(lv_costs+f_en_bus(la_bin_fin,la_bin_bus,la_bin_idl,lm_act_fin));if(lv_tmp_npv>lv_max_npv){lv_max_npv=lv_tmp_npv;}	// ENTER BUSY STATE.
					}while(la_sub_pos[m]<(la_act_idl[0]-1));
					do	// Shrink. Remove last activity, remove second last activity, add new activity.
					{	lv_nr32=static_cast<int>(floor(static_cast<double>(la_act_idl[(la_sub_pos[m]+1)])/static_cast<double>(32)));z2=ga_bin[static_cast<int>(la_act_idl[(la_sub_pos[m]+1)]-(lv_nr32*32))];la_bin_bus[lv_nr32]-=z2;la_bin_idl[lv_nr32]+=z2;lv_costs-=ga_rac[la_act_idl[(la_sub_pos[m]+1)]];m--;
						lv_nr32=static_cast<int>(floor(static_cast<double>(la_act_idl[(la_sub_pos[m]+1)])/static_cast<double>(32)));z2=ga_bin[static_cast<int>(la_act_idl[(la_sub_pos[m]+1)]-(lv_nr32*32))];la_bin_bus[lv_nr32]-=z2;la_bin_idl[lv_nr32]+=z2;lv_costs-=ga_rac[la_act_idl[(la_sub_pos[m]+1)]];la_sub_pos[m]++;
						lv_nr32=static_cast<int>(floor(static_cast<double>(la_act_idl[(la_sub_pos[m]+1)])/static_cast<double>(32)));z2=ga_bin[static_cast<int>(la_act_idl[(la_sub_pos[m]+1)]-(lv_nr32*32))];la_bin_bus[lv_nr32]+=z2;la_bin_idl[lv_nr32]-=z2;lv_costs+=ga_rac[la_act_idl[(la_sub_pos[m]+1)]];
						lv_tmp_npv=(lv_costs+f_en_bus(la_bin_fin,la_bin_bus,la_bin_idl,lm_act_fin));if(lv_tmp_npv>lv_max_npv){lv_max_npv=lv_tmp_npv;}	// ENTER BUSY STATE.
					}while((m>0) && (la_sub_pos[m]==(la_act_idl[0]-1)));
					if(m==0)
					{	if(la_sub_pos[m]==(la_act_idl[0]-1))
						{	z1=1;
	}	}	}while(z1==0);	}
	lv_nr32=static_cast<int>(floor(static_cast<double>(la_act_idl[(la_sub_pos[m]+1)])/static_cast<double>(32)));z2=ga_bin[static_cast<int>(la_act_idl[(la_sub_pos[m]+1)]-(lv_nr32*32))];la_bin_bus[lv_nr32]-=z2;la_bin_idl[lv_nr32]+=z2;
	}	}

	delete la_fnd_cod;
	delete la_act_idl;
	delete la_sub_pos;

	// Return the npv value corresponding to the optimal decision (i.e. the highest npv).
	return lv_max_npv;
}