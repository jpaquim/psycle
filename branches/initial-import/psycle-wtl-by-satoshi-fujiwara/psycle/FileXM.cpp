/** @file
 *  @brief implementation of the CFileXM class.
 *  $Date: 2004/03/08 13:50:19 $
 *  $Revision: 1.1.1.1 $
 */
#include "stdafx.h"
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

#include "FileXM.h"
#include "song.h"
#include "Instrument.h"

/*
#ifdef _DEBUG
#undef THIS_FILE
static TCHAR THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
*/
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

static const char * XM_HEADER = "extended module: ";

CFileXM::CFileXM()
{
	m_iInstrCnt=0;
}

CFileXM::~CFileXM()
{

}

bool CFileXM::Import(Song *s)
{	
	// check validity
	if(!IsValid())
		return false;

	// clear existing data
	s->DeleteAllPatterns();

	LONG iInstrStart = ImportPatterns(s);

	ImportInstruments(s,iInstrStart);
	return true;
}



bool CFileXM::IsValid()
{			

	bool bIsValid = false;

	// get header
	TCHAR * pID=AllocReadStr(17,0);

	// tracker name	
	TCHAR * pTrackerName=AllocReadStr(20,38);
	
	// get tracker version
	TCHAR * pTrackerVer=AllocReadStr(2,58);	

	// process
	ATLTRACE(_T("ͯ�ް: %s\n"),pID);
	ATLTRACE(_T("�ׯ��: %s v%i.%i\n"),pTrackerName,int(pTrackerVer[1]),int(pTrackerVer[0]));	

	// check header
	bIsValid = (!stricmp(pID,XM_HEADER));


	// cleanup
	delete[] pID;
	delete[] pTrackerName;
	delete[] pTrackerVer;

	return bIsValid;
}

long CFileXM::ImportPatterns(Song *s)
{

	// get song name
	TCHAR * pSongName = AllocReadStr(20,17);
	if(pSongName==NULL)
		return 0;
	s->Name(std::string(pSongName));	
	s->Author(std::string(_T("")));
	s->Comment(std::string(_T("Fast Tracker II Ӽޭ�ق�����߰Ă��܂����B")));

	delete[] pSongName;

	// get data
	int iHeaderLen = ReadInt4(60);
	short iSongLen = ReadInt2();
	short iRestartPos = ReadInt2();
	short iNoChannels = ReadInt2();
	short iNoPatterns = ReadInt2();
	short iNoInstruments = ReadInt2();
	short iFlags = ReadInt2();		// ignored
	short iTempoTicks = ReadInt2();
	short iTempoBPM = ReadInt2();
	
	// get pattern order	
	unsigned char playOrder[256];
	Read(playOrder,256);
	for(int i = 0;i < MAX_SONG_POSITIONS && i < 256;i++)
	{
		if ( playOrder[i] < MAX_PATTERNS ){
			s->PlayOrder(i,playOrder[i]);
		} else { 
			s->PlayOrder(i,0);
		}
	}

	if ( iSongLen > MAX_SONG_POSITIONS ){
		s->PlayLength(MAX_SONG_POSITIONS);
	} else {
		s->PlayLength(iSongLen);
	}

	s->SongTracks(iNoChannels);
	
	// tempo
	s->BeatsPerMin(6 * iTempoBPM / iTempoTicks);

	// instr count
	m_iInstrCnt = iNoInstruments;

	// get pattern data
	int nextPatStart = iHeaderLen+60;
	for(int j = 0;j < iNoPatterns && nextPatStart > 0;j++){
		nextPatStart = ImportSinglePattern(s,nextPatStart,j,iNoChannels);
	}

	return nextPatStart;
}

// import instruments
bool CFileXM::ImportInstruments(Song *s, LONG iInstrStart)
{	
	for(int i=1;i<=m_iInstrCnt;i++)
		iInstrStart = ImportInstrument(s,iInstrStart,i);

	return true;
}

TCHAR * CFileXM::AllocReadStr(LONG size, LONG start)
{
	// allocate space
	TCHAR *pData = new TCHAR[size + 1];
	if(pData==NULL)
		return NULL;

	// null terminate
	pData[size]=0;
	
	// go to offset
	if(start>=0)
		Seek(start);

	// read data
	if(Read(pData,size))
		return pData;

	delete[] pData;
	return NULL;
}





