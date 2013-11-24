#pragma once
#include <psycle/host/detail/project.hpp>
#include "Global.hpp"
#include "XMFile.hpp"
#include "FileIO.hpp"
#include "XMInstrument.hpp"

namespace psycle { namespace host {
	class Song;
	class XMSampler;

	class XMSongLoader : public OldPsyFile
	{
	public:
		XMSongLoader(void);
		virtual ~XMSongLoader(void);
		/// RIFF 
		virtual void Load(Song& song,const bool fullopen = true);
		void LoadInstrumentFromFile(Song& song, const int idx);
	private:
		bool IsValid();

		size_t LoadPatterns(Song& song);
		size_t LoadSinglePattern(Song& song, size_t start, int patIdx, int iTracks);	
		bool LoadInstruments(Song& song, size_t iInstrStart);
		size_t LoadInstrument(Song& song, XMInstrument& instr, size_t iStart, int idx,int& curSample);
		size_t LoadSampleHeader(XMInstrument::WaveData<>& sample, size_t iStart, int InstrIdx, int SampleIdx);
		size_t LoadSampleData(XMInstrument::WaveData<>& sample, size_t iStart, int InstrIdx, int SampleIdx);
		BOOL WritePatternEntry(Song& song,int patIdx,int row, int col, PatternEntry & e);
		void SetEnvelopes(XMInstrument& inst,const XMSAMPLEHEADER & sampleHeader);		
		char * AllocReadStr(int size, size_t start=-1);

		// inlines
		inline char ReadInt1()
		{	
			char i;
			return Read(&i,1)?i:0;
		}

		inline short ReadInt2()
		{
			short i;
			return Read(&i,2)?i:0;
		}

		inline int ReadInt4()
		{
			int i;
			return Read(&i,4)?i:0;
		}
		inline char ReadInt1(const size_t start)
		{	
			char i;
			if(start>=0) Seek(start);
			return Read(&i,1)?i:0;
		}

		inline short ReadInt2(const size_t start)
		{
			short i;
			if(start>=0) Seek(start);
			return Read(&i,2)?i:0;
		}

		inline int ReadInt4(const size_t start)
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
		short m_extracolumn;
		short m_maxextracolumn;
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

	class MODSongLoader : public OldPsyFile
	{
	public:
		MODSongLoader();
		virtual ~MODSongLoader();
		/// RIFF 
		virtual void Load(Song& song,const bool fullopen = true);
	private:
		bool IsValid();

		void LoadPatterns(Song & song);
		void LoadSinglePattern(Song & song, int patIdx, int iTracks);	
		unsigned char ConvertPeriodtoNote(unsigned short period);
		void LoadInstrument(Song & song, int idx);
		void LoadSampleHeader(XMInstrument::WaveData<>& _wave, int InstrIdx);
		void LoadSampleData(XMInstrument::WaveData<>& _wave, int InstrIdx);
		BOOL WritePatternEntry(Song& song,int patIdx,int row, int col, PatternEntry & e);
		char * AllocReadStr(std::int32_t size, size_t start=-1);

		// inlines
		inline unsigned char ReadUInt1()
		{	
			std::uint8_t i(0);
			return Read(&i,1)?i:0;
		}

		inline unsigned short ReadUInt2()
		{
			std::uint16_t i(0);
			return Read(&i,2)?i:0;
		}

		inline unsigned int ReadUInt4()
		{
			std::uint32_t i(0);
			return Read(&i,4)?i:0;
		}
		inline unsigned char ReadUInt1(const std::int32_t start)
		{	
			std::uint8_t i(0);
			if(start>=0) Seek(start);
			return Read(&i,1)?i:0;
		}

		inline unsigned short ReadUInt2(const std::int32_t start)
		{
			std::uint16_t i(0);
			if(start>=0) Seek(start);
			return Read(&i,2)?i:0;
		}

		inline unsigned int ReadUInt4(const std::int32_t start)
		{
			std::uint32_t i(0);
			if(start>=0) Seek(start);
			return Read(&i,4)?i:0;
		}
		static const short BIGMODPERIODTABLE[37*8];
		unsigned short smpLen[32];
		bool speedpatch;
		MODHEADER m_Header;
		MODSAMPLEHEADER m_Samples[32];
		XMSampler* m_pSampler;
	};
}}
