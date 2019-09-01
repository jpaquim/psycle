///\file
///\brief interface file for psycle::host::CFileXM.
#pragma once
#include "SongStructs.hpp"
#include "FileIO.hpp"
namespace psycle
{
	namespace host
	{
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
			BOOL WritePatternEntry(Song * s,int patIdx,int row, int col, PatternEntry & e);
			char * AllocReadStr(LONG size, LONG start=-1);
			inline char ReadInt1(LONG start=-1)
			{	
				if(start>=0) Seek(start);
				char i;
				return Read(&i,1)?i:0;
			}
			inline short ReadInt2(LONG start=-1)
			{
				if(start>=0) Seek(start);
				short i;
				return Read(&i,2)?i:0;
			}
			inline int ReadInt4(LONG start=-1)
			{
				if(start>=0) Seek(start);
				int i;
				return Read(&i,4)?i:0;
			}
			int m_iInstrCnt;
			int smpLen[256];
			char smpFlags[256];
		};
	}
}
