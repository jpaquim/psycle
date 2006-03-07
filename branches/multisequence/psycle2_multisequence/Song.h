#ifndef _SONG_H
#define _SONG_H

#if defined(_WINAMP_PLUGIN_)
//	#include <afxmt.h>
#endif // _WINAMP_PLUGIN_

#include <vector>
using namespace std;

#include "Constants.h"
#include "FileIO.h"
#include "SongStructs.h"
#include "Instrument.h"
#include "Helpers.h"
#include "Configuration.h"

class CCriticalSection;
class Machine;

class Sequence
{
public: 
	int		patternID;
};

class SequenceTrack
{
public: 
	Sequence *		pSequences[MAX_SEQUENCES];
	unsigned char * ppPatternData[MAX_PATTERNS];
	int				patternLines[MAX_PATTERNS];
	char			patternName[MAX_PATTERNS][32];
	char			trackName[32];
	int				SONGTRACKS;
	int				patCount;
	COLORREF		patColor;

	SequenceTrack()
	{
		for (int i = 0; i < MAX_SEQUENCES; i++ ) pSequences[i] = NULL;
		for (int i = 0; i < MAX_PATTERNS; i++) 
		{
			ppPatternData[i] = NULL;
			patternLines[i]	= Global::pConfig->defaultPatLines; // default 
			strcpy(patternName[i],"Untitled");
		}
		patColor = Global::pConfig->pvc_row2; // default 
		CreateNewPattern(0);
		patCount = 1; // every track has one empty pattern - ID 0 
		strcpy(trackName,"default");

	}
	~SequenceTrack()
	{
		for (int i=0; i<MAX_PATTERNS; i++)	RemovePattern(i);
	}


	unsigned char * CreateNewPattern(int ps)
	{
		RemovePattern(ps);
		ppPatternData[ps] = new unsigned char[MULTIPLY2];
		unsigned char blank[5]={255,255,255,0,0};

		unsigned char * pData = ppPatternData[ps];
		for (int i = 0; i < MULTIPLY2; i+= EVENT_SIZE)
		{
			memcpy(pData,blank,5*sizeof(unsigned char));
			pData+= EVENT_SIZE;
		}
		return ppPatternData[ps];
	}

	void ClearPattern(int ps)
	{
		unsigned char blank[5]={255,255,255,0,0};
		unsigned char * pData = ppPatternData[ps];
		for (int i = 0; i < MULTIPLY2; i+= EVENT_SIZE)
		{
			memcpy(pData,blank,5*sizeof(unsigned char));
			pData+= EVENT_SIZE;
		}
	}

	void RemovePatternFromTrack(int ps)
	{
		for ( int i = ps; i < patCount; i++ )
		{
			ppPatternData[i] = ppPatternData[i+1];
			patternLines[i]  = patternLines[i+1];
			strcpy(patternName[i],patternName[i+1]);
		}

		for ( int i = 0; i < MAX_SEQUENCES; i++ )
		{
			if ( pSequences[i] != NULL )
			{
				if ( pSequences[i]->patternID == ps )
					pSequences[i]->patternID = -1;
				if ( pSequences[i]->patternID > ps )
					pSequences[i]->patternID--;
			}
		}
	}


	void RemovePattern(int ps)
	{
		if (ppPatternData[ps])
		{
			delete ppPatternData[ps];
			ppPatternData[ps] = NULL;
		}
	}

	inline unsigned char * _ppattern(int ps)
	{
		if (!ppPatternData[ps])
		{
			return CreateNewPattern(ps);
		}
		return ppPatternData[ps];
	}

	inline unsigned char * _ptrack(int ps, int track)
	{
		if (!ppPatternData[ps])
		{
			return CreateNewPattern(ps);
		}
		return ppPatternData[ps] + (track*EVENT_SIZE);
	}	

	inline unsigned char *_ptrackline(int ps, int track, int line)
	{
		ASSERT( line >= 0 && line < MAX_LINES );
		if ( ps < 0 || ps > MAX_PATTERNS ) ps = 0;	
		if ( track < 0 || track > MAX_PATTERNS ) track = 0;
		if ( line < 0 || line > MAX_LINES ) line = 0;
		if (!ppPatternData[ps])
		{
			return CreateNewPattern(ps);
		}
		return ppPatternData[ps] + (track*EVENT_SIZE) + (line*MULTIPLY);
	}

