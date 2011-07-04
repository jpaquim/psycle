//////////////////////////////////////////////////////////////////////
// Constructor/Destructor
unsigned cpuhz=0;

psyGear::psyGear()
{
	// Create negotiation-sample-data [streamrouting] for each machine ----
	
	VolumeCounter=0;
	
	samplesLeft=new float[256];
	samplesRight=new float[256];
	
	// Clear machine buffer samples
	for (int c=0;c<256;c++)
	{
		*(samplesRight+c)=0;
		*(samplesLeft+c)=0;
	}
	
	
}

psyGear::psyInit()
{
	buzzdll=-1;
	buzzpar=0;
	
	delay_alloc=false;
	
	LMAX=0;
	RMAX=0;
	lmax=0;
	rmax=0;
	interpol=1;
	
	f_bufl0=0;
	f_bufl1=0;
	f_bufr0=0;
	f_bufr1=0;
	
	// Standard gear initalization [constructor] -------------------------
	
	lfoDegree=0;
	outDry=256;
	outWet=64;
	
	// Dalay Delay -------------------------------------------------------
	
	cpuCost=0;
	
	// Distort -----------------------------------------------------------
	
	distPosThreshold=128;
	distPosClamp=128;
	distNegThreshold=128;
	distNegClamp=128;
	
	// Sine --------------------------------------------------------------
	
	sineSpeed=1.0f;
	sineVolume=1.0f;
	sineOsc=0.0f;
	sineLfoosc=0.0f;
	
	sineRealspeed=1.0f;
	sineGlide=0.00001f;
	sineLfospeed=0.01f;
	sineLfoamp=1.0f;
	
	sinespeed=50;
	sinevolume=128;
	sineglide=50;
	sinelfospeed=50;
	sinelfoamp=50;
	
	filterCutoff=128;
	filterResonance=128;
	filterLfospeed=0;
	filterLfoamp=0;
	filterLfophase=0;
	filterMode=0;	
	
	for(int c=0;c<MAX_TRACKS;c++)
	{
		SubTrack[c]=0;
		currSubTrack[c]=0;
	}
	
	sineTick();
	UpdateDelay(11050,11050,50,50);
	UpdateFilter();
	
}

psyGear::~psyGear()
{
	delete samplesLeft;
	delete samplesRight;
	
	if(delay_alloc)
	{
		delete delayBufferL;
		delete delayBufferR;
		delay_alloc=false;
	}
}

//////////////////////////////////////////////////////////////////////
// Create machine member

bool psySong::CreateMachine(int machineType,int xl,int yl,int omac,int ndll)
{
	GETINFO bfxGetInfo =(GETINFO)GetProcAddress(hBfxDll[ndll],"GetInfo");
	CREATEMACHINE bfxGetMI =(CREATEMACHINE)GetProcAddress(hBfxDll[ndll],"CreateMachine");
	
	if((!bfxGetInfo || !bfxGetMI) && machineType==8)return false;
	
	if(machineType<0)return false;
	
	int tmac;
	
	if(omac==-1)
		tmac=GetFreeMachine();
	else
		tmac=omac;
	
	if (tmac>MAX_MACHINES-1)return false;
	
	machine[tmac]=new psyGear;
	machine[tmac]->psyInit();
	
	machine[tmac]->numSubTracks=2;
	
	machine[tmac]->machineMode=machineType;
	machine[tmac]->v=0;
	
	// Centering volume and panning
	machine[tmac]->panning=64;
	machine[tmac]->changePan(64);
	
	machine[tmac]->delay_alloc=false;
	
	// Clearing connections
	for (int c=0;c<MAX_CONNECTIONS;c++)
	{
		machine[tmac]->connectionVol[c]=1.0f;
		machine[tmac]->conection[c]=false;
		machine[tmac]->inCon[c]=false;
	}
	machine[tmac]->numInputs=0;
	machine[tmac]->numOutputs=0;
	machine[tmac]->x=xl;
	machine[tmac]->y=yl;
	
	
	switch(machine[tmac]->machineMode)
	{
	case 0:
		machine[tmac]->type=2;
		sprintf(machine[tmac]->editName,"Master");
		break;
		
	case 1:
		machine[tmac]->type=1;
		sprintf(machine[tmac]->editName,"Psychosc");
		break;
		
	case 2:
		machine[tmac]->type=1;
		sprintf(machine[tmac]->editName,"Distortion");
		break;
		
	case 3:
		machine[tmac]->type=0;
		machine[tmac]->interpol=1;
		sprintf(machine[tmac]->editName,"Sampler");
		break;
		
	case 4:
		machine[tmac]->type=1;
		sprintf(machine[tmac]->editName,"Delay");
		machine[tmac]->delay_alloc=true;
		break;
		
	case 5:
		machine[tmac]->type=1;
		sprintf(machine[tmac]->editName,"2p Filter");
		break;
		
	case 6:
		machine[tmac]->type=1;
		sprintf(machine[tmac]->editName,"Gainer");
		break;
		
	case 7:
		machine[tmac]->type=1;
		sprintf(machine[tmac]->editName,"Flanger");
		machine[tmac]->delay_alloc=true;
		
		machine[tmac]->SetPreset(7,0);
		
		//f_lfospeed=0.00001f;
		break;
		
	case 8:
		machine[tmac]->type=3;
		
		machine[tmac]->buzzdll=ndll;
		sprintf(machine[tmac]->editName,bfxGetInfo()->ShortName);
		
		machine[tmac]->mi=bfxGetMI();
		machine[tmac]->mi->pCB=&micb;
		
		machine[tmac]->BuzzInit(bfxGetInfo());
		
		if(bfxGetInfo()->Flags==3)
			machine[tmac]->type=0;
		
		break;
		
	case 9:
		machine[tmac]->type=0;
		sprintf(machine[tmac]->editName,"VST2 Instr.");
		machine[tmac]->ovst.pVST=&m_Vst;
		break;
		
	case 10:
		machine[tmac]->type=3;
		sprintf(machine[tmac]->editName,"VST2 Fx");
		machine[tmac]->ovst.pVST=&m_Vst;
		break;
		
	case 255:
		machine[tmac]->type=1;
		sprintf(machine[tmac]->editName,"Dummy");
		break;
		
	}
	
	if(machine[tmac]->delay_alloc)
		machine[tmac]->AllocateDelayBuffer();
	
	lbc=tmac;
	
	// Finally, activating machine
	Activemachine[tmac]=true;
	
	return true;
}

