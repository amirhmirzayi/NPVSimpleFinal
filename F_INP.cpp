void f_inp(void)
{
	//////////////////////////////////////////////////////////////////////////
	///////////DEFINE LOCAL VARIABLES/////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	int					buffer_base;

	unsigned short int	i;			// Simple counter.
	unsigned short int	j;			// Simple counter.
	unsigned short int	k;			// Simple counter.
	unsigned short int	z3;			// Temporary input reading variable; represents the successor number.

	//////////////////////////////////////////////////////////////////////////
	///////////INITIALIZATION/////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	char buffer [10];
	buffer_base=10;

	if(gv_batch==0)
	{

		printf("\n");
		printf("Batch RANLIB 10-120 activities \n");
		gv_rangen=1;
		gv_batch=1;
		ga_batch[1]=1;
		gv_rate=0.04;	// Set discount rate.

		// CUSTOM STARTING POINT
		printf("Choose RANGEN instance (1-1050)");
		scanf_s("%d", &ga_batch[1]);
		//ga_batch[1] =750;
	}
	else	// This is a batching assignment.
	{
		if(ga_batch[1]<1050)
		{
			ga_batch[1]++;
		}
		else
		{
			throw "End of batch; exit application";
		}
	}

	std::string fileName;
	// Construct the file name.
	fileName ="RANGEN/Pat";
	_itoa_s(ga_batch[1],buffer,buffer_base);
	fileName.append(buffer);
	fileName.append(".dat");

	// Reading the file.
	ifstream inFile;
	inFile.open(fileName.c_str());	// Path of stream = string fileName.
	if (!inFile)
	{
		printf("Unable to open file");
		throw "Unable to open file";
	}
	inFile >> gv_n;

	ga_nrs = new unsigned short int [gv_n];
	gm_pre_rel = new bool * [gv_n];
	gm_tra_rel = new bool * [gv_n];

	ga_act_dur = new double [gv_n];
	ga_rac = new double [gv_n];
	ga_batch[0] = gv_n-2;
	for(i=0;i<gv_n;i++)
	{
		inFile >> ga_act_dur[i];
		inFile >> ga_rac[i];
		gm_pre_rel[i] = new bool [gv_n];
		gm_tra_rel[i] = new bool [gv_n];
		for(j=0;j<gv_n;j++)
		{
			gm_pre_rel[i][j]=0;
			gm_tra_rel[i][j]=0;
		}
		inFile >> ga_nrs[i];
		for(j=0;j<ga_nrs[i];j++)
		{
			inFile >> z3;
			gm_pre_rel[i][(z3-1)]=1;
			gm_tra_rel[i][(z3-1)]=1;
		}
	}

	// Close the file
	inFile.close();

	// Print general precedence relationship matrix
	if(gv_noprint==0)
	{
		printf("\n");
		printf("Precedence relationship matrix: \n");
		for(i=0;i<gv_n;i++)
		{
			for(j=0;j<gv_n;j++)
			{
				printf("%d",gm_pre_rel[i][j]);
			}
			printf("\n");
		}
	}

	// Construct transitive precedence relationship matrix; make precedence relationships transitive.
	for(i=0;i<gv_n;i++)
	{
		for(j=0;j<gv_n;j++)
		{
			if(gm_tra_rel[i][j]==1)
			{
				for(k=0;k<gv_n;k++)
				{
					if(gm_tra_rel[j][k]==1)
					{
						gm_tra_rel[i][k]=1;
					}
				}
			}
		}
		gm_tra_rel[i][i]=1;
	}

	// Print transitive relationship matrix
	if(gv_noprint==0)
	{
		printf("\n");
		printf("Transitive relationship matrix: \n");
		for(i=0;i<gv_n;i++)
		{
			for(j=0;j<gv_n;j++)
			{
				printf("%d",gm_tra_rel[i][j]);
			}
			printf("\n");
		}
	}

	// Compute lambdas.
	ga_lambda = new double [gv_n];
	for(i=1;i<(gv_n-1);i++)	// Compute ga_lambda for all activities except the first and final node.
	{
		if(ga_act_dur[i]==0)
		{
			throw("Zero activity duration.");
		}
		else
		{
			ga_lambda[i]=(1/ga_act_dur[i]);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	///////////DETERMINE GV_BNR///////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	gv_bnr=static_cast<int>(ceil(static_cast<double>(gv_n)/static_cast<double>(32)));
}