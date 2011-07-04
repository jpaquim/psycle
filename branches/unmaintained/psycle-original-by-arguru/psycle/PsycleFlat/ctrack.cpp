//////////////////////////////////////////////////////////////////////
// CTrack class .cpp file for Sampler Object
//
//
//////////////////////////////////////////////////////////////////////
// Constructor & Destructor

CTrack::CTrack()
{
	tickCounter=0;
	envStage=0;
	envSustain=0;
	envCounter=0;
	r_cutoff=0;
}

//////////////////////////////////////////////////////////////////////
// Work Functions

void CTrack::GenerateZero(float *psamples, int numsamples)
{
	do{
		*psamples++=0;
	}while(--numsamples);
}

void CTrack::GetFilterEnvelopeLevel()
{
switch(fenvStage)
	{
		case 1:
		fenvValue+=fenvStep;
		
			if(fenvValue>1.0f)
			{
				fenvStage=2;
				fenvValue=1.0f;
				fenvStep=(1.0f-f_sl)/SONG->instENV_F_DT[d_inst];
			}
		break;
	
		case 2:
		fenvValue-=fenvStep;
		
			if(fenvValue<f_sl)
			{
			fenvValue=f_sl;
			fenvStage=3;
			}

		break;

		case 4:
		fenvValue-=fenvStep;
		
			if(fenvValue<0)
			{
			fenvValue=0;
			fenvStage=5;
			}
		break;

	}
}

void CTrack::GetEnvelopeLevel()
{
switch(envStage)
{
case 1:	// Attack
	envCoef+=envCoef_FR;
		if(envCoef>1.0f)
		{
		envCoef=1.0f;
		envStage=2;
		envCoef_FR=(envCoef-envSustain)/SONG->instENV_DT[d_inst];
		}
break;

case 2:	// Decay
	envCoef-=envCoef_FR;
		if(envCoef<envSustain)
		{
		envCoef=envSustain;
		envStage=3;
		envCounter=0;
		}
break;

case 4:	// Release (Note Off)
	envCoef-=envCoef_FR;
		if(envCoef<0.0f)
		{
		envCoef=0.0f;
		envStage=0;
		}
break;

case 5:	// Fast Release (Note cut)
	envCoef-=envCoef_FR;
		if(envCoef<0.0f)
		{
		envCoef=0.0f;
		envStage=0;
		}
break;
}

}

