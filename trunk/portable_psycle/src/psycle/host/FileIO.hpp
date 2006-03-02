///\file
///\brief interface file for psycle::host::RiffFile.
#pragma once
#include <universalis/compiler/numeric.hpp>

#include <diversalis/operating_system.hpp>
#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include <diversalis/compiler.hpp>
	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma warning(push)
	#endif
	#include <windows.h> // because of microsoftisms: HANDLE
	#include <tchar.h> // because of microsoftisms: TCHAR
	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma warning(pop)
	#endif
#else
	#error "this sorry file is not portable"
#endif

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
		public:
			RiffChunkHeader _header;

			virtual bool Open  (std::string const & filename);
			virtual bool Create(std::string const & filename, bool const & overwrite);
			virtual bool Close();
			virtual bool Eof();
			virtual std::size_t FileSize();
			virtual std::size_t GetPos();
			virtual std::ptrdiff_t Seek(std::ptrdiff_t const & bytes);
			virtual std::ptrdiff_t Skip(std::ptrdiff_t const & bytes);
			///\todo wtf
			virtual FILE * GetFile() { return 0; };

			bool virtual Write (void const *, std::size_t const &);
			bool virtual Read  (void       *, std::size_t const &);
			bool virtual Expect(void       *, std::size_t const &);

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

			///\todo TCHAR may be char or wchar_t depending on "build settings"
			UNIVERSALIS__COMPILER__DEPRECATED("not portable.. either use unicode explicitely or glib's utf8")
			TCHAR * ReadStringA2T(TCHAR *, std::size_t const & max_length_in_chars);

			/// pad the string with spaces
			UNIVERSALIS__COMPILER__DEPRECATED("where is this function used?")
			static unsigned __int32 FourCC(char const * null_terminated_string);

			std::string szName;
		protected:
			HANDLE _handle;
			bool _modified;
		};

		///\todo this class is a reimplementation of the base class
		///\todo but with the standard c library instead of mswindows api.
		///\todo we should simply use it instead of the base class, and get rid of the inheritance and virtual functions
		class OldPsyFile : public RiffFile
		{
		public:
			virtual bool Open  (std::string const &);
			virtual bool Create(std::string const &, bool const & overwrite);
			virtual bool Close(void);
			virtual bool Error();
			virtual bool Eof();
			virtual std::size_t FileSize();
			virtual std::size_t GetPos();
			virtual std::ptrdiff_t Seek(std::ptrdiff_t const & bytes);
			virtual std::ptrdiff_t Skip(std::ptrdiff_t const & bytes);
			virtual FILE * GetFile() { return _file; };

			virtual bool Write (void const *, std::size_t const &);
			virtual bool Read  (void       *, std::size_t const &);
			virtual bool Expect(void       *, std::size_t const &);
		protected:
			FILE* _file;
		};
	}
}
