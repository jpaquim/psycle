#include "stdafx.h"



#if defined(_WINAMP_PLUGIN_)
//	#include "global.h"
#else
	#include "Psycle.h"
	#include "NewMachine.h"
	#include "MainFrm.h"
	#include "ChildView.h"
	#include "ProgressDialog.h"

	extern CPsycleApp theApp;
#endif // _WINAMP_PLUGIN_

#include "Song.h"
#include "Machine.h" // It wouldn't be needed, since it is already included in "song.h"
#include "Sampler.h"
#include "Plugin.h"
#include "VSTHost.h"
#include "DataCompression.h"

//#include <sstream>

#ifdef CONVERT_INTERNAL_MACHINES
	#include "convert_internal_machines.h" // conversion
#endif

#if !defined(_WINAMP_PLUGIN_)

#include "Riff.h"	 // For Wave file loading.

typedef struct 
{
  char           chunkID[4];
  long           chunkSize;
  short          wFormatTag;
  unsigned short wChannels;
  unsigned long  dwSamplesPerSec;
  unsigned long  dwAvgBytesPerSec;
  unsigned short wBlockAlign;
  unsigned short wBitsPerSample;
} WavHeader;
#endif // ndef _WINAMP_PLUGIN_

typedef struct
{
	bool		valid;
	char		dllName[128];
	int			numpars;
	float*		pars;

} VSTLoader;

//////////////////////////////////////////////////////////////////////
// Create machine member

bool Song::CreateMachine(
	MachineType type,
	int x,
	int y,
	char* psPluginDll,
	int index)
{

	Machine* pMachine;
	Master* pMaster;
	Sampler* pSampler;
#if !defined(CONVERT_INTERNAL_MACHINES)
	Delay* pDelay;
	Filter2p* pFilter;
	Gainer* pGainer;
	Flanger* pFlanger;
	Sine* pSine;
	Distortion* pDistortion;
#endif
	Plugin* pPlugin;
	VSTPlugin* pVstPlugin;

	switch (type)
	{
	case MACH_MASTER:
		if (_pMachine[MASTER_INDEX])
		{
			return false;
		}
		pMachine = pMaster = new Master(index);
		index = MASTER_INDEX;
		break;
	case MACH_SAMPLER:
		pMachine = pSampler = new Sampler(index);
		break;
#if !defined(CONVERT_INTERNAL_MACHINES)
	case MACH_SINE:
		pMachine = pSine = new Sine(index);
		break;
	case MACH_DIST:
		pMachine = pDistortion = new Distortion(index);
		break;
	case MACH_DELAY:
		pMachine = pDelay = new Delay(index);
		break;
	case MACH_2PFILTER:
		pMachine = pFilter = new Filter2p(index);
		break;
	case MACH_GAIN:
		pMachine = pGainer = new Gainer(index);
		break;
	case MACH_FLANGER:
		pMachine = pFlanger = new Flanger(index);
		break;
#endif
	case MACH_PLUGIN:
		{
			pMachine = pPlugin = new Plugin(index);
#if !defined(_WINAMP_PLUGIN_)
			if ( !CNewMachine::TestFilename(psPluginDll) ) 
			{
				delete pMachine; 
				return false;
			}
#endif
			if (!pPlugin->Instance(psPluginDll))
			{
				delete pMachine; 
				return false;
			}
			break;
		}
	case MACH_VST:
		{
			pMachine = pVstPlugin = new VSTInstrument(index);
#if !defined(_WINAMP_PLUGIN_)
			if ( !CNewMachine::TestFilename(psPluginDll) ) 
			{
				delete pMachine; 
				return false;
			}
#endif
			if (pVstPlugin->Instance(psPluginDll) != VSTINSTANCE_NO_ERROR)
			{
				delete pMachine; 
				return false;
			}
			break;
		}
	case MACH_VSTFX:
		{
			pMachine = pVstPlugin = new VSTFX(index);
#if !defined(_WINAMP_PLUGIN_)
			if ( !CNewMachine::TestFilename(psPluginDll) ) 
			{
				delete pMachine; 
				return false;
			}
#endif
			if (pVstPlugin->Instance(psPluginDll) != VSTINSTANCE_NO_ERROR)
			{
				delete pMachine; 
				return false;
			}
			break;
		}

	case MACH_DUMMY:
		pMachine = new Dummy(index);
		break;
	default:
		return false;
	}

	if (index < 0)
	{
		index =	GetFreeMachine();
		if (index < 0)
		{
			return false;
		}
	}

	if (_pMachine[index])
	{
		DestroyMachine(index);
	}

	if (pMachine->_type == MACH_VSTFX || pMachine->_type == MACH_VST ) // Do not call VST Init() function
	{																	// after Instance.
		pMachine->Machine::Init();
	}
	else pMachine->Init();
	pMachine->_x = x;
	pMachine->_y = y;
	
	// Finally, activate the machine
	//
	_pMachine[index] = pMachine;

	return true;
}
#if !defined(_WINAMP_PLUGIN_)
	
int Song::FindBusFromIndex(int smac)
{
	if ( !_pMachine[smac] ) 
	{
		return 255;
	}
	return smac;
}
#endif //  !defined(_WINAMP_PLUGIN_)
//////////////////////////////////////////////////////////////////////
// Song member functions source code

Song::Song()
{
#if defined(_WINAMP_PLUGIN_)
	filesize=0;
#else
	_machineLock = false;
	Invalided = false;
	Tweaker = false;
	PW_Phase = 0;
	PW_Stage = 0;
	PW_Length = 0;

#ifndef _CYRIX_PROCESSOR_

	ULONG cpuHz;
	__asm	rdtsc				// Read time stamp to EAX
	__asm	mov	cpuHz, eax

	Sleep(1000);

	__asm	rdtsc
	__asm	sub		eax,cpuHz	// Find the difference
	__asm	mov		cpuHz, eax

	Global::_cpuHz = cpuHz;
#else
	Global::_cpuHz = 1;
#endif // _CYRIX_PROCESSOR_

#endif // ndef _WINAMP_PLUGIN_

	for (int i = 0; i<MAX_MACHINES;i++)
	{
		_pMachine[i] = NULL;
	}

	for (i=0;i<MAX_INSTRUMENTS;i++)
	{
		_pInstrument[i] = new Instrument;
	}

	// KRISC MODIFICATION 
	pSequencer = new Sequencer();
	activeTrack = 0;
	patStep = 64;
	Reset();

}

Song::~Song()
{
	DestroyAllMachines();
	DestroyAllInstruments();
	//DeleteAllPatterns();

	delete pSequencer;

}

void Song::DestroyAllMachines()
{
#if !defined(_WINAMP_PLUGIN_)
	_machineLock = true;
#endif // ndef _WINAMP_PLUGIN_
	for(int c=0; c<MAX_MACHINES; c++)
	{
		if(_pMachine[c])
		{
			for (int j=c+1; j < MAX_MACHINES; j++)
			{
				if (_pMachine[c] == _pMachine[j])
				{
					// wtf? duplicate machine? could happen if loader messes up?
					char buf[128];
					sprintf(buf,"%d and %d have duplicate pointers",c,j);
					MessageBox(NULL,buf,"Duplicate Machine",NULL);
					_pMachine[j] = NULL;
				}
			}
			DestroyMachine(c);
		}
		_pMachine[c] = NULL;
	}
#if !defined(_WINAMP_PLUGIN_)
	_machineLock = false;
#endif // ndef _WINAMP_PLUGIN_
}

//////////////////////////////////////////////////////////////////////
// Seek and destroy allocated instruments

void Song::DeleteLayer(int i,int c)
{
	_pInstrument[i]->DeleteLayer(c);
}

void Song::DeleteInstruments()
{
	// this actually just resets to a blank instrument
	for (int i=0;i<MAX_INSTRUMENTS;i++)
	{
		DeleteInstrument(i);
	}
}

void Song::DestroyAllInstruments()
{
	for (int i=0;i<MAX_INSTRUMENTS;i++)
	{
		delete _pInstrument[i];
	}
}

void Song::DeleteInstrument(int i)
{
	// this actually just resets to a blank instrument
#if !defined(_WINAMP_PLUGIN_)
	Invalided=true;
#endif
	_pInstrument[i]->Delete();
#if !defined(_WINAMP_PLUGIN_)
	Invalided=false;
#endif

}

//////////////////////////////////////////////////////////////////////
// Song Reset function

void Song::Reset(void)
{
	pSequencer->startLine	= 0;
	pSequencer->endLine		= Global::pConfig->defaultPatLines;

#if !defined(_WINAMP_PLUGIN_)
	cpuIdle=0;
	_sampCount=0;
#endif // ndef _WINAMP_PLUGIN_	

	// Cleaning pattern allocation info
	for (int i=0; i<MAX_INSTRUMENTS; i++)
	{
		for (int c=0; c<MAX_WAVES; c++)
		{
			_pInstrument[i]->waveLength[c]=0;
		}
	}
	for (i = 0; i<MAX_MACHINES;i++)
	{
		if (_pMachine[i])
		{
			delete (_pMachine[i]);
			_pMachine[i] = NULL;
		}
	}
#if !defined(_WINAMP_PLUGIN_)
	for (i=0; i<MAX_MACHINES; i++)
	{
		// All pattern reset
		if (Global::pConfig)
		{
			pSequencer->pSequenceData[i]->patternLines[i] = Global::pConfig->defaultPatLines;
		}
		else
		{
			pSequencer->pSequenceData[i]->patternLines[i]=64;
		}
		sprintf(pSequencer->pSequenceData[i]->patternName[i],"Untitled"); 
	}
#endif // _WINAMP_PLUGIN
	_trackArmedCount = 0;
	for(i=0; i<MAX_TRACKS; i++)
	{
		_trackMuted[i] = false;
		_trackArmed[i] = false;
	}
#if defined(_WINAMP_PLUGIN_)
	for (i=0; i < MAX_SONG_POSITIONS; i++)
	{
		playOrder[i]=0; // All pattern reset
	}
#else
	machineSoloed = -1;
	_trackSoloed = -1;
	playLength=1;
	for (i=0; i < MAX_SONG_POSITIONS; i++)
	{
		playOrder[i]=0; // All pattern reset
		playOrderSel[i]=false;
	}
	playOrderSel[0]=true;
#endif // _WINAMP_PLUGIN_
}

//////////////////////////////////////////////////////////////////////
// Song NEWSONG

void Song::New(void)
{
#if !defined(_WINAMP_PLUGIN_)
	CSingleLock lock(&door,TRUE);
#endif // !defined(_WINAMP_PLUGIN_)
		
	seqBus=0;
	
	// Song reset
	memset(&Name, 0, sizeof(Name));
	memset(&Author, 0, sizeof(Author));
	memset(&Comment, 0, sizeof(Comment));
	sprintf(Name,"Untitled");
	sprintf(Author,"Unnamed");
	sprintf(Comment,"No Comments");
	
	currentOctave=4;
	
	// General properties
	SetBPM(125, 4, 44100);
	LineCounter=0;
	
	LineChanged=false;
	
//	MessageBox(NULL,"Machines",NULL,NULL);
	// Clean up allocated machines.
	DestroyAllMachines();
	
//	MessageBox(NULL,"Insts",NULL,NULL);
	// Cleaning instruments
	DeleteInstruments();
	
//	MessageBox(NULL,"Pats",NULL,NULL);
	// Clear patterns
	DeleteAllPatterns();
	for (int i=0; i<MAX_MACHINES; i++)
	{
		pSequencer->pSequenceData[i]->SONGTRACKS = 16;
	}

	
	// Clear sequence

	Reset();

	waveSelected = 0;
	instSelected = 0;
	midiSelected = 0;
	auxcolSelected = 0;
	_saved=false;
#if defined(_WINAMP_PLUGIN_)
	strcpy(fileName,"Untitled.psy");
	CreateMachine(MACH_MASTER, 320, 200, NULL,MASTER_INDEX);
#else
	fileName ="Untitled.psy";
	if ((CMainFrame *)theApp.m_pMainWnd)
	{
		CreateMachine(MACH_MASTER, 
			(viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width)/2, 
			(viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height)/2, 
			NULL,MASTER_INDEX);
	}
	else
	{
		CreateMachine(MACH_MASTER, 
			320, 
			200, 
			NULL,MASTER_INDEX);
	}
#endif // _WINAMP_PLUGIN_
}
// return first free index number of machines table 
int Song::GetFreeMachine(void) 
{
	int tmac = 0;
	while (true)
	{
		if (!_pMachine[tmac])
		{
			return tmac;
		}
		if (tmac++ >= MAX_MACHINES)
		{
			return -1;
		}
	}
}

// return index number of machines table = count of machines 
int Song::GetMachineCount(void) 
{
	int tmac = 0;
	for ( int i = 0; i < MAX_MACHINES; i++ ) 
	{
		if (_pMachine[i])
		{
			tmac++;
		}
	}
	return tmac-1;
}