//////////////////////////////////////////////////////////////////////
// Machine operation members

psyGear::dllTweakAll(int numPars)
{
	for(int c=0;c<numPars;c++)
		mi->ParameterTweak(c,mi->Vals[c]);
}

int psyGear::GetFreeSubTrack(int trk)
{
	for(int c=0;c<numSubTracks;c++)
	{
		if(trackObj[trk][c].envStage==0)return c; 
	}
	return 0;
}

void psyGear::IsolateSubTracks(int trk,int strk,int pinst)
{
	// New Note Action Handler
	for(int nnac=0;nnac<numSubTracks;nnac++)
	{
		if(nnac!=strk)
		{
			switch(SONG->instNNA[pinst])
			{
			case 0: trackObj[trk][nnac].NoteOffFast();break;	// NoteCut[FastNoteRelease]
			case 1: trackObj[trk][nnac].NoteOff();break;		// NoteOff[Release]
			}
		}
	}
}

void psyGear::changePan(int newpan)
{
	if (newpan<0)newpan=0;
	if (newpan>128)newpan=128;
	rVol=newpan*0.015625f;
	lVol=2.0f-rVol;
	if(lVol>1.0f)lVol=1.0f;
	if(rVol>1.0f)rVol=1.0f;
	panning=newpan;
}

//////////////////////////////////////////////////////////////////////
// Main Tick function

void psyGear::TickFx()
{
	if(machineMode==8)
		mi->SequencerTick();
}

//////////////////////////////////////////////////////////////////////
// Main Working function

int psyGear::DSPVol(float *pSamplesL,float *pSamplesR,int numSamples)
{
	--pSamplesL;
	--pSamplesR;
	
	float vol=0.0f;
	do
	{
		float const vl=*++pSamplesL;
		float const vr=*++pSamplesR;
		
		if(vl>vol)vol=vl;
		if(vr>vol)vol=vr;
		
	}while(--numSamples);
	
	return (int)vol;
}

void psyGear::Work(float *pSamplesL, float *pSamplesR,int numSamples,int numtracks)
{
	unsigned cputime=0;
	
	__asm	rdtsc				// Read time stamp to EAX
		__asm	mov		cputime, eax
		
		switch (machineMode)
	{
		// Master
	case 0:
		MasterWork(pSamplesL,numSamples);
		break;
		
		// Sine Test
	case 1:
		WorkTestSine(pSamplesL,pSamplesR,numSamples);
		break;
		
		// Flat Distortion
	case 2:
		WorkDistortion(pSamplesL,numSamples);
		WorkDistortion(pSamplesR,numSamples);
		break;		
		
		// Generators works
	case 3:
		WorkTrackerBusMono(pSamplesL, pSamplesR, numSamples,numtracks);
		break;
		
	case 4:
		WorkDelay(pSamplesL, pSamplesR, numSamples);
		break;
		
	case 5:
		WorkFilter(pSamplesL, pSamplesR, numSamples);
		break;
		
	case 6:
		WorkGainer(pSamplesL, pSamplesR, numSamples);
		break;
		
	case 7:
		WorkFlanger(pSamplesL, pSamplesR, numSamples);
		break;
		
	case 8:
		mi->Work(pSamplesL,pSamplesR,numSamples,numtracks);	
		break;
		
	case 9:
		// VST Instrument
		ovst.Work(pSamplesL,pSamplesR,numSamples,numtracks,false);	
		break;
		
	case 10:
		// VST Fx
		ovst.Work(pSamplesL,pSamplesR,numSamples,numtracks,true);	
		break;
		
		
	}
	
	if(machineMode!=0)
	{
		int nv=DSPVol(pSamplesL,pSamplesR,numSamples);
		if(nv>VolumeCounter)VolumeCounter=nv;
	}
	
	VolumeCounter-=numSamples;
	if(VolumeCounter<0)VolumeCounter=0;
	
	__asm	rdtsc				
		__asm	sub		eax, cputime	// Find the difference
		__asm	mov		cputime, eax
		
		unsigned cpudspz=numSamples*(cpuhz/44100);
	cpuCost=(cputime*1000)/cpudspz;
}

#include "flanger.cpp"

//////////////////////////////////////////////////////////////////////
// Gainer

void psyGear::WorkGainer(float *pSamplesL,float *pSamplesR,int numSamples)
{
	float const wet=(float)outWet*0.0039062f;
	
	--pSamplesL;
	--pSamplesR;
	do{
		*++pSamplesL=*pSamplesL*wet;
		*++pSamplesR=*pSamplesR*wet;
	}while(--numSamples);
}

//////////////////////////////////////////////////////////////////////
// Master Members

