#pragma once
#include "eaiff.hpp"
namespace psycle
{
	namespace helpers
	{
#if 0
		#define CommonID 'COMM'   /* chunkID for Common Chunk */

typedef struct {
  ID             chunkID;
  long           chunkSize;

  short          numChannels;
  unsigned long  numSampleFrames;
  short          sampleSize;
  extended       sampleRate;
} CommonChunk;

#define SoundDataID 'SSND'  /* chunk ID for Sound Data Chunk */

typedef struct {
  ID             chunkID;
  long           chunkSize;

  unsigned long  offset;
  unsigned long  blockSize;
  unsigned char  WaveformData[];
}  SoundDataChunk;

typedef short  MarkerId;

typedef struct {
  MarkerID       id;
  unsigned long  position;
  pstring        markerName;
} Marker;

#define MarkerID 'MARK'  /* chunkID for Marker Chunk */

typedef  struct {
  ID              chunkID;
  long            chunkSize;

  unsigned short  numMarkers;
  Marker          Markers[];
} MarkerChunk;

typedef struct {
  short     PlayMode;
  MarkerId  beginLoop;
  MarkerId  endLoop;
} Loop;

#define InstrumentID 'INST'  /*chunkID for Instruments Chunk */

typedef struct {
  ID     chunkID;
  long   chunkSize;

  char   baseNote;
  char   detune;
  char   lowNote;
  char   highNote;
  char   lowvelocity;
  char   highvelocity;
  short  gain;
  Loop   sustainLoop;
  Loop   releaseLoop;
} InstrumentChunk;
#endif
		/*********  IFF file reader comforming to Apple Audio IFF pecifications ****/
		class AppleAIFF : EaIff {
			AppleAIFF();
			virtual ~AppleAIFF();
		};
	}
}

