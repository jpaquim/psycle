// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2003-2009 members of the psycle project http://psycle.sourceforge.net ; jeremy evers

///\interface beerz77-2 algorithm.
///
/// beerz77-2 algorithm by jeremy evers, loosely based on lz77
/// -2 designates the smaller window, faster compression version
/// designed for decompression on gameboy advance
/// due to it's efficient decompression, it is usefull for many other things... like pattern data.
///
/// SoundSquash and SoundDesquash by jeremy evers
/// designed with speed in mind
/// simple, non adaptave delta predictor, less effective with high frequency content

#ifndef PSYCLE__CORE__DATA_COMPRESSION__INCLUDED
#define PSYCLE__CORE__DATA_COMPRESSION__INCLUDED
#pragma once

#include <psycle/core/config.hpp>

namespace psy { namespace core {

typedef unsigned char byte;

class PSYCLE__CORE__DECL DataCompression {
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

}}
#endif