void psyGear::MasterWork(float *pSamples,int numSamples)
{
	float const mv=(float)outDry*0.0039062f;
	
	float *masterSampleDataLeft=samplesLeft;
	float *masterSampleDataRight=samplesRight;
	--pSamples;
	--masterSampleDataLeft;
	--masterSampleDataRight;
	
	do
	{
		++pSamples;
		++masterSampleDataLeft;
		++masterSampleDataRight;
		
		*masterSampleDataLeft*=mv;
		*masterSampleDataRight*=mv;
		
		// Left channel
		*pSamples=*masterSampleDataLeft;
		if(*masterSampleDataLeft>lmax)lmax=*masterSampleDataLeft;
		
		// Right channel
		++pSamples;
		*pSamples=*masterSampleDataRight;
		if(*masterSampleDataRight>rmax)rmax=*masterSampleDataRight;
		
		if(lmax>32768)
		{
			clip=true;
			lmax=32768;
		}
		
		if(rmax>32768)
		{
			clip=true;
			rmax=32768;
		}
		
		if(lmax>8)lmax-=8;
		if(rmax>8)rmax-=8;
		
	}while(--numSamples);
	
	LMAX=f2i(lmax);
	RMAX=f2i(rmax);
}

//////////////////////////////////////////////////////////////////////
// PsychOsc Members

void psyGear::sineTick(void)
{
	sineSpeed=(float)sinespeed*0.0025f;
	sineGlide=(float)sineglide*0.000005f;
	sineLfospeed=sinelfospeed*0.000025f;
	sineLfoamp=(float)sinelfoamp*0.0005f;	
}

void psyGear::WorkTestSine(float *pSamplesL,float *pSamplesR,int numSamples)
{
	float const rad=6.28318530717958647692528676655901f;
	float mod_value;
	
	--pSamplesL;
	--pSamplesR;
	
	do{
		mod_value=(float)sin(sineOsc)*sineRealspeed;
		
		*++pSamplesL=mod_value**pSamplesL;
		*++pSamplesR=mod_value**pSamplesR;
		
		sineOsc+=sineRealspeed+(float)sin(sineLfoosc)*sineLfoamp;
		
		sineLfoosc+=sineLfospeed;
		if(sineLfoosc>rad)sineLfoosc-=rad;
		
		if(sineRealspeed>sineSpeed)
		{
			sineRealspeed-=sineGlide;
			if(sineRealspeed<sineSpeed)sineRealspeed=sineSpeed;
		}
		
		if(sineRealspeed<sineSpeed)
		{
			sineRealspeed+=sineGlide;
			if(sineRealspeed>sineSpeed)sineRealspeed=sineSpeed;
		}
		
		if(sineOsc>rad)sineOsc-=rad;
	}while(--numSamples);
}

//////////////////////////////////////////////////////////////////////
// PsychOsc Members

void psyGear::WorkDistortion(float *pSamples,int numSamples)
{
	float dpt=distPosThreshold*128.0f;
	float dpc=distPosClamp*128.0f;
	float dnt=-distNegThreshold*128.0f;
	float dnc=-distNegClamp*128.0f;
	float in;
	
	--pSamples;
	
	do{
		in=*++pSamples;
		if(in>dpt)in=dpc;
		if(in<dnt)in=dnc;
		*pSamples=in;
	}while(--numSamples);
}

void psyGear::WorkTrackerBusMono(float *pSamples,float *pSamplesR,int numSamples,int tracks)
{
	for(int trk=0;trk<tracks;trk++)
	{
		for(int strk=0;strk<numSubTracks;strk++)
			trackObj[trk][strk].Generate(pSamples,pSamplesR,numSamples,interpol);
	}
}

void psyGear::UpdateDelay(int lt, int rt, int fdbckl, int fdbckr)
{
	if(lt!=NULL && lt!=delayTimeL)
	{
		delayTimeL=lt;
		delayCounterL=MAX_DELAY_BUFFER-1;
		delayedCounterL=delayCounterL-delayTimeL;
		
		if(delayedCounterL<0)delayedCounterL=0;
	}
	
	if(rt!=NULL  && rt!=delayTimeR)
	{
		delayTimeR=rt;
		delayCounterR=MAX_DELAY_BUFFER-1;
		delayedCounterR=delayCounterR-delayTimeR;
		
		if(delayedCounterR<0)delayedCounterR=0;
	}
	
	if(fdbckl!=0)
		delayFeedbackL=fdbckl;
	
	if(fdbckr!=0)
		delayFeedbackR=fdbckr;	
}

void psyGear::WorkDelay(float *pSamplesL,float *pSamplesR,int numSamples)
{
	float left_input;
	float right_input;
	
	float fdbkL=(float)delayFeedbackL*0.01f;
	float fdbkR=(float)delayFeedbackR*0.01f;
	float dry=(float)outDry*0.0039062f;
	float wet=(float)outWet*0.0039062f;
	
	--pSamplesL;
	--pSamplesR;
	
	do{
		left_input=*++pSamplesL;
		right_input=*++pSamplesR;
		
		left_input++;
		right_input++;
		
		if(++delayCounterL>=MAX_DELAY_BUFFER)delayCounterL=0;
		if(++delayCounterR>=MAX_DELAY_BUFFER)delayCounterR=0;
		
		if(++delayedCounterL>=MAX_DELAY_BUFFER)delayedCounterL=0;
		if(++delayedCounterR>=MAX_DELAY_BUFFER)delayedCounterR=0;
		
		delayBufferL[delayCounterL]=left_input+delayBufferL[delayedCounterL]*fdbkL;
		delayBufferR[delayCounterR]=right_input+delayBufferR[delayedCounterR]*fdbkR;
		
		*pSamplesL=left_input*dry+delayBufferL[delayedCounterL]*wet;
		*pSamplesR=right_input*dry+delayBufferR[delayedCounterR]*wet;
		
	}while(--numSamples);
}

//////////////////////////////////////////////////////////////////////
// Psysong member functions source code

