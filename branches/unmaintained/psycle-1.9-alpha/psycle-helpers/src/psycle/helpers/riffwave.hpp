#pragma once
#include "msriff.hpp"
namespace psycle { namespace helpers {

/// the riff WAVE/fmt chunk.
class WavHeader
{
public:
	char chunkID[4];
	long chunkSize;
	short wFormatTag;
	unsigned short wChannels;
	unsigned long  dwSamplesPerSec;
	unsigned long  dwAvgBytesPerSec;
	unsigned short wBlockAlign;
	unsigned short wBitsPerSample;
};

/*********  IFF file reader comforming to IBM/Microsoft WaveRIFF specifications ****/
class RiffWave : MsRiff {
	RiffWave();
	virtual ~RiffWave();
};

}}