#if !defined(_WINAMP_PLUGIN_)
bool Song::InsertConnection(int src,int dst,float value)
{
	int freebus=-1;
	int dfreebus=-1;
	
	bool error=false;
	
	Machine *srcMac = _pMachine[src];
	Machine *dstMac = _pMachine[dst];

	if (!srcMac || !dstMac)
	{
		return false;
	}
	
	if (dstMac->_mode == MACHMODE_GENERATOR)
	{
		return false;
	}
	
	// Get a free output slot on the source machine
	for (int c=MAX_CONNECTIONS-1; c>=0; c--)
	{
		if (!srcMac->_connection[c])
		{
			freebus = c;
		}
		// Checking that there's not an slot to the dest. machine already
		else if (srcMac->_outputMachines[c] == dst)
		{
			error = true;
		}
	}
	if (freebus == -1 || error)
	{
		return false;
	}

	// Get a free input slot on the destination machine
	error=false;
	for (c=MAX_CONNECTIONS-1; c>=0; c--)
	{
		if (!dstMac->_inputCon[c])
		{
			dfreebus = c;
		}
		// Checking if the destination machine is connected with the source machine to
		// avoid a loop.
		else if ((dstMac->_outputMachines[c] == src) && (dstMac->_connection[c]))
		{
			error = true;
		}
	}
	if (dfreebus == -1 || error)
	{
		return false;
	}
	
	// Calibrating in/out properties
	srcMac->_outputMachines[freebus] = dst;
	srcMac->_connection[freebus] = true;
	srcMac->_numOutputs++;
	
	dstMac->_inputMachines[dfreebus] = src;
	dstMac->_inputCon[dfreebus] = true;
	dstMac->_numInputs++;
	
	dstMac->InitWireVolume(srcMac->_type,dfreebus,value);
	
	return true;
}
#endif // ndef _WINAMP_PLUGIN_
//////////////////////////////////////////////////////////////////////
// Machine Deletion Function

void Song::DestroyMachine(int mac)
{
	// Delete and destroy the MACHINE!
#if !defined(_WINAMP_PLUGIN_)
	CSingleLock lock(&door,TRUE);
#endif // #if !defined(_WINAMP_PLUGIN_)
	
	Machine *iMac = _pMachine[mac];
	Machine *iMac2;

	if (iMac)
	{
		//Deleting the connections to/from other machines
		for (int w=0; w<MAX_CONNECTIONS; w++)
		{
			// Checking In-Wires
			if (iMac->_inputCon[w])
			{
				if ((iMac->_inputMachines[w] >= 0) && (iMac->_inputMachines[w] < MAX_MACHINES))
				{
					iMac2 = _pMachine[iMac->_inputMachines[w]];
					if (iMac2)
					{
						for (int x=0; x<MAX_CONNECTIONS; x++)
						{
							if ( iMac2->_connection[x] && iMac2->_outputMachines[x] == mac)
							{
								iMac2->_connection[x] = false;
								iMac2->_numOutputs--;
								break;
							}
						}
					}
				}
			}
			// Checking Out-Wires
			if(iMac->_connection[w])
			{
				if ((iMac->_outputMachines[w] >= 0) && (iMac->_outputMachines[w] < MAX_MACHINES))
				{
					iMac2 = _pMachine[iMac->_outputMachines[w]];
					if (iMac2)
					{
						for (int x=0; x<MAX_CONNECTIONS; x++)
						{
							if(iMac2->_inputCon[x] && iMac2->_inputMachines[x] == mac)
							{
								iMac2->_inputCon[x] = false;
								iMac2->_numInputs--;
								break;
							}
						}
					}
				}
			}
		}
	}

#if  !defined(_WINAMP_PLUGIN_)	
	if ( mac == machineSoloed ) 
	{
		machineSoloed = -1;
	}
#endif //  !defined(_WINAMP_PLUGIN_)
	delete _pMachine[mac];	// If it's a (Vst)Plugin, the destructor calls to free the .dll
	_pMachine[mac] = NULL;

	DeleteAllPatterns();
   vector <int>::iterator Iter;
   for ( Iter = pSequencer->machineList.begin( ) ; Iter != pSequencer->machineList.end( ) ; Iter++ )
   {
	   if ( *Iter == mac ) 
	   {
			pSequencer->machineList.erase(Iter);
			activeTrack = 0;
			break;
	   }
   }
   if ( pSequencer->machineList.size() == 0 ) 
   {
		pSequencer->machineList.push_back(0x80); /// MASTER !!! 
   }
}

void Song::DeleteAllPatterns()
{
	pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->SONGTRACKS = 16;
	for (int i=0; i<MAX_MACHINES; i++)
	{
		RemovePattern(i);
	}
}

void Song::RemovePattern(int ps)
{
	pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->RemovePattern(ps);
}

void Song::ClearPattern(int ps)
{
	pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->ClearPattern(ps);
}

void Song::RemovePatternFromTrack(int ps)
{
	pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->RemovePatternFromTrack(ps);
}

unsigned char * Song::CreateNewPattern(int ps)
{
	return pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->CreateNewPattern(ps);
}

#if !defined(_WINAMP_PLUGIN_)
bool Song::AllocNewPattern(int pattern,char *name,int lines,bool adaptsize)
{
	return pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->AllocNewPattern(pattern, name, lines, adaptsize);
}

#endif // ndef _WINAMP_PLUGIN_
void Song::SetBPM(int bpm, int tpb, int srate)
{
	static int sr = 0;
	BeatsPerMin = bpm;
	_ticksPerBeat = tpb;
	SamplesPerTick = (srate*15*4)/(bpm*tpb);

	// hey if our rate has changed, let everybody know!
	if (sr != srate)
	{
		sr = srate;
		for (int i=0; i<MAX_MACHINES; i++)
		{
			if(_pMachine[i])
			{
				_pMachine[i]->SetSampleRate(srate);
			}
		}
	}
}



int Song::GetNumPatternsUsed()
{
	return pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->patCount;
}
#if !defined(_WINAMP_PLUGIN_)

int Song::GetBlankPatternUnused(int rval)
{
	return pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->patCount;
}

int Song::GetFreeBus()
{
	for(int c=0;c<MAX_BUSES;c++)
	{
		if(!_pMachine[c])
		{
			return c;
		}
	}
	return -1; 
}

int Song::GetFreeFxBus()
{
	for(int c=MAX_BUSES;c<MAX_BUSES*2;c++)
	{
		if(!_pMachine[c])
		{
			return c;
		}
	}
	return -1; 
}

// IFF structure ripped by krokpitr
// Current Code Extremely modified by [JAZ] ( RIFF based )
// Advise: IFF files use Big Endian byte ordering. That's why I use
// the following structure.
//
// typedef struct {
//   unsigned char hihi;
//   unsigned char hilo;
//   unsigned char lohi;
//   unsigned char lolo;
// } ULONGINV;
// 
//
/*
** IFF Riff Header
** ----------------

char Id[4]			// "FORM"
ULONGINV hlength	// of the data contained in the file (except Id and length)
char type[4]		// "16SV" == 16bit . 8SVX == 8bit

char name_Id[4]		// "NAME"
ULONGINV hlength	// of the data contained in the header "NAME". It is 22 bytes
char name[22]		// name of the sample.

char vhdr_Id[4]		// "VHDR"
ULONGINV hlength	// of the data contained in the header "VHDR". it is 20 bytes
ULONGINV Samplength	// Lenght of the sample. It is in bytes, not in Samples.
ULONGINV loopstart	// Start point for the loop. It is in bytes, not in Samples.
ULONGINV loopLength	// Length of the loop (so loopEnd = loopstart+looplenght) In bytes.
unsigned char unknown2[5]; //Always $20 $AB $01 $00 //
unsigned char volumeHiByte;
unsigned char volumeLoByte;
unsigned char unknown3;

char body_Id[4]		// "BODY"
ULONGINV hlength	// of the data contained in the file. It is the sample length as well (in bytes)
char *data			// the sample.

*/

int Song::IffAlloc(int instrument,int layer,const char * str)
{
	if(instrument!=PREV_WAV_INS)
	{
		Invalided=true;
		Sleep(LOCK_LATENCY);
	}
	
	RiffFile file;
	RiffChunkHeader hd;
	ULONG data;
	ULONGINV tmp;
	int bits=0;

	if (!file.Open((char*)str)) // This opens the file and reads the "FORM" header.
	{
		Invalided=false;
		return 0;
	}

	DeleteLayer(instrument,layer);

	file.Read(&data,4);
	if ( data == file.FourCC("16SV")) bits = 16;
	else if ( data == file.FourCC("8SVX")) bits = 8;

	file.Read(&hd,sizeof(RiffChunkHeader));

	if ( hd._id == file.FourCC("NAME"))
	{
		file.Read(_pInstrument[instrument]->waveName[layer],22); // should be hd._size instead of "22", but it is incorrectly read.
		if ( strcmp(_pInstrument[instrument]->_sName,"empty") == 0 )
		{
			strncpy(_pInstrument[instrument]->_sName,str,31);
			_pInstrument[instrument]->_sName[31]='\0';
		}
		file.Read(&hd,sizeof(RiffChunkHeader));
	}

	if ( hd._id == file.FourCC("VHDR"))
	{
		unsigned int Datalen, ls, le;

		file.Read(&tmp,sizeof(ULONGINV));
		Datalen = (tmp.hihi<<24) + (tmp.hilo<<16) + (tmp.lohi << 8) + tmp.lolo;

		file.Read(&tmp,sizeof(ULONGINV));
		ls = (tmp.hihi<<24) + (tmp.hilo<<16) + (tmp.lohi << 8) + tmp.lolo;

		file.Read(&tmp,sizeof(ULONGINV));
		le = (tmp.hihi<<24) + (tmp.hilo<<16) + (tmp.lohi << 8) + tmp.lolo;

		if ( bits == 16 )
		{
			Datalen>>=1;		ls>>=1;		le>>=1;
		}
		_pInstrument[instrument]->waveLength[layer]=Datalen;

		if ( ls != le )
		{
			_pInstrument[instrument]->waveLoopStart[layer]=ls;
			_pInstrument[instrument]->waveLoopEnd[layer]=ls+le;
			_pInstrument[instrument]->waveLoopType[layer]=true;
		}
		file.Skip(8); // Skipping unknown bytes (and volume on bytes 6&7)
		file.Read(&hd,sizeof(RiffChunkHeader));
	}

	if ( hd._id == file.FourCC("BODY"))
	{
	    short *csamples;
		const unsigned int Datalen = _pInstrument[instrument]->waveLength[layer];

		_pInstrument[instrument]->waveStereo[layer]=false;
		_pInstrument[instrument]->waveDataL[layer]=new signed short[Datalen];
		csamples=_pInstrument[instrument]->waveDataL[layer];
		
		if ( bits == 16 )
		{
			for (unsigned int smp=0;smp<Datalen;smp++)
			{
				file.Read(&tmp,2);
				*csamples=(tmp.hilo)*256 +tmp.hihi;
				csamples++;
			}
		}
		else
		{
			for (unsigned int smp=0;smp<Datalen;smp++)
			{
				file.Read(&tmp,1);
				*csamples=(tmp.hihi)*256 +tmp.hihi;
				csamples++;
			}
		}
	}
	file.Close();
	Invalided=false;
	return 1;
}

int Song::WavAlloc(
	int iInstr,
	int iLayer,
	bool bStereo,
	long iSamplesPerChan,
	const char * sName)
{
	ASSERT(iSamplesPerChan<(1<<30)); // Since in some places, signed values are used, we cannot use the whole range.

	DeleteLayer(iInstr,iLayer);

	if(bStereo)
	{
		_pInstrument[iInstr]->waveDataL[iLayer]=new signed short[iSamplesPerChan];
		_pInstrument[iInstr]->waveDataR[iLayer]=new signed short[iSamplesPerChan];
		_pInstrument[iInstr]->waveStereo[iLayer]=true;
	}
	else
	{
		_pInstrument[iInstr]->waveDataL[iLayer]=new signed short[iSamplesPerChan];
		_pInstrument[iInstr]->waveStereo[iLayer]=false;
	}
	_pInstrument[iInstr]->waveLength[iLayer]=iSamplesPerChan;

	strncpy(_pInstrument[iInstr]->waveName[iLayer],sName,31);
	_pInstrument[iInstr]->waveName[iLayer][31]='\0';
	
	if(iLayer==0)
	{
		strncpy(_pInstrument[iInstr]->_sName,sName,31);
		_pInstrument[iInstr]->_sName[31]='\0';
	}

	return true;
}

