#include "stdafx.h"
#if defined(_WINAMP_PLUGIN_)
	#include "global.h"
#else
	#include "psycle2.h"
	#include "NewMachine.h"
#endif // _WINAMP_PLUGIN_

#include "Song.h"
#include "Machine.h" // It wouldn't be needed, since it is already included in "song.h"
#include "Sampler.h"
#include "Plugin.h"
#include "VSTHost.h"

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
	char* psPluginDll)
{

	Machine* pMachine;
	Master* pMaster;
	Sine* pSine;
	Distortion* pDistortion;
	Sampler* pSampler;
	Delay* pDelay;
	Filter2p* pFilter;
	Gainer* pGainer;
	Flanger* pFlanger;
	Plugin* pPlugin;
	VSTPlugin* pVstPlugin;

	switch (type)
	{
	case MACH_MASTER:
		if (_machineActive[0])
		{
			return false;
		}
		pMachine = pMaster = new Master;
		break;
	case MACH_SINE:
		pMachine = pSine = new Sine;
		break;
	case MACH_DIST:
		pMachine = pDistortion = new Distortion;
		break;
	case MACH_SAMPLER:
		pMachine = pSampler = new Sampler;
		break;
	case MACH_DELAY:
		pMachine = pDelay = new Delay;
		break;
	case MACH_2PFILTER:
		pMachine = pFilter = new Filter2p;
		break;
	case MACH_GAIN:
		pMachine = pGainer = new Gainer;
		break;
	case MACH_FLANGER:
		pMachine = pFlanger = new Flanger;
		break;
	case MACH_PLUGIN:
		{
		pMachine = pPlugin = new Plugin;
		if (!pPlugin->Instance(psPluginDll))
		{
			delete pMachine; return false;
		}
		break;
		}
	case MACH_VST:
		{
		pMachine = pVstPlugin = new VSTInstrument;
		if (pVstPlugin->Instance(psPluginDll) != VSTINSTANCE_NO_ERROR)
		{
			delete pMachine; return false;
		}
		break;
		}
	case MACH_VSTFX:
		{
		pMachine = pVstPlugin = new VSTFX;
		if (pVstPlugin->Instance(psPluginDll) != VSTINSTANCE_NO_ERROR)
		{
			delete pMachine; return false;
		}
		break;
		}

	case MACH_DUMMY:
		pMachine = new Dummy;
		break;
	case MACH_SCOPE:
		MessageBox(0,"DELETE THE SCOPE MACHINE BEFORE YOU SAVE YOUR SONG!\nDO NOT SAVE A SONG WITH A SCOPE MACHINE IN IT!!!\nTHE SCOPE MACHINE IS NOT GOING TO REMAIN AS A MACHINE!!!","WARNING!!!",0);
		pMachine = new Scope;
		break;
	default:
		return false;
	}

	int tmac =	GetFreeMachine();

	_pMachines[tmac] = pMachine;
	if (pMachine->_type == MACH_VSTFX || pMachine->_type == MACH_VST ) // Do not call VST Init() function
	{																	// after Instance.
		((Machine*)pMachine)->Init();
	}
	else pMachine->Init();
	pMachine->_x = x;
	pMachine->_y = y;
	
	Global::_lbc = tmac;
	
	// Finally, activate the machine
	//
	_machineActive[tmac] = true;
	return true;
}

#if !defined(_WINAMP_PLUGIN_)
int Song::FindBusFromIndex(int smac)
{
	int i;
	if ( !_machineActive[smac] ) return 255;
	for (i=0;i<MAX_BUSES;i++)
	{
		if (busMachine[i] == smac)
		{
			return i;
		}
	}
	for (i=0;i<MAX_BUSES;i++)
	{
		if (busEffect[i] == smac)
		{
			return i+MAX_BUSES;
		}
	}
	return 255;
}
#endif //  !defined(_WINAMP_PLUGIN_)
//////////////////////////////////////////////////////////////////////
// Song member functions source code

Song::Song()
{
#if !defined(_WINAMP_PLUGIN_)
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

	Reset();
}

Song::~Song()
{
	DestroyAllMachines();
	DeleteAllPatterns();
	DeleteInstruments();
}

void Song::DestroyAllMachines()
{
#if !defined(_WINAMP_PLUGIN_)
	_machineLock = true;
#endif // ndef _WINAMP_PLUGIN_
	for(int c=0; c<MAX_MACHINES; c++)
	{
		if(_machineActive[c])
		{
			DestroyMachine(c);
		}
		_pMachines[c] = NULL;
	}
#if !defined(_WINAMP_PLUGIN_)
	_machineLock = false;
#endif // ndef _WINAMP_PLUGIN_
}

//////////////////////////////////////////////////////////////////////
// Seek and destroy allocated instruments