psySong::psySong()
{
	Invalided=false;
	Tweaker=false;
	m_WaveStage=0;
	PW_Phase=0;
	PW_Stage=0;
	PW_Length=0;
	
	cpuhz=0;
	
	__asm	rdtsc				// Read time stamp to EAX
		__asm	mov		cpuhz, eax
		
		Sleep(1000);
	
	__asm	rdtsc				
		__asm	sub		eax,cpuhz	// Find the difference
		__asm	mov		cpuhz, eax
		
		CPUHZ=cpuhz;
	
	Reset();
}

psySong::~psySong()
{
	DestroyAllMachines();
	DeleteAllPatterns();
	DeleteInstruments();
	DestroyBuzzfx();
}

void psySong::DestroyAllMachines()
{
	for(int c=0;c<MAX_MACHINES;c++)
	{
		if(Activemachine[c])
			DestroyMachine(c);
	}
}

//////////////////////////////////////////////////////////////////////
// Seek and destroy allocated instruments

void psySong::DeleteLayer(int i,int c)
{
	sprintf(waveName[i][c],"empty");
	
	if(waveLength[i][c]>0)
	{
		delete waveDataL[i][c];
		if(waveStereo[i][c])
			delete waveDataR[i][c];
		waveLength[i][c]=0;
	}
	
	waveStereo[i][c]=false;
	waveLoopStart[i][c]=0;
	waveLoopEnd[i][c]=0;
	waveLoopType[i][c]=0;
	waveVolume[i][c]=100;
	waveFinetune[i][c]=0;
}

void psySong::DeleteInstruments()
{
	for(int i=0;i<MAX_INSTRUMENTS;i++)DeleteInstrument(i);
}

void psySong::DeleteInstrument(int i)
{
	// Reset envelope
	instENV_AT[i]=16;
	instENV_DT[i]=16384;
	instENV_SL[i]=64;
	instENV_RT[i]=OVERLAPTIME;
	
	instENV_F_AT[i]=16;
	instENV_F_DT[i]=16384;
	instENV_F_SL[i]=64;
	instENV_F_RT[i]=16384;
	
	instENV_F_CO[i]=64;
	instENV_F_RQ[i]=64;
	instENV_F_EA[i]=128;
	instENV_F_TP[i]=4;
	
	instLoop[i]=false;
	instLines[i]=16;
	
	instNNA[i]=0; // NNA set to Note Cut [Fast Release]
	
	instPAN[i]=128;
	instRPAN[i]=false;
	instRCUT[i]=false;
	instRRES[i]=false;
	
	for(int c=0;c<MAX_WAVES;c++)
		DeleteLayer(i,c);
	
	sprintf(instName[i],"empty");
}

//////////////////////////////////////////////////////////////////////
// Song Reset function

void psySong::Reset(void)
{
	cpuIdle=0;
	LastPatternLines=16;
	sprintf(LastPatternName,"Untitled");
	
	// Cleaning pattern allocation info
	for(int i=0;i<MAX_INSTRUMENTS;i++){for(int c=0;c<MAX_WAVES;c++){waveLength[i][c]=0;}}
	for (int c=0;c<256;c++)Activemachine[c]=false; // All machines reset
	for (c=0;c<MAX_PATTERNS;c++)
	{
		// All pattern reset
		patternLines[c]=64;
		sprintf(patternName[c],"Untitled"); 
	}
	
	for(c=0;c<MAX_TRACKS;c++)
		track_st[c]=true;
	
	playLength=1;
	for (c=0;c<MAX_SONG_POSITIONS;c++)playOrder[c]=false; // All pattern reset
	
}

//////////////////////////////////////////////////////////////////////
// PsySong NEWSONG

void psySong::newSong()
{
	
	for(int c=0;c<MAX_BUSES;c++)
		busMachine[c]=255;
	
	// psySong reset
	sprintf(Name,"Untitled");
	sprintf(Author,"Unnamed");
	sprintf(Comment,"No Comments");
	
	currentOctave=4;
	
	// General properties
	SetBPM(125,44100);
	LineCounter=0;
	
	LineChanged=false;
	
	PlayMode=0;
	
	// Clean up allocated machines.
	DestroyAllMachines();
	
	// Cleaning instruments
	DeleteInstruments();
	
	// Clear patterns
	DeleteAllPatterns();
	
	// Clear sequence
	
	waveSelected=0;
	instSelected=1;
	editPosition=0;
	playPosition=0;
	
	Reset();
	m_Vst.FreeAllVstInstances();
	CreateMachine(0,320,200,0,0);
}

int psySong::GetFreeMachine(void)
{
	int c=-1;
	bool doloop=true;
	
	do{
		if(!Activemachine[++c])doloop=false;
	}while(doloop);
	return c;
}

bool psySong::InsertConnection(int src,int dst)
{
	int freebus=-1;
	int dfreebus=-1;
	
	bool error=false;
	
	psyGear *srcMac=machine[src];
	psyGear *dstMac=machine[dst];
	
	if (dstMac->type==0)return false;
	
	// Get a free output slot on the source machine
	for(int c=MAX_CONNECTIONS-1;c>=0;c--)
	{
		if(!srcMac->conection[c])freebus=c;
	}
	if(freebus==-1)return false;
	
	// Get a free input slot on the destination machine
	for(c=0;c<MAX_CONNECTIONS;c++)
	{
		if(!dstMac->inCon[c])dfreebus=c;
	}
	if(dfreebus==-1)return false;
	
	// Checking if already exist an existing bus...
	for(c=0;c<MAX_CONNECTIONS;c++)
	{
		if(srcMac->conection[c])
		{
			if(srcMac->outputDest[c]==dst)
				error=true;
		}
	}
	
	// Checking if the destination machine have existing bus...
	for(c=0;c<MAX_CONNECTIONS;c++)
	{
		if(dstMac->conection[c])
		{
			if(dstMac->outputDest[c]==src)
				error=true;
		}
	}
	
	if(error)return false;
	
	// Calibrating in/out properties
	srcMac->outputDest[freebus]=dst;
	srcMac->conection[freebus]=true;
	srcMac->numOutputs++;
	
	dstMac->inputSource[dfreebus]=src;
	dstMac->inCon[dfreebus]=true;
	dstMac->numInputs++;
	
	//srcMac->panflag=true;
	
	return true;
}

