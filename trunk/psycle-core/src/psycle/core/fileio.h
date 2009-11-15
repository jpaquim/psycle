// This program is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// copyright 2007-2009 members of the psycle project http://psycle.sourceforge.net

///\interface psy::core::RiffFile

#ifndef PSYCLE__CORE__FILE_IO__INCLUDED
#define PSYCLE__CORE__FILE_IO__INCLUDED
#pragma once

#include <psycle/core/config.hpp>

#include <diversalis/compiler.hpp>
#include <cstdio>
#include <cstddef>
#include <fstream>
#include <cstdint>
#include <cassert>
#include <vector>
#include <typeinfo>

namespace psy { namespace core {

class RiffChunkHeader {
	public:
		/// chunk type identifier.
		/// 4-character string, hence big-endian.
		///\todo should be char id[4];
		char _id[4];

		/// size of the chunk in bytes.
		/// little endian for RIFF files ; big endian for RIFX files.
		std::uint32_t _size;
};

/// riff file format.
/// RIFF has a counterpart, RIFX, that is used to define RIFF file formats
/// that use the Motorola integer byte-ordering format rather than the Intel format.
/// A RIFX file is the same as a RIFF file, except that the first four bytes are "RIFX" instead of "RIFF",
/// and integer byte ordering is represented in Motorola format.
class PSYCLE__CORE__DECL RiffFile {
	public:
		std::string const inline & file_name() const throw() { return file_name_; }
	private:
		std::string file_name_;

	public:

		RiffFile();
		virtual ~RiffFile();

		///\todo shouldn't be public
		RiffChunkHeader _header;

		bool Open  (std::string const &);
		bool Create(std::string const &, bool const & overwrite);
		bool Close();
		bool Error();
		bool Eof();
		virtual std::size_t FileSize();
		virtual std::size_t GetPos();
		virtual int Seek(std::ptrdiff_t const & bytes);
		virtual int Skip(std::ptrdiff_t const & bytes);

	protected:
		virtual bool WriteChunk(void const *, std::size_t const &);
		virtual bool ReadChunk (void       *, std::size_t const &);
		virtual bool Expect    (void       *, std::size_t const &);

	public:
		#if defined DIVERSALIS__COMPILER__MICROSOFT && DIVERSALIS__COMPILER__VERSION < 1500
			/// workaround for msvc8 which fails to determine which overload to call
			template<typename X>
			bool Read(X & x) { return ReadChunk(&x, sizeof x); }

			/// workaround for msvc8 which fails to determine which overload to call
			template<typename X>
			bool Write(X & x) { return WriteChunk(&x, sizeof x); }
		#else
			// If std::int32_t is 'int' and std::int64_t is 'long long int',
			// this leaves a hole for the 'long int' type.
			// The following templates solve this issue.
			// If there is an overloaded specialized normal function for a given type,
			// the compiler will prefer it over these non-specialized templates.
			// So, for example, reading/writing a float will not go through these templates.
		
			template<typename X>
			bool Read(X & x) {
				return
					sizeof x == 8 ? Read(reinterpret_cast<std::uint64_t&>(x)) :
					sizeof x == 4 ? Read(reinterpret_cast<std::uint32_t&>(x)) :
					throw std::bad_cast();
			}

			template<typename X>
			bool ReadBE(X & x) {
				return
					sizeof x == 8 ? ReadBE(reinterpret_cast<std::uint64_t&>(x)) :
					sizeof x == 4 ? ReadBE(reinterpret_cast<std::uint32_t&>(x)) :
					throw std::bad_cast();
			}

			template<typename X>
			bool Write(X x) {
				return
					sizeof x == 8 ? Write(reinterpret_cast<std::uint64_t&>(x)) :
					sizeof x == 4 ? Write(reinterpret_cast<std::uint32_t&>(x)) :
					throw std::bad_cast();
			}
		#endif

		///\name 1 bit
		///\{
			bool Read(bool & x) {
				std::uint8_t c;
				if (!Read(c)) return false;
				x = c;
				return true;
			}

			bool Write(bool x) { std::uint8_t c = x; return Write(c); }
		///\}

		///\name 8 bits
		///\{
			bool Read(std::uint8_t & x) { return ReadChunk(&x,1); }
			bool Read(std::int8_t & x) { return ReadChunk(&x,1); }
			bool Read(char & x) { return ReadChunk(&x,1); } // somehow char doesn't match int8_t nor uint8_t

			bool Write(std::uint8_t x) { return WriteChunk(&x,1); }
			bool Write(std::int8_t x) { return WriteChunk(&x,1);  }
			bool Write(char x) { return WriteChunk(&x,1);  } // somehow char doesn't match int8_t nor uint8_t
		///\}

		///\name 16 bits
		///\{
			bool Read(std::uint16_t & x) {
				std::uint8_t data[2];
				if(!ReadChunk(data,2)) return false;
				x = (data[1])<<8 | data[0];
				return true;
			}

			bool Read(std::int16_t & x) { return Read(reinterpret_cast<std::uint16_t&>(x)); }

			bool ReadBE(std::uint16_t & x) {
				std::uint8_t data[2];
				if(!ReadChunk(data,2)) return false;
				x = (data[0])<<8 | data[1];
				return true;
			}

			bool ReadBE(std::int16_t & x) { return ReadBE(reinterpret_cast<std::uint16_t&>(x)); }

			bool Write(std::uint16_t x) {
				std::uint8_t data[2] = { x & 0xFF, (x>>8) & 0xFF };
				return WriteChunk(data,2);
			}
			bool Write(std::int16_t x) { return Write(reinterpret_cast<std::uint16_t&>(x)); }
		///\}