int Song::WavAlloc(int instrument,int layer,const char * Wavfile)
{ 
  
	ASSERT(Wavfile!=NULL);

	WaveFile file;
	ExtRiffChunkHeader hd;
	
	DDCRET retcode = file.OpenForRead((char*)Wavfile); //This opens the file and read the format Header.
														// Also, it skips extra data, and points to the RAW wave data.
	if ( retcode != DDC_SUCCESS) 
	{
		Invalided= false;
		return 0; 
	}

	Invalided=true;
	Sleep(LOCK_LATENCY);

	// sample type	
	int st_type= file.NumChannels();
//	int rate = file.SamplingRate();
//	int align = file.NumChannels()* file.BitsPerSample() /8;
	int bits = file.BitsPerSample();
	long Datalen=file.NumSamples();

	WavAlloc(instrument,layer,st_type==2,Datalen,Wavfile); // Initializes the layer.

// Reading of Wave data.
// I don't use the WaveFile "ReadSamples" functions, because there are two main differences:
// I need to convert 8bits to 16bits, and stereo channels are in different arrays.
	
	short *sampL=_pInstrument[instrument]->waveDataL[layer];
	
	long io;
	if ( st_type == 1 ) // mono
	{
		UINT8 smp8;
		switch(bits)
		{
			case 8:
				for(io=0;io<Datalen;io++)
				{
					file.ReadData(&smp8,1);
					*sampL=(smp8<<8)-32768;
					sampL++;
				}
				break;
			case 16:
					file.ReadData(sampL,Datalen);
				break;
			case 24:
				for(io=0;io<Datalen;io++)
				{
					file.ReadData(&smp8,1);
					file.ReadData(sampL,1);
					sampL++;
				}
				break;
			default:
				break;
		}
	}
	else // stereo
	{
		short *sampR = _pInstrument[instrument]->waveDataR[layer];

		UINT8 smp8;
		switch(bits)
		{
			case 8:
				for(io=0;io<Datalen;io++)
				{
					file.ReadData(&smp8,1);
					*sampL=(smp8<<8)-32768;
					sampL++;
					file.ReadData(&smp8,1);
					*sampR=(smp8<<8)-32768;
					sampR++;
				}
				break;
			case 16:
				for(io=0;io<Datalen;io++)
				{
					file.ReadData(sampL,1);
					file.ReadData(sampR,1);
					sampL++;
					sampR++;
				}
				break;
			case 24:
				for(io=0;io<Datalen;io++)
				{
					file.ReadData(&smp8,1);
					file.ReadData(sampL,1);
					sampL++;
					file.ReadData(&smp8,1);
					file.ReadData(sampR,1);
					sampR++;
				}
				break;
			default:
				break;
		}
	}

	retcode = file.Read((void*)&hd,8);

	while ( retcode == DDC_SUCCESS )
	{
		if ( hd.ckID == FourCC("smpl") )
		{
			char pl=0;

			file.Skip(28);
			file.Read((void*)&pl,1);
			if ( pl == 1 )
			{
				file.Skip(15);
				
				unsigned int ls=0;
				unsigned int le=0;
				file.Read((void*)&ls,4);
				file.Read((void*)&le,4);
				_pInstrument[instrument]->waveLoopStart[layer]=ls;
				_pInstrument[instrument]->waveLoopEnd[layer]=le;
//				if (!((ls <= 0) && (le >= Datalen-1))) // **** only for my bad sample collection
				{
					_pInstrument[instrument]->waveLoopType[layer]=true;
				}
			}
			file.Skip(9);
		}
		else
		{
			if ( hd.ckSize > 0 ) file.Skip(hd.ckSize);
			else file.Skip(1);
		}

		retcode = file.Read((void*)&hd,8);
	}
	
	file.Close();
	
	Invalided=false;
	return 1;

}
#endif // ndef _WINAMP_PLUGIN_

bool Song::LoadPsy2Song(RiffFile* pFile, bool fullopen)
{

#if !defined(_WINAMP_PLUGIN_)
		
		CProgressDialog Progress;
		Progress.Create();
		Progress.SetWindowText("Loading old format...");
		Progress.ShowWindow(SW_SHOW);
#endif
		int i;
		int num;

		bool _machineActive[128];
		unsigned char busEffect[64];
		unsigned char busMachine[64];

	#if !defined(_WINAMP_PLUGIN_)
		New();
	#endif // ndef _WINAMP_PLUGIN_

		pFile->Read(&Name, 32);
		pFile->Read(&Author, 32);
		pFile->Read(&Comment, 128);
		
		pFile->Read(&BeatsPerMin, sizeof(BeatsPerMin));
		pFile->Read(&SamplesPerTick, sizeof(SamplesPerTick));
		if ( SamplesPerTick <= 0 ) { // Shouldn't happen but has happened.
			_ticksPerBeat= 4; SamplesPerTick = 4315;
		}
		else _ticksPerBeat = 44100*15*4/(SamplesPerTick*BeatsPerMin);

		Global::pPlayer->bpm = BeatsPerMin;
		Global::pPlayer->tpb = _ticksPerBeat;
		
		// The old format assumes we output at 44100 samples/sec, so...
		//
	#if defined(_WINAMP_PLUGIN_)
		SamplesPerTick = SamplesPerTick*Global::pConfig->_samplesPerSec/44100;
	#else
		SamplesPerTick = SamplesPerTick*Global::pConfig->_pOutputDriver->_samplesPerSec/44100;
	#endif // _WINAMP_PLUGIN_

		pFile->Read(&currentOctave, sizeof(currentOctave));

		pFile->Read(&busMachine[0], sizeof(busMachine));

		pFile->Read(&playOrder, sizeof(playOrder));
		pFile->Read(&playLength, sizeof(playLength));
		pFile->Read(&pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->SONGTRACKS, sizeof(pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->SONGTRACKS));

		// Patterns
		//
		pFile->Read(&num, sizeof(num));
		for (i=0; i<num; i++)
		{
			pFile->Read(&pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->patternLines[i], sizeof(pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->patternLines[0]));
			pFile->Read(&pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->patternName[i][0], sizeof(pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->patternName[0]));
			if (pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->patternLines[i] > 0)
			{
				unsigned char * pData = CreateNewPattern(i);
				for (int c=0; c<pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->patternLines[i]; c++)
				{
					pFile->Read((char*)pData, OLD_MAX_TRACKS*sizeof(PatternEntry));
					pData+=MAX_TRACKS*sizeof(PatternEntry);
				}
			}
			else
			{
				pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->patternLines[i] = 64;
				RemovePattern(i);
			}
		}
#if !defined(_WINAMP_PLUGIN_)
		
		Progress.m_Progress.SetPos(2048);
		::Sleep(1);
#endif
		// Instruments
		//
		pFile->Read(&instSelected, sizeof(instSelected));
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->_sName, sizeof(_pInstrument[0]->_sName));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->_NNA, sizeof(_pInstrument[0]->_NNA));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->ENV_AT, sizeof(_pInstrument[0]->ENV_AT));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->ENV_DT, sizeof(_pInstrument[0]->ENV_DT));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->ENV_SL, sizeof(_pInstrument[0]->ENV_SL));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->ENV_RT, sizeof(_pInstrument[0]->ENV_RT));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->ENV_F_AT, sizeof(_pInstrument[0]->ENV_F_AT));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->ENV_F_DT, sizeof(_pInstrument[0]->ENV_F_DT));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->ENV_F_SL, sizeof(_pInstrument[0]->ENV_F_SL));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->ENV_F_RT, sizeof(_pInstrument[0]->ENV_F_RT));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->ENV_F_CO, sizeof(_pInstrument[0]->ENV_F_CO));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->ENV_F_RQ, sizeof(_pInstrument[0]->ENV_F_RQ));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->ENV_F_EA, sizeof(_pInstrument[0]->ENV_F_EA));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->ENV_F_TP, sizeof(_pInstrument[0]->ENV_F_TP));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->_pan, sizeof(_pInstrument[0]->_pan));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->_RPAN, sizeof(_pInstrument[0]->_RPAN));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->_RCUT, sizeof(_pInstrument[0]->_RCUT));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->_RRES, sizeof(_pInstrument[0]->_RRES));
		}
#if !defined(_WINAMP_PLUGIN_)
		
		Progress.m_Progress.SetPos(4096);
		::Sleep(1);
#endif
		// Waves
		//
		pFile->Read(&waveSelected, sizeof(waveSelected));

		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			for (int w=0; w<OLD_MAX_WAVES; w++)
			{
				pFile->Read(&_pInstrument[i]->waveLength[w], sizeof(_pInstrument[0]->waveLength[0]));
				if (_pInstrument[i]->waveLength[w] > 0)
				{
					short tmpFineTune;
					pFile->Read(&_pInstrument[i]->waveName[w], 32);
					pFile->Read(&_pInstrument[i]->waveVolume[w], sizeof(_pInstrument[0]->waveVolume[0]));
					pFile->Read(&tmpFineTune, sizeof(short));
					_pInstrument[i]->waveFinetune[w]=(int)tmpFineTune;
					pFile->Read(&_pInstrument[i]->waveLoopStart[w], sizeof(_pInstrument[0]->waveLoopStart[0]));
					pFile->Read(&_pInstrument[i]->waveLoopEnd[w], sizeof(_pInstrument[0]->waveLoopEnd[0]));
					pFile->Read(&_pInstrument[i]->waveLoopType[w], sizeof(_pInstrument[0]->waveLoopType[0]));
					pFile->Read(&_pInstrument[i]->waveStereo[w], sizeof(_pInstrument[0]->waveStereo[0]));
					_pInstrument[i]->waveDataL[w] = new signed short[_pInstrument[i]->waveLength[w]];
					pFile->Read(_pInstrument[i]->waveDataL[w], _pInstrument[i]->waveLength[w]*sizeof(short));
					if (_pInstrument[i]->waveStereo[w])
					{
						_pInstrument[i]->waveDataR[w] = new signed short[_pInstrument[i]->waveLength[w]];
						pFile->Read(_pInstrument[i]->waveDataR[w], _pInstrument[i]->waveLength[w]*sizeof(short));
					}
				}
			}
		}
#if !defined(_WINAMP_PLUGIN_)
		
		Progress.m_Progress.SetPos(4096+2048);
		::Sleep(1);
#endif
		// VST DLLs
		//

		VSTLoader vstL[MAX_PLUGINS]; 
		for (i=0; i<MAX_PLUGINS; i++)
		{
			pFile->Read(&vstL[i].valid,sizeof(bool));
			if( vstL[i].valid )
			{
				pFile->Read(vstL[i].dllName,sizeof(vstL[i].dllName));
				_strlwr(vstL[i].dllName);
				pFile->Read(&(vstL[i].numpars), sizeof(int));
				vstL[i].pars = new float[vstL[i].numpars];

				for (int c=0; c<vstL[i].numpars; c++)
				{
					pFile->Read(&(vstL[i].pars[c]), sizeof(float));
				}
			}
		}
#if !defined(_WINAMP_PLUGIN_)
		
		Progress.m_Progress.SetPos(8192);
		::Sleep(1);
#endif
		// Machines
		//
		_machineLock = true;

		pFile->Read(&_machineActive[0], sizeof(_machineActive));
		Machine* pMac[128];
		memset(pMac,0,sizeof(pMac));

#if defined(CONVERT_INTERNAL_MACHINES)
		psycle::convert_internal_machines::Converter converter; // conversion
