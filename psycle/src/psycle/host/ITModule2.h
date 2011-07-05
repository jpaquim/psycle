#pragma once
#include <psycle/host/detail/project.hpp>
#include "Global.hpp"

#include "ITFile.h"
#include <psycle/core/commands.h>
#include <psycle/core/patternevent.h>
#include <psycle/core/fileio.h>

namespace psycle {
	namespace core {
		class Song;
		class XMSampler;
	}
	namespace host {

		using namespace universalis::stdlib;

		class ITModule2 : public RiffFile
		{
		private:
			Song* s;
			XMSampler* sampler;

		public:
			ITModule2();
			virtual ~ITModule2() throw();

			// Extra class for Reading of IT compressed samples.
			class BitsBlock
			{
			public:
				BitsBlock() : pdata(0), rpos(0), rend(0), rembits(0) {}
				~BitsBlock() throw() { delete[] pdata; }
				bool ReadBlock(RiffFile* pFile);
				unsigned long ReadBits(unsigned char bitwidth);
			private:
				/// pointer to data
				unsigned char* pdata;
				/// read position
				unsigned char* rpos;
				/// read end
				unsigned char* rend;
				/// remaining bits in current pos
				unsigned char rembits;
			};
		public:
			bool LoadITModule(Song *song);
			bool LoadOldITInst(XMSampler *sampler,int iInstIdx);
			bool LoadITInst(XMSampler *sampler,int iInstIdx);
			void LoadInstrumentFromFile(XMSampler & sampler, const int idx);
			bool LoadITSample(XMSampler *sampler,int iSampleIdx);
			bool LoadITSampleData(XMSampler *sampler,int iSampleIdx,unsigned int iLen,bool bstereo,bool b16Bit, unsigned char convert);
			bool LoadITCompressedData(XMSampler *sampler,int iSampleIdx,unsigned int iLen,bool b16Bit,unsigned char convert);
			bool LoadITPattern(int patIdx,int &numchans);
			void ParseEffect(PatternEvent&pent, float& linesPerBeat, int command,int param,int channel);
		private:
			double        ReadDouble() { double        t; Read(t); return t; }
			float         ReadFloat()  { float         t; Read(t); return t; }
			int32_t  ReadInt32()  {  int32_t t; Read(t); return t; }
			uint32_t ReadUInt32() { uint32_t t; Read(t); return t; }
			int16_t  ReadInt16()  {  int16_t t; Read(t); return t; }
			uint16_t ReadUInt16() { uint16_t t; Read(t); return t; }
			int8_t   ReadInt8()   {   int8_t t; Read(t); return t; }
			uint8_t  ReadUInt8()  {  uint8_t t; Read(t); return t; }
			bool ReadHeader(itHeader& header);
			bool ReadHeader(EmbeddedMIDIData& header);
			bool ReadHeader(itInsHeader1x& header);
			bool ReadHeader(itInsHeader2x& header);
			bool ReadHeader(ITEnvStruct& header);
			bool ReadHeader(ITNotePair& header);
			bool ReadHeader(ITNodePair& header);
			bool ReadHeader(itSampleHeader& header);

			unsigned char highOffset[64];
			EmbeddedMIDIData* embeddedData;
			itHeader itFileH;


		public:
			bool LoadS3MModuleX(Song *song);
			bool LoadS3MInstX(XMSampler *sampler,int iInstIdx);
			bool LoadS3MSampleX(XMSampler *sampler,s3mSampleHeader *currHeader,int iInstIdx,int iSampleIdx);
			bool LoadS3MSampleDataX(XMSampler *sampler,int iInstIdx,int iSampleIdx,unsigned int iLen,bool bstereo,bool b16Bit,bool packed);
			bool LoadS3MPatternX(int patIdx);
		private:
			bool ReadHeader(s3mHeader& header);
			bool ReadHeader(s3mInstHeader& header);
			s3mHeader  s3mFileH;
			
		};
	}
}