//////////////////////////////////////////////////////////////////////
// Machine Deletion Function

void psySong::DestroyMachine(int mac)
{
	// Delete and destroy the MACHINE!
	// Lets search machines with connection to this one...
	CSingleLock lock(&door,TRUE);
	for (int c=0;c<MAX_MACHINES;c++)
	{
		if(Activemachine[c])
		{
			psyGear *iMac=machine[c];
			
			// Active machine found
			for(int w=0;w<MAX_CONNECTIONS;w++)
			{
				// Checking In-Wires
				if(iMac->inCon[w])
				{
					if(iMac->inputSource[w]==mac)
					{
						iMac->inCon[w]=false;
						iMac->numInputs--;
					}
				}
				
				// Checking Out-Wires
				if(iMac->conection[w])
				{
					if(iMac->outputDest[w]==mac)
					{
						iMac->conection[w]=false;
						iMac->numOutputs--;
					}
				}
			}
		}
	}
	
	
	// Destroying buzz plugin data
	if(machine[mac]->machineMode==8)
		delete machine[mac]->mi;
	
	// Clear if it's a generator bus
	
	if(machine[mac]->type==0)
	{
		int mbi=-1;
		
		for(int c=0;c<MAX_BUSES;c++)
		{
			if(busMachine[c]==mac)
				busMachine[c]=255;
		}
	}
	
	Activemachine[mac]=false;
	delete machine[mac];
	
}

void psySong::DeleteAllPatterns(void)
{
	SONGTRACKS=16;
	
	for(int c=0;c<MAX_PATTERN_BUFFER_LEN;c+=5)
	{
		pPatternData[c]=255;
		pPatternData[c+1]=255;
		pPatternData[c+2]=255;
		pPatternData[c+3]=0;
		pPatternData[c+4]=0;
	}
}

bool psySong::AllocNewPattern(int pattern,char *name,int lines)
{
	patternLines[pattern]=lines;
	sprintf(patternName[pattern],name);
	return true;
}

void psySong::SetBPM(int bpm,int srate)
{
	BeatsPerMin=bpm;
	SamplesPerTick=(srate*15)/bpm;
	micb.cbSamplesPerTick=SamplesPerTick;
	micb.cbBeatsPerMin=BeatsPerMin;
	micb.cbSamplesPerSec=srate;
}

//////////////////////////////////////////////////////////////////////
// Auxiliar miscellaneous data

void GetMachineName(char *buf,int mac)
{
	switch(mac)
	{
	case 0:sprintf(buf,"Master");break;
	case 1:sprintf(buf,"PsychOsc AM");break;
	case 2:sprintf(buf,"Distortion");break;
	case 3:sprintf(buf,"Sampler");break;
	case 4:sprintf(buf,"Dalay Delay");break;
	case 5:sprintf(buf,"2p Filter");break;
	case 6:sprintf(buf,"Gainer");break;	
	case 7:sprintf(buf,"Flanger");break;
	case 8:sprintf(buf,"Plug-In");break;
	case 9:sprintf(buf,"VST Instrument");break;
	case 10:sprintf(buf,"VST Effect");break;
		
	case 255:sprintf(buf,"DummyPlug");break;
	}
}

void psySong::Play(int line)
{
	Stop();
	LineChanged=true;
	LineCounter=line;
	playPosition=editPosition;
	playPattern=playOrder[playPosition];
	TicksRemaining=SamplesPerTick;
	PlayMode=1;
}

void psySong::Stop()
{
	// Stop song enviroment
	PlayMode=0;
	
	// Stop replay of buses
	for(int sb=0;sb<MAX_BUSES;sb++)
	{
		int mgn=busMachine[sb];
		
		if(mgn!=255)
		{
			int maxpoly=machine[mgn]->numSubTracks;
			
			if(machine[mgn]->machineMode==9)
				machine[mgn]->ovst.Stop(); // <--Call VST INSTRUMENT STOP member
			
			if(machine[mgn]->machineMode==8)
				machine[mgn]->mi->Stop(); // <--Call DLL STOP member
			
			for(int trk=0;trk<SONGTRACKS;trk++)
			{
				for(int strk=0;strk<maxpoly;strk++)
					machine[mgn]->trackObj[trk][strk].envStage=0;
			}
		}
	}
}

#include "DSPfilters.cpp"

void psyGear::DoPatternEffects(int ins,int cmd,int val,int trk)
{
	for(int t=0;t<numSubTracks;t++)
		trackObj[trk][t].Tick(255,ins,cmd,val);
	
	switch(cmd)
	{
	case 0x08:
		// CutOff frequency
		if(ins>0 && ins<MAX_INSTRUMENTS)
			SONG->instENV_F_CO[ins]=val>>1;
		break;
	}
}

void psyGear::AllocateDelayBuffer()
{
	delayBufferL=new float[MAX_DELAY_BUFFER];
	delayBufferR=new float[MAX_DELAY_BUFFER];
	
	for(int c=0;c<MAX_DELAY_BUFFER;c++)
	{
		delayBufferL[c]=0;
		delayBufferR[c]=0;
	}
}

void psySong::DestroyBuzzfx(void)
{
	for(int c=0;c<nBfxDll;c++)
		FreeLibrary(hBfxDll[c]);
}