	bool AllocNewPattern(int pattern,char *name,int lines,bool adaptsize)
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
					toffset=_ptrack(pattern,t);
					for (int l=1;l<lines;l++)
					{
						memcpy(toffset+l*MULTIPLY,toffset+f2i(l*step)*MULTIPLY,EVENT_SIZE);
					}
					while (l < patternLines[pattern])	// This wouldn't be necessary if we
					{									// really allocate a new pattern.
						memcpy(toffset+(l*MULTIPLY),blank,EVENT_SIZE);
						l++;
					}
				}
				patternLines[pattern] = lines;	// This represents the allocation of the new pattern
			}
			else if( patternLines[pattern] < lines )
			{
				step= (float)lines/patternLines[pattern];
				int nl= patternLines[pattern];
				
				for (int t=0;t<SONGTRACKS;t++)
				{
					toffset=_ptrack(pattern,t);

					for (int l=nl-1;l>0;l--)
					{
						memcpy(toffset+f2i(l*step)*MULTIPLY,toffset+l*MULTIPLY,EVENT_SIZE);
						int tz=f2i(l*step)-1;
						while (tz> (l-1)*step)
						{
							memcpy(toffset+tz*MULTIPLY,blank,EVENT_SIZE);
							tz--;
						}
					}
				}
				patternLines[pattern] = lines;	// This represents the allocation of the new pattern
			}
		}
		else
		{
			int l = patternLines[pattern];
			while (l < lines)	// This wouldn't be necessary if we
			{									// really allocate a new pattern.
				for (int t=0;t<SONGTRACKS;t++)
				{
					toffset=_ptrackline(pattern,t,l);
					memcpy(toffset,blank,EVENT_SIZE);
					l++;
				}
			}
			patternLines[pattern] = lines;
		}

		sprintf(patternName[pattern], name);

		return true;
	}

	bool IsPatternUsed(int i)
	{
		bool bUsed = FALSE;
		if (ppPatternData[i])
		{
			if (!bUsed)
			{
				// check to see if it is empty
				unsigned char blank[5]={255,255,255,0,0};
				unsigned char * pData = ppPatternData[i];
				for (int j = 0; j < MULTIPLY2; j+= EVENT_SIZE)
				{
					for (int k = 0; k < 5; k++)
					{
						if (pData[j+k] != blank[k])
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

};

class Sequencer
{
public:
	// Sequence data
	SequenceTrack * pSequenceData[MAX_MACHINES];
	int				startLine;
	int				endLine; // used to set up start and end line loop
	vector<int>		machineList; // numbering of machines in sequence tracks, rearanging track view

	Sequencer()
	{
		for (int i=0; i< MAX_MACHINES; i++)
		{
			pSequenceData[i] = new SequenceTrack();
		}
		machineList.clear();
		machineList.push_back(0x80); // wrzucamy MASTERA!!!
			
	}

	void CreateMachineSequenceTrack(int machineID, char * trackName)
	{
		SequenceTrack * pSeq = pSequenceData[machineID];
		strcpy(pSeq->trackName, trackName);
		for (int i=0; i< MAX_SEQUENCES; i++)
		{
			pSeq->pSequences[i] = new Sequence();
			pSeq->pSequences[i]->patternID = -1;
		}
		pSeq->SONGTRACKS = 16; // defalut pattern track
		if ( machineID != 128 && machineID < 0x40 ) // master
		{
			machineList.push_back(machineID);
		}
	};
};

class SeqCopyItem
{
public:
	int row;
	int col;
	int patternID;
};

class SeqCopyBlock
{
public:
	vector<SeqCopyItem> copyBlock;
	int rows;
	int cols; // number of rows and cols in selection block
	int begCol; // indicates track column in which ones can paste block
	SeqCopyBlock()
	{
		copyBlock.clear();
	}
};

class Song
{
public:

	int GetMachineCount(void);


#if defined(_WINAMP_PLUGIN_)
	char fileName[_MAX_PATH];
	long filesize;
#else
	int machineSoloed;
	CString fileName;
	CPoint viewSize;
#endif //  _WINAMP_PLUGIN_

	bool _saved;
	int _trackSoloed;

#if !defined(_WINAMP_PLUGIN_)
	CCriticalSection door;
#endif // !defined(_WINAMP_PLUGIN_)

	Song();
	~Song();

	char Name[64];								// Song Name
	char Author[64];							// Song Author
	char Comment[256];							// Song Comment

#if !defined(_WINAMP_PLUGIN_)
	bool Tweaker;
	
	unsigned cpuIdle;
	unsigned _sampCount;

	bool Invalided;
	
#endif // ndef _WINAMP_PLUGIN_

	int BeatsPerMin;
	int _ticksPerBeat;
	int SamplesPerTick;
	int LineCounter;
	bool LineChanged;
	
	char currentOctave;

	// Buses data
//	unsigned char busEffect[MAX_BUSES];
//	unsigned char busMachine[MAX_BUSES];

	// Pattern data
	//unsigned char * ppPatternData[MAX_PATTERNS];

	// KRISC
	Sequencer * pSequencer; // MAIN SEQUENCE EDITOR OBJECT
	int			activeTrack;		// ID OF ACTIVE (EDITABLE) SEQUENCE TRACK
	void		ClearPattern(int ps);
	void		RemovePatternFromTrack(int ps);
	SeqCopyBlock selBlock;	// SELECTION BLOCK FOR SEQUENCER COPY PASTE ETC
	int			patStep; // PATTERN VIEW STEP FEATURE 

	

	int playLength;
	unsigned char playOrder[MAX_SONG_POSITIONS];

#if !defined(_WINAMP_PLUGIN_)
	bool playOrderSel[MAX_SONG_POSITIONS];
#endif // ndef _WINAMP_PLUGIN_

//	int patternLines[MAX_PATTERNS];
//	char patternName[MAX_PATTERNS][32];
//	int SONGTRACKS;

	int midiSelected;
	int auxcolSelected;
	int _trackArmedCount;
	// InstrumentData
	int instSelected;
	Instrument * _pInstrument[MAX_INSTRUMENTS];

	bool _trackMuted[MAX_TRACKS];
	bool _trackArmed[MAX_TRACKS];

	// WaveData ------------------------------------------------------
	//
	int waveSelected;
	// Machines ------------------------------------------------------
	//
	bool _machineLock;
	Machine* _pMachine[MAX_MACHINES];

	int seqBus;

#if !defined(_WINAMP_PLUGIN_)
	int WavAlloc(int iInstr,int iLayer,const char * str);
	int WavAlloc(int iInstr,int iLayer,bool bStereo,long iSamplesPerChan,const char * sName);
	int IffAlloc(int instrument,int layer,const char * str);
#endif // ndef _WINAMP_PLUGIN_

	void New(void);
	void Reset(void);

	int GetFreeMachine(void);
	bool CreateMachine(MachineType type, int x, int y, char* psPluginDll, int index);
	void DestroyMachine(int mac);
	void DestroyAllMachines();
	int GetNumPatternsUsed();
#if !defined(_WINAMP_PLUGIN_)
	bool InsertConnection(int src,int dst,float value = 1.0f);
	int GetFreeBus();
	int GetFreeFxBus();
	int FindBusFromIndex(int smac);
	int GetBlankPatternUnused(int rval = 0);
	bool AllocNewPattern(int pattern,char *name,int lines,bool adaptsize);
	bool CloneMac(int src,int dst);
	bool CloneIns(int src,int dst);

#endif // ndef _WINAMP_PLUGIN_
	void DeleteAllPatterns(void);
	void DeleteInstrument(int i);
	void DeleteInstruments();
	void DeleteLayer(int i,int c);
	void DestroyAllInstruments();

	void SetBPM(int bpm, int tpb, int srate);


	bool Load(RiffFile* pFile, bool fullopen=true); // The "fullopen" attribute is used in context of the
	bool LoadPsy2Song(RiffFile* pFile, bool fullopen=true); // The "fullopen" attribute is used in context of the
	bool LoadPsy3Song(RiffFile* pFile, bool fullopen=true); // The "fullopen" attribute is used in context of the
	bool LoadPsy4Song(RiffFile* pFile, bool fullopen=true); // The "fullopen" attribute is used in context of the
	bool LoadPsy5Song(RiffFile* pFile, bool fullopen=true); // The "fullopen" attribute is used in context of the
	bool LoadPsy6Song(RiffFile* pFile, bool fullopen=true); // The "fullopen" attribute is used in context of the
	bool LoadPsy7Song(RiffFile* pFile, bool fullopen=true); // The "fullopen" attribute is used in context of the
	bool LoadPsy8Song(RiffFile* pFile, bool fullopen=true); // The "fullopen" attribute is used in context of the
	bool LoadPsy9Song(RiffFile* pFile, bool fullopen=true); // The "fullopen" attribute is used in context of the

#if !defined(_WINAMP_PLUGIN_)						// winamp/foobar player plugins, where it allows to get
	bool Save(RiffFile* pFile,bool autosave=false);	// the info of the file, without needing to open it completely.
	bool IsPatternUsed(int i);

	// Previews waving

	void PW_Work(float *psamplesL, float *pSamplesR, int numSamples);
	void PW_Play();
	
	int PW_Phase;
	int PW_Stage;
	int PW_Length;

#endif // ndef _WINAMP_PLUGIN_
	inline unsigned char * _ppattern(int ps);
	inline unsigned char * _ptrack(int ps, int track);
	inline unsigned char * _ptrackline(int ps, int track, int line);

	unsigned char * CreateNewPattern(int ps);
	void RemovePattern(int ps);

protected:

};


inline unsigned char * Song::_ppattern(int ps)
{
	return pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->_ppattern(ps);
}

inline unsigned char * Song::_ptrack(int ps, int track)
{
	return pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->_ptrack(ps,track);
}	

inline unsigned char * Song::_ptrackline(int ps, int track, int line)
{
	return pSequencer->pSequenceData[pSequencer->machineList.at(activeTrack)]->_ptrackline(ps,track,line);
}

#endif
