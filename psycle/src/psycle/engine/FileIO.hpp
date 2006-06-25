///\interface psycle::host::RiffFile
#pragma once
//#include <psycle/engine/detail/project.hpp>
#include <cstdio>
#include <cstddef>
#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>


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
				std::size_t FileSize();
				std::size_t GetPos();
				int Seek(int    const & bytes);
				int Skip(std::ptrdiff_t const & bytes);

				bool WriteChunk (void const *, std::size_t const &);
				bool ReadChunk  (void       *, std::size_t const &);
				bool Expect(void       *, std::size_t const &);

			public://private: \todo to allow endian conversion, the template should be private and only functions with numeric types made public
				template<typename X>
				bool inline Write(X const & x) { return WriteChunk(&x, sizeof x); }
				template<typename X>
				bool inline  Read(X       & x) {  return ReadChunk(&x, sizeof x); }

				/// only useful for reading 24-bit numbers
				bool inline Read24(std::int32_t & x) { x = 0; return ReadChunk(&x, 3); }

				bool               ReadString(std::string &);
				bool               ReadString(char *, std::size_t const & max_length);
				//\todo : Implement a WriteString() to complement ReadString, and a ReadSizedString() which would do the same as ReadString
				//		which won't stop on the null, but rather on the size of the array(or else indicated by the second parameter). Finally,
				//		setting the last char to null.

				/// just a usless reinterpret_cast
				///\todo disabled because the compiler accepts arguments of different size!
				//static std::uint32_t inline FourCC(char const four_cc[4]) throw() { return *reinterpret_cast<std::uint32_t const *>(four_cc); }

				/// pad the string with spaces
				///\todo is it really used with null terminated strings?
//				PSYCLE__DEPRECATED("use the char const [4] overload instead")
				static std::uint32_t          FourCC(char const * null_terminated_string);

			private:
				bool write_mode;
				std::fstream _stream;
		};
	}
}