		///\name 32 bits
		///\{
			bool Read(std::uint32_t & x) {
				std::uint8_t data[4];
				if(!ReadChunk(data,4)) return false;
				x = (data[3]<<24) | (data[2]<<16) | (data[1]<<8) | data[0];
				return true;
			}

			bool ReadBE(std::uint32_t & x) {
				std::uint8_t data[4];
				if(!ReadChunk(data,4)) return false;
				x = (data[0]<<24) | (data[1]<<16) | (data[2]<<8) | data[3];
				return true;
			}

			bool Read(std::int32_t & x) { return Read(reinterpret_cast<std::uint32_t&>(x)); }

			bool Write(std::uint32_t x) {
				std::uint8_t data[4] = { x & 0xFF, (x>>8) & 0xFF, (x>>16) & 0xFF, (x>>24) & 0xFF };
				return WriteChunk(data, 4);
			}

			bool Write(std::int32_t x) { return Write(reinterpret_cast<std::uint32_t&>(x)); }
		///\}

		///\name 64 bits
		///\{
			bool Read(std::uint64_t & x) {
				std::uint8_t data[8];
				if(!ReadChunk(data,8)) return false;
				x = (std::uint64_t(data[7])<<56) | (std::uint64_t(data[6])<<48) | (std::uint64_t(data[5])<<40) | (std::uint64_t(data[4])<<32) | (data[3]<<24) | (data[2]<<16) | (data[1]<<8) | data[0];
				return true;
			}

			bool ReadBE(std::uint64_t & x) {
				std::uint8_t data[8];
				if(!ReadChunk(data,8)) return false;
				x = (std::uint64_t(data[0])<<56) | (std::uint64_t(data[1])<<48) | (std::uint64_t(data[2])<<40) | (std::uint64_t(data[3])<<32) | (data[4]<<24) | (data[5]<<16) | (data[6]<<8) | data[7];
				return true;
			}

			bool Read(std::int64_t & x) { return Read(reinterpret_cast<std::uint64_t&>(x)); }

			bool Write(std::uint64_t x) {
				std::uint8_t data[8] = { x & 0xFF, (x>>8) & 0xFF, (x>>16) & 0xFF, (x>>24) & 0xFF, (x>>32) & 0xFF, (x>>40) & 0xFF, (x>>48) & 0xFF, (x>>56) & 0xFF };
				return WriteChunk(data, 4);
			}

			bool Write(std::int64_t x) { return Write(reinterpret_cast<std::uint64_t&>(x)); }

		///\name floating point
		///\{
			bool Read(float & x) {
				union {
					float f;
					std::uint8_t data[4];
				};
				f = 1.0f;
				if (data[0] == 63 && data[1] == 128) {
					if(!ReadChunk(data,4)) return false;
					std::swap(data[0],data[3]);
					std::swap(data[1],data[2]);
					x = f;
					return true;
				} else if (data[3] == 63 && data[2] == 128) {
					if(!ReadChunk(data,4)) return false;
					x = f;
					return true;
				} else {
					assert(!"Error: Couldn't determine 32 bit float endianness");
					return false;
				}
			}
		
			bool Write(float x) {
				union {
					float f;
					std::uint8_t data[4];
				};
				f = 1.0f;
				if (data[0] == 63 && data[1]==128) {
					f = x;
					std::swap(data[0],data[3]);
					std::swap(data[1],data[2]);
					return WriteChunk(data,4);
				} else if (data[3] == 63 && data[2]==128) {
					f = x;
					return WriteChunk(data,4);
				} else {
					assert(!"Error: Couldn't determine 32 bit float endianness");
				}
				return false;
			}
		///\}

		///\name arrays
		///\{
			template<typename T>
			bool ReadArray(T* array, int n) {
				for(int i=0;i<n;i++) if (!Read(array[i])) return false;
				return true;
			}

			template<typename T>
			bool WriteArray(T const* array, int n) {
				for(int i=0;i<n;i++) if (!Write(array[i])) return false;
				return true;
			}
		///\}

		///\name strings
		///\{
			bool ReadString(std::string &);
			bool ReadString(char *, std::size_t const max_length);
			bool WriteString(std::string const &);
			///\todo : Implement a ReadSizedString() which would do the same as ReadString
			//         which won't stop on the null, but rather on the size of the array(or else indicated by the second parameter). Finally,
			//         setting the last char to null.
			//bool ReadSizedString(std::string &, std::size_t const &numchars);
			//bool WriteSizedString(std::string const &, std::size_T const &numchars);
		///\}

		///\name riff four cc
		///\{
			static bool matchFourCC(char const a[4], char const b[4]) {
				return *(std::uint32_t const*)a == *(std::uint32_t const*)b;
			}
		///\}

		///\name riff chunk headers
		///\{
			bool Read(RiffChunkHeader &h) {
				return ReadChunk(h._id,4) && Read(h._size);
			}

			bool Write(RiffChunkHeader &h) {
				return WriteChunk(h._id,4) && Write(h._size);
			}
		///\}

	private:
		bool write_mode;
		std::fstream _stream;
};


class PSYCLE__CORE__DECL MemoryFile : public RiffFile {
	public:
		MemoryFile();
		virtual ~MemoryFile();

		bool OpenMem(std::ptrdiff_t blocksize);
		bool CloseMem();
		virtual std::size_t FileSize();
		virtual std::size_t GetPos();
		virtual int Seek(std::ptrdiff_t const & bytes);
		virtual int Skip(std::ptrdiff_t const & bytes);
		virtual bool ReadString(char *, std::size_t const & max_length);

	protected:
		virtual bool WriteChunk(void const *, std::size_t const &);
		virtual bool ReadChunk (void       *, std::size_t const &);
		virtual bool Expect    (void       *, std::size_t const &);

		std::vector<void*> memoryblocks_;
		int blocksize_;
};

}}
#endif
