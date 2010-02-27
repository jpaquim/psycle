#pragma once
#include "Global.hpp"
#include "XMFile.hpp"

#include <psycle/core/commands.h>
#include <psycle/core/patternevent.h>
#include <psycle/core/fileio.h>

namespace psycle { 
	namespace core {
		class Song;
		class XMSampler;
		class XMInstrument;
	}
	namespace host {

	class XMSongLoader : public RiffFile
	{
	public:
		XMSongLoader();
		/// RIFF 
		virtual void Load(Song& song,const bool fullopen = true);
		void LoadInstrumentFromFile(XMSampler & sampler, const int idx);
	private:
		bool IsValid();

		int LoadPatterns(Song& song);
		LONG LoadPattern(Song& song, const LONG start, const int patIdx,const int iTracks);	
		bool LoadInstruments(XMSampler& sampler, LONG iInstrStart);
		LONG LoadInstrument(XMSampler& sampler, LONG iStart, const int idx,int& curSample);
		LONG LoadSampleHeader(XMSampler& sampler, LONG iStart, const int InstrIdx, const int SampleIdx);
		LONG LoadSampleData(XMSampler& sampler, LONG iStart, const int InstrIdx, const int SampleIdx);		
		void SetEnvelopes(XMInstrument& inst,const XMSAMPLEHEADER & sampleHeader);		
		char * AllocReadStr(const LONG size, const LONG start=-1);

		// inlines
		char ReadInt1(LONG start=-1)
		{	
			char i;
			if(start>=0) Seek(start);
			return Read(i)?i:0;
		}

		short ReadInt2(LONG start=-1)
		{
			short i;
			if(start>=0) Seek(start);
			return Read(i)?i:0;
		}

		int ReadInt4(LONG start=-1)
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
		MODSongLoader();
		/// RIFF 
		virtual void Load(Song& song,const bool fullopen = true);
	private:
		bool IsValid();

		void LoadPatterns(Song & song);
		void LoadPattern(Song & song, const int patIdx,const int iTracks);	
		unsigned char ConvertPeriodtoNote(const unsigned short period);
		void LoadInstrument(XMSampler& sampler, const int idx);
		void LoadSampleHeader(XMSampler& sampler, const int InstrIdx);
		void LoadSampleData(XMSampler& sampler, const int InstrIdx);
		BOOL WritePatternEntry(Song& song,const int patIdx,const int row, const int col, PatternEvent & e);
		char * AllocReadStr(const LONG size, const LONG start=-1);

		bool ReadHeader(MODHEADER& header);
		bool ReadHeader(MODSAMPLEHEADER& header);

		// inlines

		unsigned char ReadUInt1(LONG start=-1)
		{	
			std::uint8_t i(0);
			if(start>=0) Seek(start);
			return Read(i)?i:0;
		}

		unsigned short ReadUInt2(LONG start=-1)
		{
			std::uint16_t i(0);
			if(start>=0) Seek(start);
			return Read(i)?i:0;
		}

		unsigned int ReadUInt4(LONG start=-1)
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
