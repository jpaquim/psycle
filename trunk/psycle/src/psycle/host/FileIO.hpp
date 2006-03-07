///\interface psycle::host::RiffFile
#pragma once
#include "detail/project.hpp"
#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <string>

namespace psycle
{
	namespace host
	{
		/// big-endian 32-bit unsigned integer.
		class ULONGINV
		{
		public:
			std::uint8_t hihi;
			std::uint8_t hilo;
			std::uint8_t lohi;
			std::uint8_t lolo;
		};

		class RiffChunkHeader
		{
		public:
			std::uint32_t _id; // 4-character string, hence big-endian.
			std::uint32_t _size; // This one should be big-endian (it is, at least, in the files I([JAZ]) have tested)
		};

		/// riff file format.
		/// RIFF has a counterpart, RIFX, that is used to define RIFF file formats
		/// that use the Motorola integer byte-ordering format rather than the Intel format.
		/// A RIFX file is the same as a RIFF file, except that the first four bytes are "RIFX" instead of "RIFF",
		/// and integer byte ordering is represented in Motorola format.
		class RiffFile
		{
			private:
				std::string file_name_;
			public:
				std::string const inline & file_name() const throw() { return file_name_; }

			private:
				std::FILE* file_;
			public:
				std::FILE inline * GetFile() throw() { return file_; }

			public:
				///\todo shouldn't be public
				RiffChunkHeader _header;

				bool Open  (std::string const &);
				bool Create(std::string const &, bool const & overwrite);
				bool Close(void);
				bool Error();
				bool Eof();
				std::size_t FileSize();
				std::size_t GetPos();
				std::ptrdiff_t Seek(std::ptrdiff_t const & bytes);
				std::ptrdiff_t Skip(std::ptrdiff_t const & bytes);

				bool Write (void const *, std::size_t const &);
				bool Read  (void       *, std::size_t const &);
				bool Expect(void       *, std::size_t const &);

				template<typename X>
				void inline Write(X const & x) { Write(&x, sizeof x); }
				template<typename X>
				void inline  Read(X       & x) {  Read(&x, sizeof x); }

				int inline ReadInt(int const & bytes = sizeof(int)) { int tmp(0); Read(&tmp, bytes); return tmp; }

				bool               ReadString(std::string &);
				bool               ReadString(char *, std::size_t const & max_length);

				/// pad the string with spaces
				UNIVERSALIS__COMPILER__DEPRECATED("where is this function used?")
				static std::uint32_t FourCC(char const * null_terminated_string);
		};
	}
}