#endif

		for (i=0; i<128; i++)
		{
#if !defined(CONVERT_INTERNAL_MACHINES)
			Sine* pSine;
			Distortion* pDistortion;
			Delay* pDelay;
			Filter2p* pFilter;
			Gainer* pGainer;
			Flanger* pFlanger;
#endif
			Sampler* pSampler;
			Plugin* pPlugin;
			VSTPlugin* pVstPlugin;

			int x,y,type;
			if (_machineActive[i])
			{
#if !defined(_WINAMP_PLUGIN_)
				Progress.m_Progress.SetPos(8192+i*(4096/128));
				::Sleep(1);
#endif

				pFile->Read(&x, sizeof(x));
				pFile->Read(&y, sizeof(y));

				pFile->Read(&type, sizeof(type));

#if defined(CONVERT_INTERNAL_MACHINES)
				if(converter.plugin_names().exists(type))
					pMac[i] = &converter.redirect(i, type, *pFile); // conversion
				else
#endif

				switch (type)
				{
				case MACH_MASTER:
					pMac[i] = _pMachine[MASTER_INDEX];
					pMac[i]->Init();
					pMac[i]->Load(pFile);
					break;
				case MACH_SAMPLER:
					pMac[i] = pSampler = new Sampler(i);
					pMac[i]->Init();
					pMac[i]->Load(pFile);
					break;
#if !defined(CONVERT_INTERNAL_MACHINES)
				case MACH_SINE:
					pMac[i] = pSine = new Sine(i);
					pMac[i]->Init();
					pMac[i]->Load(pFile);
					break;
				case MACH_DIST:
					pMac[i] = pDistortion = new Distortion(i);
					pMac[i]->Init();
					pMac[i]->Load(pFile);
					break;
				case MACH_DELAY:
					pMac[i] = pDelay = new Delay(i);
					pMac[i]->Init();
					pMac[i]->Load(pFile);
					break;
				case MACH_2PFILTER:
					pMac[i] = pFilter = new Filter2p(i);
					pMac[i]->Init();
					pMac[i]->Load(pFile);
					break;
				case MACH_GAIN:
					pMac[i] = pGainer = new Gainer(i);
					pMac[i]->Init();
					pMac[i]->Load(pFile);
					break;
				case MACH_FLANGER:
					pMac[i] = pFlanger = new Flanger(i);
					pMac[i]->Init();
					pMac[i]->Load(pFile);
					break;
#endif
				case MACH_PLUGIN:
					{
					pMac[i] = pPlugin = new Plugin(i);
					// Should the "Init()" function go here? -> No. Needs to load the dll first.
					if (!pMac[i]->Load(pFile))
					{
						Machine* pOldMachine = pMac[i];
						pMac[i] = new Dummy(*((Dummy*)pOldMachine));
						// dummy name goes here
						sprintf(pMac[i]->_editName,"X %s",pOldMachine->_editName);
						pMac[i]->_type = MACH_DUMMY;
						pOldMachine->_pSamplesL = NULL;
						pOldMachine->_pSamplesR = NULL;
						delete pOldMachine;
					}
					break;
					}
				case MACH_VST:
				case MACH_VSTFX:
					{
					
					if ( type == MACH_VST ) 
					{
						pMac[i] = pVstPlugin = new VSTInstrument(i);
					}
					else if ( type == MACH_VSTFX ) 
					{
						pMac[i] = pVstPlugin = new VSTFX(i);
					}
					if ((pMac[i]->Load(pFile)) && (vstL[pVstPlugin->_instance].valid)) // Machine::Init() is done Inside "Load()"
					{
						char sPath2[_MAX_PATH];
						CString sPath;
	#if defined(_WINAMP_PLUGIN_)
						sPath = Global::pConfig->GetVstDir();
						if ( fullopen && FindFileinDir(vstL[pVstPlugin->_instance].dllName,sPath) )
						{
							strcpy(sPath2,sPath);

							if (pVstPlugin->Instance(sPath2,false) != VSTINSTANCE_NO_ERROR)
							{
								Machine* pOldMachine = pMac[i];
								pMac[i] = new Dummy(*((Dummy*)pOldMachine));
								pOldMachine->_pSamplesL = NULL;
								pOldMachine->_pSamplesR = NULL;
								// dummy name goes here
								sprintf(pMac[i]->_editName,"X %s",pOldMachine->_editName);
								delete pOldMachine;
								pMac[i]->_type = MACH_DUMMY;
								((Dummy*)pMac[i])->wasVST = true;
							}
						}
						else
						{
							Machine* pOldMachine = pMac[i];
							pMac[i] = new Dummy(*((Dummy*)pOldMachine));
							pOldMachine->_pSamplesL = NULL;
							pOldMachine->_pSamplesR = NULL;
							// dummy name goes here
							sprintf(pMac[i]->_editName,"X %s",pOldMachine->_editName);
							delete pOldMachine;
							pMac[i]->_type = MACH_DUMMY;
							((Dummy*)pMac[i])->wasVST = true;
						}
	#else // if !_WINAMP_PLUGIN_
						if ( CNewMachine::dllNames.Lookup(vstL[pVstPlugin->_instance].dllName,sPath) )
						{
							strcpy(sPath2,sPath);
							if (!CNewMachine::TestFilename(sPath2))
							{
								char sError[128];
								sprintf(sError,"Missing or Corrupted VST plug-in \"%s\" - replacing with Dummy.",sPath2);
								::MessageBox(NULL,sError, "Loading Error", MB_OK);

								Machine* pOldMachine = pMac[i];
								pMac[i] = new Dummy(*((Dummy*)pOldMachine));
								pOldMachine->_pSamplesL = NULL;
								pOldMachine->_pSamplesR = NULL;
								// dummy name goes here
								sprintf(pMac[i]->_editName,"X %s",pOldMachine->_editName);
								delete pOldMachine;
								pMac[i]->_type = MACH_DUMMY;
								((Dummy*)pMac[i])->wasVST = true;
							}
							else if (pVstPlugin->Instance(sPath2,false) != VSTINSTANCE_NO_ERROR)
							{
								char sError[128];
								sprintf(sError,"Missing or Corrupted VST plug-in \"%s\" - replacing with Dummy.",sPath2);
								::MessageBox(NULL,sError, "Loading Error", MB_OK);

								Machine* pOldMachine = pMac[i];
								pMac[i] = new Dummy(*((Dummy*)pOldMachine));
								pOldMachine->_pSamplesL = NULL;
								pOldMachine->_pSamplesR = NULL;
								// dummy name goes here
								sprintf(pMac[i]->_editName,"X %s",pOldMachine->_editName);
								delete pOldMachine;
								pMac[i]->_type = MACH_DUMMY;
								((Dummy*)pMac[i])->wasVST = true;
							}
						}
						else
						{
							char sError[128];
							sprintf(sError,"Missing VST plug-in \"%s\"",vstL[pVstPlugin->_instance].dllName);
							::MessageBox(NULL,sError, "Loading Error", MB_OK);

							Machine* pOldMachine = pMac[i];
							pMac[i] = new Dummy(*((Dummy*)pOldMachine));
							pOldMachine->_pSamplesL = NULL;
							pOldMachine->_pSamplesR = NULL;
							// dummy name goes here
							sprintf(pMac[i]->_editName,"X %s",pOldMachine->_editName);
							delete pOldMachine;
							pMac[i]->_type = MACH_DUMMY;
							((Dummy*)pMac[i])->wasVST = true;
						}
	#endif // _WINAMP_PLUGIN_
					}
					else
					{
						Machine* pOldMachine = pMac[i];
						pMac[i] = new Dummy(*((Dummy*)pOldMachine));
						pOldMachine->_pSamplesL = NULL;
						pOldMachine->_pSamplesR = NULL;
						// dummy name goes here
						sprintf(pMac[i]->_editName,"X %s",pOldMachine->_editName);
						delete pOldMachine;
						pMac[i]->_type = MACH_DUMMY;
						((Dummy*)pMac[i])->wasVST = true;
					}
					break;
					}
				case MACH_SCOPE:
				case MACH_DUMMY:
					pMac[i] = new Dummy(i);
					pMac[i]->Init();
					pMac[i]->Load(pFile);
					break;
				default:
					{
						char buf[MAX_PATH];
						sprintf(buf,"unkown machine type: %i",type);
						::MessageBox(0, buf, "Loading old song", MB_ICONERROR);
					}
					pMac[i] = new Dummy(i);
					pMac[i]->Init();
					pMac[i]->Load(pFile);
				}

#if !defined(_WINAMP_PLUGIN_)
				switch (pMac[i]->_mode)
				{
				case MACHMODE_GENERATOR:
					if ( x > viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.width ) x = viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.width;
					if ( y > viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.height ) y = viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.height;
					break;
				case MACHMODE_FX:
					if ( x > viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.width ) x = viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.width;
					if ( y > viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.height ) y = viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.height;
					break;

				case MACHMODE_MASTER:
					if ( x > viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width ) x = viewSize.x-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.width;
					if ( y > viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height ) y = viewSize.y-((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sMaster.height;
					break;
				}
#endif // _WINAMP_PLUGIN_

				pMac[i]->_x = x;
				pMac[i]->_y = y;
			}
		}
#if !defined(_WINAMP_PLUGIN_)
		Progress.m_Progress.SetPos(8192+4096);
		::Sleep(1);
#endif

		// Since the old file format stored volumes on each output
		// rather than on each input, we must convert
		//
		float volMatrix[128][MAX_CONNECTIONS];
		for (i=0; i<128; i++) // First, we add the output volumes to a Matrix for latter reference
		{
			if (!_machineActive[i])
			{
				if (pMac[i])
				{
					delete pMac[i];
					pMac[i] = NULL;
				}
			}
			else if (!pMac[i])
			{
				_machineActive[i] = FALSE;
			}
			else 
			{
				for (int c=0; c<MAX_CONNECTIONS; c++)
				{
					volMatrix[i][c] = pMac[i]->_inputConVol[c];
				}
			}
		}
#if !defined(_WINAMP_PLUGIN_)
		
		Progress.m_Progress.SetPos(8192+4096+1024);
		::Sleep(1);
#endif
		for (i=0; i<128; i++) // Next, we go to fix this for each
		{
			if (_machineActive[i])		// valid machine (important, since we have to navigate!)
			{
				for (int c=0; c<MAX_CONNECTIONS; c++) // all of its input connections.
				{
					if (pMac[i]->_inputCon[c])	// If there's a valid machine in this inputconnection,
					{
						Machine* pOrigMachine = pMac[pMac[i]->_inputMachines[c]]; // We get that machine
						int d = pOrigMachine->FindOutputWire(i);

						float val = volMatrix[pMac[i]->_inputMachines[c]][d];
						if( val >= 4.000001f ) 
						{
							val*=0.000030517578125f; // BugFix
						}
						else if ( val < 0.00004f) 
						{
							val*=32768.0f; // BugFix
						}

						pMac[i]->InitWireVolume(pOrigMachine->_type,c,val);
					}
				}
			}
		}
#if !defined(_WINAMP_PLUGIN_)
		
		Progress.m_Progress.SetPos(8192+4096+2048);
		::Sleep(1);
#endif
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->_loop, sizeof(_pInstrument[0]->_loop));
		}
		for (i=0; i<OLD_MAX_INSTRUMENTS; i++)
		{
			pFile->Read(&_pInstrument[i]->_lines, sizeof(_pInstrument[0]->_lines));
		}

		if ( pFile->Read(&busEffect[0],sizeof(busEffect)) == false ) // Patch 1: BusEffects (twf)
		{
			int j=0;
			for ( i=0;i<128;i++ ) 
			{
				if (_machineActive[i] && pMac[i]->_mode != MACHMODE_GENERATOR )
				{
					busEffect[j]=i;	
					j++;
				}
			}
			for (j; j < 64; j++)
			{
				busEffect[j] = 255;
			}
		}
		// Patch 1.2: Fixes crash/inconsistence when deleting a machine which couldn't be loaded
		// (.dll not found, or Load failed), which is, then, replaced by a DUMMY machine.
		int j=0;
		for ( i=0;i<64;i++ ) 
		{
			if (busMachine[i] != 255 && _machineActive[busMachine[i]]) 
			{ // If there's a machine in the generators' bus that it is not a generator:
				if (pMac[busMachine[i]]->_mode != MACHMODE_GENERATOR ) 
				{
					pMac[busMachine[i]]->_mode = MACHMODE_FX;
					while (busEffect[j] != 255 && j<MAX_BUSES) 
					{
						j++;
					}
					busEffect[j]=busMachine[i];
					busMachine[i]=255;
				}
			}
		}
		for ( i=0;i<64;i++ ) 
		{
			if ((busMachine[i] != 255) && (_machineActive[busEffect[i]]) && (pMac[busMachine[i]]->_mode != MACHMODE_GENERATOR)) 
			{
				busMachine[i] = 255;
			}
			if ((busEffect[i] != 255) && (_machineActive[busEffect[i]]) && (pMac[busEffect[i]]->_mode != MACHMODE_FX)) 
			{
				busEffect[i] = 255;
			}
		}

		bool chunkpresent=false;
		pFile->Read(&chunkpresent,sizeof(chunkpresent)); // Patch 2: VST's Chunk.

		if ( fullopen ) for ( i=0;i<128;i++ ) 
		{
			if (_machineActive[i])
			{
				if ( pMac[i]->_type == MACH_DUMMY ) 
				{
					if (((Dummy*)pMac[i])->wasVST && chunkpresent )
					{
					// Since we don't know if the plugin saved it or not, 
					// we're stuck on letting the loading crash/behave incorrectly.
					// There should be a flag, like in the VST loading Section to be correct.
					::MessageBox(NULL,"Missing or Corrupted VST plug-in has chunk, trying not to crash.", "Loading Error", MB_OK);
					}
				}
				else if (( pMac[i]->_type == MACH_VST ) || 
						( pMac[i]->_type == MACH_VSTFX))
				{
					bool chunkread=false;
					if( chunkpresent )	chunkread=((VSTPlugin*)pMac[i])->LoadChunk(pFile);
					((VSTPlugin*)pMac[i])->SetCurrentProgram(((VSTPlugin*)pMac[i])->_program);
					if ( !chunkpresent || !chunkread )
					{
						const int vi = ((VSTPlugin*)pMac[i])->_instance;
						const int numpars=vstL[vi].numpars;
						for (int c=0; c<numpars; c++)
						{
							((VSTPlugin*)pMac[i])->SetParameter(c, vstL[vi].pars[c]);
						}
					}
				}
			}
		}
		for (i=0; i<MAX_PLUGINS; i++) // Clean "pars" array.
		{
			if( vstL[i].valid )
			{
				delete vstL[i].pars;
			}
		}

		// move machines around to where they really should go
		// now we have to remap all the inputs and outputs again... ouch
#if !defined(_WINAMP_PLUGIN_)
		
		Progress.m_Progress.SetPos(8192+4096+2048+1024);
		::Sleep(1);