void CTrack::Generate(float *psamples, float *psamplesr,int numsamples, int interpol)
{
	if(!envStage)return;

	float left_output;
	float right_output;

	tickCounter+=numsamples;
		
	if(triggerNoteoff && tickCounter>=triggerNoteoff)
	NoteOff();

	if(envStage && SONG->Invalided)
	{
		envStage=0;
		return;
	}
	
switch(interpol)
{
// No Interpolation --------------------------------------------------
	case 0:
		do{
		left_output=0;
		right_output=0;

		if(envStage)
		{		
		left_output=*(waveDataL+readPoint.half.first);
			
		if(waveStereo)
		right_output=*(waveDataR+readPoint.half.first);
		
		// Filter section
		if(f_t<4)
		{
		GetFilterEnvelopeLevel();
		r_cutoff=f_c+f2i(fenvValue*f_module);
	
		if(r_cutoff<0)r_cutoff=0;
		if(r_cutoff>127)r_cutoff=127;

		tfcoef0=OFilter.coefs[f_t][r_cutoff][f_q][0];
		tfcoef1=OFilter.coefs[f_t][r_cutoff][f_q][1];
		tfcoef2=OFilter.coefs[f_t][r_cutoff][f_q][2];
		tfcoef3=OFilter.coefs[f_t][r_cutoff][f_q][3];
		tfcoef4=OFilter.coefs[f_t][r_cutoff][f_q][4];
		
		left_output=FilterL(left_output);

		if(waveStereo)
		right_output=FilterR(right_output);
		}

		GetEnvelopeLevel();
		
		if(waveStereo)
		right_output*=waveVolCoefR*envCoef;
		else
		right_output=left_output*waveVolCoefR*envCoef;
		
		left_output*=waveVolCoefL*envCoef;

		readPoint.full+=readSpeed;

		// Loop handler
		if(waveLO && readPoint.half.first>=waveLE)
			readPoint.half.first=waveLS;

		if(readPoint.half.first>=waveLength)envStage=0;
		}	// Envexist
		
		*psamples++=*psamples+left_output;
		*psamplesr++=*psamplesr+right_output;
		}while(--numsamples);
	break;

// Linear Interpolation -----------------------------------------------
	
	case 1:
		do{
		left_output=0;
		right_output=0;

		if(envStage)
		{		
		left_output=Resampler.WorkLinear(
			*(waveDataL+readPoint.half.first),
			*(waveDataL+readPoint.half.first+1),
			readPoint.half.last,readPoint.half.first,waveLength);

		if(waveStereo)
			right_output=Resampler.WorkLinear(
			*(waveDataR+readPoint.half.first),
			*(waveDataR+readPoint.half.first+1),
			readPoint.half.last,readPoint.half.first,waveLength);
		
		// Filter section

		if(f_t<4)
		{
		GetFilterEnvelopeLevel();
		r_cutoff=f_c+f2i(fenvValue*f_module);
	
		if(r_cutoff<0)r_cutoff=0;
		if(r_cutoff>127)r_cutoff=127;

		tfcoef0=OFilter.coefs[f_t][r_cutoff][f_q][0];
		tfcoef1=OFilter.coefs[f_t][r_cutoff][f_q][1];
		tfcoef2=OFilter.coefs[f_t][r_cutoff][f_q][2];
		tfcoef3=OFilter.coefs[f_t][r_cutoff][f_q][3];
		tfcoef4=OFilter.coefs[f_t][r_cutoff][f_q][4];
		
		left_output=FilterL(left_output);

		if(waveStereo)
		right_output=FilterR(right_output);
		}

		GetEnvelopeLevel();
		
		if(waveStereo)
		right_output*=waveVolCoefR*envCoef;
		else
		right_output=left_output*waveVolCoefR*envCoef;
		
		left_output*=waveVolCoefL*envCoef;

		readPoint.full+=readSpeed;

		// Loop handler
		if(waveLO && readPoint.half.first>=waveLE)
			readPoint.half.first=waveLS;

		if(readPoint.half.first>=waveLength)envStage=0;
		}	// Envexist
		
		*psamples++=*psamples+left_output;
		*psamplesr++=*psamplesr+right_output;
		}while(--numsamples);
	break;


// Spline Interpolation -----------------------------------------------

	case 2:
		do{
		left_output=0;
		right_output=0;

		if(envStage)
		{		
		left_output=Resampler.Work(
			*(waveDataL+readPoint.half.first-1),
			*(waveDataL+readPoint.half.first),
			*(waveDataL+readPoint.half.first+1),
			*(waveDataL+readPoint.half.first+2),readPoint.half.last,readPoint.half.first,waveLength);

		if(waveStereo)
			right_output=Resampler.Work(
			*(waveDataR+readPoint.half.first-1),
			*(waveDataR+readPoint.half.first),
			*(waveDataR+readPoint.half.first+1),
			*(waveDataR+readPoint.half.first+2),readPoint.half.last,readPoint.half.first,waveLength);
		
		// Filter section

		if(f_t<4)
		{
		GetFilterEnvelopeLevel();
		r_cutoff=f_c+f2i(fenvValue*f_module);
	
		if(r_cutoff<0)r_cutoff=0;
		if(r_cutoff>127)r_cutoff=127;

		tfcoef0=OFilter.coefs[f_t][r_cutoff][f_q][0];
		tfcoef1=OFilter.coefs[f_t][r_cutoff][f_q][1];
		tfcoef2=OFilter.coefs[f_t][r_cutoff][f_q][2];
		tfcoef3=OFilter.coefs[f_t][r_cutoff][f_q][3];
		tfcoef4=OFilter.coefs[f_t][r_cutoff][f_q][4];
		
		left_output=FilterL(left_output);

		if(waveStereo)
		right_output=FilterR(right_output);
		}

		GetEnvelopeLevel();
		
		if(waveStereo)
		right_output*=waveVolCoefR*envCoef;
		else
		right_output=left_output*waveVolCoefR*envCoef;
		
		left_output*=waveVolCoefL*envCoef;

		readPoint.full+=readSpeed;

		// Loop handler
		if(waveLO && readPoint.half.first>=waveLE)
			readPoint.half.first=waveLS;

		if(readPoint.half.first>=waveLength)envStage=0;
		}	// Envexist
		
		*psamples++=*psamples+left_output;
		*psamplesr++=*psamplesr+right_output;
		}while(--numsamples);
	break;

	}//Switch
}

//////////////////////////////////////////////////////////////////////
// Triggers Members

