/** @file 
 *  @brief implementation file
 *  $Date: 2005/01/15 22:44:20 $
 *  $Revision: 1.6 $
 */
#include "stdafx.h"
#include "NewMachine.h"
//#include "MainFrm.h"
//#include "PsycleWTLView.h"
#include "ProgressDialog.h"

//	extern CPsycleApp theApp;
#include "UndoRedo.h"
#include "Song.h"
#include "Psy3SongLoader.h"
#include "Psy2SongLoader.h"
#include "Instrument.h"
#include "Machine.h" // It wouldn't be needed, since it is already included in "song.h"
#include "Sampler.h"
#include "XMInstrument.h"
#include "XMSampler.h"
#include "Plugin.h"
#include "VSTHost.h"
#include "DataCompression.h"
#include ".\psysongloader.h"
#include "PsyFSongSaver.h"
#include "XMSongLoader.h"

//#include <sstream>

#ifdef CONVERT_INTERNAL_MACHINES
	#include "convert_internal_machines.h" // conversion
#endif

#if !defined(_WINAMP_PLUGIN_)

#include "Riff.h"	 // For Wave file loading.

#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif


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



//////////////////////////////////////////////////////////////////////
// Create machine member

bool ::Song::CreateMachine(
	MachineType type,
	int x,
	int y,
	TCHAR* psPluginDll,
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
		if (m_pMachine[MASTER_INDEX])
		{
			return false;
		}
		pMachine = pMaster = new Master(index);
		index = MASTER_INDEX;
		break;
	case MACH_SAMPLER:
		pMachine = pSampler = new Sampler(index);
		break;
	case MACH_XMSAMPLER:
		pMachine = new SF::XMSampler(index);
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

	if (m_pMachine[index])
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
	m_pMachine[index] = pMachine;
	return true;
}
#if !defined(_WINAMP_PLUGIN_)
	
int ::Song::FindBusFromIndex(int smac)
{
	if ( !m_pMachine[smac] ) 
	{
		return 255;
	}
	return smac;
}
#endif //  !defined(_WINAMP_PLUGIN_)
//////////////////////////////////////////////////////////////////////
// Song member functions source code

Song::Song(iController * pController)
{
	m_MachineLock = false;
	m_bInvalided = false;
	m_bTweaking = false;
	m_PW_Phase = 0;
	m_PW_Stage = 0;
	m_PW_Length = 0;
	
	m_pController = pController;

	for (int i=0; i<MAX_PATTERNS; i++)
	{
		m_ppPatternData[i] = NULL;
	}

	for (i = 0; i<MAX_MACHINES;i++)
	{
		m_pMachine[i] = NULL;
	}

	CreateNewPattern(0);

	for (i=0;i<MAX_INSTRUMENTS;i++)
	{
		m_pInstrument[i] = new Instrument;
	}

	m_Name.reserve(SF::Song::MAX_NAME_LEN);
	m_Author.reserve(SF::Song::MAX_AUTHOR_LEN);
	m_Comment.reserve(SF::Song::MAX_COMMENT_LEN);

	m_DefaultPatternLines = 64;

	m_SongLoaderMap[_T("psy")] = new SF::PsySongLoader();
	m_SongLoaderMap[_T("psf")] = new SF::PsySongLoader();
	m_SongLoaderMap[_T("xm")] = new SF::XMSongLoader();
	
	autosaveSong = true;
	autosaveSongTime = 10;

	Reset();
}

Song::~Song()
{
	DestroyAllMachines();
	DestroyAllInstruments();
	DeleteAllPatterns();
	
	//DeleteObject(hbmMachineBkg);
	for(SongLoaderMap::iterator it = m_SongLoaderMap.begin();it != m_SongLoaderMap.end();it++)
	{
		delete it->second;
	}
}