void Song::DeleteLayer(int i,int c)
{
	sprintf(waveName[i][c],"empty");
	
	if(waveLength[i][c]>0)
	{
		delete waveDataL[i][c];
		if(waveStereo[i][c])
		{
			delete waveDataR[i][c];
		}
		waveLength[i][c] = 0;
	}
	
	waveStereo[i][c]=false;
	waveLoopStart[i][c]=0;
	waveLoopEnd[i][c]=0;
	waveLoopType[i][c]=0;
	waveVolume[i][c]=100;
	waveFinetune[i][c]=0;
	waveTune[i][c]=0;
}

void Song::DeleteInstruments()
{
	for(int i=0;i<MAX_INSTRUMENTS;i++)DeleteInstrument(i);
}

void Song::DeleteInstrument(int i)
{
	// Reset envelope
	_instruments[i].ENV_AT = 1; // 16
	_instruments[i].ENV_DT = 1; // 16386
	_instruments[i].ENV_SL = 100; // 64
	_instruments[i].ENV_RT = 16; // OVERLAPTIME
	
	_instruments[i].ENV_F_AT = 16;
	_instruments[i].ENV_F_DT = 16384;
	_instruments[i].ENV_F_SL = 64;
	_instruments[i].ENV_F_RT = 16384;
	
	_instruments[i].ENV_F_CO = 64;
	_instruments[i].ENV_F_RQ = 64;
	_instruments[i].ENV_F_EA = 128;
	_instruments[i].ENV_F_TP = 4;
	
	_instruments[i]._loop = false;
	_instruments[i]._lines = 16;
	
	_instruments[i]._NNA = 0; // NNA set to Note Cut [Fast Release]
	
	_instruments[i]._pan = 128;
	_instruments[i]._RPAN = false;
	_instruments[i]._RCUT = false;
	_instruments[i]._RRES = false;
	
	for (int c=0; c<MAX_WAVES; c++)
	{
		DeleteLayer(i,c);
	}
	
	sprintf(_instruments[i]._sName,"empty");
}

//////////////////////////////////////////////////////////////////////
// Song Reset function

void Song::Reset(void)
{
#if !defined(_WINAMP_PLUGIN_)
	cpuIdle=0;
	_sampCount=0;
#endif // ndef _WINAMP_PLUGIN_	

	// Cleaning pattern allocation info
	for (int i=0; i<MAX_INSTRUMENTS; i++)
	{
		for (int c=0; c<MAX_WAVES; c++)
		{
			waveLength[i][c]=0;
		}
	}
	for (int c=0; c<MAX_MACHINES; c++)
	{
		_machineActive[c] = false; // All machines reset
		_pMachines[c] = NULL;
	}
	for (c=0; c<MAX_PATTERNS; c++)
	{
		// All pattern reset
		patternLines[c]=64;
		sprintf(patternName[c],"Untitled"); 
	}
	_trackArmedCount = 0;
	for(c=0; c<MAX_TRACKS; c++)
	{
		_trackMuted[c] = false;
		_trackArmed[c] = false;
	}
#if defined(_WINAMP_PLUGIN_)
	for (c=0; c < MAX_SONG_POSITIONS; c++)
	{
		playOrder[c]=0; // All pattern reset
	}
#else
	machineSoloed = 0;
	_trackSoloed = -1;
	playLength=1;
	for (c=0; c < MAX_SONG_POSITIONS; c++)
	{
		playOrder[c]=0; // All pattern reset
		playOrderSel[c]=false;
	}
	playOrderSel[0]=true;
#endif // _WINAMP_PLUGIN_
}

//////////////////////////////////////////////////////////////////////
// Song NEWSONG

void Song::New(void)
{
	CSingleLock lock(&door,TRUE);

	
	for (int c=0;c<MAX_BUSES;c++)
	{
		busMachine[c]=255;
		busEffect[c]=255;
	}
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
	
	// Clean up allocated machines.
	DestroyAllMachines();
	
	// Cleaning instruments
	DeleteInstruments();
	
	// Clear patterns
	DeleteAllPatterns();
	
	// Clear sequence

	Reset();

	waveSelected = 0;
	instSelected = 0;
	midiSelected = 0;
	auxcolSelected = 0;
	_saved=false;
#if defined(_WINAMP_PLUGIN_)
	strcpy(fileName,"Untitled.psy");
#else
	fileName ="Untitled.psy";
#endif // _WINAMP_PLUGIN_
	CreateMachine(MACH_MASTER, 320, 200, NULL);
}

int Song::GetFreeMachine(void)
{
	int tmac = 0;
	while (true)
	{
		if (!_machineActive[tmac])
		{
			return tmac;
		}
		if (tmac++ >= MAX_MACHINES)
		{
			return -1;
		}
	}
}


#if !defined(_WINAMP_PLUGIN_)
bool Song::InsertConnection(int src,int dst)
{
	int freebus=-1;
	int dfreebus=-1;
	
	bool error=false;
	
	Machine *srcMac = _pMachines[src];
	Machine *dstMac = _pMachines[dst];
	
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
		else if (dstMac->_outputMachines[c] == src)
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
	
	dstMac->InitWireVolume(srcMac->_type,dfreebus,1.0f);
	
	return true;
}
#endif // ndef _WINAMP_PLUGIN_
//////////////////////////////////////////////////////////////////////
// Machine Deletion Function