void psySong::ReadBuzzfx(char *path)
{
	m_Vst.ReadDlls("Vst");
	nBfxDll=0;
	
	int list_counter=0;
	
	_chdir(path);
	
	struct _finddata_t c_file;    
	long hFile;
	
	if( (hFile = _findfirst( "*.dll", &c_file )) == -1L )
    {
		// No files
	}
	else   
	{
		// The first file (files)
		if (!(c_file.attrib&_A_SUBDIR))
		{
			hBfxDll[list_counter]=LoadLibrary(c_file.name);
			sprintf(BfxName[list_counter],c_file.name);
			
			list_counter++;
		}
		
		/* Find the rest of the files (files)*/
		while( _findnext( hFile, &c_file ) == 0 )            
		{
			if (!(c_file.attrib&_A_SUBDIR))
			{
				hBfxDll[list_counter]=LoadLibrary(c_file.name);
				sprintf(BfxName[list_counter],c_file.name);
				list_counter++;
			}
			
		} // while    
		
		_findclose( hFile );
		
	}
	
	nBfxDll=list_counter;
}

void psyGear::BuzzInit(const CMachineInfo *mif)
{
	mi->Init();
	
	for(int gbp=0;gbp<mif->numParameters;gbp++)
		mi->ParameterTweak(gbp,mif->Parameters[gbp]->DefValue);
}

int psySong::GetIntDLL(char *dllname)
{
	int rv=-1;
	for(int c=0;c<nBfxDll;c++)
	{
		if(strcmp(dllname,BfxName[c])==0)
			rv=c;
	}
	
	return rv;
}


void psySong::ExecuteLine()
{
	
	if(TicksRemaining<=0 && PlayMode==1)
	{
		psyGear *pMachine;
		LineChanged=true;
		
		for(int tc=0;tc<MAX_MACHINES;tc++)
		{
			if(Activemachine[tc])
			{
				pMachine=machine[tc];
				pMachine->TickFx();
			}
		}
		
		int pOffset=playPattern*MULTIPLY2+LineCounter*MULTIPLY;
		
		for(int track=0;track<SONGTRACKS;track++)
		{
			unsigned char * offset=pPatternData+pOffset+track*5;
			int pnote=*offset;
			int pinst=*(offset+1);
			int pgene=*(offset+2);
			int pcmdd=*(offset+3);
			int pvalu=*(offset+4);
			
			if(pgene!=255)
			{
				int mIndex=busMachine[pgene];
				
				if(Activemachine[mIndex])
				{
					psyGear *gmac=machine[busMachine[pgene]];
					
					if(gmac->machineMode==9 && track_st[track])
						gmac->ovst.SeqTick(track,pnote,pinst,pcmdd,pvalu);
					
					if(gmac->type==0 && gmac->machineMode==8 && track_st[track])
						gmac->mi->SeqTick(track,pnote,pinst,pcmdd,pvalu);
					
					if(gmac->machineMode==8 && pnote==121)
					{
						GETINFO bfxGetInfo =(GETINFO)GetProcAddress(hBfxDll[gmac->buzzdll],"GetInfo");
						
						if(pinst<bfxGetInfo()->numParameters)
						{
							int nv=(pcmdd*256)+pvalu;
							int const min=bfxGetInfo()->Parameters[pinst]->MinValue;
							int const max=bfxGetInfo()->Parameters[pinst]->MaxValue;
							
							nv+=min;
							if(nv>max)nv=max;
							
							gmac->mi->ParameterTweak(pinst,nv);
							Tweaker=true;
						}
					}
					
					if(gmac->machineMode==3)
					{
						gmac->DoPatternEffects(pinst,pcmdd,pvalu,track);
						
						if (pnote==120)
						{
							for(int c=0;c<gmac->numSubTracks;c++)
								gmac->trackObj[track][c].NoteOff();
						}
						
						if(track_st[track])
						{
							if(gmac->trackObj[track][gmac->SubTrack[track]].Tick(pnote,pinst,pcmdd,pvalu)==TR_TRIGGERED)
							{
								gmac->currSubTrack[track]=gmac->SubTrack[track];
								gmac->IsolateSubTracks(track,gmac->SubTrack[track],pinst);
								if(++gmac->SubTrack[track]>=gmac->numSubTracks)
									gmac->SubTrack[track]=0;
							}
						}
						
					}// MODE=3
				}// MACHINEACTIVED
			}// VALID PGENE
		}//TRACKBUCLE
		
		// Advance position in the  sequencer
		TicksRemaining=SamplesPerTick;
		LineCounter++;
		
		if(LineCounter>=patternLines[playPattern])
		{
			LineCounter=0;
			
			if(++playPosition>=playLength)playPosition=0;
			
			playPattern=SONG->playOrder[playPosition];
		}
	}
}

int psySong::GetNumPatternsUsed()
{
	int rval=0;
	
	for(int c=0;c<MAX_SONG_POSITIONS;c++)
	{
		if(rval<playOrder[c])rval=playOrder[c];
	}
	
	++rval;
	
	return rval;
}

int psySong::GetFreeBus()
{
	int val=-1;
	
	for(int c=MAX_BUSES-1;c>-1;c--)
	{
		if(busMachine[c]==255)val=c;
	}
	
	return val; 
}