void Song::DestroyAllMachines()
{
#if !defined(_WINAMP_PLUGIN_)
	m_MachineLock = true;
#endif // ndef _WINAMP_PLUGIN_
	for(int c=0; c<MAX_MACHINES; c++)
	{
		if(m_pMachine[c])
		{
			for (int j=c+1; j < MAX_MACHINES; j++)
			{
				if (m_pMachine[c] == m_pMachine[j])
				{
					// wtf? duplicate machine? could happen if loader messes up?
					MessageBox(NULL,
						(SF::tformat(SF::CResourceString(IDS_ERR_MSG0045)) % c % j).str().data(),
						SF::CResourceString(IDS_ERR_MSG0046),NULL);
					m_pMachine[j] = NULL;
				}
			}
			DestroyMachine(c);
		}
		m_pMachine[c] = NULL;
	}
#if !defined(_WINAMP_PLUGIN_)
	m_MachineLock = false;
#endif // ndef _WINAMP_PLUGIN_
}

//////////////////////////////////////////////////////////////////////
// Seek and destroy allocated instruments

void Song::DeleteLayer(int i,int c)
{
	m_pInstrument[i]->DeleteLayer(c);
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
		delete m_pInstrument[i];
	}
}

void Song::DeleteInstrument(int i)
{
	// this actually just resets to a blank instrument
#if !defined(_WINAMP_PLUGIN_)
	m_bInvalided=true;
#endif
	m_pInstrument[i]->Delete();
#if !defined(_WINAMP_PLUGIN_)
	m_bInvalided=false;
#endif

}

//////////////////////////////////////////////////////////////////////
// Song Reset function

void Song::Reset(void)
{
#if !defined(_WINAMP_PLUGIN_)
	m_CpuIdle=0;
	m_SampCount=0;
#endif // ndef _WINAMP_PLUGIN_	

	// Cleaning pattern allocation info
	for (int i = 0; i<MAX_INSTRUMENTS; i++)
	{
		for (int c = 0; c < MAX_WAVES; c++)
		{
			m_pInstrument[i]->waveLength[c]=0;
		}
	}

	for (i = 0; i<MAX_MACHINES;i++)
	{
		if (m_pMachine[i])
		{
			delete (m_pMachine[i]);
			m_pMachine[i] = NULL;
		}
	}

	for (i = 0; i < MAX_PATTERNS; i++)
	{
		m_PatternLines[i] = m_DefaultPatternLines;
		_stprintf(m_PatternName[i],_T("Untitled")); 
	}

	m_TrackArmedCount = 0;

	for(i = 0; i < MAX_TRACKS ; i++)
	{
		m_TrackMuted[i] = false;
		m_TrackArmed[i] = false;
	}

	m_MachineSoloed = -1;
	m_TrackSoloed = -1;
	m_PlayLength = 1;
	for (i = 0; i < MAX_SONG_POSITIONS; i++)
	{
		m_PlayOrder[i] = 0; // All pattern reset
		m_PlayOrderSel[i] = false;
	}
	m_PlayOrderSel[0] = true;
}

//////////////////////////////////////////////////////////////////////
// Song NEWSONG

void Song::New(void)
{
	CComCritSecLock<CComAutoCriticalSection> lock(m_Door,TRUE);
	
	m_UndoController.Clear();

	m_SeqBus = 0;
	
	// Song reset
	
	m_Name = _T("Untitled");
	m_Author = _T("Unnamed");
	m_Comment = _T("No Comments");
	
	m_CurrentOctave = 4;
	
	// General properties
	SetBPM(125, 4, 44100);
	m_LineCounter=0;
	
	m_bLineChanged=false;
	
//	MessageBox(NULL,_T("Machines"),NULL,NULL);
	// Clean up allocated machines.
	DestroyAllMachines();
	
//	MessageBox(NULL,_T("Insts"),NULL,NULL);
	// Cleaning instruments
	DeleteInstruments();
	
//	MessageBox(NULL,_T("Pats"),NULL,NULL);
	// Clear patterns
	DeleteAllPatterns();
	
	// Clear sequence

	Reset();

	m_WaveSelected = 0;
	m_InstSelected = 0;
	m_MidiSelected = 0;
	m_AuxcolSelected = 0;
	m_bSaved=false;

	// m_FileName = _T("Untitled.psy");
	m_FileName = _T("Untitled.psf");

	if (CMainFrame::GetInstance().m_hWnd)
	{
		CreateMachine(MACH_MASTER, 
			(m_ViewSize.x - (CMainFrame::GetInstance()).m_view.MachineCoords.sMaster.width)/2, 
			(m_ViewSize.y - (CMainFrame::GetInstance()).m_view.MachineCoords.sMaster.height)/2, 
			NULL,MASTER_INDEX);
	}
	else
	{
		CreateMachine(MACH_MASTER, 
			320, 
			200, 
			NULL,MASTER_INDEX);
	}
}

