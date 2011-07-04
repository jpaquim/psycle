// -*- mode:c++; indent-tabs-mode:t -*-
///\file
		///\brief implementation file for beerz77-2 algorithm.
		/// beerz77-2 algorithm by jeremy evers, loosely based on lz77
		/// -2 designates the smaller window, faster compression version
		/// designed for decompression on gameboy advance
		/// due to it's efficient decompression, it is usefull for many other things... like pattern data.
		///
		/// SoundSquash and SoundDesquash by jeremy evers
		/// designed with speed in mind
		/// simple, non adaptave delta predictor, less effective with high frequency content
#ifndef DATACOMPRESSION_H
#define DATACOMPRESSION_H

typedef unsigned char byte;

class DataCompression {
	public:

		DataCompression();
		~DataCompression();
	// compresses.
	static int BEERZ77Comp2(byte * pSource, byte ** pDestination, int size);
	/// decompresses.
	static bool BEERZ77Decomp2(byte * pSourcePos, byte ** pDestination);

	/// squashes.
	static int SoundSquash(signed short * pSource, byte ** pDestination, int size);
	/// desquashes.
	static bool SoundDesquash(byte * pSourcePos, signed short ** pDestination);
};

#endif
