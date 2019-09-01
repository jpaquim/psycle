// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2007 psycledelics http://psycle.pastnotecut.org : johan boule

///\file
///\brief explicitly sized, crossplatform (processor endianess independant) serializable types for numbers.
#pragma once
#include "project.hpp"
#include "detail/bits.hpp"
#include <iostream>
#if defined COMPILER__ECLIPSE
	;
#endif
/// functionalities provided by hardware processors.
///\see namespace serialize
namespace processor
{
	/// explicitly sized, crossplatform (processor endianess independant) serializable types for numbers.
	namespace serialize
	{
		/// inputs big endian ordered data, converting to processor internal ordering format.
		/// \todo what about sign format?
		class istream
		{
		public:
			inline istream(std::istream & istream) : istream_(istream) {}
			template<typename Processor_Format> inline istream & operator>>(Processor_Format & processor_format)
			{
				#if defined PROCESSOR__ENDIAN__BIG
					return istream_ >> processor_format;
				#elif defined PROCESSOR__ENDIAN__LITTLE
					Processor_Format big_endian;
					istream_ >> big_endian;
					processor_format = bytes::reversed(big_endian);
				#else
					#error "processor endianess not supported"
				#endif
			}
		private:
			std::istream & istream_;
		};

		/// outputs data always in a big endian order, wether or not the processor uses big or little endian ordering format internally.
		/// \todo what about sign format?
		class ostream
		{
		public:
			inline ostream(std::ostream & ostream) : ostream_(ostream) {}
			template<typename Processor_Format> inline ostream & operator<<(const Processor_Format & processor_format)
			{
				#if defined PROCESSOR__ENDIAN__BIG
					return ostream_ << processor_format;
				#elif defined PROCESSOR__ENDIAN__LITTLE
					return ostream_ << bytes::reversed(processor_format);
				#else
					#error "processor endianess not supported"
				#endif
			}
		private:
			std::ostream & ostream_;
		};

		/// crossplatform (processor endianess independant) serializable 80-bit ieee-754 floating point number.
		typedef serializable<processor::float80> float80;
		/// crossplatform (processor endianess independant) serializable 64-bit ieee-754 floating point number.
		typedef serializable<processor::float64> float64;
		/// crossplatform (processor endianess independant) serializable 32-bit ieee-754 floating point number.
		typedef serializable<processor::float32> float32;
		/// crossplatform (processor endianess independant) serializable unsigned 64-bit integer number.
		typedef serializable<processor::uint64> uint64;
		/// crossplatform (processor endianess independant) serializable signed 64-bit integer number.
		typedef serializable<processor::sint64> sint64;
		/// crossplatform (processor endianess independant) serializable signed 32-bit integer number.
		typedef serializable<processor::sint32> sint32;
		/// crossplatform (processor endianess independant) serializable unsigned 32-bit integer number.
		typedef serializable<processor::uint32> uint32;
		/// crossplatform (processor endianess independant) serializable signed 16-bit integer number.
		typedef serializable<processor::sint16> sint16;
		/// crossplatform (processor endianess independant) serializable unsigned 16-bit integer number.
		typedef serializable<processor::uint16> uint16;
		/// crossplatform (processor endianess independant) serializable signed 8-bit integer number.
		/// (not really needed for just one byte, but exists for consistency \todo maybe because of sign format)
		typedef serializable<processor::sint8> sint8;
		/// crossplatform (processor endianess independant) serializable unsigned 8-bit integer number.
		/// (not really needed for just one byte, but exists for consistency \todo maybe because of sign format)
		typedef serializable<processor::uint8> uint8;

		/// inputs big endian ordered data, converting to processor internal ordering format.
		/// \todo what about sign format?
		template<typename Processor_Format> inline std::istream & operator>>(std::istream & istream, serializable<Processor_Format> & serializable)
		{
			Processor_Format big_endian;
			istream >> big_endian;
			#if defined PROCESSOR__ENDIAN__BIG
				serializable = big_endian;
			#elif defined PROCESSOR__ENDIAN__LITTLE
				serializable = bytes::reversed(big_endian);
			#else
				#error "processor endianess not supported"
			#endif
			return istream;
		}
		
		/// outputs data always in a big endian order, wether or not the processor uses big or little endian ordering format internally.
		/// \todo what about sign format?
		template<typename Processor_Format> inline std::ostream & operator<<(std::ostream & ostream, const serializable<Processor_Format> & serializable)
		{
			#if defined PROCESSOR__ENDIAN__BIG
				const serialize::serializable<Processor_Format> & big_endian(serializable);
			#elif defined PROCESSOR__ENDIAN__LITTLE
				const serialize::serializable<Processor_Format> big_endian(bytes::reversed(serializable));
			#else
				#error "processor endianess not supported"
			#endif
			return ostream << static_cast<Processor_Format>(big_endian);
		}
	}
}

// arch-tag: 3d028ee8-dc6a-49c3-a619-3b4a56113167
