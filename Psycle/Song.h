#ifndef _SONG_H
#define _SONG_H

#if defined(_WINAMP_PLUGIN_)
//	#include <afxmt.h>
#endif // _WINAMP_PLUGIN_

#include "Constants.h"
#include "FileIO.h"
#include "SongStructs.h"
#include "Instrument.h"

class CCriticalSection;
class Machine;

class Song
{
public:

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
	unsigned char * ppPatternData[MAX_PATTERNS];

	int playLength;
	unsigned char playOrder[MAX_SONG_POSITIONS];

#if !defined(_WINAMP_PLUGIN_)
	bool playOrderSel[MAX_SONG_POSITIONS];
#endif // ndef _WINAMP_PLUGIN_

	int patternLines[MAX_PATTERNS];
	char patternName[MAX_PATTERNS][32];
	int SONGTRACKS;

	int midiSelected;
	int auxcolSelected;
	int _trackArmedCount;
	// InstrumentData
	int instSelected;
	Instrument _instruments[MAX_INSTRUMENTS];

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
#endif // ndef _WINAMP_PLUGIN_
	void DeleteAllPatterns(void);
	void DeleteInstrument(int i);
	void DeleteInstruments();
	void DeleteLayer(int i,int c);
	void SetBPM(int bpm, int tpb, int srate);


	bool Load(RiffFile* pFile);
#if !defined(_WINAMP_PLUGIN_)
	bool Save(RiffFile* pFile);

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
	if (!ppPatternData[ps])
	{
		return CreateNewPattern(ps);
	}
	return ppPatternData[ps];
}

inline unsigned char * Song::_ptrack(int ps, int track)
{
	if (!ppPatternData[ps])
	{
		return CreateNewPattern(ps);
	}
	return ppPatternData[ps] + (track*EVENT_SIZE);
}	

inline unsigned char * Song::_ptrackline(int ps, int track, int line)
{
	if (!ppPatternData[ps])
	{
		return CreateNewPattern(ps);
	}
	return ppPatternData[ps] + (track*EVENT_SIZE) + (line*MULTIPLY);
}

#endif
