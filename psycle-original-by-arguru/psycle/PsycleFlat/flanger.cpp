//////////////////////////////////////////////////////////////////////
// Flanger code

void psyGear::WorkFlanger(float *pSamplesL,float *pSamplesR,int numSamples)
{
	float left_input;
	float right_input;
	float fdbkL=(float)delayFeedbackL*0.01f;
	float fdbkR=(float)delayFeedbackR*0.01f;
	float dry=(float)outDry*0.0039062f;
	float wet=(float)outWet*0.0039062f;
	
	float acc_timel=delayTimeL+sin(lfoDegree)*f_lfoamp;
	float acc_timer=delayTimeL+sin(lfoDegree+f_lfophase)*f_lfoamp;
	
	int dtimel=(int)acc_timel;
	int dtimer=(int)acc_timer;

	int const MTH=MAX_DELAY_BUFFER-1;

	--pSamplesL;
	--pSamplesR;

	do{
	left_input=*++pSamplesL;
	right_input=*++pSamplesR;

	left_input++;
	right_input++;
	
	if(++delayCounterL>=MAX_DELAY_BUFFER)delayCounterL=0;
	
	delayedCounterL=delayCounterL-dtimel;
	delayedCounterR=delayCounterL-dtimer;

	if(delayedCounterL<0)delayedCounterL=MTH+delayedCounterL;
	if(delayedCounterR<0)delayedCounterR=MTH+delayedCounterR;

	float const y_l=delayBufferL[delayedCounterL];
	float const y_r=delayBufferR[delayedCounterR];

	delayBufferL[delayCounterL]=left_input+y_l*fdbkL;
	delayBufferR[delayCounterL]=right_input+y_r*fdbkR;

	*pSamplesL=left_input*dry+y_l*wet;
	*pSamplesR=right_input*dry+y_r*wet;

	lfoDegree+=f_lfospeed;
	if(lfoDegree>DPI)lfoDegree-=DPI;

	}while(--numSamples);
}

void psyGear::UpdateFlanger()
{
	f_lfospeed=(float)filterLfospeed*0.000000003f;
	f_lfoamp=(float)filterLfoamp*0.00390625f*float(delayTimeL-1);
	f_lfophase=(float)filterLfophase*0.0122718f;
}