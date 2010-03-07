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
		int32_t LoadPattern(Song& song, int32_t start, int patIdx, int iTracks);	
		bool LoadInstruments(XMSampler& sampler, int32_t iInstrStart);
		int32_t LoadInstrument(XMSampler& sampler, int32_t iStart, int idx, int & curSample);
		int32_t LoadSampleHeader(XMSampler& sampler, int32_t iStart, int InstrIdx, int SampleIdx);
		int32_t LoadSampleData(XMSampler& sampler, int32_t iStart, int InstrIdx, int SampleIdx);		
		void SetEnvelopes(XMInstrument& inst, const XMSAMPLEHEADER & sampleHeader);		
		char * AllocReadStr(int32_t size, int32_t start=-1);

		// inlines
		char ReadInt1(int32_t start=-1)
		{	
			char i;
			if(start>=0) Seek(start);
			return Read(i)?i:0;
		}

		short ReadInt2(int32_t start=-1)
		{
			short i;
			if(start>=0) Seek(start);
			return Read(i)?i:0;
		}

		int ReadInt4(int32_t start=-1)
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
		void LoadPattern(Song & song, int patIdx, int iTracks);	
		unsigned char ConvertPeriodtoNote(unsigned short period);
		void LoadInstrument(XMSampler& sampler, int idx);
		void LoadSampleHeader(XMSampler& sampler, int InstrIdx);
		void LoadSampleData(XMSampler& sampler, int InstrIdx);
		BOOL WritePatternEntry(Song& song, int patIdx, int row, int col, PatternEvent & e);
		char * AllocReadStr(int32_t size, int32_t start=-1);

		bool ReadHeader(MODHEADER& header);
		bool ReadHeader(MODSAMPLEHEADER& header);

		// inlines

		unsigned char ReadUInt1(int32_t start=-1)
		{	
			std::uint8_t i(0);
			if(start>=0) Seek(start);
			return Read(i)?i:0;
		}

		unsigned short ReadUInt2(int32_t start=-1)
		{
			std::uint16_t i(0);
			if(start>=0) Seek(start);
			return Read(i)?i:0;
		}

		unsigned int ReadUInt4(int32_t start=-1)
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