// return address of next pattern, 0 for invalid
LONG CFileXM::ImportSinglePattern(Song *s, LONG start, int patIdx, int iTracks)
{

	int iHeaderLen = ReadInt4(start);
	char iPackingType = ReadInt1();
	short iNumRows = ReadInt2();
	short iPackedSize = ReadInt2();

	if(patIdx < MAX_PATTERNS)
		s->PatternLines(patIdx,iNumRows);

	PatternEntry e;

	if(iPackedSize==0)
	{
		// build empty PatternEntry
		e._note=-1;
		e._inst=-1;
		e._mach=-1;
		e._cmd=0;
		e._parameter=0;
		e._volume=0;

		// build empty pattern
		for(int row=0;row<iNumRows;row++)
			for(int col=0;col<iTracks;col++)
				WritePatternEntry(s,patIdx,row,col,e);	
	}	
	else
	{
		// get next values
		for(int row=0;row<iNumRows;row++)
		{
			for(int col=0;col<iTracks;col++)
			{	
				// reset
				char note=-1;
				char instr=-1;
				char vol=0;
				char type=0;
				char param=0;

				// read note
				note = ReadInt1();

				// is compression bit set?
				if(note&0x80)
				{
					char bReadNote = note&0x01;
					char bReadInstr = note&0x02;
					char bReadVol = note&0x04;
					char bReadType = note&0x08;
					char bReadParam  = note&0x10;

					note = 0;
					if(bReadNote) note = ReadInt1(); 
					if(bReadInstr) instr = ReadInt1();
					if(bReadVol) vol = ReadInt1();
					if(bReadType) type = ReadInt1();
					if(bReadParam) param = ReadInt1();
				}
				else
				{
					// read all values
					instr = ReadInt1();
					vol = ReadInt1();
					type = ReadInt1();
					param = ReadInt1();				
				}								

				// translate
				e._inst = instr;	
				e._mach = 0;
				e._volume = 0;

				
				
				// volume/command
				if(vol >= 0x10 && vol <= 0x50)
				{
					// translate volume
					//e._cmd = 0x0C;
					//e._parameter = 4*(vol-0x10);
					e._volume = 4 * (vol - 0x10);
				}
//				else
				e._cmd = type;
				e._parameter =  param;					

				// instrument/note
				note = note & 0x7f;
				switch(note)
				{
					case 0x00: 
						e._note=255;
						break;// no note

					case 0x61:
						e._note=120;
						e._inst=-1;
						e._mach=0;
						break;// noteoff		
					
					default: 
						if(note>=96||note<0)
							ATLTRACE(_T("�͂��H\n"));
						
						// force note into range
						if (note>=30)
							e._note = note - 30;
						else if (note>=30-12)
							e._note = note - (30-12);
						else if (note>=30-24)
							e._note = note - (30-24);
						else if (note>=30-36)
							e._note = note - (30-36);

						break;	// transpose
				}

				if ((e._note == 255) && (e._cmd == 00) && (e._parameter == 00) && (e._inst == 255) && (e._volume == 0))
				{
					e._mach = -1;
				}
				WritePatternEntry(s,patIdx,row,col,e);	
			}
		}
	}

	//int z = ftell(_file);
	return start + iPackedSize + iHeaderLen;

}


BOOL CFileXM::WritePatternEntry(Song * s, int patIdx, int row, int col,PatternEntry &e)
{
	// don't overflow song buffer 
	if(patIdx>=MAX_PATTERNS) return false;

	PatternEntry* pData = (PatternEntry*) s->_ptrackline(patIdx,col,row);

	*pData = e;

	return true;
}	

LONG CFileXM::ImportInstrument(Song *s, LONG iStart, int idx)
{
	// read header
	Seek(iStart);
	int iInstrSize = ReadInt4();
	ATLASSERT(iInstrSize==0x107||iInstrSize==0x21);

	char * sInstrName = AllocReadStr(22);
	int iInstrType = ReadInt1();
	int iSampleCount = ReadInt2();

	if(iSampleCount>1)
 		ATLTRACE(_T("����ٶ��� = %d\n"),iSampleCount);

	// store instrument name
	strcpy(s->pInstrument(idx)->_sName,sInstrName);
	delete [] sInstrName;

	int iSampleHeader = ReadInt4();
	ATLASSERT(iSampleHeader==0x28);
	// exit if empty

	iStart += iInstrSize;
	
	if(iSampleCount==0)
		return iStart;

	// read instrument data	
	
	// load individual samples
	for(int i=0;i<iSampleCount;i++)
		iStart = ImportSampleHeader(s,iStart,idx,i);
	for(i=0;i<iSampleCount;i++)
		iStart = ImportSampleData(s,iStart,idx,i);

	return iStart;
}

