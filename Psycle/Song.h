#ifndef _SONG_H
#define _SONG_H

#if defined(_WINAMP_PLUGIN_)
	#include <afxmt.h>
#endif // _WINAMP_PLUGIN_

#include "Machine.h"
#include "FileIO.h"
#include "SongStructs.h"

#define MAX_PLUGINS				256 // Legacy! It is used in File loading/saving
#define MAX_BUSES				64	// Power of 2! Important!
#define MAX_MACHINES			128
#define MAX_TRACKS				32	// Note: changing this breaks file format
#define MAX_WAVES				16
#define MAX_LINES				256
#define MAX_INSTRUMENTS			255
#define PREV_WAV_INS			254
#define LOCK_LATENCY			256
#define MAX_PATTERNS			128		// MAX NUM OF PATTERNS
#define MULTIPLY				MAX_TRACKS * 5				// you don't need to calc these by hand, 
#define MULTIPLY2				MULTIPLY * MAX_LINES		// precompiler will do that for you
#define MAX_PATTERN_BUFFER_LEN	MULTIPLY2 * MAX_PATTERNS	// without any affect on the generated code
#define MAX_SONG_POSITIONS		128 // Note: changing this breaks file format

class CCriticalSection;

class Instrument
{
public:
	char _sName[32];

	//////////////////////////////////////////////////////////////////
	// Loop stuff

	bool _loop;
	int _lines;

	//////////////////////////////////////////////////////////////////
	// NNA values overview:
	//
	// 0 = Note Cut			[Fast Release 'Default']
	// 1 = Note Release		[Release Stage]
	// 2 = Note Continue	[No NNA]
	unsigned char _NNA;
	
	//////////////////////////////////////////////////////////////////
	// Amplitude Envelope overview:
	//
	int ENV_AT;	// Attack Time [in Samples at 44.1Khz]
	int ENV_DT;	// Decay Time [in Samples at 44.1Khz]
	int ENV_SL;	// Sustain Level [in %]
	int ENV_RT;	// Release Time [in Samples at 44.1Khz]
	
	// Filter 
	int ENV_F_AT;	// Attack Time [in Samples at 44.1Khz]
	int ENV_F_DT;	// Decay Time [in Samples at 44.1Khz]
	int ENV_F_SL;	// Sustain Level [0..128]
	int ENV_F_RT;	// Release Time [in Samples at 44.1Khz]

	int ENV_F_CO;	// Cutoff Frequency [0-127]
	int ENV_F_RQ;	// Resonance [0-127]
	int ENV_F_EA;	// EnvAmount [-128,128]
	int ENV_F_TP;	// Filter Type [0-4]

	int _pan;
	bool _RPAN;
	bool _RCUT;
	bool _RRES;
};

class Song
{
public:

#if defined(_WINAMP_PLUGIN_)
	char fileName[_MAX_PATH];
#else
	int machineSoloed;
	CString fileName;
	CPoint viewSize;
#endif //  _WINAMP_PLUGIN_

	bool _saved;
	int _trackSoloed;
	CCriticalSection door;

	Song();
	~Song();

	char Name[64];								// Song Name
	char Author[64];							// Song Author
	char Comment[256];							// Song Comment

#if !defined(_WINAMP_PLUGIN_)
	bool Tweaker;
	
	unsigned cpuIdle;
	unsigned _sampCount;

#endif // ndef _WINAMP_PLUGIN_

	int BeatsPerMin;
	int _ticksPerBeat;
	int SamplesPerTick;
	int LineCounter;
	bool LineChanged;
	
	bool Invalided;

	char currentOctave;

	// Buses data
	unsigned char busEffect[MAX_BUSES];
	unsigned char busMachine[MAX_BUSES];

	// Pattern data
	unsigned char pPatternData[MAX_PATTERN_BUFFER_LEN];

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
	char waveName[MAX_INSTRUMENTS][MAX_WAVES][32];
	unsigned short waveVolume[MAX_INSTRUMENTS][MAX_WAVES];
	signed short *waveDataL[MAX_INSTRUMENTS][MAX_WAVES];
	signed short *waveDataR[MAX_INSTRUMENTS][MAX_WAVES];
	unsigned int waveLength[MAX_INSTRUMENTS][MAX_WAVES];
	unsigned int waveLoopStart[MAX_INSTRUMENTS][MAX_WAVES];
	unsigned int waveLoopEnd[MAX_INSTRUMENTS][MAX_WAVES];
	int waveTune[MAX_INSTRUMENTS][MAX_WAVES];
	int waveFinetune[MAX_INSTRUMENTS][MAX_WAVES];	

	bool waveLoopType[MAX_INSTRUMENTS][MAX_WAVES];
	bool waveStereo[MAX_INSTRUMENTS][MAX_WAVES];

	// Machines ------------------------------------------------------
	//
	bool _machineLock;
	bool _machineActive[MAX_MACHINES];
	Machine* _pMachines[MAX_MACHINES];

	int seqBus;

#if !defined(_WINAMP_PLUGIN_)
	int WavAlloc(int iInstr,int iLayer,const char * str);
	int WavAlloc(int iInstr,int iLayer,bool bStereo,long iSamplesPerChan,const char * sName);
	int IffAlloc(int instrument,int layer,const char * str);
#endif // ndef _WINAMP_PLUGIN_

	void New(void);
	void Reset(void);

	int GetFreeMachine(void);
	bool CreateMachine(MachineType type, int x, int y, char* psPluginDll);
	void DestroyMachine(int mac);
	void DestroyAllMachines();
#if !defined(_WINAMP_PLUGIN_)
	bool InsertConnection(int src,int dst);
	int GetFreeBus();
	int GetFreeFxBus();
	int GetNumPatternsUsed();
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

protected:

};

#endif
