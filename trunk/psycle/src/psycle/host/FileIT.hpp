// -*- mode:c++; indent-tabs-mode:t -*-
///\file
///\brief interface file for psycle::host::CFileIT.
#pragma once

#include "SongStructs.hpp"
#include "FileIO.hpp"
#include "it.hpp"
namespace psycle
	{
	namespace host
		{
		class CFileIT : public OldPsyFile
		{
		public:
			CFileIT();
			virtual ~CFileIT();
			bool Import(Song * s);	

		protected:
			bool IsValid();

			LONG ImportPatterns(Song * s);
			LONG ImportSinglePattern(Song * s, int patIdx,int iTracks);	

			bool ImportInstruments(Song * s, LONG iInstrStart);
			LONG ImportInstrument(Song *s, LONG iStart, int idx);
			LONG ImportSampleHeader(Song *s, LONG iStart, int InstrIdx, int SampleIdx);
			LONG ImportSampleData(Song *s, LONG iStart, int InstrIdx, int SampleIdx);

		protected:

			BOOL WritePatternEntry(Song * s,int patIdx,int row, int col, PatternEntry & e);
			
		protected:
			
			char * AllocReadStr(LONG size, LONG start=-1);

			ITModule mod;
			int m_iInstrCnt;

			int smpLen[256];
			char smpFlags[256];
		};
	}
}

