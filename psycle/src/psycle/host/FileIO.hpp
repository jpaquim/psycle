/* -*- mode:c++, indent-tabs-mode:t -*- */
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// For nice and portable code, get the file from psycle-core
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

///\file
///\brief interface file for psycle::host::RiffFile.
#pragma once
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
			unsigned char hihi;
			unsigned char hilo;
			unsigned char lohi;
			unsigned char lolo;
		};

		class RiffChunkHeader
		{
		public:
			std::uint32_t _id;
			std::uint32_t _size; // This one should be ULONGINV (it is, at least, in the files I([JAZ]) have tested)
		};

		/// riff file format.
		/// RIFF has a counterpart, RIFX, that is used to define RIFF file formats
		/// that use the Motorola integer byte-ordering format rather than the Intel format.
		/// A RIFX file is the same as a RIFF file, except that the first four bytes are "RIFX" instead of "RIFF",
		/// and integer byte ordering is represented in Motorola format.
		/// <bohan> haha... now the problem is...
		/// <bohan> " the first four bytes are "RIFX" instead of "RIFF" ",
		/// <bohan> so, on x86 cpus, should we read "XFIR"?
		///////////////////////////////////////////////////////////
		////////////////////// \todo handle endianess
		///////////////////////////////////////////////////////////
		class RiffFile
		{
		public://private:
			virtual bool Read(void * pData, std::size_t numBytes);
		private:
			template<typename T> void ReadRaw(T & t) { Read(&t, sizeof t); }
		public:
			void Read(double        & value) { ReadRaw(value); }
			void Read(float         & value) { ReadRaw(value); }
			void Read(std:: int32_t & value) { ReadRaw(value); }
			void Read(std::uint32_t & value) { ReadRaw(value); }
			void Read(std:: int16_t & value) { ReadRaw(value); }
			void Read(std::uint16_t & value) { ReadRaw(value); }
			void Read(std::  int8_t & value) { ReadRaw(value); }
			void Read(std:: uint8_t & value) { ReadRaw(value); }
			void Read(bool          & value) { ReadRaw(value); }
			bool ReadString(std::string & string);
			bool ReadString(char* pData, std::size_t maxBytes);
			#if 1 // why to we need this one?
			void Read(signed int    & value) { ReadRaw(value); }
			#endif

			const double        ReadDouble() { double        t; Read(t); return t; }
			const float         ReadFloat()  { float         t; Read(t); return t; }
			const std:: int32_t ReadInt32()  { std:: int32_t t; Read(t); return t; }
			const std::uint32_t ReadUInt32() { std::uint32_t t; Read(t); return t; }
			const std:: int16_t ReadInt16()  { std:: int16_t t; Read(t); return t; }
			const std::uint16_t ReadUInt16() { std::uint16_t t; Read(t); return t; }
			const std::  int8_t ReadInt8()   { std::  int8_t t; Read(t); return t; }
			const std:: uint8_t ReadUInt8()  { std:: uint8_t t; Read(t); return t; }
			///\todo MSWINDOWS-SPEFOIHEZOFIHDSLKHGSIFIC CODE!!!!!!!!!
			const TCHAR * RiffFile::ReadStringA2T(TCHAR* pData, const std::size_t maxLength);

		public://private:
			virtual bool Write(void const * pData, std::size_t numBytes);
		private:
			template<typename T> void WriteRaw(T const & t) { Write(&t, sizeof t); }
		public:
			void Write(double        const value) { WriteRaw(value); }
			void Write(float         const value) { WriteRaw(value); }
			void Write(std:: int32_t const value) { WriteRaw(value); }
			void Write(std::uint32_t const value) { WriteRaw(value); }
			void Write(std:: int16_t const value) { WriteRaw(value); }
			void Write(std::uint16_t const value) { WriteRaw(value); }
			void Write(std::  int8_t const value) { WriteRaw(value); }
			void Write(std:: uint8_t const value) { WriteRaw(value); }
			void Write(bool          const value) { WriteRaw(value); }
			#if 1 // why to we need this one?
			void Write(signed int    const value) { WriteRaw(value); }
			void Write(std::size_t   const value) { WriteRaw(value); }
			#endif

		public:
			virtual bool Open(std::string const & FileName);
			virtual bool Create(std::string const & FileName, bool overwrite);
			virtual bool Close();
			virtual bool Expect(void* pData, std::size_t numBytes);
			virtual int Seek(std::size_t offset);
			virtual int Skip(std::size_t numBytes);
			virtual bool Eof();
			virtual std::size_t FileSize();
			virtual std::size_t GetPos();
			virtual FILE* GetFile() { return 0; }
		protected:
			///\todo MSWINDOWS-SPEFOIHEZOFIHDSLKHGSIFIC CODE!!!!!!!!!
			HANDLE _handle;
			bool _modified;

		public:
			///\todo private
			std::string szName;
			///\todo private
			RiffChunkHeader _header;
			static std::uint32_t FourCC(char const * null_terminated_string);
		};

		class OldPsyFile : public RiffFile
		{
		public:
			bool Read(void* pData, std::size_t numBytes);
			bool Write(const void* pData, std::size_t numBytes);

			bool Open(std::string const & FileName);
			bool Create(std::string const & FileName, bool overwrite);
			bool Close();
			bool Expect(void* pData, std::size_t numBytes);
			int Seek(std::size_t offset);
			int Skip(std::size_t numBytes);
			bool Eof();
			std::size_t FileSize();
			std::size_t GetPos();
			FILE* GetFile() { return _file; }
		public:
			bool Error();
		protected:
			FILE* _file;
		};
	}
}
