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
		namespace endian
		{
			namespace big
			{
				/// big-endian 32-bit unsigned integer.
				class uint32_t
				{
					public:
						std::uint8_t hihi;
						std::uint8_t hilo;
						std::uint8_t lohi;
						std::uint8_t lolo;
				};
			}
		}

		class RiffChunkHeader
		{
			public:
				/// chunk type identifier.
				/// 4-character string, hence big-endian.
				///\todo should be char id[4];
				std::uint32_t _id;
				/// size of the chunk in bytes.
				/// little endian for RIFF files ; big endian for RIFX files.
				std::uint32_t _size;
		};

		/// riff file format.
		/// RIFF has a counterpart, RIFX, that is used to define RIFF file formats
		/// that use the Motorola integer byte-ordering format rather than the Intel format.
		/// A RIFX file is the same as a RIFF file, except that the first four bytes are "RIFX" instead of "RIFF",
		/// and integer byte ordering is represented in Motorola format.
		class RiffFile
		{
			public:
				std::string const inline & file_name() const throw() { return file_name_; }
			private:
				std::string file_name_;

			public:
				///\todo shouldn't be public
				RiffChunkHeader _header;

				bool Open  (std::string const &);
				bool Create(std::string const &, bool const & overwrite);
				bool Close(void);
				bool Error();
				bool Eof();
				std::fpos_t FileSize();
				std::fpos_t GetPos();
				std::fpos_t Seek(std::fpos_t    const & bytes);
				std::fpos_t Skip(std::ptrdiff_t const & bytes);

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

			private:
				std::FILE        * file_;
		};
	}
}
