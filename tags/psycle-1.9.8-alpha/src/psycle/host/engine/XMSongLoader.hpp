#pragma once

#include <psycle/host/engine/SongStructs.hpp>
#include <psycle/host/engine/FileIO.hpp>
#include <psycle/host/engine/XMFile.hpp>
class Song;
class XMSampler;
class XMInstrument;

namespace psycle{
namespace host{
	class XMSongLoader : public RiffFile
	{
	public:
		XMSongLoader(void);
		virtual ~XMSongLoader(void);
		/// RIFF 
		virtual void Load(Song& song,const bool fullopen = true);
	private:
		const bool IsValid();

		const LONG LoadPatterns(Song & song);
		const LONG LoadSinglePattern(Song & song, const LONG start, const int patIdx,const int iTracks);	
		const bool LoadInstruments(XMSampler & sampler, LONG iInstrStart);
		const LONG LoadInstrument(XMSampler & sampler, LONG iStart, const int idx,int& curSample);
		const LONG LoadSampleHeader(XMSampler & sampler, LONG iStart, const int InstrIdx, const int SampleIdx);
		const LONG LoadSampleData(XMSampler & sampler, LONG iStart, const int InstrIdx, const int SampleIdx);
		const BOOL WritePatternEntry(Song & song,const int patIdx,const int row, const int col, PatternEntry & e);
		void SetEnvelopes(XMInstrument & inst,const XMSAMPLEHEADER & sampleHeader);		
		char * AllocReadStr(const LONG size, const LONG start=-1);

		// inlines
		const char ReadInt1(LONG start=-1)
		{	
			char i;
			if(start>=0) Seek(start);
			return Read(&i,1)?i:0;
		}

		const short ReadInt2(LONG start=-1)
		{
			short i;
			if(start>=0) Seek(start);
			return Read(&i,2)?i:0;
		}

		const int ReadInt4(LONG start=-1)
		{
			int i;
			if(start>=0) Seek(start);
			return Read(&i,4)?i:0;
		}

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
		XMFILEHEADER m_Header;
		XMSampler* m_pSampler;
	};



	struct MODHEADER
	{
		unsigned char songlength;
		unsigned char unused;
		unsigned char order[128];
		unsigned char pID[4];
	};
	struct MODSAMPLEHEADER
	{
		char sampleName[22];
		unsigned short sampleLength;
		unsigned char finetune;
		unsigned char volume;
		unsigned short loopStart;
		unsigned short loopLength;
	};

	class MODSongLoader : public RiffFile
	{
	public:
		MODSongLoader(void);
		virtual ~MODSongLoader(void);
		/// RIFF 
		virtual void Load(Song& song,const bool fullopen = true);
	private:
		const bool IsValid();

		const void LoadPatterns(Song & song);
		const void LoadSinglePattern(Song & song, const int patIdx,const int iTracks);	
		const unsigned char ConvertPeriodtoNote(const unsigned short period);
		const void LoadInstrument(XMSampler & sampler, const int idx);
		const void LoadSampleHeader(XMSampler & sampler, const int InstrIdx);
		const void LoadSampleData(XMSampler & sampler, const int InstrIdx);
		const BOOL WritePatternEntry(Song & song,const int patIdx,const int row, const int col, PatternEntry & e);
		char * AllocReadStr(const LONG size, const LONG start=-1);

		// inlines
		const unsigned char ReadUInt1(LONG start=-1)
		{	
			unsigned char i;
			if(start>=0) Seek(start);
			return Read(&i,1)?i:0;
		}

		const unsigned short ReadUInt2(LONG start=-1)
		{
			unsigned short i;
			if(start>=0) Seek(start);
			return Read(&i,2)?i:0;
		}

		const unsigned int ReadUInt4(LONG start=-1)
		{
			unsigned int i;
			if(start>=0) Seek(start);
			return Read(&i,4)?i:0;
		}
		static const short BIGMODPERIODTABLE[37*8];
		unsigned short smpLen[32];
		MODHEADER m_Header;
		MODSAMPLEHEADER m_Samples[32];
		XMSampler* m_pSampler;
	};
}
}