#pragma once
#include "eaiff.hpp"
namespace psycle
{
	namespace helpers
	{

				// IFF structure ripped by krokpitr
		// Current Code Extremely modified by [JAZ] ( RIFF based )
		// Advise: IFF files use Big Endian byte ordering. That's why I use
		// the following structure.
		//
		// typedef struct {
		//   unsigned char hihi;
		//   unsigned char hilo;
		//   unsigned char lohi;
		//   unsigned char lolo;
		// } ULONGINV;
		// 
		//
		/*
		** IFF Riff Header
		** ----------------

		char Id[4]			// "FORM"
		ULONGINV hlength	// of the data contained in the file (except Id and length)
		char type[4]		// "16SV" == 16bit . 8SVX == 8bit

		char name_Id[4]		// "NAME"
		ULONGINV hlength	// of the data contained in the header "NAME". It is 22 bytes
		char name[22]		// name of the sample.

		char vhdr_Id[4]		// "VHDR"
		ULONGINV hlength	// of the data contained in the header "VHDR". it is 20 bytes
		ULONGINV Samplength	// length of the sample. It is in bytes, not in Samples.
		ULONGINV loopstart	// Start point for the loop. It is in bytes, not in Samples.
		ULONGINV loopLength	// Length of the loop (so loopEnd = loopstart+looplength) In bytes.
		unsigned char sampRateHiByte; samples per second.  (Unsigned 16-bit quantity.)
		unsigned char sampRateLoByte;
		unsigned char numOctaves; //number of octaves of waveforms in sample. (Multisample?)
		unsigned char compressMode; // data compression (0=none, 1=Fibonacci-delta encoding).
		unsigned char volumeHiByte;
		unsigned char volumeLoByte;
		unsigned char volumeHiBytePoint;
		unsigned char volumeLoBytePoint; (FIXED) = volume.  (The number 65536 means 1.0 or full volume.)

		//A Voice holds waveform data for one or more octaves. 
		//The one-shot part is played once and the repeat part is looped.
		//The sum of oneShotHiSamples and repeatHiSamples is the full length
		// of the highest octave waveform. Each following octave waveform is twice
		// as long as the previous one. 


		char body_Id[4]		// "BODY"
		ULONGINV hlength	// of the data contained in the file. It is the sample length as well (in bytes)
		char *data			// the sample.

		*/

		/*********  IFF file reader comforming to Amiga audio format specifications ****/
		class AmigaSvx : EaIff {
			AmigaSvx();
			virtual ~AmigaSvx();
		};
	}
}

