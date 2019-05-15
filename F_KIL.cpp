void f_kil(void)
{
	unsigned short int	i;
	unsigned short int	j;
	unsigned short int	k;

	for(k=0;k<gv_bnr;k++)
	{
		for(i=0;i<gv_n;i++)
		{
			delete gm_str_ptg[k][i];
		}
		delete gm_str_ptg[k];
	}
	delete gm_str_ptg;

	for(i=0;i<gv_n;i++)
	{
		for(j=0;j<ga_ptg[i];j++)
		{
			delete gm_lnk_ptg[i][j];
			delete gm_dpt_ptg[i][j];
			delete gm_nio_ptg[i][j];
		}
		delete gm_lnk_ptg[i];
		delete gm_dpt_ptg[i];
		delete gm_nio_ptg[i];
		delete gm_ptg_nrc[i];
	}
	delete gm_lnk_ptg;
	delete gm_dpt_ptg;
	delete gm_nio_ptg;
	delete gm_ptg_nrc;

	for(i=0;i<gv_n;i++)
	{
		delete gm_pre_rel[i];
		delete gm_tra_rel[i];
	}
	delete gm_pre_rel;
	delete gm_tra_rel;

	delete ga_rac;
	delete ga_act_dur;
	delete ga_lambda;
	delete ga_nrs;
	delete ga_ptg;
}