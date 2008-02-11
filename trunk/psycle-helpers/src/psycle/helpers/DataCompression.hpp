/* -*- mode:c++, indent-tabs-mode:t -*- */
///\interface beerz77-2 algorithm.
#pragma once
#include <psycle/host/detail/project.hpp>
#include <cstddef>
#include <cstdint>
namespace psycle
{
	namespace host
	{
		/// compresses.
		std::ptrdiff_t BEERZ77Comp2(std::uint8_t const * pSource, std::uint8_t ** pDestination, std::size_t const &);
		/// decompresses.
		bool           BEERZ77Decomp2(std::uint8_t const * pSourcePos, std::uint8_t ** pDestination);

		/// squashes.
		std::ptrdiff_t SoundSquash(std::int16_t const * pSource, std::uint8_t ** pDestination, std::size_t const &);
		/// desquashes.
		bool           SoundDesquash(std::uint8_t const * pSourcePos, std::int16_t ** pDestination);
	}
}