int psySong::WavAlloc(int instrument,int layer,const char * str)
{
	if(instrument!=PREV_WAV_INS)
	{
		Invalided=true;
		Sleep(LOCK_LATENCY);
	}
	
	long Datalen=0;
	int ld0=0,ld1=0,ld2=0,ld3=0;
	int Freeindex2=0;
	int idchk=0;
	FILE *in;
	
	int rate=0;
	int bits=0;
	short *csamples,*csamples2;
	char st_type=0;
	short inx=0;
	int fmtchklen=0;
	
	const char *Wavfile=str;
	idchk=0;
	
	if (Wavfile!=NULL && (in = fopen(Wavfile,"rb"))!=NULL)
	{
		fseek(in,8,SEEK_SET);
		idchk=fgetc(in)*fgetc(in)*fgetc(in)*fgetc(in);
		
		if (idchk==33556770)
		{
			DeleteLayer(instrument,layer);
			
			// WAV Magic number identified
			rewind(in);
			fseek(in,22,SEEK_SET);
			st_type=fgetc(in);
			
			fseek(in,24,SEEK_SET);
			rate=fgetc(in);
			rate+=fgetc(in)*256;
			
			fseek(in,34,SEEK_SET);
			bits=fgetc(in);
			
			fseek(in,16,SEEK_SET); /* Get length of fmtchk */
			fmtchklen=fgetc(in);
			
			// Set cursor at possible list
			fseek(in,20+fmtchklen,SEEK_SET);
			unsigned int lchk=0;
			fread(&lchk,4,1,in);
			
			if(lchk==1414744396)
			{
				fmtchklen+=8;// LIST HANDLED!
				
				// handling size of LIST chunk
				unsigned char lelist;
				fread(&lelist,1,1,in);
				fmtchklen+=lelist;
			}
			
			fseek(in,24+fmtchklen,SEEK_SET);
			ld0=fgetc(in);
			
			fseek(in,25+fmtchklen,SEEK_SET);
			ld1=fgetc(in);
			
			fseek(in,26+fmtchklen,SEEK_SET);
			ld2=fgetc(in);
			
			fseek(in,27+fmtchklen,SEEK_SET);
			ld3=fgetc(in);
			
			Datalen=(ld3*16777216)+(ld2*65536)+(ld1*256)+ld0;
			
			if (bits==16)Datalen/=2;
			
			if (st_type==2)Datalen/=2;
			
			if (st_type==2)
			{
				waveDataL[instrument][layer]=new signed short[Datalen];
				waveDataR[instrument][layer]=new signed short[Datalen];
				csamples=waveDataL[instrument][layer];
				csamples2=waveDataR[instrument][layer];
				waveStereo[instrument][layer]=true;
			}
			else
			{
				waveDataL[instrument][layer]=new signed short[Datalen];
				csamples=waveDataL[instrument][layer];
				waveStereo[instrument][layer]=false;
			}
			
			waveLength[instrument][layer]=Datalen;
			
			fseek(in,28+fmtchklen,SEEK_SET);
			
			for(long io=0;io<Datalen;io++)
			{
				if (bits==8)
				{
					inx=fgetc(in)*256;
				}
				else
				{
					inx=fgetc(in);
					inx+=fgetc(in)*256;
				}
				
				if (st_type==2 && bits==16)
				{
					*csamples2=fgetc(in);
					*csamples2+=fgetc(in)*256;
					*csamples2-=65536;
					csamples2++;
				}
				
				if (st_type==2 && bits==8)
				{
					*csamples2=(fgetc(in)*256)-32768;
					csamples2++;
				}
				
				if (bits==16)
					*csamples=inx-65536;
				else
					*csamples=inx-32768;
				csamples++;
			}
			
			sprintf(waveName[instrument][layer],Wavfile);
			
			sprintf(instName[instrument],Wavfile);
			
			char smpc[16];
			smpc[4]=0;
			fread(&smpc,4,1,in);
			if(strcmp("smpl",smpc)==0)
			{
				// Loop Found
				fseek(in,32,SEEK_CUR);
				
				char pl=0;
				
				fread(&pl,1,1,in);
				
				if(pl==1)
				{
					fseek(in,15,SEEK_CUR);
					
					unsigned int ls=0;
					unsigned int le=0;
					fread(&ls,sizeof(unsigned int),1,in);
					fread(&le,sizeof(unsigned int),1,in);
					waveLoopStart[instrument][layer]=ls;
					waveLoopEnd[instrument][layer]=le;
					waveLoopType[instrument][layer]=true;
				}
			}
			fclose(in);
			Invalided=false;
			return 1;
	}
	else
	{
		fclose(in);
		Invalided=false;
		return 0;
	}
	
	}
	else
	{
		Invalided=false;
		return 0;
	}
}

void psySong::StartRecord(char *Wavefilename)
{
	if(m_WaveStage==0)
	{
		m_WaveFile.OpenForWrite(Wavefilename,44100,16,2);
		m_WaveStage=1;
	}
}

void psySong::StopRecord()
{
	if(m_WaveStage==1)
	{
		m_WaveFile.Close();
		m_WaveStage=0;
	}
}

void psySong::PW_Play()
{
	if(PW_Stage==0)
	{
		PW_Stage=1;
		PW_Length=waveLength[PREV_WAV_INS][0];
		PW_Phase=0;
		
		if(PW_Length<1)
			PW_Stage=0;
	}
}

void psySong::PW_Work(float *pSamplesL, float *pSamplesR, int numSamples)
{
	--pSamplesL;
	--pSamplesR;
	
	if(PW_Stage==1)
	{
		signed short *wl=waveDataL[PREV_WAV_INS][0];
		signed short *wr=waveDataR[PREV_WAV_INS][0];
		bool const stereo=waveStereo[PREV_WAV_INS][0];
		float ld=0;
		float rd=0;
		
		do
		{
			ld=*(wl+PW_Phase);
			
			if(stereo)
				rd=*(wr+PW_Phase);
			else
				rd=ld;
			
			*++pSamplesL+=ld;
			*++pSamplesR+=rd;
			
			if(++PW_Phase>=PW_Length)
			{
				PW_Stage=0;
				return;
			}
			
		}while(--numSamples);
	}
}

#include "presets.cpp"