int Song::GetFreeMachine(void)
{
	int tmac = 0;
	while (true)
	{
		if (!m_pMachine[tmac])
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
bool Song::InsertConnection(int src,int dst,float value)
{
	int freebus=-1;
	int dfreebus=-1;
	
	bool error=false;
	
	Machine *srcMac = m_pMachine[src];
	Machine *dstMac = m_pMachine[dst];

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
	CComCritSecLock<CComAutoCriticalSection> lock(m_Door,TRUE);
#endif // #if !defined(_WINAMP_PLUGIN_)
	
	Machine *iMac = m_pMachine[mac];
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
					iMac2 = m_pMachine[iMac->_inputMachines[w]];
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
					iMac2 = m_pMachine[iMac->_outputMachines[w]];
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
	if ( mac == m_MachineSoloed ) 
	{
		m_MachineSoloed = -1;
	}
#endif //  !defined(_WINAMP_PLUGIN_)
	delete m_pMachine[mac];	// If it's a (Vst)Plugin, the destructor calls to free the .dll
	m_pMachine[mac] = NULL;
}

void Song::DeleteAllPatterns()
{
	m_SongTracks = 16;

	for (int i=0; i<MAX_PATTERNS; i++)
	{
		RemovePattern(i);
	}
}

void Song::RemovePattern(int ps)
{
	if (m_ppPatternData[ps])
	{
		delete m_ppPatternData[ps];
		m_ppPatternData[ps] = NULL;
	}
}

unsigned char * Song::CreateNewPattern(int ps)
{
	RemovePattern(ps);
	m_ppPatternData[ps] = new unsigned char[MULTIPLY2];
//	unsigned char blank[5]={255,255,255,0,0};

	unsigned char * pData = m_ppPatternData[ps];
	for (int i = 0; i < MULTIPLY2; i+= EVENT_SIZE)
	{
		memcpy(pData,BLANK_EVENT,EVENT_SIZE * sizeof(unsigned char));
		pData += EVENT_SIZE;
	}
	return m_ppPatternData[ps];
}

#if !defined(_WINAMP_PLUGIN_)
bool Song::AllocNewPattern(int pattern,TCHAR *name,int lines,bool adaptsize)
{
//	unsigned char blank[5]={255,255,255,0,0};
	unsigned char *toffset;
	if (adaptsize)
	{
		float step;

		if( m_PatternLines[pattern] > lines ) 
		{
			step= (float)m_PatternLines[pattern]/lines;

			for (int t=0;t<m_SongTracks;t++)
			{
				toffset=_ptrack(pattern,t);
				for (int l=1;l<lines;l++)
				{
					memcpy(toffset+l*MULTIPLY,toffset+f2i(l*step)*MULTIPLY,EVENT_SIZE);
				}
				while (l < m_PatternLines[pattern])	// This wouldn't be necessary if we
				{									// really allocate a new pattern.
					memcpy(toffset+(l*MULTIPLY),BLANK_EVENT,EVENT_SIZE);
					l++;
				}
			}
			m_PatternLines[pattern] = lines;	// This represents the allocation of the new pattern
		}
		else if( m_PatternLines[pattern] < lines )
		{
			step= (float)lines/m_PatternLines[pattern];
			int nl= m_PatternLines[pattern];
			
			for (int t=0;t<m_SongTracks;t++)
			{
				toffset=_ptrack(pattern,t);

				for (int l=nl-1;l>0;l--)
				{
					memcpy(toffset+f2i(l*step)*MULTIPLY,toffset+l*MULTIPLY,EVENT_SIZE);
					int tz=f2i(l*step)-1;
					while (tz> (l-1)*step)
					{
						memcpy(toffset+tz*MULTIPLY,BLANK_EVENT,EVENT_SIZE);
						tz--;
					}
				}
			}
			m_PatternLines[pattern] = lines;	// This represents the allocation of the new pattern
		}
	}
	else
	{
		int l = m_PatternLines[pattern];
		while (l < lines)	// This wouldn't be necessary if we
		{									// really allocate a new pattern.
			for (int t=0;t<m_SongTracks;t++)
			{
				toffset=_ptrackline(pattern,t,l);
				memcpy(toffset,BLANK_EVENT,EVENT_SIZE);
				l++;
			}
		}
		m_PatternLines[pattern] = lines;
	}

	_stprintf(m_PatternName[pattern], name);

	return true;
}
#endif // ndef _WINAMP_PLUGIN_
void Song::SetBPM(int bpm, int tpb, int srate)
{
	m_BeatsPerMin = bpm;
	m_TicksPerBeat = tpb;
	m_SamplesPerTick = (srate*15*4)/(bpm*tpb);
}
int Song::GetNumPatternsUsed()
{
	int rval=0;
	
	for(int c=0;c<m_PlayLength;c++)
	{
		if(rval<m_PlayOrder[c])
		{
			rval=m_PlayOrder[c];
		}
	}
	
	++rval;
	
	if (rval > MAX_PATTERNS-1)
	{
		rval = MAX_PATTERNS-1;
	}
	return rval;
}
#if !defined(_WINAMP_PLUGIN_)

int Song::GetBlankPatternUnused(int rval)
{
	for (int i=0; i<MAX_PATTERNS; i++)
	{
		if (!IsPatternUsed(i))
		{
			return i;
		}
	}

//	const unsigned char blank[5]={255,255,255,0,0};
	BOOL bTryAgain = TRUE;

	while ((bTryAgain) && (rval < MAX_PATTERNS-1))
	{
		for(int c=0;c<m_PlayLength;c++)
		{
			if(rval == m_PlayOrder[c]) 
			{
				rval++;
				c=-1;
			}
		}
		// now test to see if data is really blank
		bTryAgain = FALSE;
		if (rval < MAX_PATTERNS-1)
		{
			unsigned char *offset_source=_ppattern(rval);
			
			for (int t=0;t<MULTIPLY2;t+=EVENT_SIZE)
			{
				for (int i = 0; i < EVENT_SIZE; i++)
				{
					if (offset_source[i] != BLANK_EVENT[i])
					{
						rval++;
						bTryAgain = TRUE;
						t=MULTIPLY2;
						i=EVENT_SIZE;
					}
					offset_source+=EVENT_SIZE;
				}
			}
		}
	}

	if (rval > MAX_PATTERNS-1)
	{
		rval = 0;
		for(int c=0;c<m_PlayLength;c++)
		{
			if(rval == m_PlayOrder[c]) 
			{
				rval++;
				c=-1;
			}
		}
		if (rval > MAX_PATTERNS-1)
		{
			rval = MAX_PATTERNS-1;
		}
	}
	return rval;
}

int Song::GetFreeBus()
{
	for(int c=0;c<MAX_BUSES;c++)
	{
		if(!m_pMachine[c])
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
		if(!m_pMachine[c])
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

int Song::IffAlloc(int instrument,int layer,const TCHAR * str)
{
	if(instrument!=PREV_WAV_INS)
	{
		m_bInvalided=true;
		Sleep(LOCK_LATENCY);
	}
	
	RiffFile file;
	RiffChunkHeader hd;
	ULONG data;
	ULONGINV tmp;
	int bits=0;

	if (!file.Open(const_cast<TCHAR*>(str))) // This opens the file and reads the "FORM" header.
	{
		m_bInvalided=false;
		return 0;
	}

	DeleteLayer(instrument,layer);

	file.Read(&data,4);
	if ( data == file.FourCC("16SV")) bits = 16;
	else if ( data == file.FourCC("8SVX")) bits = 8;

	file.Read(&hd,sizeof(RiffChunkHeader));

	if ( hd._id == file.FourCC("NAME"))
	{
		file.ReadStringA2T(m_pInstrument[instrument]->waveName[layer],22); // should be hd._size instead of "22", but it is incorrectly read.
		if ( _tcscmp(m_pInstrument[instrument]->_sName,_T("empty")) == 0 )
		{
			_tcsncpy(m_pInstrument[instrument]->_sName,str,31);
			m_pInstrument[instrument]->_sName[31]=_T('\0');
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
		m_pInstrument[instrument]->waveLength[layer]=Datalen;

		if ( ls != le )
		{
			m_pInstrument[instrument]->waveLoopStart[layer]=ls;
			m_pInstrument[instrument]->waveLoopEnd[layer]=ls+le;
			m_pInstrument[instrument]->waveLoopType[layer]=true;
		}
		file.Skip(8); // Skipping unknown bytes (and volume on bytes 6&7)
		file.Read(&hd,sizeof(RiffChunkHeader));
	}

	if ( hd._id == file.FourCC("BODY"))
	{
	    short *csamples;
		const unsigned int Datalen = m_pInstrument[instrument]->waveLength[layer];

		m_pInstrument[instrument]->waveStereo[layer]=false;
		m_pInstrument[instrument]->waveDataL[layer]=new signed short[Datalen];
		csamples=m_pInstrument[instrument]->waveDataL[layer];
		
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
	m_bInvalided=false;
	return 1;
}

int Song::WavAlloc(
	int iInstr,
	int iLayer,
	bool bStereo,
	long iSamplesPerChan,
	const TCHAR * sName)
{
	ATLASSERT(iSamplesPerChan<(1<<30)); // Since in some places, signed values are used, we cannot use the whole range.

	DeleteLayer(iInstr,iLayer);

	if(bStereo)
	{
		m_pInstrument[iInstr]->waveDataL[iLayer]=new signed short[iSamplesPerChan];
		m_pInstrument[iInstr]->waveDataR[iLayer]=new signed short[iSamplesPerChan];
		m_pInstrument[iInstr]->waveStereo[iLayer]=true;
	}
	else
	{
		m_pInstrument[iInstr]->waveDataL[iLayer]=new signed short[iSamplesPerChan];
		m_pInstrument[iInstr]->waveStereo[iLayer]=false;
	}
	m_pInstrument[iInstr]->waveLength[iLayer]=iSamplesPerChan;

	_tcsncpy(m_pInstrument[iInstr]->waveName[iLayer],sName,31);
	m_pInstrument[iInstr]->waveName[iLayer][31]=_T('\0');
	
	if(iLayer==0)
	{
		_tcsncpy(m_pInstrument[iInstr]->_sName,sName,31);
		m_pInstrument[iInstr]->_sName[31]=_T('\0');
	}

	return true;
}

int Song::WavAlloc(int instrument,int layer,const TCHAR * Wavfile)
{ 
  
	ATLASSERT(Wavfile!=NULL);

	WaveFile file;
	ExtRiffChunkHeader hd;
	
	DDCRET retcode = file.OpenForRead(const_cast<TCHAR*>(Wavfile)); //This opens the file and read the format Header.
														// Also, it skips extra data, and points to the RAW wave data.
	if ( retcode != DDC_SUCCESS) 
	{
		m_bInvalided= false;
		return 0; 
	}

	m_bInvalided=true;
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
	
	short *sampL=m_pInstrument[instrument]->waveDataL[layer];
	
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
		short *sampR = m_pInstrument[instrument]->waveDataR[layer];

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
				m_pInstrument[instrument]->waveLoopStart[layer]=ls;
				m_pInstrument[instrument]->waveLoopEnd[layer]=le;
//				if (!((ls <= 0) && (le >= Datalen-1))) // **** only for my bad sample collection
				{
					m_pInstrument[instrument]->waveLoopType[layer]=true;
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
	
	m_bInvalided=false;
	return 1;

}
#endif // ndef _WINAMP_PLUGIN_

void Song::PW_Play()
{
	if (m_PW_Stage==0)
	{
		m_PW_Length=m_pInstrument[PREV_WAV_INS]->waveLength[0];
		if (m_PW_Length>0 )
		{
			m_PW_Stage=1;
			m_PW_Phase=0;
		}
	}
}

void Song::PW_Work(float *pInSamplesL, float *pInSamplesR, int numSamples)
{
	float *pSamplesL = pInSamplesL;
	float *pSamplesR = pInSamplesR;
	--pSamplesL;
	--pSamplesR;
	
	signed short *wl=m_pInstrument[PREV_WAV_INS]->waveDataL[0];
	signed short *wr=m_pInstrument[PREV_WAV_INS]->waveDataR[0];
	bool const stereo=m_pInstrument[PREV_WAV_INS]->waveStereo[0];
	float ld = 0;
	float rd = 0;
		
	do
	{
		ld=*(wl+m_PW_Phase);
		
		if(stereo)
			rd=*(wr+m_PW_Phase);
		else
			rd=ld;
			
		*++pSamplesL+=ld;
		*++pSamplesR+=rd;
			
		if(++m_PW_Phase>=m_PW_Length)
		{
			m_PW_Stage=0;
			return;
		}
		
	}while(--numSamples);
}

const bool Song::CloneMac(const int src,const int dst)
{
	// src has to be occupied and dst must be empty
	if (m_pMachine[src] && m_pMachine[dst])
	{
		return false;
	}

	if (m_pMachine[dst])
	{
		int temp = src;
		src = dst;
		dst = temp;
	}

	if (!m_pMachine[src])
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
	m_view.AddMacViewUndo();

	string filepath = m_pController->configuration()->InitialSongDir();
	filepath += _T("\\psycle.tmp");
	DeleteFile(filepath.data());

	OldPsyFile file;

	if (!file.Create(filepath.data(), true))
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

	m_pMachine[src]->SaveFileChunk(file);

	long pos2 = file.GetPos(); 
	size = pos2-pos-sizeof(size);
	file.Seek(pos);
	file.Write(&size,sizeof(size));
	file.Close();

	// now load it

	if (!file.Open(filepath.data()))
	{
		DeleteFile(filepath.data());
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
			DeleteFile(filepath.data());
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
				m_pMachine[index] = Machine::LoadFileChunk(file,index,version);
			}
			else
			{
				file.Close();
				DeleteFile(filepath.data());
				return false;
			}
		}
	}
	else
	{
		file.Close();
		DeleteFile(filepath.data());
		return false;
	}
	file.Close();
	DeleteFile(filepath.data());

	// oh and randomize the dst's position

	int xs,ys,x,y;
	if (src >= MAX_BUSES)
	{
		xs = m_pController->m_view.MachineCoords.sEffect.width;
		ys = CMainFrame::GetInstance().m_view.MachineCoords.sEffect.height;
	}
	else 
	{
		xs = CMainFrame::GetInstance().m_view.MachineCoords.sGenerator.width;
		ys = CMainFrame::GetInstance().m_view.MachineCoords.sGenerator.height;
	}

	x = m_pMachine[dst]->_x + 32;
	y = m_pMachine[dst]->_y + ys + 8;

	bool bCovered = TRUE;
	while (bCovered)
	{
		bCovered = FALSE;
		for (int i=0; i < MAX_MACHINES; i++)
		{
			if (i != dst)
			{
				if (m_pMachine[i])
				{
					if ((abs(m_pMachine[i]->_x - x) < 32) &&
						(abs(m_pMachine[i]->_y - y) < 32))
					{
						bCovered = TRUE;
						i = MAX_MACHINES;
						x = (rand())%(CMainFrame::GetInstance().m_view.CW - xs);
						y = (rand())%(CMainFrame::GetInstance().m_view.CH - ys);
					}
				}
			}
		}
	}
	m_pMachine[dst]->_x = x;
	m_pMachine[dst]->_y = y;

	// oh and delete all connections

	m_pMachine[dst]->_numInputs = 0;
	m_pMachine[dst]->_numOutputs = 0;

	for (int i = 0; i < MAX_CONNECTIONS; i++)
	{
		if (m_pMachine[dst]->_connection[i])
		{
			m_pMachine[dst]->_connection[i] = false;
			m_pMachine[dst]->_outputMachines[i] = 255;
		}

		if (m_pMachine[dst]->_inputCon[i])
		{
			m_pMachine[dst]->_inputCon[i] = false;
			m_pMachine[dst]->_inputMachines[i] = 255;
		}
	}

	int number = 1;
	TCHAR buf[sizeof(m_pMachine[dst]->_editName)+4];
	_tcscpy(buf,m_pMachine[dst]->_editName);
	TCHAR* ps = _tcsrchr(buf,_T(' '));
	if (ps)
	{
		number = _tstoi(ps);
		if (number < 1)
		{
			number =1;
		}
		else
		{
			ps[0] = 0;
			ps = _tcsrchr(m_pMachine[dst]->_editName,_T(' '));
			ps[0] = 0;
		}
	}

	for (i = 0; i < MAX_MACHINES-1; i++)
	{
		if (i!=dst)
		{
			if (m_pMachine[i])
			{
				if (_tcscmp(m_pMachine[i]->_editName,buf)==0)
				{
					number++;
					_stprintf(buf,_T("%s %d"),m_pMachine[dst]->_editName,number);
					i = -1;
				}
			}
		}
	}

	buf[sizeof(m_pMachine[dst]->_editName)-1] = 0;
	_tcscpy(m_pMachine[dst]->_editName,buf);

	return true;
}


const bool Song::CloneIns(const int src,const int dst)
{
	// src has to be occupied and dst must be empty
	if (!m_pInstrument[src]->Empty() && !m_pInstrument[dst]->Empty())
	{
		return false;
	}

	if (!m_pInstrument[dst]->Empty())
	{
		int temp = src;
		src = dst;
		dst = temp;
	}

	if (m_pInstrument[src]->Empty())
	{
		return false;
	}
	// ok now we get down to business

	CMainFrame::GetInstance().m_view.AddMacViewUndo();

	// save our file

	CString filepath(Global::pConfig->GetInitialSongDir().data());
	filepath += _T("\\psycle.tmp");
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

	m_pInstrument[src]->SaveFileChunk(file);

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
				m_pInstrument[index]->LoadFileChunk(file,version);
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
	bool bUsed = FALSE;
	if (m_ppPatternData[i])
	{
		// we could also check to see if pattern is unused AND blank.
		for (int j = 0; j < m_PlayLength; j++)
		{
			if (m_PlayOrder[j] == i)
			{
				bUsed = TRUE;
				break;
			}
		}

		if (!bUsed)
		{
			// check to see if it is empty
//			unsigned char blank[5]={255,255,255,0,0};
			unsigned char * pData = m_ppPatternData[i];
			for (j = 0; j < MULTIPLY2; j+= EVENT_SIZE)
			{
				for (int k = 0; k < EVENT_SIZE; k++)
				{
					if (pData[j+k] != BLANK_EVENT[k])
					{
						bUsed = TRUE;
						j = MULTIPLY2;
						break;
					}
				}
			}
		}
	}
	return bUsed;
}

void Song::Load()
{
	OPENFILENAME ofn;       // common dialog box structure
	TCHAR szFile[_MAX_PATH];       // buffer for file name
	
	szFile[0] = _T('\0');
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = this->GetParent().m_hWnd;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);

	SF::CResourceString _filter(IDS_MSG0022);
	boost::scoped_array<TCHAR> _pfilter(new TCHAR[ _filter.Length() + 2 ]);
	memcpy(_pfilter.get(),_filter,_filter.Length() * sizeof(TCHAR));
	
	_pfilter[_filter.Length() - 1] = 0;
	_pfilter[_filter.Length()] = 0;

	ofn.lpstrFilter = _pfilter.get();
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = m_pController->configuration()->SongDir();
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	
	// Display the Open dialog box. 
	
	if (GetOpenFileName(&ofn) == TRUE)
	{
		Load(szFile, ofn.nFilterIndex);
	}

}

void Song::Load(const TCHAR* fName,const int fType)
{
	if ( fType == 3 )
	{
		FILE* hFile = _tfopen(fName,_T("rb"));
		LoadBlock(hFile);
		fclose(hFile);
	}
	else
	{
		if (CheckUnsavedSong(SF::CResourceString(IDS_MSG0025)))
		{
			Load(fName);
		}
	}
}

void Song::Load(const TCHAR * fName)
{
	WTL::CFindFile file;
	iController* _pcon = m_pController;
	iMainFrame * _pFrame = m_pController->MainFrame();
	Song *_psong =  m_pController->Song();


	if(!file.FindFile(fName)){
		_pFrame->MessageBox(SF::CResourceString(IDS_ERR_MSG0063),
			SF::CResourceString(IDS_ERR_MSG0064), MB_OK);
	}

	string f = fName;
	string ext = f.substr(f.find_last_of(_T(".")) + 1,f.length() - (f.find_last_of(_T(".")) + 1));
	_tcslwr(const_cast<TCHAR *>(ext.c_str()));

	SongLoaderMap::iterator it = m_SongLoaderMap.find(ext);

	if(it == m_SongLoaderMap.end())
	{
		_pFrame->MessageBox(
			(SF::tformat(SF::CResourceString(IDS_ERR_MSG0065)) % fName).str().c_str());
		return;
	}
	
	_pcon->InitializeBeforeLoading();

	try {
		it->second->Load(string(fName),*_psong);
	} catch(std::exception e) {
		MessageBox((SF::tformat(SF::CResourceString(IDS_ERR_MSG0066)) % CA2T(e.what())).str().c_str());
		_psong->New();
		return;
	}

//	_pFrame->AppendToRecent(fName);
	_pFrame->SongLoaded();
	
	if(ext == _T("psf")){
		ext = _T("");
	} else {
		ext = _T(".psf");
	}

	_pcon->SetSongDirAndFileName(fName,ext.c_str());
	_pcon->ProcessAfterLoading();

	if (_pcon->configuration()->IsShowSongInfoOnLoad())
	{
		TCHAR buffer[512];
		_stprintf(buffer,_T("'%s'\n\n%s\n\n%s")
			,_pSong->Name().c_str()
			,_pSong->Author().c_str()
			,_pSong->Comment().c_str());
		
		_pFrame->MessageBox(buffer,SF::CResourceString(IDS_ERR_MSG0067),MB_OK);
	}
}

void Song::Save(const string& fileName)
{
	OldPsyFile file;

	if (!file.Create(str.c_str(), true))
	{
			m_pController->MainFrame()->MessageDialog(
				SF::CResourceString(IDS_ERR_MSG0062),
				SF::CResourceString(IDS_ERR_MSG0043), MB_OK);
			return;
	}
			
	std::auto_ptr<SF::IPsySongSaver> _saver(new SF::PsyFSongSaver());
	try {
		_saver->Save(file,*this);
	} catch (std::exception e ){
			m_pController->MainFrame()->MessageDialog(CA2T(e.what()), SF::CResourceString(IDS_ERR_MSG0043), MB_OK);
	}

	IsSaved(true);
}
//#endif // ndef _WINAMP_PLUGIN_

