#ifndef _INSTRUMENT_H
#define _INSTRUMENT_H

#if defined(_WINAMP_PLUGIN_)
//	#include <afxmt.h>
#endif // _WINAMP_PLUGIN_

#include "Constants.h"
#include "FileIO.h"

class Instrument
{
public:

	Instrument();
	~Instrument();
	void Delete();
	void DeleteLayer(int c);
	void LoadFileChunk(RiffFile* pFile,int version,bool fullopen=true);
	void SaveFileChunk(RiffFile* pFile);
	bool Empty();

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

	char _sName[32];

	// wave stuff

	unsigned int waveLength[MAX_WAVES];
	unsigned short waveVolume[MAX_WAVES];
	unsigned int waveLoopStart[MAX_WAVES];
	unsigned int waveLoopEnd[MAX_WAVES];
	int waveTune[MAX_WAVES];
	int waveFinetune[MAX_WAVES];	
	bool waveLoopType[MAX_WAVES];
	bool waveStereo[MAX_WAVES];
	char waveName[MAX_WAVES][32];

	signed short *waveDataL[MAX_WAVES];
	signed short *waveDataR[MAX_WAVES];

};


#endif
