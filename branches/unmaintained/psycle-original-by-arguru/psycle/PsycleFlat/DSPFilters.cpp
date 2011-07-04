//////////////////////////////////////////////////////////////////////
// Filters implementation
 
///////////////////////////////////////////////////////////////////////
// 2p Filter work

void psyGear::WorkFilter(float *pSamplesL,float *pSamplesR,int numSamples)
{
	float co1=f_cutoff+(float)sin(lfoDegree)*f_lfoamp;
	float co2=f_cutoff+(float)sin(lfoDegree+f_lfophase)*f_lfoamp;

	// Init filter intermediate vars
	if(co1<0.01f)co1=0.01f;
	if(co1>0.99f)co1=0.99f;
	float fa1 = float(1.0 - co1); 
	float fb1 = float(f_resonance * (1.0 + (1.0/fa1)));

	if(co2<0.01f)co2=0.01f;
	if(co2>0.99f)co2=0.99f;
	float fa2 = float(1.0 - co2); 
	float fb2 = float(f_resonance * (1.0 + (1.0/fa2)));

	--pSamplesL;
	--pSamplesR;

	do
	{
		*++pSamplesL=Filter2p_l(*pSamplesL,co1,f_resonance,fa1,fb1);
		*++pSamplesR=Filter2p_r(*pSamplesR,co2,f_resonance,fa2,fb2);
		
		lfoDegree+=f_lfospeed;
		if(lfoDegree>DPI)lfoDegree-=DPI;
	
	}while(--numSamples);
}

float psyGear::Filter2p_l(float input,float f,float q,float fa,float fb)
{
  input+=1.0f;

  f_bufl0 = fa * f_bufl0 + f * (input + fb * (f_bufl0 - f_bufl1)); 
  f_bufl1 = fa * f_bufl1 + f * f_bufl0;

  switch(filterMode)
  {
	case 0:
		return f_bufl1; 
	break;

	case 1:
		return input-f_bufl1; 
	break;
  }
	
  return 0;
}

float psyGear::Filter2p_r(float input,float f,float q,float fa,float fb)
{
  input+=1.0f;

  f_bufr0 = fa * f_bufr0 + f * (input + fb * (f_bufr0 - f_bufr1));
  f_bufr1 = fa * f_bufr1 + f * f_bufr0;
  
  switch(filterMode)
  {
	case 0:
		return f_bufr1; 
	break;

	case 1:
		return input-f_bufr1; 
	break;
  }

  return 0;
}

void psyGear::UpdateFilter()
{
f_cutoff=filterCutoff*0.00390625f;
f_resonance=filterResonance*0.00390625f;
f_lfospeed=filterLfospeed*0.00000003f;
f_lfoamp=filterLfoamp*0.00390625f;
f_lfophase=filterLfophase*0.0122718f;
}
