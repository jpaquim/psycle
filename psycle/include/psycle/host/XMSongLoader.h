#pragma once
#pragma unmanaged
/** @file
 *  @brief XMSongLoader Implement Class.
 *  $Date$
 *  $Revision$
 *  参考:MPT のソース
 */

#include "isongloader.h"
#include "XMFile.h"

class OldPsyFile;

namespace SF {
	class XMSongLoader : public ISongLoader
	{
	public:
		XMSongLoader(void);
		virtual ~XMSongLoader(void);
		/// RIFFファイルからソングをロードする
		virtual void Load(SF::string& fileName,Song& song,const bool fullopen = true);
	private:
		const bool IsValid();

		const LONG LoadPatterns(Song & song);
		const LONG LoadSinglePattern(Song & song, const LONG start, const int patIdx,const int iTracks);	
		const bool LoadInstruments(XMSampler & sampler, LONG iInstrStart);
		const LONG LoadInstrument(XMSampler & sampler, LONG iStart, const int idx);
		const LONG LoadSampleHeader(XMSampler & sampler, LONG iStart, const int InstrIdx, const int SampleIdx);
		const LONG LoadSampleData(XMSampler & sampler, LONG iStart, const int InstrIdx, const int SampleIdx);
		const BOOL WritePatternEntry(Song & song,const int patIdx,const int row, const int col, PatternEntry & e);
		void ReadEnvelopes(XMInstrument & inst,const XMSAMPLEHEADER & sampleHeader);		
		char * AllocReadStr(const LONG size, const LONG start=-1);

		// inlines
		const char ReadInt1(LONG start=-1)
		{	
			char i;
			if(start>=0) m_File.Seek(start);
			return m_File.Read(&i,1)?i:0;
		}

		const short ReadInt2(LONG start=-1)
		{
			short i;
			if(start>=0) m_File.Seek(start);
			return m_File.Read(&i,2)?i:0;
		}

		const int ReadInt4(LONG start=-1)
		{
			int i;
			if(start>=0) m_File.Seek(start);
			return m_File.Read(&i,4)?i:0;
		}

		int m_iInstrCnt;
		int smpLen[256];
		char smpFlags[256];
		OldPsyFile m_File;
		short m_iTempoTicks;
		short m_iTempoBPM;
		XMFILEHEADER m_Header;
		XMSampler* m_pSampler;
	};
}