#endif

		for (i = 0; i < 64; i++)
		{
			if ((busMachine[i] < MAX_MACHINES-1) && (busMachine[i] > 0))
			{
				if (_machineActive[busMachine[i]])
				{
					if (pMac[busMachine[i]]->_mode == MACHMODE_GENERATOR)
					{
						_pMachine[i] = pMac[busMachine[i]];
						_machineActive[busMachine[i]] = FALSE; // don't update this twice;

						for (int c=0; c<MAX_CONNECTIONS; c++)
						{
							if (_pMachine[i]->_inputCon[c])
							{
								for (int x=0; x<64; x++)
								{
									if (_pMachine[i]->_inputMachines[c] == busMachine[x])
									{
										_pMachine[i]->_inputMachines[c] = x;
										break;
									}
									else if (_pMachine[i]->_inputMachines[c] == busEffect[x])
									{
										_pMachine[i]->_inputMachines[c] = x+MAX_BUSES;
										break;
									}
								}
							}

							if (_pMachine[i]->_connection[c])
							{
								if (_pMachine[i]->_outputMachines[c] == 0)
								{
									_pMachine[i]->_outputMachines[c] = MASTER_INDEX;
								}
								else
								{
									for (int x=0; x<64; x++)
									{
										if (_pMachine[i]->_outputMachines[c] == busMachine[x])
										{
											_pMachine[i]->_outputMachines[c] = x;
											break;
										}
										else if (_pMachine[i]->_outputMachines[c] == busEffect[x])
										{
											_pMachine[i]->_outputMachines[c] = x+MAX_BUSES;
											break;
										}
									}
								}
							}
						}
					}
				}
			}
			if ((busEffect[i] < MAX_MACHINES-1) && (busEffect[i] > 0))
			{
				if (_machineActive[busEffect[i]])
				{
					if (pMac[busEffect[i]]->_mode == MACHMODE_FX)
					{
						_pMachine[i+MAX_BUSES] = pMac[busEffect[i]];
						_machineActive[busEffect[i]] = FALSE; // don't do this again

						for (int c=0; c<MAX_CONNECTIONS; c++)
						{
							if (_pMachine[i+MAX_BUSES]->_inputCon[c])
							{
								for (int x=0; x<64; x++)
								{
									if (_pMachine[i+MAX_BUSES]->_inputMachines[c] == busMachine[x])
									{
										_pMachine[i+MAX_BUSES]->_inputMachines[c] = x;
										break;
									}
									else if (_pMachine[i+MAX_BUSES]->_inputMachines[c] == busEffect[x])
									{
										_pMachine[i+MAX_BUSES]->_inputMachines[c] = x+MAX_BUSES;
										break;
									}
								}
							}
							if (_pMachine[i+MAX_BUSES]->_connection[c])
							{
								if (_pMachine[i+MAX_BUSES]->_outputMachines[c] == 0)
								{
									_pMachine[i+MAX_BUSES]->_outputMachines[c] = MASTER_INDEX;
								}
								else
								{
									for (int x=0; x<64; x++)
									{
										if (_pMachine[i+MAX_BUSES]->_outputMachines[c] == busMachine[x])
										{
											_pMachine[i+MAX_BUSES]->_outputMachines[c] = x;
											break;
										}
										else if (_pMachine[i+MAX_BUSES]->_outputMachines[c] == busEffect[x])
										{
											_pMachine[i+MAX_BUSES]->_outputMachines[c] = x+MAX_BUSES;
											break;
										}
									}
								}
							}
						}
					}
				}
			}
		}

		// fix machine #s

		for (i = 0; i < MAX_MACHINES-1; i++)
		{
			if (_pMachine[i])
			{
				_pMachine[i]->_macIndex = i;
				for (j = i+1; j < MAX_MACHINES-1; j++)
				{
					if (_pMachine[i] == _pMachine[j])
					{
						// we have duplicate machines...
						// this should NEVER happen
						// delete the second one :(
						_pMachine[j] = NULL;
						// and we should remap anything that had wires to it to the first one
					}
				}
			}
		}
#if !defined(_WINAMP_PLUGIN_)
		
		Progress.m_Progress.SetPos(8192+4096+2048+1024+512);
		::Sleep(1);
#endif
		// test all connections

		for (int c=0; c<MAX_CONNECTIONS; c++)
		{
			if (_pMachine[MASTER_INDEX]->_inputCon[c])
			{
				for (int x=0; x<64; x++)
				{
					if (_pMachine[MASTER_INDEX]->_inputMachines[c] == busMachine[x])
					{
						_pMachine[MASTER_INDEX]->_inputMachines[c] = x;
						break;
					}
					else if (_pMachine[MASTER_INDEX]->_inputMachines[c] == busEffect[x])
					{
						_pMachine[MASTER_INDEX]->_inputMachines[c] = x+MAX_BUSES;
						break;
					}
				}
			}
		}
#if !defined(_WINAMP_PLUGIN_)
		
		Progress.m_Progress.SetPos(16384);
		::Sleep(1);
#endif
		// test all connections for invalid machines. disconnect invalid machines.
		for (i = 0; i < MAX_MACHINES; i++)
		{
			if (_pMachine[i])
			{
				_pMachine[i]->_numInputs = 0;
				_pMachine[i]->_numOutputs = 0;

				for (int c = 0; c < MAX_CONNECTIONS; c++)
				{
					if (_pMachine[i]->_connection[c])
					{
						if (_pMachine[i]->_outputMachines[c] < 0 || _pMachine[i]->_outputMachines[c] >= MAX_MACHINES)
						{
							_pMachine[i]->_connection[c]=FALSE;
							_pMachine[i]->_outputMachines[c]=255;
						}
						else if (!_pMachine[_pMachine[i]->_outputMachines[c]])
						{
							_pMachine[i]->_connection[c]=FALSE;
							_pMachine[i]->_outputMachines[c]=255;
						}
						else 
						{
							_pMachine[i]->_numOutputs++;
						}
					}
					else
					{
						_pMachine[i]->_outputMachines[c]=255;
					}

					if (_pMachine[i]->_inputCon[c])
					{
						if (_pMachine[i]->_inputMachines[c] < 0 || _pMachine[i]->_inputMachines[c] >= MAX_MACHINES-1)
						{
							_pMachine[i]->_inputCon[c]=FALSE;
							_pMachine[i]->_inputMachines[c]=255;
						}
						else if (!_pMachine[_pMachine[i]->_inputMachines[c]])
						{
							_pMachine[i]->_inputCon[c]=FALSE;
							_pMachine[i]->_inputMachines[c]=255;
						}
						else
						{
							_pMachine[i]->_numInputs++;
						}
					}
					else
					{
						_pMachine[i]->_inputMachines[c]=255;
					}
				}
			}
		}

#ifdef CONVERT_INTERNAL_MACHINES
		if ( fullopen ) converter.retweak(*this); // conversion
#endif

		_machineLock = false;
		seqBus=0;
#if !defined(_WINAMP_PLUGIN_)
		
		Progress.OnCancel();
#endif
		if (!pFile->Close())
		{
			char error[MAX_PATH];
			sprintf(error,"Error reading from \"%s\"!!!",pFile->szName);
			MessageBox(NULL,error,"File Error!!!",0);
			return false;
		}

		return true;
}