void vstmi::Stop()
{
	m_midievent.type=kVstMidiType;
	m_midievent.byteSize=24;
	m_midievent.deltaFrames=0;
	m_midievent.flags=0;
	m_midievent.detune=0;
	m_midievent.noteLength=0;
	m_midievent.noteOffset=0;
	m_midievent.reserved1=0;
	m_midievent.reserved2=0;
	m_midievent.noteOffVelocity=127;
	m_midievent.midiData[0]=0xB0 | midichannel; /* Midi Off last note*/
	m_midievent.midiData[1]=123;
	m_midievent.midiData[2]=0;
	m_midievent.midiData[3]=0;
	SendVstMidi();
	
}

vstmi::vstmi()
{
	for(int tr=0;tr<MAX_TRACKS;tr++)
		ln[tr]=32;
	
	midichannel=0;
	replace=false;
	ANY=false;
}

vstmi::~vstmi()
{
	// Destroy dinamically allocated objects/memory here
}

void vstmi::Init()
{
	// Initialize your stuff here
}


void vstmi::SequencerTick()
{
}


void vstmi::Work(float *psamplesleft, float* psamplesright, int numsamples,int tracks,bool iseffect)
{
	if (ANY)
	{
		inputs[0]=psamplesleft;
		inputs[1]=psamplesright;
		
		// Creating output buffers
		
		outputs[0]=new float[numsamples];
		outputs[1]=new float[numsamples];
		
		float *f1=inputs[0];
		float *f2=inputs[1];
		float *f3=outputs[0];
		float *f4=outputs[1];
		
		// Clear output / input buffer samples
		--f1;
		--f2;
		--f3;
		--f4;
		
		if(iseffect)
		{
			float const coef=1.0f/32768.0f;
			
			for(int c=0;c<numsamples;c++)
			{
				*++f1=(*f1)*coef;
				*++f2=(*f2)*coef;
				*++f3=0.0f;
				*++f4=0.0f;
			}
		}
		else
		{
			for(int c=0;c<numsamples;c++)
			{
				*++f1=0.0f;
				*++f2=0.0f;
				*++f3=0.0f;
				*++f4=0.0f;
			}
		}
		
		// Call process
		effect->process(effect,inputs,outputs,numsamples);
		
		--psamplesleft;
		--psamplesright;
		
		// Amplify
		for (int i=0;i<numsamples;i++) 
		{
			*++psamplesleft=outputs[0][i]*32768;
			*++psamplesright=outputs[1][i]*32768;
		}
		
		// Delete output buffers
		delete outputs[0];
		delete outputs[1];
		
	}
}
void vstmi::NoteOff(int track)
{
	m_midievent.type=kVstMidiType;
	m_midievent.byteSize=24;
	m_midievent.deltaFrames=0;
	m_midievent.flags=0;
	m_midievent.detune=0;
	m_midievent.noteLength=0;
	m_midievent.noteOffset=0;
	m_midievent.reserved1=0;
	m_midievent.reserved2=0;
	m_midievent.noteOffVelocity=127;
	m_midievent.midiData[0]=0x80 | midichannel; /* Midi Off last note*/
	m_midievent.midiData[1]=ln[track];
	m_midievent.midiData[2]=0;
	m_midievent.midiData[3]=0;
	SendVstMidi();
}

void vstmi::SeqTick(int track, int note, int ins, int cmd, int val)
{
	if(ANY)
	{
		if(note==120)
		{
			m_midievent.type=kVstMidiType;
			m_midievent.byteSize=24;
			m_midievent.deltaFrames=0;
			m_midievent.flags=0;
			m_midievent.detune=0;
			m_midievent.noteLength=0;
			m_midievent.noteOffset=0;
			m_midievent.reserved1=0;
			m_midievent.reserved2=0;
			m_midievent.noteOffVelocity=127;
			m_midievent.midiData[0]=0x80 | midichannel; /* Midi Off last note*/
			m_midievent.midiData[1]=ln[track];
			m_midievent.midiData[2]=0;
			m_midievent.midiData[3]=0;
			SendVstMidi();
		}
		
		if(note<120)
		{
			m_midievent.type=kVstMidiType;
			m_midievent.byteSize=24;
			m_midievent.deltaFrames=0;
			m_midievent.flags=0;
			m_midievent.detune=0;
			m_midievent.noteLength=0;
			m_midievent.noteOffset=0;
			m_midievent.reserved1=0;
			m_midievent.reserved2=0;
			m_midievent.noteOffVelocity=127;
			
			m_midievent.midiData[0]=0x90 | midichannel; /* Midi On */
			m_midievent.midiData[1]=note;
			m_midievent.midiData[2]=127;
			m_midievent.midiData[3]=0;
			ln[track]=note;
			SendVstMidi();
		}
		
		if(note==121)
		{
			// Tweak par
			if(ANY)
			{
				float const value=float((cmd*256)+ins)/65535.0f;
				effect->setParameter(effect,ins,value);
			}
		}
	}
}

void vstmi::SendVstMidi()
{
	if(ANY)
	{
		m_events.numEvents=1;
		m_events.events[0]=(VstEvent *)&m_midievent;
		effect->dispatcher(effect,effProcessEvents,0,0,&m_events,0);
	}
}	

bool vstmi::DescribeValue(char* txt,int const param, int const value)
{
	switch(param)
	{
	case 0:
		
		if(pVST->Instanced[value])
			sprintf(txt,pVST->GetPlugName(value));
		else
			sprintf(txt,"Not instanced");
		
		return true;
		break;
	}
	
	return false;
}

void vstmi::SetVstInstance(int i)
{
	if(pVST->Instanced[i])
	{
		effect=pVST->effect[i];
		instance=i;
		ANY=true;
	}
	else
	{
		ANY=false;
	}
}

#include "riff.cpp"

