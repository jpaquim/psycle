///\interface psy::core::RiffFile
#pragma once

#include <diversalis/compiler.hpp>
#include <cstdio>
#include <cstddef>
#include <fstream>
#include <cstdint>
#include <cassert>

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
class RiffFile {
	public:
		std::string const inline & file_name() const throw() { return file_name_; }
	private:
		std::string file_name_;

	public:

		RiffFile();
		~RiffFile();

		///\todo shouldn't be public
		RiffChunkHeader _header;

		bool Open  (std::string const &);
		bool Create(std::string const &, bool const & overwrite);
		bool Close();
		bool Error();
		bool Eof();
		std::size_t FileSize();
		std::size_t GetPos();
		int Seek(std::ptrdiff_t const & bytes);
		int Skip(std::ptrdiff_t const & bytes);

	private:
		bool WriteChunk(void const *, std::size_t const &);
		bool ReadChunk (void       *, std::size_t const &);
		bool Expect    (void       *, std::size_t const &);

	public:
		#if defined DIVERSALIS__COMPILER__MICROSOFT
			/// workaround for msvc8 which fails to determine which overload to call
			template<typename X>
			bool Read(X & x) { return ReadChunk(&x, sizeof x); }

			/// workaround for msvc8 which fails to determine which overload to call
			template<typename X>
			bool Write(X & x) { return WriteChunk(&x, sizeof x); }
		#endif

		bool Read(std::uint8_t & x) { return ReadChunk(&x,1); }
		bool Read(std::int8_t & x) { return ReadChunk(&x,1); }
		bool Read(char & x) { return ReadChunk(&x,1); } // somehow char doesn't match int8_t nor uint8_t

		bool Write(std::uint8_t x) { return WriteChunk(&x,1); }
		bool Write(std::int8_t x) { return WriteChunk(&x,1);  }
		bool Write(char x) { return WriteChunk(&x,1);  } // somehow char doesn't match int8_t nor uint8_t

		bool Write(bool x) { std::uint8_t c = x; return Write(c); }

		bool Read(bool & x) {
			std::uint8_t c;
			if (!Read(c)) return false;
			x = c;
			return true;
		}

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
			std::uint8_t data[4] = { x, x>>8, x>>16, x>>24 };
			return WriteChunk(data,4);
		}

		bool Write(std::int32_t x) { return Write(reinterpret_cast<std::uint32_t&>(x)); }

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
		}

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

		bool Read(RiffChunkHeader &h) {
			return ReadChunk(h._id,4) && Read(h._size);
		}

		bool Write(RiffChunkHeader &h) {
			return WriteChunk(h._id,4) && Write(h._size);
		}


		bool ReadString(std::string &);
		bool ReadString(char *, std::size_t const & max_length);
		///\todo : Implement a WriteString() to complement ReadString, and a ReadSizedString() which would do the same as ReadString
		//         which won't stop on the null, but rather on the size of the array(or else indicated by the second parameter). Finally,
		//         setting the last char to null.

		static bool matchFourCC(char const a[4], char const b[4]) {
			return *(std::uint32_t const*)a == *(std::uint32_t const*)b;
		}

	private:
		bool write_mode;
		std::fstream _stream;
};

}}
