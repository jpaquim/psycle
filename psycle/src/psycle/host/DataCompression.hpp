/* -*- mode:c++, indent-tabs-mode:t -*- */
#pragma once
///\file
///\brief interface file for beerz77-2 algorithm.
namespace psycle
{
	namespace host
	{
		/// compresses.
		int BEERZ77Comp2(byte * pSource, byte ** pDestination, int size);
		/// decompresses.
		bool BEERZ77Decomp2(byte * pSourcePos, byte ** pDestination);

		/// squashes.
		int SoundSquash(signed short * pSource, byte ** pDestination, int size);
		/// desquashes.
		bool SoundDesquash(byte * pSourcePos, signed short ** pDestination);
	}
}
