#pragma once
#include "SongStructs.hpp"
#include "FileIO.hpp"

class Song;

namespace psycle
	{
	namespace host
		{
		class ITModule2 : public OldPsyFile
		{
		public:
			ITModule2(void);
			~ITModule2(void);
			char * AllocReadStr(LONG size, LONG start=-1);
			bool LoadITModule(Song *s);
			bool LoadS3MModule(Song *s);
			bool LoadS3MInst(Song *s,int iInstIdx);
			bool LoadS3MSample(Song *s,int iInstrIdx,int iSampleIdx);
			bool LoadS3MSampleData(Song *s,int iInstrIdx,int iSampleIdx,int iLen,bool bstereo,bool b16Bit,bool packed);
			bool LoadS3MPattern(Song *s,int patIdx);
		};
	}
}