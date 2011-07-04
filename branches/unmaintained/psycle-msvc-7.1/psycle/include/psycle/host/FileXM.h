// FileXM.h: interface for the CFileXM class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILEXM_H__A81CE1CB_9D31_4321_BF9F_55F208522988__INCLUDED_)
#define AFX_FILEXM_H__A81CE1CB_9D31_4321_BF9F_55F208522988__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "SongStructs.h"
#include "FileIO.h"

class CFileXM : public OldPsyFile
{
public:
	CFileXM();
	virtual ~CFileXM();
	bool Import(Song * s);	

protected:
	bool IsValid();

	LONG ImportPatterns(Song * s);
	LONG ImportSinglePattern(Song * s, LONG start, int patIdx,int iTracks);	

	bool ImportInstruments(Song * s, LONG iInstrStart);
	LONG ImportInstrument(Song *s, LONG iStart, int idx);
	LONG ImportSampleHeader(Song *s, LONG iStart, int InstrIdx, int SampleIdx);
	LONG ImportSampleData(Song *s, LONG iStart, int InstrIdx, int SampleIdx);

protected:

	BOOL WritePatternEntry(Song * s,int patIdx,int row, int col, PatternEntry & e);
	
protected:
	
	char * AllocReadStr(LONG size, LONG start=-1);

	// inlines
	char ReadInt1(LONG start=-1)
	{	
		char i;
		if(start>=0) Seek(start);
		return Read(&i,1)?i:0;
	}
	short ReadInt2(LONG start=-1)
	{
		short i;
		if(start>=0) Seek(start);
		return Read(&i,2)?i:0;
	}
	int ReadInt4(LONG start=-1)
	{
		int i;
		if(start>=0) Seek(start);
		return Read(&i,4)?i:0;
	}

	int m_iInstrCnt;

	int smpLen[256];
	char smpFlags[256];
};

#endif // !defined(AFX_FILEXM_H__A81CE1CB_9D31_4321_BF9F_55F208522988__INCLUDED_)
