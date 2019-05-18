void f_udc(void)
{
	unsigned long int	*** lm_udc_str;			// Holds the UDC structure of the network (i.e. the binary codes of all UDCs).
	unsigned long int	*** lm_udc_fin_str;		// Holds all finished activities when at a given UDC.
	unsigned short int	*** lm_udc_io;			// Holds the target udc number of each link at each UDC at each recursion depth.
	unsigned short int	*** lm_udc_io_dpth;		// Holds the recursion depth of the target udc of each link at each UDC at each recursion depth.
	unsigned short int	*** lm_udc_nio;			// Holds the number of incoming [0] and outgoing [1] links at each UDC at each recursion depth.
	unsigned short int	*** lm_udc_mrg_io;		// Holds the target udc number of each link at each UDC at each recursion depth (merger).
	unsigned short int	*** lm_udc_mrg_io_dpth;	// Holds the recursion depth of the target udc of each link at each UDC at each recursion depth (merger).
	unsigned short int	*** lm_udc_mrg_nio;		// Holds the number of incoming [0] and outgoing [1] links at each UDC at each recursion depth (merger).

	unsigned long int	** lm_ava_bin;			// Holds the binary value of available activities of a regular action (excluding activities that are already finished).

	unsigned long int	* la_fin_bin;			// Holds binary value of finished activities.
	unsigned long int	* la_ava_bin;			// Holds binary value of available activities (i.e. the current UDC).
	unsigned short int	* la_nru;				// Holds the number of UDCs at each of the recursion depths.
	bool				* la_blcklst_exe;		// Indicates if action i has been executed.
	bool				* la_blcklst_ava;		// Indicates if an action i is available for execution.

	unsigned short int	lv_dpth;				// Holds the recursion depth. Depth is defined as the number of activities finished upon entering a UDC.

	unsigned short int	i;
	unsigned short int	j;
	unsigned short int	k;

	lm_udc_str = new unsigned long int ** [gv_bnr];
	lm_udc_fin_str = new unsigned long int ** [gv_bnr];
	for(k=0;k<gv_bnr;k++)
	{
		lm_udc_str[k] = new unsigned long int * [gv_n];	// For each recursion depth.
		lm_udc_fin_str[k] = new unsigned long int * [gv_n];	// For each recursion depth.
		for(i=0;i<gv_n;i++)
		{
			lm_udc_str[k][i] = new unsigned long int [ga_bin_min[16]];	// Initial UDC structure supporting up to [ga_bin_min[16]] UDCs per recursion depth.
			lm_udc_fin_str[k][i] = new unsigned long int [ga_bin_min[16]];	// Initial UDC structure supporting up to [ga_bin_min[16]] UDCs per recursion depth.
		}
	}
	lm_udc_io = new unsigned short int ** [gv_n];
	lm_udc_io_dpth = new unsigned short int ** [gv_n];
	for(i=0;i<gv_n;i++)	// For all recursion depths.
	{
		lm_udc_io[i] = new unsigned short int * [ga_bin_min[16]];
		lm_udc_io_dpth[i] = new unsigned short int * [ga_bin_min[16]];
		// Remark: do not create further data structure, only create it when required in order to save memory. For now reserve storage for ga_bin_min[16] udcs at each of the recursion levels. When a new udc is encountered, create the required storage on the fly.
	}
	lm_udc_io[0][0] = new unsigned short int [gv_reg];
	lm_udc_io[0][0][0]=0;	// Initialize the first udc; it links to the first udc at recursion depth 1 (after finishing the first activity).
	lm_udc_io_dpth[0][0] = new unsigned short int [gv_reg];
	lm_udc_io_dpth[0][0][0]=1;	// Initialize the first ucd; it links to the first udc at recursion depth 1 (after finishing the first activity).
	lm_udc_nio = new unsigned short int ** [2];	// Record both incoming [0] as well as outgoing [1] links.
	for(k=0;k<2;k++)
	{
		lm_udc_nio[k] = new unsigned short int * [gv_n];
		for(i=0;i<gv_n;i++)
		{
			lm_udc_nio[k][i] = new unsigned short int [ga_bin_min[16]];
			// Remark: do not initialize the data structure. Initialize on the fly when creating a new udc.
		}
	}
	lm_udc_nio[0][0][0]=0;	// There are no incoming links at the first udc.
	lm_udc_nio[1][0][0]=0;	// There is only one outgoing link at the first udc but that one will be recorded in the recursion itself.

	lm_ava_bin = new unsigned long int * [gv_bnr];
	for(k=0;k<gv_bnr;k++)
	{
		lm_ava_bin[k] = new unsigned long int [gv_reg];
	}

	la_fin_bin = new unsigned long int [gv_bnr];
	la_ava_bin = new unsigned long int [gv_bnr];
	la_nru = new unsigned short int [gv_n];

	la_blcklst_exe = new bool [gv_reg];
	la_blcklst_ava = new bool [gv_reg];

	// Prototype functions.
	void f_udc_rec(unsigned long int ***lm_udc_str, unsigned long int ***lm_udc_fin_str, unsigned short int ***lm_udc_io, unsigned short int ***lm_udc_io_dpth, unsigned short int ***lm_udc_nio,  unsigned long int **lm_ava_bin, unsigned long int *la_fin_bin, unsigned long int *la_ava_bin, unsigned short int *la_nru, bool *la_blcklst_exe, bool *la_blcklst_ava, unsigned short int lv_dpth);
	void f_udc_lnk(unsigned long int ***lm_udc_str, unsigned long int ***lm_udc_fin_str, unsigned short int ***lm_udc_io, unsigned short int ***lm_udc_io_dpth, unsigned short int ***lm_udc_nio, unsigned short int ***lm_udc_mrg_io, unsigned short int ***lm_udc_mrg_io_dpth, unsigned short int ***lm_udc_mrg_nio, unsigned short int *la_nru);

	// Initialize first recursion (only the first activity is available, no actions are finished, nothing has yet been recorded).
	for(k=0;k<gv_bnr;k++)
	{
		lm_udc_str[k][0][0]=0;	// Initialize the structure of the first UDC.
		lm_udc_fin_str[k][0][0]=0;
	}
	lm_udc_str[0][0][0]=1;	// Indicate that the only UDC at the first recursion depth has a binary value of 1 (indicating that activity 0 is available).
	for(k=0;k<gv_bnr;k++)
	{
		for(i=0;i<gv_reg;i++)
		{
			lm_ava_bin[k][i]=0;	// Indicate that no regular actions (except the first) are available yet.
		}
	}
	lm_ava_bin[0][0]=1;	// Indicate that the first action is allowed to be executed.
	for(k=0;k<gv_bnr;k++)
	{
		la_fin_bin[k]=0;	// Indicate that no activities have been finished yet.
		la_ava_bin[k]=0;	// Indicate that no activities (except the first) are available.
	}
	for(i=0;i<gv_n;i++)
	{
		la_nru[i]=0;	// Initialize the number of UDCs found at each recursion depth.
	}
	la_nru[0]=1;	// Indicate that there is one UDC at the first recursion depth.
	for(i=0;i<gv_reg;i++)
	{
		la_blcklst_exe[i]=0;	// Indicate that no action has finished so far.
		la_blcklst_ava[i]=1;	// Indicate that no action (except the first) is available so far.
	}
	la_blcklst_ava[0]=0;	// Indicate that the first action is available.

	lv_dpth=0;	// Initialize the first UDC depth (zero activities are finished).

	// Enter UDC creation recursion.

	f_udc_rec(lm_udc_str, lm_udc_fin_str, lm_udc_io, lm_udc_io_dpth, lm_udc_nio, lm_ava_bin, la_fin_bin, la_ava_bin, la_nru, la_blcklst_exe, la_blcklst_ava, lv_dpth);

	// Check all UDCs and their linkage (observe each of the UDCs; check all actions (regular and merged); observe if they are feasible; if they are feasible, to which UDC do they link).

	lm_udc_mrg_io = new unsigned short int ** [gv_n];
	lm_udc_mrg_io_dpth = new unsigned short int ** [gv_n];
	for(i=0;i<gv_n;i++)	// Check all recursion depths.
	{
		lm_udc_mrg_io[i] = new unsigned short int * [la_nru[i]];
		lm_udc_mrg_io_dpth[i] = new unsigned short int * [la_nru[i]];
		for(j=0;j<la_nru[i];j++)	// Check all udcs at all recursion depths.
		{
			lm_udc_mrg_io[i][j] = new unsigned short int [gv_mrg];	// A maximum of gv_mrg links may be made using merged actions.
			lm_udc_mrg_io_dpth[i][j] = new unsigned short int [gv_mrg];

		}
	}
	lm_udc_mrg_nio = new unsigned short int ** [2];
	for(k=0;k<2;k++)	// Check both incoming [0] as well as outgoing [1] links.
	{
		lm_udc_mrg_nio[k] = new unsigned short int * [gv_n];
		for(i=0;i<gv_n;i++)	// Check all recursion depths.
		{
			lm_udc_mrg_nio[k][i] = new unsigned short int [la_nru[i]];
			for(j=0;j<la_nru[i];j++)	// Check all udcs at all recursion depths.
			{
				lm_udc_mrg_nio[k][i][j]=0;	// Initialize the number of incoming and outgoing links at each udc at each recursion depth.
			}
		}
	}

	f_udc_lnk(lm_udc_str, lm_udc_fin_str, lm_udc_io, lm_udc_io_dpth, lm_udc_nio, lm_udc_mrg_io, lm_udc_mrg_io_dpth, lm_udc_mrg_nio, la_nru);

	// Create permanent structure.
	ga_ptg = new unsigned short int [gv_n];
	for(i=0;i<gv_n;i++)
	{
		ga_ptg[i]=la_nru[i];
	}
	gm_str_ptg = new unsigned long int ** [gv_bnr];
	for(k=0;k<gv_bnr;k++)
	{
		gm_str_ptg[k] = new unsigned long int * [gv_n];
		for(i=0;i<gv_n;i++)
		{
			gm_str_ptg[k][i] = new unsigned long int [ga_ptg[i]];
			for(j=0;j<ga_ptg[i];j++)
			{
				gm_str_ptg[k][i][j]=lm_udc_str[k][i][j];	// Copy ptg structure.
			}
		}
	}
	gm_lnk_ptg = new unsigned short int ** [gv_n];
	gm_dpt_ptg = new unsigned short int ** [gv_n];
	gm_nio_ptg = new unsigned short int ** [gv_n];
	gm_nio_ptg2 = new unsigned short int ** [gv_n];
	for(i=0;i<gv_n;i++)
	{
		gm_lnk_ptg[i] = new unsigned short int * [ga_ptg[i]];
		gm_dpt_ptg[i] = new unsigned short int * [ga_ptg[i]];
		gm_nio_ptg[i] = new unsigned short int * [ga_ptg[i]];
		gm_nio_ptg2[i] = new unsigned short int * [ga_ptg[i]];
		for(j=0;j<ga_ptg[i];j++)
		{
			gm_lnk_ptg[i][j] = new unsigned short int [lm_udc_mrg_nio[1][i][j]+lm_udc_nio[1][i][j]];	// Number of outgoing links (regular + merged).
			gm_dpt_ptg[i][j] = new unsigned short int [lm_udc_mrg_nio[1][i][j]+lm_udc_nio[1][i][j]];	// Number of outgoing links (regular + merged).
			gm_nio_ptg[i][j] = new unsigned short int [2];
			gm_nio_ptg2[i][j] = new unsigned short int [2];
			gm_nio_ptg[i][j][0]=lm_udc_nio[1][i][j]+lm_udc_mrg_nio[1][i][j];	// Copy outgoing link structure.
			gm_nio_ptg2[i][j][0]=lm_udc_nio[1][i][j]+lm_udc_mrg_nio[1][i][j];	// Copy outgoing link structure.
			gm_nio_ptg[i][j][1]=lm_udc_nio[0][i][j]+lm_udc_mrg_nio[0][i][j];	// Copy incoming link structure.
			gm_nio_ptg2[i][j][1]=lm_udc_nio[0][i][j]+lm_udc_mrg_nio[0][i][j];	// Copy incoming link structure.
			for(k=0;k<lm_udc_nio[1][i][j];k++)	// Copy link structure for all regular links.
			{
				gm_lnk_ptg[i][j][k]=lm_udc_io[i][j][k];	// Copy ptg link structure.
				gm_dpt_ptg[i][j][k]=lm_udc_io_dpth[i][j][k];	// Copy ptg link depth structure.
			}
			for(k=0;k<lm_udc_mrg_nio[1][i][j];k++)	// Copy link structure for all merged links.
			{
				gm_lnk_ptg[i][j][(k+lm_udc_nio[1][i][j])]=lm_udc_mrg_io[i][j][k];	// Copy ptg link structure.
				gm_dpt_ptg[i][j][(k+lm_udc_nio[1][i][j])]=lm_udc_mrg_io_dpth[i][j][k];	// Copy ptg link depth structure.
			}
		}
	}

	// Delete all obsolete data.
	for(k=0;k<gv_bnr;k++)
	{
		for(i=0;i<gv_n;i++)
		{
			delete lm_udc_str[k][i];
			delete lm_udc_fin_str[k][i];
		}
		delete lm_udc_str[k];
		delete lm_udc_fin_str[k];
	}
	delete lm_udc_str;
	delete lm_udc_fin_str;
	for(i=0;i<gv_n;i++)
	{
		for(j=0;j<la_nru[i];j++)
		{
			delete lm_udc_io[i][j];
			delete lm_udc_io_dpth[i][j];
			delete lm_udc_mrg_io[i][j];
			delete lm_udc_mrg_io_dpth[i][j];
		}
		delete lm_udc_io[i];
		delete lm_udc_io_dpth[i];
		delete lm_udc_mrg_io[i];
		delete lm_udc_mrg_io_dpth[i];
	}
	delete lm_udc_io;
	delete lm_udc_io_dpth;
	delete lm_udc_mrg_io;
	delete lm_udc_mrg_io_dpth;
	for(k=0;k<2;k++)
	{
		for(i=0;i<gv_n;i++)
		{
			delete lm_udc_nio[k][i];
			delete lm_udc_mrg_nio[k][i];
		}
		delete lm_udc_nio[k];
		delete lm_udc_mrg_nio[k];
	}
	delete lm_udc_nio;
	delete lm_udc_mrg_nio;

	for(k=0;k<gv_bnr;k++)
	{
		delete lm_ava_bin[k];
	}
	delete lm_ava_bin;

	delete la_fin_bin;
	delete la_ava_bin;
	delete la_nru;
	delete la_blcklst_exe;
	delete la_blcklst_ava;

	//////////////////////////////////////////////////////////////////////////
	///////////PRINT & DEBUG SERVICE//////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	unsigned long int * la_fnd_cod2;

	unsigned short int i2;
	unsigned short int j2;
	unsigned short int k2;
	unsigned short int l2;
	unsigned short int m2;

	la_fnd_cod2 = new unsigned long int [gv_bnr];

	if(gv_noprint==0)
	{
		m2=0;
#pragma omp parallel for schedule(dynamic) reduction(+:m2)
		for(i2=0;i2<gv_n;i2++)
		{
			for(j2=0;j2<ga_ptg[i2];j2++)
			{
				for(l2=0;l2<(gm_nio_ptg[i2][j2][0]-1);l2++)
				{
					for(k2=(l2+1);k2<gm_nio_ptg[i2][j2][0];k2++)
					{
						if(gm_lnk_ptg[i2][j2][l2]==gm_lnk_ptg[i2][j2][k2]&& gm_dpt_ptg[i2][j2][l2] == gm_dpt_ptg[i2][j2][k2])
						{
							m2++;
						}
					}
				}
			}
		}
		gv_double=m2;
		printf("\n");
		for (i2=0;i2<gv_n;i2++)
		{
			for(j2=0;j2<ga_ptg[i2];j2++)
			{
				printf("\n");
				printf("DEPTH: %d -*- PTG: %d -*- LINKS: IN[%d] OUT[%d] -*- Activities: \n",i2,j2,gm_nio_ptg[i2][j2][1],gm_nio_ptg[i2][j2][0]);
				for(k2=0;k2<gv_bnr;k2++)
				{
					// Check first 16 bits.
					la_fnd_cod2[k2]=(gm_str_ptg[k2][i2][j2] & ga_bin_min[16]);	// Binary operation such that only the first 16 bits remain (bitwise & 1111111111111111 (16 ones) and the first 16 bits of the ptg code).
					if(la_fnd_cod2[k2]>0)
					{
						la_fnd_cod2[k2]--;	// Correct for one bit of data too short.
						for(l2=0;l2<gm_bin_act_cod[la_fnd_cod2[k2]][0];l2++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
						{
							printf("[%d]",(gm_bin_act_cod[la_fnd_cod2[k2]][(l2+1)]+1+(k2*32)));
						}
					}
					// Check second 16 bits.
					la_fnd_cod2[k2]=(gm_str_ptg[k2][i2][j2] >> 16);	// Right shift 16 spaces; as such we only retain the second 16 bits.
					if(la_fnd_cod2[k2]>0)
					{
						la_fnd_cod2[k2]--;	// Correct for one bit of data too short.
						for(l2=0;l2<gm_bin_act_cod[la_fnd_cod2[k2]][0];l2++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
						{
							printf("[%d]",(gm_bin_act_cod[la_fnd_cod2[k2]][(l2+1)]+1+16+(k2*32)));
						}
					}
				}
				printf("\n");
				printf("LINKED PTGs:");
				printf("\n");
				for(m2=0;m2<gm_nio_ptg[i2][j2][0];m2++)	// Check all outgoing links of this ptg.
				{
					for(k2=0;k2<gv_bnr;k2++)
					{
						// Check first 16 bits.
						la_fnd_cod2[k2]=(gm_str_ptg[k2][gm_dpt_ptg[i2][j2][m2]][gm_lnk_ptg[i2][j2][m2]] & ga_bin_min[16]);	// Binary operation such that only the first 16 bits remain (bitwise & 1111111111111111 (16 ones) and the first 16 bits of the ptg code).
						if(la_fnd_cod2[k2]>0)
						{
							la_fnd_cod2[k2]--;	// Correct for one bit of data too short.
							for(l2=0;l2<gm_bin_act_cod[la_fnd_cod2[k2]][0];l2++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
							{
								printf("[%d]",(gm_bin_act_cod[la_fnd_cod2[k2]][(l2+1)]+1+(k2*32)));
							}
						}
						// Check second 16 bits.
						la_fnd_cod2[k2]=(gm_str_ptg[k2][gm_dpt_ptg[i2][j2][m2]][gm_lnk_ptg[i2][j2][m2]] >> 16);	// Right shift 16 spaces; as such we only retain the second 16 bits.
						if(la_fnd_cod2[k2]>0)
						{
							la_fnd_cod2[k2]--;	// Correct for one bit of data too short.
							for(l2=0;l2<gm_bin_act_cod[la_fnd_cod2[k2]][0];l2++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
							{
								printf("[%d]",(gm_bin_act_cod[la_fnd_cod2[k2]][(l2+1)]+1+16+(k2*32)));
							}
						}
					}
					printf("\n");
				}
			}
			printf("\n");
		}

		printf("\n");
		printf("THERE ARE %d DOUBLE OUTGOING LINKS",gv_double);
		printf("\n");

		j2=0;
		for(i2=0;i2<gv_n;i2++)
		{
			j2+=ga_ptg[i2];
		}
		printf("\n");
		printf("NR OF PTGS: %d",j2);
		printf("\n");
		k2=0;
		for(i2=0;i2<gv_n;i2++)
		{
			for(j2=0;j2<ga_ptg[i2];j2++)
			{
				k2+=gm_nio_ptg[i2][j2][0];
			}
		}
		printf("NR OF OUTGOING LINKS: %d",k2);
		printf("\n");
	}
	delete la_fnd_cod2;
}

void f_udc_lnk(unsigned long int ***lm_udc_str, unsigned long int ***lm_udc_fin_str, unsigned short int ***lm_udc_io, unsigned short int ***lm_udc_io_dpth, unsigned short int ***lm_udc_nio, unsigned short int ***lm_udc_mrg_io, unsigned short int ***lm_udc_mrg_io_dpth, unsigned short int ***lm_udc_mrg_nio, unsigned short int *la_nru)
{
	unsigned long int	**lm_ava_lnk_bin;		// Holds the available activities (binary) of an action at a given UDC.
	unsigned long int	**lm_ava_mrg_lnk_bin;	// Holds the available activities (binary) of an action at a given UDC.

	unsigned long int	* la_ava_mrg_lnk_bin;	// Holds the available activities of a given merged action at a given UDC.
	unsigned long int	* la_fin_lnk_bin;
	unsigned long int	* la_ava_lnk_bin;
	unsigned long int	* la_fnd_cod;
	unsigned short int	* la_sub_act;			// Holds the subactions of the action currently under investigation.
	bool				* la_whtlst_exe;

	unsigned short int	lv_nrs;					// Holds the number of subactions of a given action.
	unsigned short int	lv_dpth;

	unsigned short int	z0;
	bool				z1;

	unsigned short int	i;
	unsigned short int	j;
	unsigned short int	k;
	unsigned short int	m;
	unsigned short int	p;
	unsigned short int	q;

	lm_ava_lnk_bin = new unsigned long int * [gv_bnr];
	lm_ava_mrg_lnk_bin = new unsigned long int * [gv_bnr];
	for(k=0;k<gv_bnr;k++)
	{
		lm_ava_lnk_bin[k] = new unsigned long int [gv_reg];	// Hold available activities for all regular actions.
		lm_ava_mrg_lnk_bin[k] = new unsigned long int [gv_mrg];	// Hold available activities for all merged  actions.
	}

	la_ava_mrg_lnk_bin = new unsigned long int [gv_bnr];
	la_fin_lnk_bin = new unsigned long int [gv_bnr];
	la_ava_lnk_bin = new unsigned long int [gv_bnr];
	la_fnd_cod = new unsigned long int [gv_bnr];
	la_sub_act = new unsigned short int [gv_reg];	// Only regular actions are allowed to be subactions.
	la_whtlst_exe = new bool [gv_reg];	// Only regular actions may be put on the whitelist.

	for(i=0;i<gv_n;i++)	// Check all recursion depths.
	{
		for(j=0;j<la_nru[i];j++)	// Check all UDCs at this recursion depth.
		{
			// CREATE WHITELIST.
			for(m=0;m<gv_reg;m++)
			{
				la_whtlst_exe[m]=0;	// Initialize the whitelist (the whitelist contains all regular actions that are feasible in the current udc; i.e. all predecessors are finished and at least one activity is still available for execution).
			}
			for(m=0;m<gv_reg;m++)	// Create a whitelist of all regular actions. This whitelist will be used to determine if it is possible to execute an action without establishing other links.
			{
				z0=0;
				for(k=0;k<gv_bnr;k++)
				{
					if((gm_ava_bin[k][m] & lm_udc_fin_str[k][i][j])==gm_ava_bin[k][m])	// An action m is feasible if all its predecessors are finished in the UDC in question. gm_ava_bin holds the binary equivalent of activities required to be finished prior to an action to become available; lm_udc_fin_str holds which activities are finished in the given UDC.
					{
						z0++;
					}
				}
				if(z0==gv_bnr)	// All predecessors are finished, next check: are member activities still available or are they finished as well).
				{
					z0=0;
					for(k=0;k<gv_bnr;k++)
					{
						if((gm_reg_bin[k][m] & lm_udc_str[k][i][j])==0)	// In order to be feasible, an action m has to contain at least one activitiy that is still available (i.e. not finished).
						{
							z0++;
						}
					}
					if(z0<gv_bnr)	// Member activities of action m are still available, action m is feasible. Put it on a whitelist.
					{
						la_whtlst_exe[m]=1;	// Whitelist action m to indicate that it is feasible.
					}
				}
			}
			// Create available activities for whitelisted actions at the current UDC.
			for(m=0;m<gv_reg;m++)
			{
				if(la_whtlst_exe[m]==1)
				{
					for(k=0;k<gv_bnr;k++)
					{
						lm_ava_lnk_bin[k][m]=gm_reg_bin[k][m] & lm_udc_str[k][i][j];	// Holds the available activities of action m in the current UDC.
					}
				}
			}
			// CREATE MERGED ACTIONS LINKAGE.
			for(m=0;m<gv_mrg;m++)	// Check all merged actions. First observe if they are feasible.
			{
				z0=0;
				for(k=0;k<gv_bnr;k++)
				{
					if((gm_ava_mrg_bin[k][m] & lm_udc_fin_str[k][i][j])==gm_ava_mrg_bin[k][m])	// A merged action m is feasible if all its predecessors are finished in the UDC in question. gm_ava_mrg_bin holds the binary equivalent of activities required to be finished prior to a merged action to become available; lm_udc_fin_str holds which activities are finished in the given UDC.
					{
						z0++;
					}
				}
				if(z0==gv_bnr)	// All predecessors are finished, next check: are member activities still available or are they finished as well).
				{
					z0=0;
					for(k=0;k<gv_bnr;k++)
					{
						la_ava_mrg_lnk_bin[k]=(gm_mrg_bin[k][m] & lm_udc_str[k][i][j]);	// Create the set of all available activities of the merged action.
						if(la_ava_mrg_lnk_bin[k]==0)	// In order to be feasible, a merged action m has to contain at least one activitiy that is still available (i.e. not finished).
						{
							z0++;
						}
					}
					if(z0<gv_bnr)	// Member activities of merged action m are still available, merged action m is feasible. Check if it can be executed.
					{
						// Get all subactions.
						lv_nrs=0;	// Initialize the number of subactions for merged action m.
						for(p=0;p<gv_reg;p++)	// Check all regular actions.
						{
							if(la_whtlst_exe[p]==1)	// The regular actions should be on the whitelist.
							{
								z0=0;
								for(k=0;k<gv_bnr;k++)
								{
									if((la_ava_mrg_lnk_bin[k] & lm_ava_lnk_bin[k][p])==lm_ava_lnk_bin[k][p])	// Regular action p might be a subaction of merged action m. Remark that la_ava_mrg_lnk_bin gives the available activities of the merged action m at the current udc. lm_ava_lnk_bin gives the available activities of regular action p at the current udc.
									{
										z0++;
									}
								}
								if(z0==gv_bnr)	// Regular action p is a subaction of merged action m.
								{
									la_sub_act[lv_nrs]=p;	// Record that regular action p is a subaction of merged action m.
									lv_nrs++;	// Increase the number of subactions for merged action m.
								}
							}
						}
						// Create all pairs of subactions and observe if a pair exists that does nog share activities among pair-member actions.
						z1=0;
						if(lv_nrs>1)	// At least two subactions are required to form a pair.
						{
							for(p=0;p<(lv_nrs-1);p++)	// Check all subactions.
							{
								for(q=(p+1);q<lv_nrs;q++)	// Check all other subactions.
								{
									z0=0;
									for(k=0;k<gv_bnr;k++)
									{
										if((lm_ava_lnk_bin[k][la_sub_act[p]] & lm_ava_lnk_bin[k][la_sub_act[q]])==0)	// Regular action p and regular action q might not share any activities.
										{
											z0++;
										}
									}
									if(z0==gv_bnr)	// Regular action p and regular action q do not share any activities. It is impossible to execute merged action m. Exit the loop.
									{
										z1=1;
										q=lv_nrs;
										p=q;
									}
								}
							}
						}
						// Do or do not execute action m.
						if(z1==0)	// Merged action m is allowed to be executed; not enough subactions were present OR no pairs of subactions could be constructed that did not share activities.
						{
							lv_dpth=0;	// Initialize the number of finished activities; i.e. the recursion depth. Remark that you are currently at recursion depth i.
							for(k=0;k<gv_bnr;k++)
							{
								la_fin_lnk_bin[k]=(la_ava_mrg_lnk_bin[k] | lm_udc_fin_str[k][i][j]);	// All availabile activities in merged action i become finished. Bitwise | to avoid doubles.
								la_ava_lnk_bin[k]=(lm_udc_str[k][i][j] ^ la_ava_mrg_lnk_bin[k]);		// All remaining available activities; the set of initial available activities (i.e. the udc) minus the available activities of the merged action executed.
								la_fnd_cod[k]=(la_fin_lnk_bin[k] & ga_bin_min[16]);	// Binary operation such that only the first 16 bits remain (bitwise & 1111111111111111 (16 ones) and the first 16 bits of the udc code).
								if(la_fnd_cod[k]>0)
								{
									la_fnd_cod[k]--;	// Correct for one bit of data too short.
									lv_dpth+=gm_bin_act_cod[la_fnd_cod[k]][0];	// Increase the recursion depth.
								}
								la_fnd_cod[k]=(la_fin_lnk_bin[k] >> 16);	// Right shift 16 spaces; as such we only retain the second 16 bits.
								if(la_fnd_cod[k]>0)
								{
									la_fnd_cod[k]--;	// Correct for one bit of data too short.
									lv_dpth+=gm_bin_act_cod[la_fnd_cod[k]][0];	// Increase the recursion depth.
								}
							}
							for(p=0;p<gv_reg;p++)	// Check all other regular actions p to see if they are finished as well (i.e. if they do liberate other activities).
							{
								if(la_whtlst_exe[p]==1)	// Of course, regular action p should be available.
								{
									z0=0;
									for(k=0;k<gv_bnr;k++)
									{
										if((la_fin_lnk_bin[k] & lm_ava_lnk_bin[k][p])==lm_ava_lnk_bin[k][p])	// Regular action p might be finished.
										{
											z0++;
										}
									}
									if(z0==gv_bnr)	// Regular action p has finished. Liberate activities.
									{
										for(k=0;k<gv_bnr;k++)
										{
											la_ava_lnk_bin[k]=(la_ava_lnk_bin[k] | gm_lib_bin[k][p]);	// All activities liberated by action p are recorded in la_ava_lnk_bin.
										}
									}
								}
							}
							// RECORD THE LINK. Departure UDC and depth: lm_udc_str[k][i][j] and i. Arrival UDC and depth: la_ava_lnk_bin[k] and lv_dpth.
							for(p=0;p<la_nru[lv_dpth];p++)	// Check all udcs recorded at the target depth.
							{
								z0=0;
								for(k=0;k<gv_bnr;k++)
								{
									if(lm_udc_str[k][lv_dpth][p]==la_ava_lnk_bin[k])	// We might have found the matching UDC.
									{
										z0++;
									}
								}
								if(z0==gv_bnr)	// A matching UDC was found. Record link if it does not yet exist.
								{
									z1=0;
									for(q=0;q<lm_udc_nio[1][i][j];q++)	// Check all regular links to see if they exist. Check links at the departure udc.
									{
										if(lm_udc_io_dpth[i][j][q]==lv_dpth)	// If the recursion depth of the target UDC already occurs.
										{
											if(lm_udc_io[i][j][q]==p)	// And the targetted UDC itself is also recorded.
											{
												z1=1;
												q=lm_udc_nio[1][i][j];	// Exit loop.
											}
										}
									}
									if(z1==0)	// Perhaps the link does not yet exist. Still check merged actions.
									{
										for(q=0;q<lm_udc_mrg_nio[1][i][j];q++)	// Check all merged links to see if they exist. Check links at the departure udc.
										{
											if(lm_udc_mrg_io_dpth[i][j][q]==lv_dpth)	// If the recursion depth of the target UDC already occurs.
											{
												if(lm_udc_mrg_io[i][j][q]==p)	// And the targetted UDC itself is also recorded.
												{
													z1=1;
													q=lm_udc_mrg_nio[1][i][j];	// Exit loop.
												}
											}
										}
									}
									if(z1==0)	// The link does not yet exist, record it.
									{
										lm_udc_mrg_io[i][j][lm_udc_mrg_nio[1][i][j]]=p;					// Record the number of the target udc.
										lm_udc_mrg_io_dpth[i][j][lm_udc_mrg_nio[1][i][j]]=lv_dpth;		// Record the depth of the target udc.
										lm_udc_mrg_nio[0][lv_dpth][p]++;								// Increment the number of incoming links at the target udc.
										p=la_nru[lv_dpth];	// Exit the loop.
										lm_udc_mrg_nio[1][i][j]++;	// Increase the number of outgoing links at UDC j at depth i.
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// Destruction phase.
	for(k=0;k<gv_bnr;k++)
	{
		delete lm_ava_lnk_bin[k];
		delete lm_ava_mrg_lnk_bin[k];
	}
	delete la_ava_mrg_lnk_bin;
	delete la_fin_lnk_bin;
	delete la_ava_lnk_bin;
	delete la_fnd_cod;
	delete la_sub_act;
	delete la_whtlst_exe;
}

void f_udc_rec(unsigned long int ***lm_udc_str, unsigned long int ***lm_udc_fin_str, unsigned short int ***lm_udc_io, unsigned short int ***lm_udc_io_dpth, unsigned short int ***lm_udc_nio, unsigned long int **lm_ava_bin, unsigned long int *la_fin_bin, unsigned long int *la_ava_bin, unsigned short int *la_nru, bool *la_blcklst_exe, bool *la_blcklst_ava, unsigned short int lv_dpth)
{
	unsigned long int	* la_fnd_cod;
	unsigned short int	* la_sub_act;	// Holds the subactions of the action currently under investigation.

	unsigned short int	lv_nrs;			// Holds the number of subactions an action currently under investigation has.

	unsigned short int	z0;
	bool				z1;

	unsigned short int	i;
	unsigned short int	j;
	unsigned short int	k;
	unsigned short int	m;
	unsigned short int	q;

	la_fnd_cod = new unsigned long int [gv_bnr];
	la_sub_act = new unsigned short int [gv_reg];	// Only regular actions are allowed to be subactions.

	// Prototyping functions.
	void f_prnt(unsigned long int *la_ava_bin);

	// Create copies of changeable parameters passed in the recursion (not to be changed parameters passed in the recursion are: la_nru, lm_udc_io, lm_udc_io_dpth, lm_udc_nio, lm_udc_str, lm_udc_fin_str).
	unsigned long int	** lm_cpy_ava_bin;			// Holds the binary value of available activities of a regular action (excluding activities that are already finished).

	unsigned long int	* la_cpy_fin_bin;			// Holds binary value of finished activities.
	unsigned long int	* la_cpy_ava_bin;			// Holds binary value of available activities (i.e. the current UDC).
	bool				* la_cpy_blcklst_exe;		// Indicates if action i has been executed.
	bool				* la_cpy_blcklst_ava;		// Indicates if an action i is available for execution.

	unsigned short int	lv_cpy_dpth;				// Holds the recursion depth.

	lm_cpy_ava_bin = new unsigned long int * [gv_bnr];
	for(k=0;k<gv_bnr;k++)
	{
		lm_cpy_ava_bin[k] = new unsigned long int [gv_reg];
	}

	la_cpy_fin_bin = new unsigned long int [gv_bnr];
	la_cpy_ava_bin = new unsigned long int [gv_bnr];
	la_cpy_blcklst_exe = new bool [gv_reg];
	la_cpy_blcklst_ava = new bool [gv_reg];
	
	for(i=0;i<gv_reg;i++)	// Check all regular actions.
	{
//		if(la_blcklst_exe[i]==0)	// Finished actions do not need to be recorded, they are not available and will not become available.
		{
			for(k=0;k<gv_bnr;k++)
			{
				lm_cpy_ava_bin[k][i]=lm_ava_bin[k][i];
			}
		}
	}

	for(k=0;k<gv_bnr;k++)
	{
		la_cpy_fin_bin[k]=la_fin_bin[k];
		la_cpy_ava_bin[k]=la_ava_bin[k];
	}
	for(i=0;i<gv_reg;i++)
	{
		la_cpy_blcklst_exe[i]=la_blcklst_exe[i];
		la_cpy_blcklst_ava[i]=la_blcklst_ava[i];
	}

	lv_cpy_dpth=lv_dpth;

	// For each action: (1) check if it is already executed; (2) check if it can be executed (all activities available/finished); (3) check if it does not contain 2 or more regular, available actions that do not share activities.
	// CHECK ALL REGULAR ACTIONS.
	i=0;
	do
	{
		if(la_blcklst_exe[i]==0)	// Action i may not be finished already.
		{
			if(la_blcklst_ava[i]==0)	// Action i should be available.
			{
				lv_nrs=0;	// Initialize the number of subactions.
				// Check all other, available regular actions whose remainder is a subaction of action i.
				for(j=0;j<gv_reg;j++)	// Check all other regular actions j.
				{
					if(i!=j)	// Of course i and j have to differ.
					{
						if(la_blcklst_exe[j]==0)	// Action j may not be finished already.
						{
							if(la_blcklst_ava[j]==0)	// Action j should be available.
							{
								z0=0;
								for(k=0;k<gv_bnr;k++)
								{
									if((lm_ava_bin[k][j] & lm_ava_bin[k][i])==lm_ava_bin[k][j])	// Action j might be a subaction of action i.
									{
										z0++;
									}
								}
								if(z0==gv_bnr)	// Action j is a subaction of action i. Record it for further use.
								{
									la_sub_act[lv_nrs]=j;	// Record that action j is a subaction of action i.
									lv_nrs++;	// Increase the number of subactions for action i.
								}
							}
						}
					}
				}
				// Create all pairs of subactions and observe if a pair exists that does nog share activities among pair-member actions.
				z1=0;
				if(lv_nrs>1)	// At least two subactions are required to form a pair.
				{
					for(j=0;j<(lv_nrs-1);j++)	// Check all subactions.
					{
						for(m=(j+1);m<lv_nrs;m++)	// Check all other subactions.
						{
							z0=0;
							for(k=0;k<gv_bnr;k++)
							{
								if((lm_ava_bin[k][la_sub_act[j]] & lm_ava_bin[k][la_sub_act[m]])==0)	// Action j and action m might not share any activities.
								{
									z0++;
								}
							}
							if(z0==gv_bnr)	// Action j and action m do not share any activities. It is impossible to execute action i. Exit the loop.
							{
								z1=1;
								m=lv_nrs;
								j=m;
							}
						}
					}
				}
				// Do or do not execute action i.
				if(z1==0)	// Only if not enough subactions were present or if no subaction pairs could be constructed that did not share activities, action i is allowed to be executed.
				{
					lv_dpth=0;	// Initialize the number of finished activities; i.e. the recursion depth.
					for(k=0;k<gv_bnr;k++)
					{
						la_fin_bin[k]=(lm_ava_bin[k][i] | la_fin_bin[k]);	// All availabile activities in action i become finished. Bitwise | to avoid doubles.
						la_ava_bin[k]=((la_ava_bin[k] | la_fin_bin[k]) ^ la_fin_bin[k]);	// Create set of available AND finished activities. Subtract finished activities to obtain remaining available activities.
						la_ava_bin[k]=(la_ava_bin[k] | gm_lib_bin[k][i]);	// Add to the available activities the activities liberated by performing action i.
						la_fnd_cod[k]=(la_fin_bin[k] & ga_bin_min[16]);	// Binary operation such that only the first 16 bits remain (bitwise & 1111111111111111 (16 ones) and the first 16 bits of the udc code).
						if(la_fnd_cod[k]>0)
						{
							la_fnd_cod[k]--;	// Correct for one bit of data too short.
							lv_dpth+=gm_bin_act_cod[la_fnd_cod[k]][0];	// Increase the recursion depth.
						}
						la_fnd_cod[k]=(la_fin_bin[k] >> 16);	// Right shift 16 spaces; as such we only retain the second 16 bits.
						if(la_fnd_cod[k]>0)
						{
							la_fnd_cod[k]--;	// Correct for one bit of data too short.
							lv_dpth+=gm_bin_act_cod[la_fnd_cod[k]][0];	// Increase the recursion depth.
						}
					}
					la_blcklst_exe[i]=1;	// Action i has been executed.
					la_blcklst_ava[i]=1;	// Action i is no longer available.

					for(j=0;j<gv_reg;j++)	// Check all other regular actions j to see if they are finished as well (i.e. if they do liberate other activities).
					{
						if(la_blcklst_exe[j]==0)	// Action j should not have been executed already.
						{
							if(la_blcklst_ava[j]==0)	// Action j should have been available.
							{
								z0=0;
								for(k=0;k<gv_bnr;k++)
								{
									if((la_fin_bin[k] & lm_ava_bin[k][j])==lm_ava_bin[k][j])	// If the finished activities contain all available activities in action j, action j is finished.
									{
										z0++;
									}
								}
								if(z0==gv_bnr)	// All available activities of action j have finished, action j has finished.
								{
									la_blcklst_exe[j]=1;	// Action j has been executed.
									la_blcklst_ava[j]=1;	// Action j is no longer available.
									for(k=0;k<gv_bnr;k++)
									{
										la_ava_bin[k]=(la_ava_bin[k] | gm_lib_bin[k][j]);	// All activities liberated by action j are recorded in la_ava_bin.
									}
								}
								else	// Action j is not finished, however, its available activities may have been altered (some activities may have been finished by executing action i).
								{
									for(k=0;k<gv_bnr;k++)
									{
										lm_ava_bin[k][j]=((lm_ava_bin[k][j] | la_fin_bin[k]) ^ la_fin_bin[k]);	// Bitwise | to create set that contains available AND finished activities. Bitwise ^ to subtract the finished activities and to obtain all remaining available activities.
									}
								}
							}
							else	// Action j is unavailable, perhaps now with the finishing of action i it has become available. In other words, all activities in gm_ava_bin have to be finished (i.e. all predecessors of activities in gm_reg_bin[k][j]).
							{
								z0=0;
								for(k=0;k<gv_bnr;k++)
								{
									if((gm_ava_bin[k][j] & la_fin_bin[k])==gm_ava_bin[k][j])	// All predecessors of action j are finished.
									{
										z0++;
									}
								}
								if(z0==gv_bnr)	// All activities required to be finished in order to execute action j have been finished. Action j becomes available.
								{
									la_blcklst_ava[j]=0;	// Action j becomes available.
									for(k=0;k<gv_bnr;k++)
									{
										lm_ava_bin[k][j]=(la_ava_bin[k] & gm_reg_bin[k][j]);	// Select only the activities that are available and that are required to execute action j.
									}
								}
							}
						}
					}
					// EXECUTE ACTION i AND WRITE UDC IF IT IS NEW. A UDC IS NEW IF ITS AVAILABLE ACTIVITIES DIFFER FROM PREVIOUS UDCs.
					z1=0;
					if(la_nru[lv_dpth]>0)	// At least one UDC should already be recorded, otherwise the UDC is new.
					{
						for(j=0;j<la_nru[lv_dpth];j++)
						{
							z0=0;
							for(k=0;k<gv_bnr;k++)
							{
								if(la_ava_bin[k]==lm_udc_str[k][lv_dpth][j])
								{
									z0++;
								}
							}
							if(z0==gv_bnr)	// The UDC is not new, exit loop.
							{
								z1=1;
								z0=j;	// Record the UDC number that matches the target UDC to be used to record linkage from departure to target udc.
								j=la_nru[lv_dpth];	// Exit loop.
							}
						}
					}
//					f_prnt(la_cpy_ava_bin);
//					f_prnt(la_ava_bin);
					if(z1==0)	// The UDC is new. Write away and initialize input output.
					{
						for(k=0;k<gv_bnr;k++)
						{
							lm_udc_str[k][lv_dpth][la_nru[lv_dpth]]=la_ava_bin[k];
							lm_udc_fin_str[k][lv_dpth][la_nru[lv_dpth]]=la_fin_bin[k];
						}
						// The departure depth and number of departure udc are given by lv_dpth_cpy and (la_nru[lv_cpy_dpth]-1). The depth and number of arrival udc are given by lv_dpth and la_nru[lv_dpth].
						lm_udc_nio[0][lv_dpth][la_nru[lv_dpth]]=1;	// Indicate that the new UDC has a first incoming link.
						lm_udc_nio[1][lv_dpth][la_nru[lv_dpth]]=0;	// Indicate that the new UDC does not yet have outgoing links.
						lm_udc_io[lv_dpth][la_nru[lv_dpth]] = new unsigned short int [gv_reg];	// Record storage for all possible links (no more than gv_reg regular links are to be established).
						lm_udc_io_dpth[lv_dpth][la_nru[lv_dpth]] = new unsigned short int [gv_reg];	// Record storage for all possible links (no more than gv_reg regular links are to be established).
						lm_udc_io[lv_cpy_dpth][(la_nru[lv_cpy_dpth]-1)][lm_udc_nio[1][lv_cpy_dpth][(la_nru[lv_cpy_dpth]-1)]]=la_nru[lv_dpth];	// Record the number of the target udc. the depth of the departure udc is lv_cpy_dpth. The number of the departure udc is (la_nru[lv_cpy_dpth]-1). The link of the departure udc leading to this target udc is lm_udc_nio[1][lv_cpy_dpth][(la_nru[lv_cpy_dpth]-1)].
						lm_udc_io_dpth[lv_cpy_dpth][(la_nru[lv_cpy_dpth]-1)][lm_udc_nio[1][lv_cpy_dpth][(la_nru[lv_cpy_dpth]-1)]]=lv_dpth;		// Record the depth of the target udc. the depth of the departure udc is lv_cpy_dpth. The number of the departure udc is (la_nru[lv_cpy_dpth]-1). The link of the departure udc leading to this target udc is lm_udc_nio[1][lv_cpy_dpth][(la_nru[lv_cpy_dpth]-1)].
						lm_udc_nio[1][lv_cpy_dpth][(la_nru[lv_cpy_dpth]-1)]++;	// Increment the number of outgoing links from the UDC we originate from (at recursion depth lv_cpy_dpth and UDC number la_nru[lv_cpy_dpth]-1).
						la_nru[lv_dpth]++;
						// ENTER A NEW RECURSION.
//						f_prnt(la_ava_bin);
						f_udc_rec(lm_udc_str, lm_udc_fin_str, lm_udc_io, lm_udc_io_dpth, lm_udc_nio, lm_ava_bin, la_fin_bin, la_ava_bin, la_nru, la_blcklst_exe, la_blcklst_ava, lv_dpth);
					}
					else	// The UDC is not new. Increase input output and record linkage if link does not yet exist.
					{
						z1=0;
						for(q=0;q<lm_udc_nio[1][lv_cpy_dpth][(la_nru[lv_cpy_dpth]-1)];q++)	// Check all links to see if they exist. Check links at the departure udc.
						{
							if(lm_udc_io_dpth[lv_cpy_dpth][(la_nru[lv_cpy_dpth]-1)][q]==lv_dpth)	// If the recursion depth of the target UDC already occurs.
							{
								if(lm_udc_io[lv_cpy_dpth][(la_nru[lv_cpy_dpth]-1)][q]==z0)	// And the targetted UDC itself is also recorded.
								{
									z1=1;
									q=lm_udc_nio[1][lv_cpy_dpth][(la_nru[lv_cpy_dpth]-1)];	// Exit loop.
								}
							}
						}
						if(z1==0)	// The link does not yet exist, record it.
						{
							lm_udc_nio[0][lv_dpth][z0]++;	// Increment the number of incoming links at the target udc.
							lm_udc_io[lv_cpy_dpth][(la_nru[lv_cpy_dpth]-1)][lm_udc_nio[1][lv_cpy_dpth][(la_nru[lv_cpy_dpth]-1)]]=z0;			// Record the number of the target udc. the depth of the departure udc is lv_cpy_dpth. The number of the departure udc is (la_nru[lv_cpy_dpth]-1). The link of the departure udc leading to this target udc is lm_udc_nio[1][lv_cpy_dpth][(la_nru[lv_cpy_dpth]-1)].
							lm_udc_io_dpth[lv_cpy_dpth][(la_nru[lv_cpy_dpth]-1)][lm_udc_nio[1][lv_cpy_dpth][(la_nru[lv_cpy_dpth]-1)]]=lv_dpth;	// Record the depth of the target udc. the depth of the departure udc is lv_cpy_dpth. The number of the departure udc is (la_nru[lv_cpy_dpth]-1). The link of the departure udc leading to this target udc is lm_udc_nio[1][lv_cpy_dpth][(la_nru[lv_cpy_dpth]-1)].
							lm_udc_nio[1][lv_cpy_dpth][(la_nru[lv_cpy_dpth]-1)]++;	// Increment the number of outgoing links from the UDC we originate from (at recursion depth lv_cpy_dpth and UDC number la_nru[lv_cpy_dpth]-1).
						}
					}

					// Reset all parameters prior to moving to the next action.
					for(j=0;j<gv_reg;j++)	// Copy all regular actions.
					{
//						if(la_cpy_blcklst_exe[j]==0)	// Only copy actions that were previously not finished.
						{
							for(k=0;k<gv_bnr;k++)
							{
								lm_ava_bin[k][j]=lm_cpy_ava_bin[k][j];
							}
						}
					}
					for(k=0;k<gv_bnr;k++)
					{
						la_fin_bin[k]=la_cpy_fin_bin[k];
						la_ava_bin[k]=la_cpy_ava_bin[k];
					}
					for(j=0;j<gv_reg;j++)
					{
						la_blcklst_exe[j]=la_cpy_blcklst_exe[j];
						la_blcklst_ava[j]=la_cpy_blcklst_ava[j];
					}
					lv_dpth=lv_cpy_dpth;
				}
			}
		}
		i++;
	}while(i<gv_reg);	// Do until all regular actions have been checked.

	// DESTRUCTION PHASE.
	for(k=0;k<gv_bnr;k++)
	{
		delete lm_cpy_ava_bin[k];
	}
	delete lm_cpy_ava_bin;

	delete la_sub_act;
	delete la_fnd_cod;
	delete la_cpy_fin_bin;
	delete la_cpy_ava_bin;
	delete la_cpy_blcklst_exe;
	delete la_cpy_blcklst_ava;
}

void f_prnt(unsigned long int *la_ava_bin)
{
	unsigned long int	* la_fnd_cod;
	unsigned short int	l;
	unsigned short int	k;

	la_fnd_cod = new unsigned long int [gv_bnr];

	printf("\n");
	for(k=0;k<gv_bnr;k++)
	{
		// Check first 16 bits.
		la_fnd_cod[k]=(la_ava_bin[k] & ga_bin_min[16]);	// Binary operation such that only the first 16 bits remain (bitwise & 1111111111111111 (16 ones) and the first 16 bits of the udc code).
		if(la_fnd_cod[k]>0)
		{
			la_fnd_cod[k]--;	// Correct for one bit of data too short.
			for(l=0;l<gm_bin_act_cod[la_fnd_cod[k]][0];l++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
			{
				printf("[%d]",(gm_bin_act_cod[la_fnd_cod[k]][(l+1)]+1+(k*32)));
			}
		}
		// Check second 16 bits.
		la_fnd_cod[k]=(la_ava_bin[k] >> 16);	// Right shift 16 spaces; as such we only retain the second 16 bits.
		if(la_fnd_cod[k]>0)
		{
			la_fnd_cod[k]--;	// Correct for one bit of data too short.
			for(l=0;l<gm_bin_act_cod[la_fnd_cod[k]][0];l++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
			{
				printf("[%d]",(gm_bin_act_cod[la_fnd_cod[k]][(l+1)]+1+16+(k*32)));
			}
		}
	}
	printf("\n");
	delete la_fnd_cod;
}