int CTrack::Tick(int note, int inst, int cmd, int val)
{
	if(SONG->Invalided)return 0;

	int triggered=0;
	int layer=0;
	unsigned __int64 w_offset=0;

	tickCounter=0;

	if((cmd&0xF0)==0xF0)
	triggerNoteoff=(SONG->SamplesPerTick/6)*(cmd&0xF);
	else
	triggerNoteoff=0;

	// Get Layer WaveLength
	int twlength=SONG->waveLength[inst][layer];

	if(note<120 && twlength>0)
	{
	d_inst=inst;
	d_wave=0;

	// Init Amplitude Envelope
	envStage=1;
	envCounter=0;
	envCoef_FR=1.0f/SONG->instENV_AT[d_inst];
	envCoef=0.0f;
	envSustain=(float)SONG->instENV_SL[d_inst]*0.01f;

	// Init filter synthesizer
	
	x1=0;
	x2=0;
	y1=0;
	y2=0;

	sx1=0;
	sx2=0;
	sy1=0;
	sy2=0;

	if(SONG->instRCUT[d_inst])
	f_c=alteRand(SONG->instENV_F_CO[d_inst]);
	else
	f_c=SONG->instENV_F_CO[d_inst];

	if(SONG->instRRES[d_inst])
	f_q=alteRand(SONG->instENV_F_RQ[d_inst]);
	else
	f_q=SONG->instENV_F_RQ[d_inst];

	f_t= SONG->instENV_F_TP[d_inst];
	f_module= (float)SONG->instENV_F_EA[d_inst];
	f_sl=(float)SONG->instENV_F_SL[d_inst]*0.0078125f;
		
	fenvStage=1;
	fenvStep=1.0f/SONG->instENV_F_AT[d_inst];
	fenvValue=0;
	
	// Init Wave
	waveDataL=SONG->waveDataL[inst][layer];
	waveDataR=SONG->waveDataR[inst][layer];
	waveLength=twlength;
	
	// Init loop
	if(SONG->waveLoopType[inst][layer])
	{
		waveLO=true;
		waveLS=SONG->waveLoopStart[inst][layer];
		waveLE=SONG->waveLoopEnd[inst][layer];
	
	}
	else
	{
		waveLO=false;
	}
	
	// Calculating volume coef ---------------------------------------
	
	waveVolumeCoef=(float)SONG->waveVolume[inst][layer]*0.01f;
	
	if(cmd==0x0c)
	waveVolumeCoef*=(float)val*0.00390625f;

	// Panning calculation -------------------------------------------

	float panFactor;

	if(SONG->instRPAN[inst])
	panFactor=(float)rand()*0.000030517578125f;
	else
	panFactor=(float)SONG->instPAN[inst]*0.0039062f;

	waveVolCoefR=panFactor;
	waveVolCoefL=1-panFactor;
	
	if(waveVolCoefR>0.5f)waveVolCoefR=0.5f;
	if(waveVolCoefL>0.5f)waveVolCoefL=0.5f;

	waveVolCoefR*=waveVolumeCoef;
	waveVolCoefL*=waveVolumeCoef;

	waveStereo=SONG->waveStereo[inst][layer];

	// Handle wave_start_offset cmd

	if(cmd==9)
	{
	w_offset=val*waveLength;
	readPoint.full=(w_offset<<24);
	}
	else
	readPoint.full=0;

	// Init Resampler
	
	if(SONG->instLoop[d_inst])
	{
	double const totalsamples=double(SONG->SamplesPerTick*SONG->instLines[d_inst]);
	
	readSpeed=(waveLength/totalsamples)*4294967296.0f;
	}	
	else
	{
	note-=48;
	float const finetune=SONG->waveFinetune[inst][layer]*0.00390625f;
	readSpeed=pow(2.0f,(note+finetune)/12.0f)*4294967296.0f;
	}

	triggered=1;
	}

	return triggered;
}

void CTrack::NoteOff()
{
	if(envStage)
	{
	envStage=4;
	envCounter=0;
	envCoef_FR=envCoef/SONG->instENV_RT[d_inst];

	fenvStage=4;
	fenvStep=fenvValue/SONG->instENV_F_RT[d_inst];
	}
}

void CTrack::NoteOffFast()
{
	if(envStage)
	{
	envStage=5;
	envCounter=0;
	envCoef_FR=envCoef/OVERLAPTIME;
	
	fenvStage=4;
	fenvStep=fenvValue/OVERLAPTIME;
	}
}

int CTrack::alteRand(int x)
{
return (x*rand())/32768;
}

//////////////////////////////////////////////////////////////////////
// Spline Line resampler members

Cubic::Cubic() /* The resampler constructor */
{
RESOLUTION=2048;
// Initialize table...
for (int i=0;i<RESOLUTION;i++)
{
    float x = (float)i/(float)RESOLUTION;
    at[i] = float(-0.5*x*x*x+x*x-0.5*x);
    bt[i] = float(1.5*x*x*x-2.5*x*x+1);
    ct[i] = float(-1.5*x*x*x+2*x*x+0.5*x);
    dt[i] = float(0.5*x*x*x-0.5*x*x);
	lt[i] = (float)i/(float)RESOLUTION;
}

}