LONG CFileXM::ImportSampleHeader(Song *s, LONG iStart, int iInstrIdx, int iSampleIdx)
{
	// get sample header
	Seek(iStart);
	int iLen = ReadInt4();

	// loop data
	int iLoopStart = ReadInt4();
	int iLoopLength = ReadInt4();

	// params
	char iVol = ReadInt1();
	char iFineTune = ReadInt1();
	char iFlags = ReadInt1();
	char iPanning = ReadInt1();
	char iRelativeNote = ReadInt1();
	char iReserved = ReadInt1();	

	// sample name
	char * sName = AllocReadStr(22);
	
	// parse
	BOOL bLoop = (iFlags & 0x01 || iFlags & 0x02) && (iLoopLength>0);
	BOOL bPingPong = iFlags & 0x02;
	BOOL b16Bit = iFlags & 0x10;

	// alloc wave memory
	if(b16Bit)
		s->WavAlloc(iInstrIdx,iSampleIdx,false,iLen/2,sName);
	else
		s->WavAlloc(iInstrIdx,iSampleIdx,false,iLen,sName);
	delete[] sName;

	if(bLoop)
	{
		s->pInstrument(iInstrIdx)->waveLoopType[iSampleIdx]=true;
		if(b16Bit)
		{
			s->pInstrument(iInstrIdx)->waveLoopStart[iSampleIdx]=iLoopStart/2;
			s->pInstrument(iInstrIdx)->waveLoopEnd[iSampleIdx]=(iLoopLength+iLoopStart)/2;
		}
		else
		{
			s->pInstrument(iInstrIdx)->waveLoopStart[iSampleIdx]=iLoopStart;
			s->pInstrument(iInstrIdx)->waveLoopEnd[iSampleIdx]=iLoopLength+iLoopStart;
		}
	}


	s->pInstrument(iInstrIdx)->waveVolume[iSampleIdx]= iVol;
	s->pInstrument(iInstrIdx)->waveTune[iSampleIdx] = iRelativeNote;	
//	s->pInstrument(iInstrIdx)->waveFinetune[iSampleIdx] = int((100.0*iFineTune)/128);
	s->pInstrument(iInstrIdx)->waveFinetune[iSampleIdx] = iFineTune*2;

	smpLen[iSampleIdx] = iLen;
	smpFlags[iSampleIdx] = iFlags;

	return iStart+40;

}

LONG CFileXM::ImportSampleData(Song *s, LONG iStart, int iInstrIdx, int iSampleIdx)
{
	// parse
	
	BOOL b16Bit = smpFlags[iSampleIdx] & 0x10;
	
	short wNew=0;

	// cache sample data
	Seek(iStart);
	char * smpbuf = new char[smpLen[iSampleIdx]];
	Read(smpbuf,smpLen[iSampleIdx]);

	int sampleCnt = smpLen[iSampleIdx];

	// unpack sample data
	if(b16Bit)
	{				
		// 16 bit mono sample, delta
		int out=0;
		for(int j=0;j<sampleCnt;j+=2)
		{
			wNew += 0xFF & smpbuf[j] | smpbuf[j+1]<<8;				
			s->pInstrument(iInstrIdx)->waveDataL[iSampleIdx][out] = wNew;
			out++;
		}   
	}
	else
	{
		// 8 bit mono sample
		for(int j=0;j<sampleCnt;j++)
		{			
			wNew += (smpbuf[j]<<8);// | char(rand())); // scale + dither
			s->pInstrument(iInstrIdx)->waveDataL[iSampleIdx][j] = wNew;
		}
	}

	// cleanup
	delete[]smpbuf;

	// complete			
	iStart += smpLen[iSampleIdx];
	return iStart;
}