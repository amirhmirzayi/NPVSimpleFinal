void f_bin(void)
{
	//////////////////////////////////////////////////////////////////////////
	///////////DEFINE LOCAL VARIABLES/////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	unsigned short int	* la_act_cod;

	unsigned short int	i;	// Simple counter.
	unsigned short int	j;	// Simple counter.
	unsigned short int	k;	// Simple counter.

	//////////////////////////////////////////////////////////////////////////
	///////////CREATE AND INITIALIZE GM_BIN_ACT_COD///////////////////////////
	//////////////////////////////////////////////////////////////////////////

	// Create la_act_cod and gm_bin_act_cod.
	la_act_cod = new unsigned short int [16];
	gm_bin_act_cod = new unsigned short int * [ga_bin_min[16]];	// Maximum array size.

	// Initialize gm_bin_act_cod.
	for(i=0;i<ga_bin_min[16];i++)	// Bitwise compare i to all other numbers to see if they match. Store them in a temporary array la_act_cod and keep track of the number of activities in each code using counter k.
	{
		k=0;
		for(j=0;j<16;j++)	// Bitwise & i to all other activities to see if there is a match. If there is a match we store the activity in question in a temporary array.
		{
			if(((i+1) & ga_bin[j])==ga_bin[j])	// Activity j is a member of i.
			{
				la_act_cod[k]=j;
				k++;
			}
		}
		// Further create and initialize gm_bin_act_cod.
		gm_bin_act_cod[i] = new unsigned short int [(k+1)];
		gm_bin_act_cod[i][0]=k;	// Record the number of activities in i.
//		printf("i: - %d :: k: %d :: code: ",i,k);
		for(j=0;j<k;j++)
		{
			gm_bin_act_cod[i][(j+1)]=la_act_cod[j];
//			printf("[%d]",la_act_cod[j]);
		}
//		printf("\n");
	}

	//////////////////////////////////////////////////////////////////////////
	///////////DESTRUCTION PHASE//////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////

	delete la_act_cod;	// Delete this array while it is no longer required.
}