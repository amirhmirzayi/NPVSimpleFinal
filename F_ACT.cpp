void f_act(void)
{
	// Initialization.
	unsigned long int	** lm_lib_bin;		// Holds the liberating code of regular action [i].
	unsigned long int	** lm_mrg_bin;		// Holds the binary code of merger [i].
	unsigned long int	** lm_act_bin;		// Holds the binary code of action [i].
	unsigned short int	** lm_mrg_act;		// Holds for each action [i] the set of actions that merge with action [i].
	bool				** lm_mrg_rel;		// Holds which actions are direct mergers.

	unsigned long int	* la_tmp_bin;		// Temporary binary bin.
	unsigned long int	* la_fnd_cod;		// Local array temporary code.
	unsigned short int	* la_nrm;			// Number of mergers for each action.
	unsigned short int	* la_sub_pos;
	bool				* la_blcklst;		// Blacklist array of actions.
	bool				* la_blcklst_mrg;	// Blacklist array of merged actions.

	unsigned short int	lv_nr32;
	unsigned short int	lv_nrm;				// Number of merged actions.

	unsigned short int	z0;

	unsigned short int	i;
	unsigned short int	j;
	unsigned short int	k;
	unsigned short int	m;
	unsigned short int	p;

	lm_lib_bin = new unsigned long int * [gv_bnr];
	lm_mrg_bin = new unsigned long int * [gv_bnr];
	lm_act_bin = new unsigned long int * [gv_bnr];
	for(k=0;k<gv_bnr;k++)
	{
		lm_mrg_bin[k] = new unsigned long int [ga_bin_min[16]];	// Further initialization is not required.
		lm_lib_bin[k] = new unsigned long int [gv_n];
		lm_act_bin[k] = new unsigned long int [gv_n];
		for(i=0;i<gv_n;i++)
		{
			lm_lib_bin[k][i]=0;	// Initialize matrix holding the binary codes of the liberated activities by executing action i.
			lm_act_bin[k][i]=0;	// Initialize matrix holding the binary codes of regular actions.
		}
	}

	lm_mrg_rel = new bool * [gv_n];
	for(i=0;i<gv_n;i++)
	{
		lm_mrg_rel[i] = new bool [gv_n];
		for(j=0;j<gv_n;j++)
		{
			lm_mrg_rel[i][j]=0;	// Initialize matrix holding which actions are allowed to merge with each other.
		}
	}

	la_tmp_bin = new unsigned long int [gv_bnr];
	la_fnd_cod = new unsigned long int [gv_bnr];
	la_nrm = new unsigned short int [gv_n];
	la_sub_pos = new unsigned short int [gv_n];
	la_blcklst = new bool [gv_n];
	for(i=0;i<gv_n;i++)
	{
		la_nrm[i]=0;	// Initialize the number of actions an action i is allowed to merge with.
		la_sub_pos[i]=0;	// Initialize the position array of activities to create mergers.
		la_blcklst[i]=0;	// Initialize the blacklist of regular actions.
	}

	gv_reg = gv_n;	// Initialize the number of regular actions.
	gv_mrg = 0;	// Intialize the number of merged actions.

	// Create liberated activities structure.
	for(i=0;i<gv_n;i++)	// Do for all regular actions i.
	{
		lv_nr32=static_cast<int>(floor(static_cast<double>(i)/static_cast<double>(32)));
		lm_lib_bin[lv_nr32][i]=ga_bin[i-(lv_nr32*32)];	// Set binary code.
	}

	// Create regular actions.
	for(i=0;i<gv_n;i++)	// Check all actions i.
	{
		for(j=0;j<i;j++)	// Check all predecessor actions j of action i.
		{
			if(gm_pre_rel[j][i]==1)	// Activity j is a predecessor of i.
			{
				lv_nr32=static_cast<int>(floor(static_cast<double>(j)/static_cast<double>(32)));
				lm_act_bin[lv_nr32][i]+=ga_bin[j-(lv_nr32*32)];	// Increase binary code of predecessors.
			}
		}
	}

	// Remove doubles.
	for(i=0;i<gv_n;i++)	// Check all actions i.
	{
		if(la_blcklst[i]==0)
		{
			for(j=(i+1);j<gv_n;j++)	// Check all other actions j.
			{
				if(la_blcklst[j]==0)
				{
					z0=0;
					for(k=0;k<gv_bnr;k++)
					{
						if(lm_act_bin[k][i]==lm_act_bin[k][j])
						{
							z0++;
						}
					}
					if(z0==gv_bnr)	// Both actions are doubles. Blacklist action j.
					{
						la_blcklst[j]=1;
						gv_reg--;	// Decrement the number of regular actions.
						for(k=0;k<gv_bnr;k++)
						{
							lm_lib_bin[k][i]+=lm_lib_bin[k][j];	// Increment liberating power of action i (i.e. action i does not only liberate activity i, but also activities liberated by action j.
						}
					}
				}
			}
		}
	}

	// Create merged actions.
	// Create merged information network.
	for(i=0;i<gv_n;i++)	// Check all actions i.
	{
		if(la_blcklst[i]==0)
		{
			for(j=(i+1);j<gv_n;j++)	// Check all other actions j.
			{
				if(la_blcklst[j]==0)
				{
					z0=0;
					for(k=0;k<gv_bnr;k++)
					{
						if((lm_act_bin[k][i] & lm_act_bin[k][j])>0)
						{
							z0++;
						}
					}
					if(z0>0)	// Some activities are shared, a merger is possible. Remark that subsets of larger sets may also merge. These mergers are deleted anyhow with the deletion of doubles.
					{
						la_nrm[i]++;	// Increase the number of mergers at action i.
						lm_mrg_rel[i][j]=1;	// Indicate that action i and action j are allowed to merge.
						lm_mrg_rel[j][i]=1;	// Indicate that action j and action i are allowed to merge.
					}
				}
			}
		}
	}

	lm_mrg_act = new unsigned short int * [gv_n];
	for(i=0;i<gv_n;i++)
	{
		if(la_nrm[i]>0)
		{
			m=0;
			lm_mrg_act[i] = new unsigned short int [la_nrm[i]];
			for(j=(i+1);j<gv_n;j++)
			{
				if(lm_mrg_rel[i][j]==1)
				{
					lm_mrg_act[i][m]=j;
					m++;
				}
			}
		}
	}

	// Create mergers themselves.
	lv_nrm=0;	// Initialize the number of mergers found so far.
	for(i=0;i<gv_n;i++)	// Check all actions i.
	{
		if(la_blcklst[i]==0)
		{
			if(la_nrm[i]>0)	// Check if mergers are possible.
			{
				// Create first merger of action i and action lm_mrg_act[i][0]. This merger is always legit.
				for(k=0;k<gv_bnr;k++)
				{
					lm_mrg_bin[k][lv_nrm]=(lm_act_bin[k][i] | lm_act_bin[k][lm_mrg_act[i][0]]);	// Bitwise or in order to incorporate all activities in both actions.
				}
				lv_nrm++;	// Indicate that a first merger is found.
				if(la_nrm[i]>1)	// Multiple mergers are to be formed; they are only allowed to merge if activities are shared among all constituting actions. So i has to merge with j and m and j has to merge with m.
				{
					m=0;	// Indicates the position. Start at first position.
					la_sub_pos[m]=0;	// Set first position to equal the first activity (this merger has already been written away.
					do
					{
						z0=0;
						do
						{
							if(z0<gv_bnr)	// The previous expand was legit, we can expand further.
							{
								m++;	// Expand.
								la_sub_pos[m]=la_sub_pos[(m-1)]+1;	// Set new position.
							}
							else
							{
								la_sub_pos[m]++;
							}
							// Write combination (check if all actions are allowed to merge (i.e. do they share at least one activity) => bitwise & binary codes and if result > 0 => create merger)
							for(k=0;k<gv_bnr;k++)
							{
								la_tmp_bin[k]=(lm_act_bin[k][i] & lm_act_bin[k][lm_mrg_act[i][la_sub_pos[0]]]);	// Merge the action i and the action at the first position (this merger always is legit).
							}
							for(j=1;j<=m;j++)
							{
								for(k=0;k<gv_bnr;k++)
								{
									la_tmp_bin[k]=(la_tmp_bin[k] & lm_act_bin[k][lm_mrg_act[i][la_sub_pos[j]]]);	// Merge result of previous merger and the action at the next position.
								}
							}
							z0=0;
							for(k=0;k<gv_bnr;k++)
							{
								if(la_tmp_bin[k]==0)
								{
									z0++;
								}
							}
							if(z0<gv_bnr)	// The merger is legit, activities are shared among the merged actions.
							{
								for(k=0;k<gv_bnr;k++)	// Bitwise or the first action.
								{
									lm_mrg_bin[k][lv_nrm]=(lm_act_bin[k][i] | lm_act_bin[k][lm_mrg_act[i][la_sub_pos[0]]]);	// Bitwise or in order to incorporate all activities in all actions.
								}
								for(j=1;j<=m;j++)	// Bitwise or the remaining actions.
								{
									for(k=0;k<gv_bnr;k++)
									{
										lm_mrg_bin[k][lv_nrm]=(lm_mrg_bin[k][lv_nrm] | lm_act_bin[k][lm_mrg_act[i][la_sub_pos[j]]]);	// Bitwise or in order to incorporate all activities in all actions.
									}
								}
								lv_nrm++;	// Indicate that a first merger is found.
							}
						}while(la_sub_pos[m]<(la_nrm[i]-1));	// Do until the position is maximal => you end up at a maximized position.
//						}while((la_sub_pos[m]<(la_nrm[i]-1)) && (z0<gv_bnr));	// Do until the position is maximal => you end up at a maximized position. If z0==gv_bnr, the merger is not legit, any further extensions of this merger are also not legit. Shrink procedure is required.

						// IF MERGER IS NOT LEGIT, IT DOES NOT MEAN THE MERGER CANNOT BE MADE WITH OTHER BASTARDS. SHRINKING IS NOT YET THE BEST DECISION.
						// SHRINKING IS NOT, BUT LOCAL INCREASE IS STILL POSSIBLE, AFTER LOCAL INCREASE, EXPAND MIGHT CONTINUE.

						do
						{
							m--;	// Shrink.
							la_sub_pos[m]++;	// Increase position.
							// Write combination (check if all actions are allowed to merge (i.e. do they share at least one activity) => bitwise & binary codes and if result > 0 => create merger)
							for(k=0;k<gv_bnr;k++)
							{
								la_tmp_bin[k]=(lm_act_bin[k][i] & lm_act_bin[k][lm_mrg_act[i][la_sub_pos[0]]]);	// Merge the action i and the action at the first position (this merger always is legit).
							}
							for(j=1;j<=m;j++)
							{
								for(k=0;k<gv_bnr;k++)
								{
									la_tmp_bin[k]=(la_tmp_bin[k] & lm_act_bin[k][lm_mrg_act[i][la_sub_pos[j]]]);	// Merge result of previous merger and the action at the next position.
								}
							}
							z0=0;
							for(k=0;k<gv_bnr;k++)
							{
								if(la_tmp_bin[k]==0)
								{
									z0++;
								}
							}
							// Remark. If you're coming from a legit expand, and you shrink, the shrink will always be legit as well. On the other hand, if you are coming from an expand that is not legit, the shrink might nog be legit. For instance, suppose 2,3,4 is a legit expand, you shrink and end up in 2,4 which also will be legit. On the other hand if 2,3,4 is illegit, a shrink resulting in 2,4 might be illegit as well. If this is the case, further shrinking is required, an expand of the illegit 2,4 will result in other illegit combinations.
							if(z0<gv_bnr)	// The merger is legit, activities are shared among the merged actions.
							{
								for(k=0;k<gv_bnr;k++)	// Bitwise | the first action.
								{
									lm_mrg_bin[k][lv_nrm]=(lm_act_bin[k][i] | lm_act_bin[k][lm_mrg_act[i][la_sub_pos[0]]]);	// Bitwise or in order to incorporate all activities in all actions.
								}
								for(j=1;j<=m;j++)	// Bitwise | the remaining actions.
								{
									for(k=0;k<gv_bnr;k++)
									{
										lm_mrg_bin[k][lv_nrm]=(lm_mrg_bin[k][lv_nrm] | lm_act_bin[k][lm_mrg_act[i][la_sub_pos[j]]]);	// Bitwise or in order to incorporate all activities in all actions.
									}
								}
								lv_nrm++;	// Indicate that a merger is found.
							}
//						}while(((la_sub_pos[m]==(la_nrm[i]-1)) || (z0==gv_bnr)) && (m>0));	// Do as long as the position is maximal and the first position is not maximized => you end up at a position which may be expanded or at a position in which m=0. Also if you are at an illegit shrink, further shrinking is required while expands of an illegit shrink would only result in other illegit combinations.
						}while(((la_sub_pos[m]==(la_nrm[i]-1))) && (m>0));
					}while(la_sub_pos[m]<(la_nrm[i]-1));	// You keep decreasing the position until you end up at the first position. If you are at the first position and it is maximized, we exit the loop.
				}
			}
		}
	}

	// Remove doubles.
	gv_mrg = lv_nrm;	// Initialize number of merged actions.
	la_blcklst_mrg = new bool [lv_nrm];
	for(i=0;i<lv_nrm;i++)
	{
		la_blcklst_mrg[i]=0;	// Initialize merged action blacklist.
	}
	for(i=0;i<lv_nrm;i++)	// Check all merged actions i.
	{
		if(la_blcklst_mrg[i]==0)
		{
			for(j=(i+1);j<lv_nrm;j++)	// Check all other merged actions j.
			{
				if(la_blcklst_mrg[j]==0)
				{
					z0=0;
					for(k=0;k<gv_bnr;k++)
					{
						if(lm_mrg_bin[k][i]==lm_mrg_bin[k][j])
						{
							z0++;
						}
					}
					if(z0==gv_bnr)	// Both merged actions are identical, blacklist action j.
					{
						gv_mrg--;	// Decrement the number of merged actions.
						la_blcklst_mrg[j]=1;
					}
				}
			}
			for(j=0;j<gv_n;j++)	// Check all regular actions as well.
			{
				if(la_blcklst[j]==0)
				{
					z0=0;
					for(k=0;k<gv_bnr;k++)
					{
						if(lm_mrg_bin[k][i]==lm_act_bin[k][j])
						{
							z0++;
						}
					}
					if(z0==gv_bnr)	// Both actions are identical, blacklist merged action i.
					{
						gv_mrg--;	// Decrement the number of merged actions.
						la_blcklst_mrg[i]=1;
					}
				}
			}
		}
	}

	// Remove first zero action.
	gv_reg--;
	la_blcklst[0]=1;

	// Create final structure. gv_reg = number of regular actions. gv_mrg = number of merged actions. gm_bin_reg = regular actions binary finishing code. gm_bin_mrg = merged actions binary finishing code. gm_bin_lib = regular actions liberating binary code.
	gm_lib_bin = new unsigned long int * [gv_bnr];
	gm_reg_bin = new unsigned long int * [gv_bnr];
	gm_mrg_bin = new unsigned long int * [gv_bnr];
	for(k=0;k<gv_bnr;k++)
	{
		gm_lib_bin[k] = new unsigned long int [gv_reg];
		gm_reg_bin[k] = new unsigned long int [gv_reg];
		gm_mrg_bin[k] = new unsigned long int [gv_mrg];
	}
	j=0;
	for(i=0;i<gv_n;i++)	// Check all actions.
	{
		if(la_blcklst[i]==0)
		{
			for(k=0;k<gv_bnr;k++)
			{
				gm_lib_bin[k][j]=lm_lib_bin[k][i];	// Copy liberating structure.
				gm_reg_bin[k][j]=lm_act_bin[k][i];	// Copy regular finishing structure.
			}
			j++;
		}
	}
	j=0;
	for(i=0;i<lv_nrm;i++)	// Check all mergers.
	{
		if(la_blcklst_mrg[i]==0)
		{
			for(k=0;k<gv_bnr;k++)
			{
				gm_mrg_bin[k][j]=lm_mrg_bin[k][i];	// Copy merger finishing structure.
			}
			j++;
		}
	}

	// Create gm_ava_bin and gm_ava_mrg_bin holding the info on which activities have to be finished in order for the action to be available (i.e. all predecessors of activities comprised in the action should be finished).
	gm_ava_bin = new unsigned long int * [gv_bnr];
	for(k=0;k<gv_bnr;k++)
	{
		gm_ava_bin[k] = new unsigned long int [gv_reg];
		for(i=0;i<gv_reg;i++)
		{
			gm_ava_bin[k][i]=0;	// Initialize the binary code of activities required to be finished prior to action i becoming available.
		}
	}
	for(i=0;i<gv_reg;i++)	// Check all regular actions.
	{
		for(k=0;k<gv_bnr;k++)
		{
			// Check first 16 bits.
			la_fnd_cod[k]=(gm_reg_bin[k][i] & ga_bin_min[16]);	// Binary operation such that only the first 16 bits remain (bitwise & 1111111111111111 (16 ones) and the first 16 bits of the udc code).
			if(la_fnd_cod[k]>0)
			{
				la_fnd_cod[k]--;	// Correct for one bit of data too short.
				for(m=0;m<gm_bin_act_cod[la_fnd_cod[k]][0];m++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
				{
					j=(gm_bin_act_cod[la_fnd_cod[k]][(m+1)]+(k*32));	// Indicates the member activity of action i.
					for(p=0;p<gv_bnr;p++)
					{
						gm_ava_bin[p][i]=(gm_ava_bin[p][i] | lm_act_bin[p][j]);	// The activities required to be finished prior to action i becoming available include the predecessors of activity j, which is a member activity of action i.
					}
				}
			}
			// Check second 16 bits.
			la_fnd_cod[k]=(gm_reg_bin[k][i] >> 16);	// Right shift 16 spaces; as such we only retain the second 16 bits.
			if(la_fnd_cod[k]>0)
			{
				la_fnd_cod[k]--;	// Correct for one bit of data too short.
				for(m=0;m<gm_bin_act_cod[la_fnd_cod[k]][0];m++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
				{
					j=(gm_bin_act_cod[la_fnd_cod[k]][(m+1)]+16+(k*32));	// Indicates the member activity of action i.
					for(p=0;p<gv_bnr;p++)
					{
						gm_ava_bin[p][i]=(gm_ava_bin[p][i] | lm_act_bin[p][j]);	// The activities required to be finished prior to action i becoming available include the predecessors of activity j, which is a member activity of action i.
					}
				}
			}
		}
	}

	gm_ava_mrg_bin = new unsigned long int * [gv_bnr];
	for(k=0;k<gv_bnr;k++)
	{
		gm_ava_mrg_bin[k] = new unsigned long int [gv_mrg];
		for(i=0;i<gv_mrg;i++)
		{
			gm_ava_mrg_bin[k][i]=0;	// Initialize the binary code of activities required to be finished prior to merged action i becoming available.
		}
	}
	for(i=0;i<gv_mrg;i++)	// Check all merged actions.
	{
		for(k=0;k<gv_bnr;k++)
		{
			// Check first 16 bits.
			la_fnd_cod[k]=(gm_mrg_bin[k][i] & ga_bin_min[16]);	// Binary operation such that only the first 16 bits remain (bitwise & 1111111111111111 (16 ones) and the first 16 bits of the udc code).
			if(la_fnd_cod[k]>0)
			{
				la_fnd_cod[k]--;	// Correct for one bit of data too short.
				for(m=0;m<gm_bin_act_cod[la_fnd_cod[k]][0];m++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
				{
					j=(gm_bin_act_cod[la_fnd_cod[k]][(m+1)]+(k*32));	// Indicates the member activity of action i.
					for(p=0;p<gv_bnr;p++)
					{
						gm_ava_mrg_bin[p][i]=(gm_ava_mrg_bin[p][i] | lm_act_bin[p][j]);	// The activities required to be finished prior to action i becoming available include the predecessors of activity j, which is a member activity of action i.
					}
				}
			}
			// Check second 16 bits.
			la_fnd_cod[k]=(gm_mrg_bin[k][i] >> 16);	// Right shift 16 spaces; as such we only retain the second 16 bits.
			if(la_fnd_cod[k]>0)
			{
				la_fnd_cod[k]--;	// Correct for one bit of data too short.
				for(m=0;m<gm_bin_act_cod[la_fnd_cod[k]][0];m++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
				{
					j=(gm_bin_act_cod[la_fnd_cod[k]][(m+1)]+16+(k*32));	// Indicates the member activity of action i.
					for(p=0;p<gv_bnr;p++)
					{
						gm_ava_mrg_bin[p][i]=(gm_ava_mrg_bin[p][i] | lm_act_bin[p][j]);	// The activities required to be finished prior to action i becoming available include the predecessors of activity j, which is a member activity of action i.
					}
				}
			}
		}
	}

	// Printing service (All regular actions, liberation and merged actions).
	if(gv_noprint==0)
	{
		printf("\n");
		printf("Regular actions: \n");
		for(i=0;i<gv_reg;i++)
		{
			printf("Reg action [%u][%u][%u] - ",i,gm_reg_bin[0][i],gm_ava_bin[0][i]);
			for(k=0;k<gv_bnr;k++)
			{
				// Check first 16 bits.
				la_fnd_cod[k]=(gm_reg_bin[k][i] & ga_bin_min[16]);	// Binary operation such that only the first 16 bits remain (bitwise & 1111111111111111 (16 ones) and the first 16 bits of the udc code).
				if(la_fnd_cod[k]>0)
				{
					la_fnd_cod[k]--;	// Correct for one bit of data too short.
					for(m=0;m<gm_bin_act_cod[la_fnd_cod[k]][0];m++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
					{
						printf("[%d]",(gm_bin_act_cod[la_fnd_cod[k]][(m+1)]+1+(k*32)));
					}
				}
				// Check second 16 bits.
				la_fnd_cod[k]=(gm_reg_bin[k][i] >> 16);	// Right shift 16 spaces; as such we only retain the second 16 bits.
				if(la_fnd_cod[k]>0)
				{
					la_fnd_cod[k]--;	// Correct for one bit of data too short.
					for(m=0;m<gm_bin_act_cod[la_fnd_cod[k]][0];m++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
					{
						printf("[%d]",(gm_bin_act_cod[la_fnd_cod[k]][(m+1)]+1+16+(k*32)));
					}
				}
			}
			printf(" - Lib - ");
			for(k=0;k<gv_bnr;k++)
			{
				// Check first 16 bits.
				la_fnd_cod[k]=(gm_lib_bin[k][i] & ga_bin_min[16]);	// Binary operation such that only the first 16 bits remain (bitwise & 1111111111111111 (16 ones) and the first 16 bits of the udc code).
				if(la_fnd_cod[k]>0)
				{
					la_fnd_cod[k]--;	// Correct for one bit of data too short.
					for(m=0;m<gm_bin_act_cod[la_fnd_cod[k]][0];m++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
					{
						printf("[%d]",(gm_bin_act_cod[la_fnd_cod[k]][(m+1)]+1+(k*32)));
					}
				}
				// Check second 16 bits.
				la_fnd_cod[k]=(gm_lib_bin[k][i] >> 16);	// Right shift 16 spaces; as such we only retain the second 16 bits.
				if(la_fnd_cod[k]>0)
				{
					la_fnd_cod[k]--;	// Correct for one bit of data too short.
					for(m=0;m<gm_bin_act_cod[la_fnd_cod[k]][0];m++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
					{
						printf("[%d]",(gm_bin_act_cod[la_fnd_cod[k]][(m+1)]+1+16+(k*32)));
					}
				}
			}
			printf("\n");
		}
		for(i=0;i<gv_mrg;i++)
		{
			printf("Mrg action [%d][%d][%d] - ",i,gm_mrg_bin[0][i],gm_ava_mrg_bin[0][i]);
			for(k=0;k<gv_bnr;k++)
			{
				// Check first 16 bits.
				la_fnd_cod[k]=(gm_mrg_bin[k][i] & ga_bin_min[16]);	// Binary operation such that only the first 16 bits remain (bitwise & 1111111111111111 (16 ones) and the first 16 bits of the udc code).
				if(la_fnd_cod[k]>0)
				{
					la_fnd_cod[k]--;	// Correct for one bit of data too short.
					for(m=0;m<gm_bin_act_cod[la_fnd_cod[k]][0];m++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
					{
						printf("[%d]",(gm_bin_act_cod[la_fnd_cod[k]][(m+1)]+1+(k*32)));
					}
				}
				// Check second 16 bits.
				la_fnd_cod[k]=(gm_mrg_bin[k][i] >> 16);	// Right shift 16 spaces; as such we only retain the second 16 bits.
				if(la_fnd_cod[k]>0)
				{
					la_fnd_cod[k]--;	// Correct for one bit of data too short.
					for(m=0;m<gm_bin_act_cod[la_fnd_cod[k]][0];m++)	// Evaluate all activities corresponding to this 16 bits code. Recall that position [0] holds the number of activities.
					{
						printf("[%d]",(gm_bin_act_cod[la_fnd_cod[k]][(m+1)]+1+16+(k*32)));
					}
				}
			}
			printf("\n");
		}
	}

	// Delete local structures.
	for(k=0;k<gv_bnr;k++)
	{
		delete lm_lib_bin[k];
		delete lm_mrg_bin[k];
		delete lm_act_bin[k];
	}
	delete lm_lib_bin;
	delete lm_mrg_bin;
	delete lm_act_bin;

	for(i=0;i<gv_n;i++)
	{
		if(la_nrm[i]>0)
		{
			delete lm_mrg_act[i];
		}
		delete lm_mrg_rel[i];
	}
	delete lm_mrg_act;
	delete lm_mrg_rel;

	delete la_tmp_bin;
	delete la_fnd_cod;
	delete la_nrm;
	delete la_sub_pos;
	delete la_blcklst;
	delete la_blcklst_mrg;
}