bool Song::LoadPsy3Song(RiffFile* pFile, bool fullopen)
{
	char Header[9];
	Header[8]=0;

#if !defined(_WINAMP_PLUGIN_)
		
		CProgressDialog Progress;
		Progress.Create();
		Progress.SetWindowText("Loading...");
		Progress.ShowWindow(SW_SHOW);
#endif

		UINT version = 0;
		UINT size = 0;
		UINT index = 0;
		int temp;
		int solo;
		int chunkcount;
		int patID = 0;

		Header[4]=0;

		long filesize = pFile->FileSize();

		pFile->Read(&version,sizeof(version));
		pFile->Read(&size,sizeof(size));
		pFile->Read(&chunkcount,sizeof(chunkcount));

		if (version > CURRENT_FILE_VERSION)
		{
			// there is an error, this file is newer than this build of psycle
			MessageBox(NULL,"File is from a newer version of psycle! You should get a new one immediately!",NULL,NULL);
		}
		pFile->Skip(size-sizeof(chunkcount));
		/*
		else
		{
			// there is currently no data in this segment
		}
		*/
		DestroyAllMachines();
		_machineLock = true;
		DeleteInstruments();
		DeleteAllPatterns();

		while(pFile->Read(&Header, 4))
		{
#if !defined(_WINAMP_PLUGIN_)
			Progress.m_Progress.SetPos(f2i((pFile->GetPos()*16384.0f)/filesize));
			::Sleep(1);
#endif
			// we should use the size to update the index, but for now we will skip it
			if (strcmp(Header,"INFO")==0)
			{
				chunkcount--;
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				if (version > CURRENT_FILE_VERSION_INFO)
				{
					// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Info Seqment of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
				else
				{
					pFile->ReadString(Name,sizeof(Name));
					pFile->ReadString(Author,sizeof(Author));
					pFile->ReadString(Comment,sizeof(Comment));
				}
			}
			else if (strcmp(Header,"SNGI")==0)
			{
				chunkcount--;
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				if (version > CURRENT_FILE_VERSION_SNGI)
				{
					// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Song Segment of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
				else
				{
					// why all these temps?  to make sure if someone changes the defs of
					// any of these members, the rest of the file reads ok.  assume 
					// everything is an int, when we write we do the same thing.

					pFile->Read(&temp,sizeof(temp));  // # of tracks for whole song
					pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->SONGTRACKS = temp;
					pFile->Read(&temp,sizeof(temp));  // bpm
					BeatsPerMin = temp;
					pFile->Read(&temp,sizeof(temp));  // tpb
					_ticksPerBeat = temp;
					pFile->Read(&temp,sizeof(temp));  // current octave
					currentOctave = temp;
					pFile->Read(&temp,sizeof(temp));  // machineSoloed
					solo = temp;	// we need to buffer this because destroy machine will clear it

					pFile->Read(&temp,sizeof(temp));  // trackSoloed
					_trackSoloed=temp;

					pFile->Read(&temp,sizeof(temp));  
					seqBus=temp;

					pFile->Read(&temp,sizeof(temp));  
					midiSelected=temp;
					pFile->Read(&temp,sizeof(temp));  
					auxcolSelected=temp;
					pFile->Read(&temp,sizeof(temp));  
					instSelected=temp;

					pFile->Read(&temp,sizeof(temp));  // sequence width, for multipattern

					_trackArmedCount = 0;
					for (int i = 0; i < pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->SONGTRACKS; i++)
					{
						pFile->Read(&_trackMuted[i],sizeof(_trackMuted[i]));
						pFile->Read(&_trackArmed[i],sizeof(_trackArmed[i])); // remember to count them
						if (_trackArmed[i])
						{
							_trackArmedCount++;
						}
					}

					Global::pPlayer->bpm = BeatsPerMin;
					Global::pPlayer->tpb = _ticksPerBeat;
					// calculate samples per tick
		
#if defined(_WINAMP_PLUGIN_)
					SamplesPerTick = (Global::pConfig->_samplesPerSec*15*4)/(Global::pPlayer->bpm*Global::pPlayer->tpb);
#else
					SamplesPerTick = (Global::pConfig->_pOutputDriver->_samplesPerSec*15*4)/(Global::pPlayer->bpm*Global::pPlayer->tpb);
#endif
				}
			}
			else if (strcmp(Header,"SEQD")==0)
			{
				chunkcount--;
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				if (version > CURRENT_FILE_VERSION_SEQD)
				{
					// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Sequence section of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
				else
				{
					pSequencer->machineList.clear();
					pSequencer->CreateMachineSequenceTrack(0, "default");
					pFile->Read(&index,sizeof(index)); // index, for multipattern - for now always 0
					if (index < MAX_SEQUENCES)
					{
						char pTemp[256];
						pFile->Read(&temp,sizeof(temp)); // play length for this sequence
						playLength = temp;

						pFile->ReadString(pTemp,sizeof(pTemp)); // name, for multipattern, for now unused

						for (int i = 0; i < playLength; i++)
						{
							pFile->Read(&temp,sizeof(temp));
							playOrder[i] = temp;
						}
					}
					else
					{
//						MessageBox(NULL,"Sequence section of File is from a newer version of psycle!",NULL,NULL);
						pFile->Skip(size-sizeof(index));
					}
				}
			}
			else if (strcmp(Header,"PATD")==0)
			{
				chunkcount--;
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));

				if (version > CURRENT_FILE_VERSION_PATD)
				{
					// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Pattern section of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
				else
				{
					pFile->Read(&index,sizeof(index)); // index
					if (index < MAX_PATTERNS)
					{
						pFile->Read(&temp,sizeof(temp)); // num lines
						RemovePattern(index); // clear it out if it already exists
						pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->patternLines[index] = temp;
						pFile->Read(&temp,sizeof(temp)); // num tracks per pattern // eventually this may be variable per pattern, like when we get multipattern

						pFile->ReadString(pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->patternName[index],32);

						pFile->Read(&size,sizeof(size));
						byte* pSource = new byte[size];
						pFile->Read(pSource,size);
						byte* pDest;

						BEERZ77Decomp2(pSource, &pDest);
						delete pSource;
						pSource = pDest;

						for (int y = 0; y < pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->patternLines[index]; y++)
						{
							unsigned char* pData = pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->_ppattern(index)+(y*MULTIPLY);
							memcpy(pData,pSource,pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->SONGTRACKS*EVENT_SIZE);
							pSource+= pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->SONGTRACKS*EVENT_SIZE;
						}
						delete pDest;
						// add new pattern and set it name
						char buff[32];
						sprintf(buff,"%X",index);
						strcpy(pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->patternName[index],buff);
						patID++;

					}
					else
					{
//						MessageBox(NULL,"Pattern section of File is from a newer version of psycle!",NULL,NULL);
						pFile->Skip(size-sizeof(index));
					}
				}
			}
			else if (strcmp(Header,"MACD")==0)
			{
				int curpos=0;
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				chunkcount--;
				if ( !fullopen )
				{
					curpos=pFile->GetPos();
				}
				if (version > CURRENT_FILE_VERSION_MACD)
				{
					// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Machine section of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
				else
				{
					pFile->Read(&index,sizeof(index));
					if (index < MAX_MACHINES)
					{
						// we had better load it
						//DestroyMachine(index);
						_pMachine[index] = Machine::LoadFileChunk(pFile,index,version,fullopen);
						if ( index > 0 ) pSequencer->CreateMachineSequenceTrack(index, _pMachine[index]->_editName);
						if ( !fullopen ) pFile->Seek(curpos+size); // skips specific chunk.
					}
					else
					{
//						MessageBox(NULL,"Instrument section of File is from a newer version of psycle!",NULL,NULL);
						pFile->Skip(size-sizeof(index));
					}
				}
			}
			else if (strcmp(Header,"INSD")==0)
			{
				int curpos=0;
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				chunkcount--;
				if (version > CURRENT_FILE_VERSION_INSD)
				{
					// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Instrument section of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
				else
				{
					pFile->Read(&index,sizeof(index));
					if (index < MAX_INSTRUMENTS)
					{
						_pInstrument[index]->LoadFileChunk(pFile,version,fullopen);
					}
					else
					{
//						MessageBox(NULL,"Instrument section of File is from a newer version of psycle!",NULL,NULL);
						pFile->Skip(size-sizeof(index));
					}
				}
			}
			else 
			{
				// we are not at a valid header for some weird reason.  
				// probably there is some extra data.
				// shift back 3 bytes and try again
				pFile->Skip(-3);
			}
		}

		int lines = 0;
		for (int i = 0; i < playLength; i++)
		{
			Sequence * tmpSeq = new Sequence();
			tmpSeq->patternID = playOrder[i];
			pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->pSequences[lines] = tmpSeq;
			lines += pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->patternLines[playOrder[i]];
		}
		pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->patCount = patID;
		pSequencer->endLine = ((lines / 64)+1)*64; // predef length 


		// now that we have loaded all the modules, time to prepare them.
#if !defined(_WINAMP_PLUGIN_)
		
		Progress.m_Progress.SetPos(16384);
		::Sleep(1);
#endif
		// test all connections for invalid machines. disconnect invalid machines.
		for (int i = 0; i < MAX_MACHINES; i++)
		{
			if (_pMachine[i])
			{
				_pMachine[i]->_numInputs = 0;
				_pMachine[i]->_numOutputs = 0;

				for (int c = 0; c < MAX_CONNECTIONS; c++)
				{
					if (_pMachine[i]->_connection[c])
					{
						if (_pMachine[i]->_outputMachines[c] < 0 || _pMachine[i]->_outputMachines[c] >= MAX_MACHINES)
						{
							_pMachine[i]->_connection[c]=FALSE;
							_pMachine[i]->_outputMachines[c]=255;
						}
						else if (!_pMachine[_pMachine[i]->_outputMachines[c]])
						{
							_pMachine[i]->_connection[c]=FALSE;
							_pMachine[i]->_outputMachines[c]=255;
						}
						else 
						{
							_pMachine[i]->_numOutputs++;
						}
					}
					else
					{
						_pMachine[i]->_outputMachines[c]=255;
					}

					if (_pMachine[i]->_inputCon[c])
					{
						if (_pMachine[i]->_inputMachines[c] < 0 || _pMachine[i]->_inputMachines[c] >= MAX_MACHINES)
						{
							_pMachine[i]->_inputCon[c]=FALSE;
							_pMachine[i]->_inputMachines[c]=255;
						}
						else if (!_pMachine[_pMachine[i]->_inputMachines[c]])
						{
							_pMachine[i]->_inputCon[c]=FALSE;
							_pMachine[i]->_inputMachines[c]=255;
						}
						else
						{
							_pMachine[i]->_numInputs++;
						}
					}
					else
					{
						_pMachine[i]->_inputMachines[c]=255;
					}
				}
			}
		}

		// translate any data that is required
#ifndef _WINAMP_PLUGIN_
		((CMainFrame *)theApp.m_pMainWnd)->UpdateComboGen();
		machineSoloed = solo;
#endif
		// allow stuff to work again
		_machineLock = false;

#if !defined(_WINAMP_PLUGIN_)
		Progress.OnCancel();
#endif
		if ((!pFile->Close()) || (chunkcount))
		{
			char error[MAX_PATH];
			sprintf(error,"Error reading from \"%s\"!!!",pFile->szName);
			MessageBox(NULL,error,"File Error!!!",0);
			return false;
		}
		
		return true;
}

bool Song::LoadPsy4Song(RiffFile* pFile, bool fullopen)
{

#if !defined(_WINAMP_PLUGIN_)
		
		CProgressDialog Progress;
		Progress.Create();
		Progress.SetWindowText("Loading...");
		Progress.ShowWindow(SW_SHOW);
#endif

		UINT version = 0;
		UINT size = 0;
		UINT index = 0;
		int temp;
		int solo;
		int chunkcount;
		char Header[9];

		Header[4]=0;

		long filesize = pFile->FileSize();

		pFile->Read(&version,sizeof(version));
		pFile->Read(&size,sizeof(size));
		pFile->Read(&chunkcount,sizeof(chunkcount));

		if (version > CURRENT_FILE_VERSION)
		{
			// there is an error, this file is newer than this build of psycle
			MessageBox(NULL,"File is from a newer version of psycle! You should get a new one immediately!",NULL,NULL);
		}
		pFile->Skip(size-sizeof(chunkcount));
		/*
		else
		{
			// there is currently no data in this segment
		}
		*/
		DestroyAllMachines();
		_machineLock = true;
		DeleteInstruments();
		DeleteAllPatterns();

		while(pFile->Read(&Header, 4))
		{
#if !defined(_WINAMP_PLUGIN_)
			Progress.m_Progress.SetPos(f2i((pFile->GetPos()*16384.0f)/filesize));
			::Sleep(1);
#endif
			// we should use the size to update the index, but for now we will skip it
			if (strcmp(Header,"INFO")==0)
			{
				chunkcount--;
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				if (version > CURRENT_FILE_VERSION_INFO)
				{
					// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Info Seqment of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
				else
				{
					pFile->ReadString(Name,sizeof(Name));
					pFile->ReadString(Author,sizeof(Author));
					pFile->ReadString(Comment,sizeof(Comment));
				}
			}
			else if (strcmp(Header,"SNGI")==0)
			{
				chunkcount--;
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				if (version > CURRENT_FILE_VERSION_SNGI)
				{
					// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Song Segment of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
				else
				{
					// why all these temps?  to make sure if someone changes the defs of
					// any of these members, the rest of the file reads ok.  assume 
					// everything is an int, when we write we do the same thing.

					pFile->Read(&temp,sizeof(temp));  // pat step 
					patStep = temp;
					pFile->Read(&temp,sizeof(temp));  // # of tracks for whole song
					pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->SONGTRACKS = temp;
					pFile->Read(&temp,sizeof(temp));  // bpm
					BeatsPerMin = temp;
					pFile->Read(&temp,sizeof(temp));  // tpb
					_ticksPerBeat = temp;
					pFile->Read(&temp,sizeof(temp));  // current octave
					currentOctave = temp;
					pFile->Read(&temp,sizeof(temp));  // machineSoloed
					solo = temp;	// we need to buffer this because destroy machine will clear it

					pFile->Read(&temp,sizeof(temp));  // trackSoloed
					_trackSoloed=temp;

					pFile->Read(&temp,sizeof(temp));  
					seqBus=temp;

					pFile->Read(&temp,sizeof(temp));  
					midiSelected=temp;
					pFile->Read(&temp,sizeof(temp));  
					auxcolSelected=temp;
					pFile->Read(&temp,sizeof(temp));  
					instSelected=temp;

					pFile->Read(&temp,sizeof(temp));  // sequence width, for multipattern

					_trackArmedCount = 0;
					for (int i = 0; i < MAX_TRACKS; i++)
					{
						pFile->Read(&_trackMuted[i],sizeof(_trackMuted[i]));
						pFile->Read(&_trackArmed[i],sizeof(_trackArmed[i])); // remember to count them
						if (_trackArmed[i])
						{
							_trackArmedCount++;
						}
					}

					Global::pPlayer->bpm = BeatsPerMin;
					Global::pPlayer->tpb = _ticksPerBeat;
					// calculate samples per tick
		
#if defined(_WINAMP_PLUGIN_)
					SamplesPerTick = (Global::pConfig->_samplesPerSec*15*4)/(Global::pPlayer->bpm*Global::pPlayer->tpb);
#else
					SamplesPerTick = (Global::pConfig->_pOutputDriver->_samplesPerSec*15*4)/(Global::pPlayer->bpm*Global::pPlayer->tpb);
#endif
				}
			}
			else if (strcmp(Header,"SEQD")==0)
			{
				chunkcount--;
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				if (version > CURRENT_FILE_VERSION_SEQD)
				{
					// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Sequence section of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
				else
				{
					pFile->Read(&index,sizeof(index)); // index, for multipattern - for now always 0
					if (index < MAX_SEQUENCES)
					{
						char pTemp[256];
						pFile->Read(&temp,sizeof(temp)); // start loop pos 
						pSequencer->startLine = temp;
						pFile->Read(&temp,sizeof(temp)); // end loop pos
						pSequencer->endLine = temp;
						pFile->Read(&temp,sizeof(temp)); 
						unsigned int seqCount = temp;
						pFile->Read(&temp,sizeof(temp)); 
						unsigned int trackCount = temp;
						pFile->ReadString(pTemp,sizeof(pTemp)); // name, for multipattern, for now unused

						pSequencer->machineList.clear();
						for ( unsigned int track = 0; track < trackCount; track++ )
						{
							pFile->Read(&temp,sizeof(temp));
							int trackID = temp;
							pSequencer->CreateMachineSequenceTrack(trackID, "default");
							// track pattern colour 
							pFile->Read(&temp,sizeof(temp));
							pSequencer->pSequenceData[trackID]->patColor = temp;
							// track name
							char trName[32];
							pFile->Read(trName,32);
							strcpy(pSequencer->pSequenceData[trackID]->trackName, trName);

						}
						for ( unsigned int seq = 0; seq < seqCount; seq++ )
						{
							pFile->Read(&temp,sizeof(temp));
							int row = temp;
							pFile->Read(&temp,sizeof(temp));
							int col = temp;
							pFile->Read(&temp,sizeof(temp));
							int patID = temp;
							pSequencer->pSequenceData[col]->pSequences[row]->patternID = patID;
						}
					}
					else
					{
//						MessageBox(NULL,"Sequence section of File is from a newer version of psycle!",NULL,NULL);
						pFile->Skip(size-sizeof(index));
					}
				}
			}
			else if (strcmp(Header,"PATD")==0)
			{
				chunkcount--;
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				if (version > CURRENT_FILE_VERSION_PATD)
				{
					// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Pattern section of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
				else
				{
					pFile->Read(&temp,sizeof(temp)); // num tracks
					unsigned int trackCount = temp;

					for ( unsigned int track = 0; track < trackCount; track++ )
					{
						pFile->Read(&temp,sizeof(temp)); // num patterns in track
						int patCount = temp;
						pFile->Read(&temp,sizeof(temp)); // trackID in machine list 
						int trackID = temp;	
						pSequencer->pSequenceData[trackID]->patCount = patCount;

						for (int i = 0; i < patCount; i++)
						{
							pFile->Read(&index,sizeof(index)); // index
							if (index < MAX_PATTERNS)
							{
								pFile->Read(&temp,sizeof(temp)); // num lines
								//RemovePattern(index); // clear it out if it already exists
								pSequencer->pSequenceData[trackID]->patternLines[index] = temp;
								pFile->Read(&temp,sizeof(temp)); // num tracks per pattern // eventually this may be variable per pattern, like when we get multipattern
								pSequencer->pSequenceData[trackID]->SONGTRACKS = temp;

								pFile->ReadString(pSequencer->pSequenceData[trackID]->patternName[index], 
									sizeof(pSequencer->pSequenceData[trackID]->patternName[index]));

								pFile->Read(&size,sizeof(size));
								byte* pSource = new byte[size];
								pFile->Read(pSource,size);
								byte* pDest;

								BEERZ77Decomp2(pSource, &pDest);
								delete pSource;
								pSource = pDest;

								for (int y = 0; y < pSequencer->pSequenceData[trackID]->patternLines[index]; y++)
								{
									unsigned char* pData = pSequencer->pSequenceData[trackID]->_ppattern(index)+(y*MULTIPLY);
									memcpy(pData,pSource,pSequencer->pSequenceData[trackID]->SONGTRACKS*EVENT_SIZE);
									pSource += pSequencer->pSequenceData[trackID]->SONGTRACKS*EVENT_SIZE;
								}
								delete pDest;
							}
							else
							{
		//						MessageBox(NULL,"Pattern section of File is from a newer version of psycle!",NULL,NULL);
								pFile->Skip(size-sizeof(index));
							}
						}
					}
				}
			}
			else if (strcmp(Header,"MACD")==0)
			{
				int curpos=0;
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				chunkcount--;
				if ( !fullopen )
				{
					curpos=pFile->GetPos();
				}
				if (version > CURRENT_FILE_VERSION_MACD)
				{
					// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Machine section of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
				else
				{
					pFile->Read(&index,sizeof(index));
					if (index < MAX_MACHINES)
					{
						// we had better load it
						//DestroyMachine(index);
						_pMachine[index] = Machine::LoadFileChunk(pFile,index,version,fullopen);
						if ( !fullopen ) pFile->Seek(curpos+size); // skips specific chunk.
					}
					else
					{
//						MessageBox(NULL,"Instrument section of File is from a newer version of psycle!",NULL,NULL);
						pFile->Skip(size-sizeof(index));
					}
				}
			}
			else if (strcmp(Header,"INSD")==0)
			{
				int curpos=0;
				pFile->Read(&version,sizeof(version));
				pFile->Read(&size,sizeof(size));
				chunkcount--;
				if (version > CURRENT_FILE_VERSION_INSD)
				{
					// there is an error, this file is newer than this build of psycle
//					MessageBox(NULL,"Instrument section of File is from a newer version of psycle!",NULL,NULL);
					pFile->Skip(size);
				}
				else
				{
					pFile->Read(&index,sizeof(index));
					if (index < MAX_INSTRUMENTS)
					{
						_pInstrument[index]->LoadFileChunk(pFile,version,fullopen);
					}
					else
					{
//						MessageBox(NULL,"Instrument section of File is from a newer version of psycle!",NULL,NULL);
						pFile->Skip(size-sizeof(index));
					}
				}
			}
			else 
			{
				// we are not at a valid header for some weird reason.  
				// probably there is some extra data.
				// shift back 3 bytes and try again
				pFile->Skip(-3);
			}
		}
		// now that we have loaded all the modules, time to prepare them.
#if !defined(_WINAMP_PLUGIN_)
		
		Progress.m_Progress.SetPos(16384);
		::Sleep(1);
#endif
		// test all connections for invalid machines. disconnect invalid machines.
		for (int i = 0; i < MAX_MACHINES; i++)
		{
			if (_pMachine[i])
			{
				_pMachine[i]->_numInputs = 0;
				_pMachine[i]->_numOutputs = 0;

				for (int c = 0; c < MAX_CONNECTIONS; c++)
				{
					if (_pMachine[i]->_connection[c])
					{
						if (_pMachine[i]->_outputMachines[c] < 0 || _pMachine[i]->_outputMachines[c] >= MAX_MACHINES)
						{
							_pMachine[i]->_connection[c]=FALSE;
							_pMachine[i]->_outputMachines[c]=255;
						}
						else if (!_pMachine[_pMachine[i]->_outputMachines[c]])
						{
							_pMachine[i]->_connection[c]=FALSE;
							_pMachine[i]->_outputMachines[c]=255;
						}
						else 
						{
							_pMachine[i]->_numOutputs++;
						}
					}
					else
					{
						_pMachine[i]->_outputMachines[c]=255;
					}

					if (_pMachine[i]->_inputCon[c])
					{
						if (_pMachine[i]->_inputMachines[c] < 0 || _pMachine[i]->_inputMachines[c] >= MAX_MACHINES)
						{
							_pMachine[i]->_inputCon[c]=FALSE;
							_pMachine[i]->_inputMachines[c]=255;
						}
						else if (!_pMachine[_pMachine[i]->_inputMachines[c]])
						{
							_pMachine[i]->_inputCon[c]=FALSE;
							_pMachine[i]->_inputMachines[c]=255;
						}
						else
						{
							_pMachine[i]->_numInputs++;
						}
					}
					else
					{
						_pMachine[i]->_inputMachines[c]=255;
					}
				}
			}
		}

		// translate any data that is required
#ifndef _WINAMP_PLUGIN_
		((CMainFrame *)theApp.m_pMainWnd)->UpdateComboGen();
		machineSoloed = solo;
#endif
		// allow stuff to work again
		_machineLock = false;

#if !defined(_WINAMP_PLUGIN_)
		Progress.OnCancel();
#endif
		if ((!pFile->Close()) || (chunkcount))
		{
			char error[MAX_PATH];
			sprintf(error,"Error reading from \"%s\"!!!",pFile->szName);
			MessageBox(NULL,error,"File Error!!!",0);
			return false;
		}

		return true;
}



// for the next song file changes 
bool Song::LoadPsy5Song(RiffFile* pFile, bool fullopen)
{
	return false;
}
bool Song::LoadPsy6Song(RiffFile* pFile, bool fullopen)
{
	return false;
}
bool Song::LoadPsy7Song(RiffFile* pFile, bool fullopen)
{
	return false;
}
bool Song::LoadPsy8Song(RiffFile* pFile, bool fullopen)
{
	return false;
}
bool Song::LoadPsy9Song(RiffFile* pFile, bool fullopen)
{
	return false;
}


bool Song::Load(RiffFile* pFile, bool fullopen)
{
	char Header[9];
	pFile->Read(&Header, 8);
	Header[8]=0;
	bool ret = false;

	if (strcmp(Header,"PSY2SONG")==0)
	{
		ret = LoadPsy2Song(pFile,fullopen);
	}
	else if (strcmp(Header,"PSY3SONG")==0)
	{
		ret = LoadPsy3Song(pFile,fullopen);
	}
	else if (strcmp(Header,"PSY4SONG")==0)
	{
		ret = LoadPsy4Song(pFile,fullopen);
	}

	// load did not work
	if ( ret == false ) 
	{
		MessageBox(NULL,"Incorrect file format","Error",MB_OK);
		return false;
	}
	else return true;
}


#if !defined(_WINAMP_PLUGIN_)
bool Song::Save(RiffFile* pFile,bool autosave)
{
	// NEW FILE FORMAT!!!
	// this is much more flexible, making maintenance a breeze compared to that old hell.
	// now you can just update one module without breaking the whole thing.

	// header, this has to be at the top of the file

	CProgressDialog Progress;
	if ( !autosave ) 
	{
		Progress.Create();
		Progress.SetWindowText("Saving...");
		Progress.ShowWindow(SW_SHOW);
	}

	int chunkcount = 3; // 3 chunks plus:
	chunkcount++; // patterns!

	for (int i = 0; i < MAX_MACHINES; i++)
	{
		// check every pattern for validity
		if (_pMachine[i])
		{
			chunkcount++;
		}
	}

	for (i = 0; i < MAX_INSTRUMENTS; i++)
	{
		if (!_pInstrument[i]->Empty())
		{
			chunkcount++;
		}
	}

	if ( !autosave ) 
	{
		Progress.m_Progress.SetRange(0,chunkcount);
		Progress.m_Progress.SetStep(1);
	}

	/*
	===================
	FILE HEADER
	===================
	id = "PSY3SONG"; // PSY2 was 1.66
	NEW PSY4SONG FILE - KRISSCE
	*/

	pFile->Write("PSY4SONG", 8);

	UINT version = CURRENT_FILE_VERSION;
	UINT size = sizeof(chunkcount);
	UINT index = 0;
	int temp;

	pFile->Write(&version,sizeof(version));
	pFile->Write(&size,sizeof(size));
	pFile->Write(&chunkcount,sizeof(chunkcount));

	if ( !autosave ) 
	{
		Progress.m_Progress.StepIt();
		::Sleep(1);
	}

	// the rest of the modules can be arranged in any order

	/*
	===================
	SONG INFO TEXT
	===================
	id = "INFO"; 
	*/

	pFile->Write("INFO",4);
	version = CURRENT_FILE_VERSION_INFO;
	size = strlen(Name)+strlen(Author)+strlen(Comment)+3;
	pFile->Write(&version,sizeof(version));
	pFile->Write(&size,sizeof(size));

	pFile->Write(&Name,strlen(Name)+1);
	pFile->Write(&Author,strlen(Author)+1);
	pFile->Write(&Comment,strlen(Comment)+1);

	if ( !autosave ) 
	{
		Progress.m_Progress.StepIt();
		::Sleep(1);
	}

	/*
	===================
	SONG INFO
	===================
	id = "SNGI"; 
	*/

	pFile->Write("SNGI",4);
	version = CURRENT_FILE_VERSION_SNGI;
	size = (6*sizeof(temp));
	pFile->Write(&version,sizeof(version));
	pFile->Write(&size,sizeof(size));

	
	// save pattern view step
	temp = patStep;
	pFile->Write(&temp,sizeof(temp));

	temp = pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->SONGTRACKS;
	pFile->Write(&temp,sizeof(temp));
	temp = BeatsPerMin;
	pFile->Write(&temp,sizeof(temp));
	temp = _ticksPerBeat;
	pFile->Write(&temp,sizeof(temp));
	temp = currentOctave;
	pFile->Write(&temp,sizeof(temp));
	temp = machineSoloed;
	pFile->Write(&temp,sizeof(temp));
	temp = _trackSoloed;
	pFile->Write(&temp,sizeof(temp));

	temp = seqBus;
	pFile->Write(&temp,sizeof(temp));

	temp = midiSelected;
	pFile->Write(&temp,sizeof(temp));
	temp = auxcolSelected;
	pFile->Write(&temp,sizeof(temp));
	temp = instSelected;
	pFile->Write(&temp,sizeof(temp));

	temp = 1; // sequence width
	pFile->Write(&temp,sizeof(temp));

	for (i = 0; i < MAX_TRACKS; i++)
	{
		pFile->Write(&_trackMuted[i],sizeof(_trackMuted[i]));
		pFile->Write(&_trackArmed[i],sizeof(_trackArmed[i])); // remember to count them
	}

	if ( !autosave ) 
	{
		Progress.m_Progress.StepIt();
		::Sleep(1);
	}

	/*
	===================
	SEQUENCE DATA
	===================
	id = "SEQD"; 
	*/

	int seqCount = 0;
	for ( unsigned int mach = 0; mach < pSequencer->machineList.size(); mach++ )
	{
		int trackID = pSequencer->machineList.at(mach);
		for (i = 0; i < MAX_SEQUENCES; i++)
		{
			if (pSequencer->pSequenceData[trackID]->pSequences[i]->patternID != -1) seqCount++;
		}
	}

	char* pSequenceName = "seq0\0";

	pFile->Write("SEQD",4);
	version = CURRENT_FILE_VERSION_SEQD;
	size = ((seqCount*3+4)*sizeof(temp))+strlen(pSequenceName)+1; // seqCount*3 = row,col,patID
	pFile->Write(&version,sizeof(version));
	pFile->Write(&size,sizeof(size));

	index = 0; // index
	pFile->Write(&index,sizeof(index));
	temp = pSequencer->startLine;
	pFile->Write(&temp,sizeof(temp));
	temp = pSequencer->endLine;
	pFile->Write(&temp,sizeof(temp));
	temp = seqCount;
	pFile->Write(&temp,sizeof(temp));
	temp = pSequencer->machineList.size();// nuber of tracks 
	pFile->Write(&temp,sizeof(temp));
	pFile->Write(pSequenceName,strlen(pSequenceName)+1);

	for ( unsigned int mach = 0; mach < pSequencer->machineList.size(); mach++ )
	{
		int trackID = pSequencer->machineList.at(mach);
		pFile->Write(&trackID,sizeof(trackID));
		pFile->Write(&pSequencer->pSequenceData[trackID]->patColor,sizeof(COLORREF));
		pFile->Write(&pSequencer->pSequenceData[trackID]->trackName,32);
	}

	for ( unsigned int mach = 0; mach < pSequencer->machineList.size(); mach++ )
	{
		int trackID = pSequencer->machineList.at(mach);
		for (unsigned int row = 0; row < MAX_SEQUENCES; row++)
		{
			if (pSequencer->pSequenceData[trackID]->pSequences[row]->patternID != -1) 
			{
				pFile->Write(&row,sizeof(row));
				pFile->Write(&trackID,sizeof(trackID));
				temp = pSequencer->pSequenceData[trackID]->pSequences[row]->patternID;
				pFile->Write(&temp,sizeof(temp));
			}
		}
	}

	if ( !autosave ) 
	{
		Progress.m_Progress.StepIt();
		::Sleep(1);
	}

	/*
	===================
	PATTERN DATA
	===================
	id = "PATD"; 
	*/
	pFile->Write("PATD",4);
	version = CURRENT_FILE_VERSION_PATD;

	pFile->Write(&version,sizeof(version));
	pFile->Write(&size,sizeof(size));

	temp = pSequencer->machineList.size(); // saving number of tracks
	pFile->Write(&temp,sizeof(temp));

	for ( unsigned int mach = 0; mach < pSequencer->machineList.size(); mach++ )
	{
		int trackID = pSequencer->machineList.at(mach);
		temp = pSequencer->pSequenceData[trackID]->patCount; // saving number of patterns per track
		pFile->Write(&temp,sizeof(temp));
		temp = trackID; // saving ID number of track
		pFile->Write(&temp,sizeof(temp));

		for (i = 0; i < pSequencer->pSequenceData[trackID]->patCount; i++)
		{
			// ok save it
			byte* pSource = new byte[pSequencer->pSequenceData[trackID]->SONGTRACKS * 
				pSequencer->pSequenceData[trackID]->patternLines[i] * EVENT_SIZE];
			byte* pCopy = pSource;

			for (int y = 0; y < pSequencer->pSequenceData[trackID]->patternLines[i]; y++)
			{
				unsigned char* pData = pSequencer->pSequenceData[trackID]->ppPatternData[i]+(y*MULTIPLY);
				memcpy(pCopy,pData,EVENT_SIZE * pSequencer->pSequenceData[trackID]->SONGTRACKS);
				pCopy += EVENT_SIZE * pSequencer->pSequenceData[trackID]->SONGTRACKS;
			}

			size = BEERZ77Comp2(pSource, &pCopy, pSequencer->pSequenceData[trackID]->SONGTRACKS * 
				pSequencer->pSequenceData[trackID]->patternLines[i]*EVENT_SIZE) + 
				(3*sizeof(temp))+strlen(pSequencer->pSequenceData[trackID]->patternName[i])+1;
			delete pSource;

			index = i; // index
			pFile->Write(&index,sizeof(index));
			temp = pSequencer->pSequenceData[trackID]->patternLines[i];
			pFile->Write(&temp,sizeof(temp));
			temp = pSequencer->pSequenceData[trackID]->SONGTRACKS; // eventually this may be variable per pattern
			pFile->Write(&temp,sizeof(temp));

			pFile->Write(&pSequencer->pSequenceData[trackID]->patternName[i],
				strlen(pSequencer->pSequenceData[trackID]->patternName[i])+1);

			size -= (3*sizeof(temp)) + strlen(pSequencer->pSequenceData[trackID]->patternName[i])+1;
			pFile->Write(&size,sizeof(size));
			pFile->Write(pCopy,size);
			delete pCopy;

			if ( !autosave ) 
			{
				Progress.m_Progress.StepIt();
				::Sleep(1);
			}
		}
	}
	// machine and instruments handle their save and load in their respective classes

	for (i = 0; i < MAX_MACHINES; i++)
	{
		if (_pMachine[i])
		{
			pFile->Write("MACD",4);
			version = CURRENT_FILE_VERSION_MACD;
			pFile->Write(&version,sizeof(version));
			long pos = pFile->GetPos();
			size = 0;
			pFile->Write(&size,sizeof(size));

			index = i; // index
			pFile->Write(&index,sizeof(index));

			_pMachine[i]->SaveFileChunk(pFile);

			long pos2 = pFile->GetPos(); 
			size = pos2-pos-sizeof(size);
			pFile->Seek(pos);
			pFile->Write(&size,sizeof(size));
			pFile->Seek(pos2);

			if ( !autosave ) 
			{
				Progress.m_Progress.StepIt();
				::Sleep(1);
			}
		}
	}

	for (i = 0; i < MAX_INSTRUMENTS; i++)
	{
		if (!_pInstrument[i]->Empty())
		{
			pFile->Write("INSD",4);
			version = CURRENT_FILE_VERSION_INSD;
			pFile->Write(&version,sizeof(version));
			long pos = pFile->GetPos();
			size = 0;
			pFile->Write(&size,sizeof(size));

			index = i; // index
			pFile->Write(&index,sizeof(index));

			_pInstrument[i]->SaveFileChunk(pFile);

			long pos2 = pFile->GetPos(); 
			size = pos2-pos-sizeof(size);
			pFile->Seek(pos);
			pFile->Write(&size,sizeof(size));
			pFile->Seek(pos2);

			if ( !autosave ) 
			{
				Progress.m_Progress.StepIt();
				::Sleep(1);
			}
		}
	}

	if ( !autosave ) 
	{
		Progress.m_Progress.SetPos(chunkcount);
		::Sleep(1);

		Progress.OnCancel();
	}
	
	if (!pFile->Close())
	{
		char error[MAX_PATH];
		sprintf(error,"Error writing to \"%s\"!!!",pFile->szName);
		MessageBox(NULL,error,"File Error!!!",0);
		return false;
	}

	return true;
}

void Song::PW_Play()
{
	if (PW_Stage==0)
	{
		PW_Length=_pInstrument[PREV_WAV_INS]->waveLength[0];
		if (PW_Length>0 )
		{
			PW_Stage=1;
			PW_Phase=0;
		}
	}
}

void Song::PW_Work(float *pInSamplesL, float *pInSamplesR, int numSamples)
{
	float *pSamplesL = pInSamplesL;
	float *pSamplesR = pInSamplesR;
	--pSamplesL;
	--pSamplesR;
	
	signed short *wl=_pInstrument[PREV_WAV_INS]->waveDataL[0];
	signed short *wr=_pInstrument[PREV_WAV_INS]->waveDataR[0];
	bool const stereo=_pInstrument[PREV_WAV_INS]->waveStereo[0];
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

bool Song::CloneMac(int src,int dst)
{
	// src has to be occupied and dst must be empty
	if (_pMachine[src] && _pMachine[dst])
	{
		return false;
	}
	if (_pMachine[dst])
	{
		int temp = src;
		src = dst;
		dst = temp;
	}
	if (!_pMachine[src])
	{
		return false;
	}
	// check to see both are same type
	if (((dst < MAX_BUSES) && (src >= MAX_BUSES))
		|| ((dst >= MAX_BUSES) && (src < MAX_BUSES)))
	{
		return false;
	}

	if ((src >= MAX_MACHINES-1) || (dst >= MAX_MACHINES-1))
	{
		return false;
	}

	// save our file
	((CMainFrame *)theApp.m_pMainWnd)->m_wndView.AddMacViewUndo();

	CString filepath = Global::pConfig->GetInitialSongDir();
	filepath += "\\psycle.tmp";
	DeleteFile(filepath);
	OldPsyFile file;
	if (!file.Create(filepath.GetBuffer(1), true))
	{
		return false;
	}

	file.Write("MACD",4);
	UINT version = CURRENT_FILE_VERSION_MACD;
	file.Write(&version,sizeof(version));
	long pos = file.GetPos();
	UINT size = 0;
	file.Write(&size,sizeof(size));

	int index = dst; // index
	file.Write(&index,sizeof(index));

	_pMachine[src]->SaveFileChunk(&file);

	long pos2 = file.GetPos(); 
	size = pos2-pos-sizeof(size);
	file.Seek(pos);
	file.Write(&size,sizeof(size));
	file.Close();

	// now load it

	if (!file.Open(filepath.GetBuffer(1)))
	{
		DeleteFile(filepath);
		return false;
	}
	char Header[5];
	file.Read(&Header, 4);
	Header[4] = 0;
	if (strcmp(Header,"MACD")==0)
	{
		file.Read(&version,sizeof(version));
		file.Read(&size,sizeof(size));
		if (version > CURRENT_FILE_VERSION_MACD)
		{
			// there is an error, this file is newer than this build of psycle
			file.Close();
			DeleteFile(filepath);
			return false;
		}
		else
		{
			file.Read(&index,sizeof(index));
			index = dst;
			if (index < MAX_MACHINES)
			{
				// we had better load it
				DestroyMachine(index);
				_pMachine[index] = Machine::LoadFileChunk(&file,index,version);
			}
			else
			{
				file.Close();
				DeleteFile(filepath);
				return false;
			}
		}
	}
	else
	{
		file.Close();
		DeleteFile(filepath);
		return false;
	}
	file.Close();
	DeleteFile(filepath);

	// oh and randomize the dst's position

	int xs,ys,x,y;
	if (src >= MAX_BUSES)
	{
		xs = ((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.width;
		ys = ((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sEffect.height;
	}
	else 
	{
		xs = ((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.width;
		ys = ((CMainFrame *)theApp.m_pMainWnd)->m_wndView.MachineCoords.sGenerator.height;
	}
	x=_pMachine[dst]->_x+32;
	y=_pMachine[dst]->_y+ys+8;

	bool bCovered = TRUE;
	while (bCovered)
	{
		bCovered = FALSE;
		for (int i=0; i < MAX_MACHINES; i++)
		{
			if (i != dst)
			{
				if (_pMachine[i])
				{
					if ((abs(_pMachine[i]->_x - x) < 32) &&
						(abs(_pMachine[i]->_y - y) < 32))
					{
						bCovered = TRUE;
						i = MAX_MACHINES;
						x = (rand())%(((CMainFrame *)theApp.m_pMainWnd)->m_wndView.CW-xs);
						y = (rand())%(((CMainFrame *)theApp.m_pMainWnd)->m_wndView.CH-ys);
					}
				}
			}
		}
	}
	_pMachine[dst]->_x = x;
	_pMachine[dst]->_y = y;

	// oh and delete all connections

	_pMachine[dst]->_numInputs = 0;
	_pMachine[dst]->_numOutputs = 0;

	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		if (_pMachine[dst]->_connection[i])
		{
			_pMachine[dst]->_connection[i] = false;
			_pMachine[dst]->_outputMachines[i] = 255;
		}

		if (_pMachine[dst]->_inputCon[i])
		{
			_pMachine[dst]->_inputCon[i] = false;
			_pMachine[dst]->_inputMachines[i] = 255;
		}
	}

	int number = 1;
	char buf[sizeof(_pMachine[dst]->_editName)+4];
	strcpy (buf,_pMachine[dst]->_editName);
	char* ps = strrchr(buf,' ');
	if (ps)
	{
		number = atoi(ps);
		if (number < 1)
		{
			number =1;
		}
		else
		{
			ps[0] = 0;
			ps = strchr(_pMachine[dst]->_editName,' ');
			ps[0] = 0;
		}
	}

	for (i = 0; i < MAX_MACHINES-1; i++)
	{
		if (i!=dst)
		{
			if (_pMachine[i])
			{
				if (strcmp(_pMachine[i]->_editName,buf)==0)
				{
					number++;
					sprintf(buf,"%s %d",_pMachine[dst]->_editName,number);
					i = -1;
				}
			}
		}
	}

	buf[sizeof(_pMachine[dst]->_editName)-1] = 0;
	strcpy(_pMachine[dst]->_editName,buf);

	return true;
}


bool Song::CloneIns(int src,int dst)
{
	// src has to be occupied and dst must be empty
	if (!Global::_pSong->_pInstrument[src]->Empty() && !Global::_pSong->_pInstrument[dst]->Empty())
	{
		return false;
	}
	if (!Global::_pSong->_pInstrument[dst]->Empty())
	{
		int temp = src;
		src = dst;
		dst = temp;
	}
	if (Global::_pSong->_pInstrument[src]->Empty())
	{
		return false;
	}
	// ok now we get down to business

	((CMainFrame *)theApp.m_pMainWnd)->m_wndView.AddMacViewUndo();

	// save our file

	CString filepath = Global::pConfig->GetInitialSongDir();
	filepath += "\\psycle.tmp";
	DeleteFile(filepath);
	OldPsyFile file;
	if (!file.Create(filepath.GetBuffer(1), true))
	{
		return false;
	}

	file.Write("INSD",4);
	UINT version = CURRENT_FILE_VERSION_INSD;
	file.Write(&version,sizeof(version));
	long pos = file.GetPos();
	UINT size = 0;
	file.Write(&size,sizeof(size));

	int index = dst; // index
	file.Write(&index,sizeof(index));

	_pInstrument[src]->SaveFileChunk(&file);

	long pos2 = file.GetPos(); 
	size = pos2-pos-sizeof(size);
	file.Seek(pos);
	file.Write(&size,sizeof(size));

	file.Close();

	// now load it

	if (!file.Open(filepath.GetBuffer(1)))
	{
		DeleteFile(filepath);
		return false;
	}
	char Header[5];
	file.Read(&Header, 4);
	Header[4] = 0;

	if (strcmp(Header,"INSD")==0)
	{
		file.Read(&version,sizeof(version));
		file.Read(&size,sizeof(size));
		if (version > CURRENT_FILE_VERSION_INSD)
		{
			// there is an error, this file is newer than this build of psycle
			file.Close();
			DeleteFile(filepath);
			return false;
		}
		else
		{
			file.Read(&index,sizeof(index));
			index = dst;
			if (index < MAX_INSTRUMENTS)
			{
				// we had better load it
				_pInstrument[index]->LoadFileChunk(&file,version);
			}
			else
			{
				file.Close();
				DeleteFile(filepath);
				return false;
			}
		}
	}
	else
	{
		file.Close();
		DeleteFile(filepath);
		return false;
	}
	file.Close();
	DeleteFile(filepath);
	return true;
}

bool Song::IsPatternUsed(int i)
{
	return pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->IsPatternUsed(i);
}


#endif // ndef _WINAMP_PLUGIN_

