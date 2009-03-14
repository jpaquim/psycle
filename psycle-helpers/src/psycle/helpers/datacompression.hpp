///\file
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

#pragma once
#include <cstddef>
#include <cstdint>
namespace psycle
{
	namespace helpers
	{
		class DataCompression {
			public:
				DataCompression();
				~DataCompression();
		
				/// compresses.
				static std::ptrdiff_t BEERZ77Comp2(std::uint8_t const * pSource, std::uint8_t ** pDestination, std::size_t const &);
		
		
		
		
				/// decompresses.
				static bool           BEERZ77Decomp2(std::uint8_t const * pSourcePos, std::uint8_t ** pDestination);
		
				/// squashes.
				static std::ptrdiff_t SoundSquash(std::int16_t const * pSource, std::uint8_t ** pDestination, std::size_t const &);
				/// desquashes.
				static bool           SoundDesquash(std::uint8_t const * pSourcePos, std::int16_t ** pDestination);
		};
	}
}
