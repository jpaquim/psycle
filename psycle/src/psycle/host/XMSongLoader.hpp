#pragma once
#include "Global.hpp"

#include <universalis/stdlib/cstdint.hpp>

#include <psycle/core/commands.h>
#include <psycle/core/patternevent.h>
#include <psycle/corE/fileio.h>

#include "XMFile.hpp"


namespace psycle {
	namespace core {
		class Song;
		class XMSampler;
		class XMInstrument;
	}
}
using namespace psycle::core;


namespace psycle { 
	namespace host {

	class XMSongLoader : public RiffFile
	{
	public:
		XMSongLoader(void);
		virtual ~XMSongLoader(void);
		/// RIFF 
		virtual void Load(Song& song,const bool fullopen = true);
		void LoadInstrumentFromFile(XMSampler & sampler, const int idx);
	private:
		const bool IsValid();

		const LONG LoadPatterns(Song& song);
		const LONG LoadPattern(Song& song, const LONG start, const int patIdx,const int iTracks);	
		const bool LoadInstruments(XMSampler& sampler, LONG iInstrStart);
		const LONG LoadInstrument(XMSampler& sampler, LONG iStart, const int idx,int& curSample);
		const LONG LoadSampleHeader(XMSampler& sampler, LONG iStart, const int InstrIdx, const int SampleIdx);
		const LONG LoadSampleData(XMSampler& sampler, LONG iStart, const int InstrIdx, const int SampleIdx);		
		void SetEnvelopes(XMInstrument& inst,const XMSAMPLEHEADER & sampleHeader);		
		char * AllocReadStr(const LONG size, const LONG start=-1);

		// inlines
		const char ReadInt1(LONG start=-1)
		{	
			char i;
			if(start>=0) Seek(start);
			return Read(i)?i:0;
		}

		const short ReadInt2(LONG start=-1)
		{
			short i;
			if(start>=0) Seek(start);
			return Read(i)?i:0;
		}

		const int ReadInt4(LONG start=-1)
		{
			int32_t i;
			if(start>=0) Seek(start);
			return Read(i)?i:0;
		}

		bool ReadHeader(XMFILEHEADER& header);
		bool ReadHeader(XMPATTERNHEADER& header);
		bool ReadHeader(XMINSTRUMENTHEADER& header);
		bool ReadHeader(XMSAMPLEHEADER& header);
		bool ReadHeader(XMSAMPLESTRUCT& header);
		bool ReadHeader(XMSAMPLEFILEHEADER& header);
		bool ReadHeader(XMINSTRUMENTFILEHEADER& header);

		int m_iInstrCnt;
		int smpLen[256];
		char smpFlags[256];
		unsigned char highOffset[32];
		unsigned char memPortaUp[32];
		unsigned char memPortaDown[32];
		unsigned char memPortaNote[32];
		unsigned char memPortaPos[32];

		Song* m_pSong;
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
		const void LoadPattern(Song & song, const int patIdx,const int iTracks);	
		const unsigned char ConvertPeriodtoNote(const unsigned short period);
		const void LoadInstrument(XMSampler& sampler, const int idx);
		const void LoadSampleHeader(XMSampler& sampler, const int InstrIdx);
		const void LoadSampleData(XMSampler& sampler, const int InstrIdx);
		const BOOL WritePatternEntry(Song& song,const int patIdx,const int row, const int col, PatternEvent & e);
		char * AllocReadStr(const LONG size, const LONG start=-1);

		bool ReadHeader(MODHEADER& header);
		bool ReadHeader(MODSAMPLEHEADER& header);


		// inlines
		const unsigned char ReadUInt1(LONG start=-1)
		{	
			std::uint8_t i(0);
			if(start>=0) Seek(start);
			return Read(i)?i:0;
		}

		const unsigned short ReadUInt2(LONG start=-1)
		{
			std::uint16_t i(0);
			if(start>=0) Seek(start);
			return Read(i)?i:0;
		}

		const unsigned int ReadUInt4(LONG start=-1)
		{
			std::uint32_t i(0);
			if(start>=0) Seek(start);
			return Read(i)?i:0;
		}
		static const short BIGMODPERIODTABLE[37*8];
		Song* m_pSong;
		unsigned short smpLen[32];
		MODHEADER m_Header;
		MODSAMPLEHEADER m_Samples[32];
		XMSampler* m_pSampler;
	};
}}
