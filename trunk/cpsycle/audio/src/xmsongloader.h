// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_XM_SONGLOADER_H
#define psy_audio_XM_SONGLOADER_H

#include "../../detail/psydef.h"

// local
#include "songio.h"
#include "xmdefs.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XMSongLoader {
	int m_iInstrCnt;
	int smpLen[256];
	char smpFlags[256];
	unsigned char highOffset[32];
	unsigned char memPortaUp[32];
	unsigned char memPortaDown[32];
	unsigned char memPortaNote[32];
	unsigned char memPortaPos[32];

	short m_iTempoTicks;
	short m_iTempoBPM;
	short m_extracolumn;
	short m_maxextracolumn;
	XMFILEHEADER m_Header;
	// XMSampler* m_pSampler;
	psy_audio_SongFile* songfile;
} XMSongLoader;

void xmsongloader_init(XMSongLoader*, psy_audio_SongFile*);
void xmsongloader_dispose(XMSongLoader*);

bool xmsongloader_load(XMSongLoader*);

/*
// class Song;
// class XMSampler;
// class CProgressDialog;


/*
//public:	
	/// RIFF 
virtual bool Load(Song& song, CProgressDialog& progress, bool fullopen = true);
int LoadInstrumentFromFile(LoaderHelper& loadhelp);
// private
bool IsValid();

size_t LoadPatterns(Song& song, std::map<int, int>& xmtovirtual);
size_t LoadSinglePattern(Song& song, size_t start, int patIdx, int iTracks, std::map<int, int>& xmtovirtual);
bool LoadInstruments(Song& song, size_t iInstrStart, std::map<int, int>& xmtovirtual);
size_t LoadInstrument(Song& song, XMInstrument& instr, size_t iStart, int idx, int& curSample);
size_t LoadSampleHeader(XMInstrument::WaveData<>& sample, size_t iStart, int InstrIdx, int SampleIdx);
size_t LoadSampleData(XMInstrument::WaveData<>& sample, size_t iStart, int InstrIdx, int SampleIdx);
BOOL WritePatternEntry(Song& song, int patIdx, int row, int col, PatternEntry& e);
void SetEnvelopes(XMInstrument& inst, const XMSAMPLEHEADER& sampleHeader);
char* AllocReadStr(int size, signed int start = -1); */

/*
// inlines
inline char ReadInt1()
{
	char i;
	return Read(&i, 1) ? i : 0;
}

inline short ReadInt2()
{
	short i;
	return Read(&i, 2) ? i : 0;
}

inline int ReadInt4()
{
	int i;
	return Read(&i, 4) ? i : 0;
}
inline char ReadInt1(const signed int start)
{
	char i;
	if (start >= 0) Seek(start);
	return Read(&i, 1) ? i : 0;
}

inline short ReadInt2(const signed int start)
{
	short i;
	if (start >= 0) Seek(start);
	return Read(&i, 2) ? i : 0;
}

inline int ReadInt4(const signed int start)
{
	int i;
	if (start >= 0) Seek(start);
	return Read(&i, 4) ? i : 0;
}




/* 
// public
	/// RIFF
	virtual bool Load(Song &song,CProgressDialog& progress, bool fullopen=true);
	bool IsValid();

	void LoadPatterns(Song & song, std::map<int,int>& modtovirtual);
	void LoadSinglePattern(Song & song, int patIdx, int iTracks, std::map<int,int>& modtovirtual);
	unsigned char ConvertPeriodtoNote(unsigned short period);
	void LoadInstrument(Song & song, int idx);
	void LoadSampleHeader(XMInstrument::WaveData<>& _wave, int InstrIdx);
	void LoadSampleData(XMInstrument::WaveData<>& _wave, int InstrIdx);
	BOOL WritePatternEntry(Song& song,int patIdx,int row, int col, PatternEntry & e);
	char * AllocReadStr(int32_t size, signed int start=-1);

	



*/

typedef struct MODHEADER
{
	unsigned char songlength;
	unsigned char unused;
	unsigned char order[128];
	unsigned char pID[4];
} MODHEADER;

typedef struct MODSAMPLEHEADER
{
	char sampleName[22];
	unsigned short sampleLength;
	unsigned char finetune;
	unsigned char volume;
	unsigned short loopStart;
	unsigned short loopLength;
} MODSAMPLEHEADER;

typedef struct MODSongLoader
{
	unsigned short smpLen[32];
	bool speedpatch;
	MODHEADER m_Header;
	MODSAMPLEHEADER m_Samples[32];
	// XMSampler* m_pSampler;
	psy_audio_SongFile* songfile;
} MODSongLoader;

void modsongloader_init(MODSongLoader*, psy_audio_SongFile*);
void modsongloader_dispose(MODSongLoader*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_XM_SONGLOADER_H */
