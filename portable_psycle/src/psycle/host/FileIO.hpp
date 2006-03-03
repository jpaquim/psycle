///\file
///\brief interface file for psycle::host::RiffFile.
#pragma once
#include <universalis/compiler/numeric.hpp>

namespace psycle
{
	namespace host
	{
		/// big-endian 32-bit unsigned integer.
		class ULONGINV
		{
		public:
			unsigned char hihi;
			unsigned char hilo;
			unsigned char lohi;
			unsigned char lolo;
		};

		class RiffChunkHeader
		{
		public:
			unsigned __int32 _id; // 4-character string, hence big-endian.
			unsigned __int32 _size; // This one should be big-endian (it is, at least, in the files I([JAZ]) have tested)
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
				FILE* file_;
			public:
				FILE inline * GetFile() throw() { return file_; }

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

				UNIVERSALIS__COMPILER__DEPRECATED("use explicit size instead")
				bool               virtual inline ReadBool()   {               bool tmp; Read(tmp); return tmp; }

				UNIVERSALIS__COMPILER__DEPRECATED("use explicit size instead")
				unsigned      char virtual inline ReadUChar()  { unsigned      char tmp; Read(tmp); return tmp; }

				UNIVERSALIS__COMPILER__DEPRECATED("use explicit size instead")
				signed      char virtual inline ReadChar()   {   signed      char tmp; Read(tmp); return tmp; }

				UNIVERSALIS__COMPILER__DEPRECATED("use explicit size instead")
				unsigned short int virtual inline ReadUShort() { unsigned short int tmp; Read(tmp); return tmp; }

				UNIVERSALIS__COMPILER__DEPRECATED("use explicit size instead")
				signed short int virtual inline ReadShort()  {   signed short int tmp; Read(tmp); return tmp; }

				UNIVERSALIS__COMPILER__DEPRECATED("use explicit size instead")
				UINT               virtual inline ReadUINT()   {               UINT tmp; Read(tmp); return tmp; }

				UNIVERSALIS__COMPILER__DEPRECATED("use explicit size instead")
				float              virtual inline ReadFloat()  {              float tmp; Read(tmp); return tmp; }

				UNIVERSALIS__COMPILER__DEPRECATED("use explicit size instead")
				double             virtual inline ReadDouble() {             double tmp; Read(tmp); return tmp; }

				bool               virtual ReadString(std::string &);
				bool               virtual ReadString(char *, std::size_t const & max_length);

				/// pad the string with spaces
				UNIVERSALIS__COMPILER__DEPRECATED("where is this function used?")
				static unsigned __int32 FourCC(char const * null_terminated_string);
		};
	}
}