void Song::DestroyMachine(int mac)
{
	// Delete and destroy the MACHINE!
	CSingleLock lock(&door,TRUE);

	Machine *iMac = _pMachines[mac];
	Machine *iMac2;

	//Deleting the connections to/from other machines
	for (int w=0; w<MAX_CONNECTIONS; w++)
	{
		// Checking In-Wires
		if (iMac->_inputCon[w])
		{
			iMac2 = _pMachines[iMac->_inputMachines[w]];
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
		// Checking Out-Wires
		if(iMac->_connection[w])
		{
			iMac2 = _pMachines[iMac->_outputMachines[w]];
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

	// Clear the Generator/Effects bus

	unsigned char *bus;
	if (_pMachines[mac]->_mode != MACHMODE_MASTER )
	{
		if (_pMachines[mac]->_mode == MACHMODE_GENERATOR)
		{
			bus=busMachine;
		}
		else
		{
			bus=busEffect;
		}
		for (int c=0; c<MAX_BUSES; c++)
		{
			if(bus[c] == mac)
			{
				bus[c] = 255;
				break;
			}
		}
	}
#if  !defined(_WINAMP_PLUGIN_)	
	if ( mac == machineSoloed ) machineSoloed = 0;
#endif //  !defined(_WINAMP_PLUGIN_)
	_machineActive[mac] = false;
	delete _pMachines[mac];	// If it's a (Vst)Plugin, the destructor calls to free the .dll
	
}

void Song::DeleteAllPatterns(void)
{
	SONGTRACKS = 16;
	
	unsigned char blank[5]={255,255,255,0,0};
	for(int c=0; c<MAX_PATTERN_BUFFER_LEN; c+=5)
	{
		memcpy(pPatternData+c,blank,5);
	}
}
#if !defined(_WINAMP_PLUGIN_)
bool Song::AllocNewPattern(int pattern,char *name,int lines,bool adaptsize)
{
	unsigned char blank[5]={255,255,255,0,0};
	unsigned char *toffset;
	if (adaptsize)
	{
		float step;

		if( patternLines[pattern] > lines ) 
		{
			step= (float)patternLines[pattern]/lines;

			for (int t=0;t<SONGTRACKS;t++)
			{
				toffset=pPatternData+pattern*MULTIPLY2+t*5;
				for (int l=1;l<lines;l++)
				{
					memcpy(toffset+l*MULTIPLY,toffset+f2i(l*step)*MULTIPLY,5);
				}
				while (l < patternLines[pattern])	// This wouldn't be necessary if we
				{									// really allocate a new pattern.
					memcpy(toffset+(l*MULTIPLY),blank,5);
					l++;
				}
			}
			patternLines[pattern] = lines;	// This represents the allocation of the new pattern
		}
		else if( patternLines[pattern] < lines )
		{
			step= (float)lines/patternLines[pattern];
			int nl= patternLines[pattern];
			
			patternLines[pattern] = lines;	// This represents the allocation of the new pattern

			for (int t=0;t<SONGTRACKS;t++)
			{
				toffset=pPatternData+pattern*MULTIPLY2+t*5;
				int t;

				for (int l=nl-1;l>0;l--)
				{
					memcpy(toffset+f2i(l*step)*MULTIPLY,toffset+l*MULTIPLY,5);
					t=f2i(l*step)-1;
					while (t> (l-1)*step)
					{
						memcpy(toffset+t*MULTIPLY,blank,5);
						t--;
					}
				}
			}
		}
	}
	else
	{
		int l = patternLines[pattern];
		while (l < lines)	// This wouldn't be necessary if we
		{									// really allocate a new pattern.
			for (int t=0;t<SONGTRACKS;t++)
			{
				toffset=pPatternData+pattern*MULTIPLY2+t*5;
				memcpy(toffset+(l*MULTIPLY),blank,5);
				l++;
			}
		}
		patternLines[pattern] = lines;
	}

	sprintf(patternName[pattern], name);

	return true;
}
#endif // ndef _WINAMP_PLUGIN_
void Song::SetBPM(int bpm, int tpb, int srate)
{
	BeatsPerMin = bpm;
	_ticksPerBeat = tpb;
	SamplesPerTick = (srate*15*4)/(bpm*tpb);
}
#if !defined(_WINAMP_PLUGIN_)
int Song::GetNumPatternsUsed()
{
	int rval=0;
	
	for(int c=0;c<MAX_SONG_POSITIONS;c++)
	{
		if(rval<playOrder[c])rval=playOrder[c];
	}
	
	++rval;
	
	return rval;
}

int Song::GetFreeBus()
{
	int val=-1;
	
	for(int c=MAX_BUSES-1;c>-1;c--)
	{
		if(busMachine[c]==255) val=c;
	}
	
	return val; 
}

int Song::GetFreeFxBus()
{
	int val=-1;
	
	for(int c=MAX_BUSES-1;c>-1;c--)
	{
		if(busEffect[c]==255) val=c;
	}
	
	return val; 
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
		file.Read(waveName[instrument][layer],22); // should be hd._size instead of "22", but it is incorrectly read.
		if ( strcmp(_instruments[instrument]._sName,"empty") == 0 )
		{
			strncpy(_instruments[instrument]._sName,str,31);
			_instruments[instrument]._sName[31]='\0';
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
		waveLength[instrument][layer]=Datalen;

		if ( ls != le )
		{
			waveLoopStart[instrument][layer]=ls;
			waveLoopEnd[instrument][layer]=ls+le;
			waveLoopType[instrument][layer]=true;
		}
		file.Skip(8); // Skipping unknown bytes (and volume on bytes 6&7)
		file.Read(&hd,sizeof(RiffChunkHeader));
	}

	if ( hd._id == file.FourCC("BODY"))
	{
	    short *csamples;
		const unsigned int Datalen = waveLength[instrument][layer];

		waveStereo[instrument][layer]=false;
		waveDataL[instrument][layer]=new signed short[Datalen];
		csamples=waveDataL[instrument][layer];
		
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
	ASSERT(iSamplesPerChan<2*1024*1024); // why is this limit here - i wanna open 10 meg wavefiles, why can i open only 2 megs?

	DeleteLayer(iInstr,iLayer);

	if(bStereo)
	{
		waveDataL[iInstr][iLayer]=new signed short[iSamplesPerChan];
		waveDataR[iInstr][iLayer]=new signed short[iSamplesPerChan];
		waveStereo[iInstr][iLayer]=true;
	}
	else
	{
		waveDataL[iInstr][iLayer]=new signed short[iSamplesPerChan];
		waveStereo[iInstr][iLayer]=false;
	}
	waveLength[iInstr][iLayer]=iSamplesPerChan;

	strncpy(waveName[iInstr][iLayer],sName,31);
	waveName[iInstr][iLayer][31]='\0';
	
	if(iLayer==0)
	{
		strncpy(_instruments[iInstr]._sName,sName,31);
		_instruments[iInstr]._sName[31]='\0';
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
	
	short *sampL=waveDataL[instrument][layer];
	
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
		short *sampR = waveDataR[instrument][layer];

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
				waveLoopStart[instrument][layer]=ls;
				waveLoopEnd[instrument][layer]=le;
				waveLoopType[instrument][layer]=true;

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

bool Song::Load(
				RiffFile* pFile)
{
	int i;
	int num;

	if (!pFile->Expect("PSY2SONG", 8))
	{
		::MessageBox(NULL,"Incorrect file format","Error",MB_OK);
		return false;
	}
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
	pFile->Read(&SONGTRACKS, sizeof(SONGTRACKS));

	// Patterns
	//
	pFile->Read(&num, sizeof(num));
	for (i=0; i<num; i++)
	{
		pFile->Read(&patternLines[i], sizeof(patternLines[0]));
		pFile->Read(&patternName[i][0], sizeof(patternName[0]));
		pFile->Read((char*)pPatternData+MULTIPLY2*i, patternLines[i]*MAX_TRACKS*sizeof(PatternEntry));
	}

	// Instruments
	//
	pFile->Read(&instSelected, sizeof(instSelected));
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i]._sName, sizeof(_instruments[0]._sName));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i]._NNA, sizeof(_instruments[0]._NNA));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i].ENV_AT, sizeof(_instruments[0].ENV_AT));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i].ENV_DT, sizeof(_instruments[0].ENV_DT));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i].ENV_SL, sizeof(_instruments[0].ENV_SL));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i].ENV_RT, sizeof(_instruments[0].ENV_RT));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i].ENV_F_AT, sizeof(_instruments[0].ENV_F_AT));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i].ENV_F_DT, sizeof(_instruments[0].ENV_F_DT));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i].ENV_F_SL, sizeof(_instruments[0].ENV_F_SL));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i].ENV_F_RT, sizeof(_instruments[0].ENV_F_RT));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i].ENV_F_CO, sizeof(_instruments[0].ENV_F_CO));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i].ENV_F_RQ, sizeof(_instruments[0].ENV_F_RQ));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i].ENV_F_EA, sizeof(_instruments[0].ENV_F_EA));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i].ENV_F_TP, sizeof(_instruments[0].ENV_F_TP));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i]._pan, sizeof(_instruments[0]._pan));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i]._RPAN, sizeof(_instruments[0]._RPAN));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i]._RCUT, sizeof(_instruments[0]._RCUT));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i]._RRES, sizeof(_instruments[0]._RRES));
	}

	// Waves
	//
	pFile->Read(&waveSelected, sizeof(waveSelected));

	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		for (int w=0; w<MAX_WAVES; w++)
		{
			pFile->Read(&waveLength[i][w], sizeof(waveLength[0][0]));
			if (waveLength[i][w] > 0)
			{
				short tmpFineTune;
				pFile->Read(&waveName[i][w], sizeof(waveName[0][0]));
				pFile->Read(&waveVolume[i][w], sizeof(waveVolume[0][0]));
				pFile->Read(&tmpFineTune, sizeof(short));
				waveFinetune[i][w]=(int)tmpFineTune;
				pFile->Read(&waveLoopStart[i][w], sizeof(waveLoopStart[0][0]));
				pFile->Read(&waveLoopEnd[i][w], sizeof(waveLoopEnd[0][0]));
				pFile->Read(&waveLoopType[i][w], sizeof(waveLoopType[0][0]));
				pFile->Read(&waveStereo[i][w], sizeof(waveStereo[0][0]));
				waveDataL[i][w] = new signed short[waveLength[i][w]];
				pFile->Read(waveDataL[i][w], waveLength[i][w]*sizeof(short));
				if (waveStereo[i][w])
				{
					waveDataR[i][w] = new signed short[waveLength[i][w]];
					pFile->Read(waveDataR[i][w], waveLength[i][w]*sizeof(short));
				}
			}
		}
	}

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

	// Machines
	//
	_machineLock = true;

	pFile->Read(&_machineActive[0], sizeof(_machineActive));
	for (i=0; i<MAX_MACHINES; i++)
	{
		Machine* pMachine;
		Sine* pSine;
		Distortion* pDistortion;
		Sampler* pSampler;
		Delay* pDelay;
		Filter2p* pFilter;
		Gainer* pGainer;
		Flanger* pFlanger;
		Plugin* pPlugin;
		VSTPlugin* pVstPlugin;

		int x,y,type;
		if (_machineActive[i])
		{
			pFile->Read(&x, sizeof(x));
			pFile->Read(&y, sizeof(y));
#if !defined(_WINAMP_PLUGIN_)
			if ( x > viewSize.x ) x = viewSize.x;
			if ( y > viewSize.y ) y = viewSize.y;
#endif // _WINAMP_PLUGIN_

			pFile->Read(&type, sizeof(type));

			switch (type)
			{
			case MACH_MASTER:
				pMachine = _pMachines[0];
				pMachine->Init();
				pMachine->Load(pFile);
				break;
			case MACH_SINE:
				pMachine = pSine = new Sine;
				pMachine->Init();
				pMachine->Load(pFile);
				break;
			case MACH_DIST:
				pMachine = pDistortion = new Distortion;
				pMachine->Init();
				pMachine->Load(pFile);
				break;
			case MACH_SAMPLER:
				pMachine = pSampler = new Sampler;
				pMachine->Init();
				pMachine->Load(pFile);
				break;
			case MACH_DELAY:
				pMachine = pDelay = new Delay;
				pMachine->Init();
				pMachine->Load(pFile);
				break;
			case MACH_2PFILTER:
				pMachine = pFilter = new Filter2p;
				pMachine->Init();
				pMachine->Load(pFile);
				break;
			case MACH_GAIN:
				pMachine = pGainer = new Gainer;
				pMachine->Init();
				pMachine->Load(pFile);
				break;
			case MACH_FLANGER:
				pMachine = pFlanger = new Flanger;
				pMachine->Init();
				pMachine->Load(pFile);
				break;
			case MACH_SCOPE:
				MessageBox(0,"DELETE THE SCOPE MACHINE AND RESAVE YOUR SONG!\nDO NOT SAVE A SONG WITH A SCOPE MACHINE IN IT!!!\nTHE SCOPE MACHINE IS NOT GOING TO REMAIN AS A MACHINE!!!","WARNING!!!",0);
				pMachine = new Scope;
				pMachine->Init();
				pMachine->Load(pFile);
				break;
			case MACH_PLUGIN:
				{
				pMachine = pPlugin = new Plugin;
				// Should the "Init()" function go here? -> No. Needs to load the dll first.
				if (!pMachine->Load(pFile))
				{
					Machine* pOldMachine = pMachine;
					pMachine = new Dummy(*((Dummy*)pOldMachine));
					pMachine->_type = MACH_DUMMY;
					pOldMachine->_pSamplesL = NULL;
					pOldMachine->_pSamplesR = NULL;
					delete pOldMachine;
				}
				break;
				}
			case MACH_VST:
			case MACH_VSTFX:
				{
				
				if ( type == MACH_VST ) pMachine = pVstPlugin = new VSTInstrument;
				else if ( type == MACH_VSTFX ) pMachine = pVstPlugin = new VSTFX;
#if  !defined(_WINAMP_PLUGIN_)
				pVstPlugin->macindex = FindBusFromIndex(i);
#endif //  !defined(_WINAMP_PLUGIN_)
				if ((pMachine->Load(pFile)) && (vstL[pVstPlugin->_instance].valid)) // Machine::Init() is done Inside "Load()"
				{
					CString sPath;
					char sPath2[_MAX_PATH];

#if defined(_WINAMP_PLUGIN_)
					sPath = Global::pConfig->GetVstDir();
					if ( FindFileinDir(vstL[pVstPlugin->_instance].dllName,sPath) )
					{
						strcpy(sPath2,sPath);
						if (pVstPlugin->Instance(sPath2,false) != VSTINSTANCE_NO_ERROR)
						{
							Machine* pOldMachine = pMachine;
							pMachine = new Dummy(*((Dummy*)pOldMachine));
							pOldMachine->_pSamplesL = NULL;
							pOldMachine->_pSamplesR = NULL;
							delete pOldMachine;
							pMachine->_type = MACH_DUMMY;
							pMachine->wasVST = true;
						}
					}
					else
					{
						Machine* pOldMachine = pMachine;
						pMachine = new Dummy(*((Dummy*)pOldMachine));
						pOldMachine->_pSamplesL = NULL;
						pOldMachine->_pSamplesR = NULL;
						delete pOldMachine;
						pMachine->_type = MACH_DUMMY;
						pMachine->wasVST = true;
					}
#else // if !_WINAMP_PLUGIN_
					if ( CNewMachine::dllNames.Lookup(vstL[pVstPlugin->_instance].dllName,sPath) )
					{
						strcpy(sPath2,sPath);
						if (pVstPlugin->Instance(sPath2,false) != VSTINSTANCE_NO_ERROR)
						{
							char sError[128];
							sprintf(sError,"Missing or Corrupted VST plug-in \"%s\"",sPath2);
							::MessageBox(NULL,sError, "Loading Error", MB_OK);

							Machine* pOldMachine = pMachine;
							pMachine = new Dummy(*((Dummy*)pOldMachine));
							pOldMachine->_pSamplesL = NULL;
							pOldMachine->_pSamplesR = NULL;
							delete pOldMachine;
							pMachine->_type = MACH_DUMMY;
							pMachine->wasVST = true;
						}
					}
					else
					{
						char sError[128];
						sprintf(sError,"Missing VST plug-in \"%s\"",vstL[pVstPlugin->_instance].dllName);
						::MessageBox(NULL,sError, "Loading Error", MB_OK);

						Machine* pOldMachine = pMachine;
						pMachine = new Dummy(*((Dummy*)pOldMachine));
						pOldMachine->_pSamplesL = NULL;
						pOldMachine->_pSamplesR = NULL;
						delete pOldMachine;
						pMachine->_type = MACH_DUMMY;
						pMachine->wasVST = true;
					}
#endif // _WINAMP_PLUGIN_
				}
				else
				{
					Machine* pOldMachine = pMachine;
					pMachine = new Dummy(*((Dummy*)pOldMachine));
					pOldMachine->_pSamplesL = NULL;
					pOldMachine->_pSamplesR = NULL;
					delete pOldMachine;
					pMachine->_type = MACH_DUMMY;
					pMachine->wasVST = true;
				}
				break;
				}
			case MACH_DUMMY:
				pMachine = new Dummy;
				pMachine->Init();
				pMachine->Load(pFile);
				break;
			}

			_pMachines[i] = pMachine;
			pMachine->_x = x;
			pMachine->_y = y;
		}
	}

	// Since the old file format stored volumes on each output
	// rather than on each input, we must convert
	//
	float volMatrix[MAX_MACHINES][MAX_CONNECTIONS];
	for (i=0; i<MAX_MACHINES; i++) // First, we add the output volumes to a Matrix for latter reference
	{
		if (_machineActive[i])
		{
			for (int c=0; c<MAX_CONNECTIONS; c++)
			{
				volMatrix[i][c] = _pMachines[i]->_inputConVol[c];
			}
		}
	}
	for (i=0; i<MAX_MACHINES; i++) // Next, we go to fix this for each
	{
		if (_machineActive[i])		// valid machine (important, since we have to navigate!)
		{
			for (int c=0; c<MAX_CONNECTIONS; c++) // all of its input connections.
			{
				if (_pMachines[i]->_inputCon[c])	// If there's a valid machine in this inputconnection,
				{
					Machine* pOrigMachine = _pMachines[_pMachines[i]->_inputMachines[c]]; // We get that machine
					int d = pOrigMachine->FindOutputWire(i);

					float val = volMatrix[_pMachines[i]->_inputMachines[c]][d];
					if( val > 2 ) val*=0.000030517578125f; // BugFix
					else if ( val < 0.00004) val*=32768.0f; // BugFix

					_pMachines[i]->InitWireVolume(pOrigMachine->_type,c,val);
				}
			}
		}
	}

	_machineLock = false;

	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i]._loop, sizeof(_instruments[0]._loop));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Read(&_instruments[i]._lines, sizeof(_instruments[0]._lines));
	}

	if ( pFile->Read(&busEffect[0],sizeof(busEffect)) == false ) // Patch 1: BusEffects (twf)
	{
		int j=0;
		for ( i=0;i<MAX_MACHINES;i++ ) {
			if (_machineActive[i] && _pMachines[i]->_mode != MACHMODE_GENERATOR )
			{
				busEffect[j]=i;	j++;
			}
		}
	}
	// Patch 1.2: Fixes crash/inconsistence when deleting a machine which couldn't be loaded
	// (.dll not found, or Load failed), which is, then, replaced by a DUMMY machine.
	int j=0;
	for ( i=0;i<MAX_BUSES;i++ ) {
		if (busMachine[i] != 255 && _machineActive[busMachine[i]]) { // If there's a machine in the generators' bus that it is not a generator:
			if (_pMachines[busMachine[i]]->_mode != MACHMODE_GENERATOR ) {
				_pMachines[busMachine[i]]->_mode = MACHMODE_FX;
				while (busEffect[j] != 255 && j<MAX_BUSES) j++;
				busEffect[j]=busMachine[i];
				busMachine[i]=255;
			}
		}
	}


	bool chunkpresent=false;
	pFile->Read(&chunkpresent,sizeof(chunkpresent)); // Patch 2: VST's Chunk.

	for ( i=0;i<MAX_MACHINES;i++ ) {
		if (_machineActive[i])
		{
			if ( _pMachines[i]->wasVST && chunkpresent )
			{
				// Since we don't know if the plugin saved it or not, 
				// we're stuck on letting the loading crash/behave incorrectly.
				// There should be a flag, like in the VST loading Section to be correct.
			}
			else if (( _pMachines[i]->_type == MACH_VST ) || 
					( _pMachines[i]->_type == MACH_VSTFX))
			{
				bool chunkread=false;
				if( chunkpresent )	chunkread=((VSTPlugin*)_pMachines[i])->LoadChunk(pFile);
				((VSTPlugin*)_pMachines[i])->SetCurrentProgram(((VSTPlugin*)_pMachines[i])->_program);
				if ( !chunkpresent || !chunkread )
				{
					const int vi = ((VSTPlugin*)_pMachines[i])->_instance;
					const int numpars=vstL[vi].numpars;
					for (int c=0; c<numpars; c++)
					{
						((VSTPlugin*)_pMachines[i])->SetParameter(c, vstL[vi].pars[c]);
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
	return true;
}


#if !defined(_WINAMP_PLUGIN_)
bool Song::Save(
	RiffFile* pFile)
{
	int i;
	char junk[256];
	memset(&junk, sizeof(junk), 0);

	pFile->Write("PSY2SONG", 8);

	pFile->Write(&Name, 32);
	pFile->Write(&Author, 32);
	pFile->Write(&Comment, 128);
	
	pFile->Write(&BeatsPerMin, sizeof(BeatsPerMin));
	i = SamplesPerTick*44100/Global::pConfig->_pOutputDriver->_samplesPerSec;
	pFile->Write(&i, sizeof(SamplesPerTick));
	pFile->Write(&currentOctave, sizeof(currentOctave));

	pFile->Write(&busMachine[0], sizeof(busMachine));

	pFile->Write(&playOrder, sizeof(playOrder));
	pFile->Write(&playLength, sizeof(playLength));
	pFile->Write(&SONGTRACKS, sizeof(SONGTRACKS));

	// Patterns
	//
	i = GetNumPatternsUsed();
	pFile->Write(&i, sizeof(i));
	for (int p=0; p<i; p++)
	{
		pFile->Write(&patternLines[p], sizeof(patternLines[0]));
		pFile->Write(&patternName[p][0], sizeof(patternName[0]));
		pFile->Write(pPatternData+MULTIPLY2*p, patternLines[p]*MAX_TRACKS*sizeof(PatternEntry));
	}

	// Instruments
	//
	pFile->Write(&instSelected, sizeof(instSelected));
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i]._sName, sizeof(_instruments[0]._sName));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i]._NNA, sizeof(_instruments[0]._NNA));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i].ENV_AT, sizeof(_instruments[0].ENV_AT));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i].ENV_DT, sizeof(_instruments[0].ENV_DT));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i].ENV_SL, sizeof(_instruments[0].ENV_SL));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i].ENV_RT, sizeof(_instruments[0].ENV_RT));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i].ENV_F_AT, sizeof(_instruments[0].ENV_F_AT));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i].ENV_F_DT, sizeof(_instruments[0].ENV_F_DT));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i].ENV_F_SL, sizeof(_instruments[0].ENV_F_SL));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i].ENV_F_RT, sizeof(_instruments[0].ENV_F_RT));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i].ENV_F_CO, sizeof(_instruments[0].ENV_F_CO));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i].ENV_F_RQ, sizeof(_instruments[0].ENV_F_RQ));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i].ENV_F_EA, sizeof(_instruments[0].ENV_F_EA));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i].ENV_F_TP, sizeof(_instruments[0].ENV_F_TP));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i]._pan, sizeof(_instruments[0]._pan));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i]._RPAN, sizeof(_instruments[0]._RPAN));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i]._RCUT, sizeof(_instruments[0]._RCUT));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i]._RRES, sizeof(_instruments[0]._RRES));
	}

	// Waves
	//
	pFile->Write(&waveSelected, sizeof(waveSelected));

	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		for (int w=0; w<MAX_WAVES; w++)
		{
			pFile->Write(&waveLength[i][w], sizeof(waveLength[0][0]));
			if (waveLength[i][w] > 0)
			{
				pFile->Write(&waveName[i][w], sizeof(waveName[0][0]));
				pFile->Write(&waveVolume[i][w], sizeof(waveVolume[0][0]));
				pFile->Write(&waveFinetune[i][w], sizeof(short));
				pFile->Write(&waveLoopStart[i][w], sizeof(waveLoopStart[0][0]));
				pFile->Write(&waveLoopEnd[i][w], sizeof(waveLoopEnd[0][0]));
				pFile->Write(&waveLoopType[i][w], sizeof(waveLoopType[0][0]));
				pFile->Write(&waveStereo[i][w], sizeof(waveStereo[0][0]));
				pFile->Write(waveDataL[i][w], waveLength[i][w]*sizeof(short));
				if (waveStereo[i][w])
				{
					pFile->Write(waveDataR[i][w], waveLength[i][w]*sizeof(short));
				}
			}
		}
	}

	// VST DLLs
	int num = 0;
	for (i=0; i<MAX_MACHINES; i++)
	{
		if (_machineActive[i] && (_pMachines[i]->_type == MACH_VST || _pMachines[i]->_type == MACH_VSTFX))
		{
			bool b = true;
			pFile->Write(&b, sizeof(b));

			CString str = ((VSTPlugin*)_pMachines[i])->GetDllName();
			char str2[128];
			strcpy(str2,str.Mid(str.ReverseFind('\\')+1));// if not found, -1+1 = 0 -> Starting letter
			pFile->Write(&str2,sizeof(str2));

			long numpar = ((VSTPlugin*)_pMachines[i])->NumParameters();
			pFile->Write(&numpar, sizeof(int));
			for (int c=0; c<numpar; c++)
			{
				float f = ((VSTPlugin*)_pMachines[i])->GetParameter(c);
				pFile->Write(&f, sizeof(f));
			}
			((VSTPlugin*)_pMachines[i])->_instance = num;
			num++;
		}
	}
	for (; num<MAX_PLUGINS; num++)
	{
		bool b = false;
		pFile->Write(&b, sizeof(b));
	}

	// Since the old file format stored volumes on each output
	// rather than on each input, we must convert
	//
	float volMatrix[MAX_MACHINES][MAX_CONNECTIONS];
	for (i=0; i<MAX_MACHINES; i++)
	{
		if (_machineActive[i])
		{
			for (int c=0; c<MAX_CONNECTIONS; c++)
			{
				volMatrix[i][c] = _pMachines[i]->_inputConVol[c];
			}
		}
	}
	for (i=0; i<MAX_MACHINES; i++)
	{
		if (_machineActive[i])
		{
			for (int c=0; c<MAX_CONNECTIONS; c++)
			{
				if (_pMachines[i]->_connection[c])
				{
					Machine* pDstMachine = _pMachines[_pMachines[i]->_outputMachines[c]];
					int d = pDstMachine->FindInputWire(i);
					_pMachines[i]->_inputConVol[c]=volMatrix[_pMachines[i]->_outputMachines[c]][d]*pDstMachine->_wireMultiplier[d];
				}
			}
		}
	}

	// Machines
	//
	pFile->Write(&_machineActive[0], sizeof(_machineActive));
	for (i=0; i<MAX_MACHINES; i++)
	{
		if (_machineActive[i])
		{
			_pMachines[i]->Save(pFile);
		}
	}

	// Now convert back to the internal format
	//
	for (i=0; i<MAX_MACHINES; i++)
	{
		if (_machineActive[i])
		{
			for (int c=0; c<MAX_CONNECTIONS; c++)
			{
				_pMachines[i]->_inputConVol[c] = volMatrix[i][c];
			}
		}
	}

	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i]._loop, sizeof(_instruments[0]._loop));
	}
	for (i=0; i<MAX_INSTRUMENTS; i++)
	{
		pFile->Write(&_instruments[i]._lines, sizeof(_instruments[0]._lines));
	}

	pFile->Write(&busEffect[0], sizeof(busEffect)); // Patch 1: BusEffect ( twk );

	bool isfirst=true;// Patch 2: VST's Chunk.
	for ( i=0;i<MAX_MACHINES;i++ ) {
		if (_machineActive[i])
		{
			if (( _pMachines[i]->_type == MACH_VST ) ||
				( _pMachines[i]->_type == MACH_VSTFX))
			{
				((VSTPlugin*)_pMachines[i])->SaveChunk(pFile,isfirst);
			}
		}
	}

	return true;
}

void Song::PW_Play()
{
	if (PW_Stage==0)
	{
		PW_Length=waveLength[PREV_WAV_INS][0];
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

#endif // ndef _WINAMP_PLUGIN_
