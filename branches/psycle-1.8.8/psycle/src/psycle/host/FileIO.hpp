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
#include <psycle/host/detail/project.hpp>
#include <universalis/stdlib/cstdint.hpp>
#include <cstddef>
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

		public://private:
			virtual bool Read(void * pData, std::size_t numBytes);
			virtual bool Write(void const * pData, std::size_t numBytes);
		private:
			template<typename T> void WriteRaw(T const & t) { Write(&t, sizeof t); }
			template<typename T> void ReadRaw(T & t) { Read(&t, sizeof t); }
		public:
			bool ReadString(std::string & string);
			bool ReadString(char* pData, std::size_t maxBytes);
			bool WriteString(std::string & string);

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

		public:
		///\name 1 bit
		///\{
			bool Read(bool & x) {
				uint8_t c;
				if (!Read(c)) return false;
				x = c;
				return true;
			}

			bool Write(bool x) { uint8_t c = x; return Write(c); }
		///\}

		///\name 8 bits
		///\{
			bool Read(uint8_t & x) { return Read(&x,1); }
			bool Read(int8_t & x) { return Read(&x,1); }
			bool Read(char & x) { return Read(&x,1); } // somehow char doesn't match int8_t nor uint8_t

			bool Write(uint8_t x) { return Write(&x,1); }
			bool Write(int8_t x) { return Write(&x,1);  }
			bool Write(char x) { return Write(&x,1);  } // somehow char doesn't match int8_t nor uint8_t
		///\}

		///\name 16 bits
		///\{
			bool Read(uint16_t & x) {
				uint8_t data[2];
				if(!Read(data,2)) return false;
				x = (data[1])<<8 | data[0];
				return true;
			}

			bool Read(int16_t & x) { return Read(reinterpret_cast<uint16_t&>(x)); }

			bool ReadBE(uint16_t & x) {
				uint8_t data[2];
				if(!Read(data,2)) return false;
				x = (data[0])<<8 | data[1];
				return true;
			}

			bool ReadBE(int16_t & x) { return ReadBE(reinterpret_cast<uint16_t&>(x)); }

			bool Write(uint16_t x) {
				uint8_t data[2] = { x & 0xFF, (x>>8) & 0xFF };
				return Write(data,2);
			}
			bool Write(int16_t x) { return Write(reinterpret_cast<uint16_t&>(x)); }
		///\}

		///\name 32 bits
		///\{
			bool Read(uint32_t & x) {
				uint8_t data[4];
				if(!Read(data,4)) return false;
				x = (data[3]<<24) | (data[2]<<16) | (data[1]<<8) | data[0];
				return true;
			}

			bool Read(int32_t & x) { return Read(reinterpret_cast<uint32_t&>(x)); }

			bool ReadBE(uint32_t & x) {
				uint8_t data[4];
				if(!Read(data,4)) return false;
				x = (data[0]<<24) | (data[1]<<16) | (data[2]<<8) | data[3];
				return true;
			}

			bool ReadBE(int32_t & x) { return ReadBE(reinterpret_cast<uint32_t&>(x)); }

			bool Write(uint32_t x) {
				uint8_t data[4] = { x & 0xFF, (x>>8) & 0xFF, (x>>16) & 0xFF, (x>>24) & 0xFF };
				return Write(data, 4);
			}

			bool Write(int32_t x) { return Write(reinterpret_cast<uint32_t&>(x)); }
		///\}

		///\name 64 bits
		///\{
			bool Read(uint64_t & x) {
				uint8_t data[8];
				if(!Read(data,8)) return false;
				x = (uint64_t(data[7])<<56) | (uint64_t(data[6])<<48) | (uint64_t(data[5])<<40) | (uint64_t(data[4])<<32) | (uint64_t(data[3])<<24) | (data[2]<<16) | (data[1]<<8) | data[0];
				return true;
			}

			bool Read(int64_t & x) { return Read(reinterpret_cast<uint64_t&>(x)); }

			bool ReadBE(uint64_t & x) {
				uint8_t data[8];
				if(!Read(data,8)) return false;
				x = (uint64_t(data[0])<<56) | (uint64_t(data[1])<<48) | (uint64_t(data[2])<<40) | (uint64_t(data[3])<<32) | (uint64_t(data[4])<<24) | (data[5]<<16) | (data[6]<<8) | data[7];
				return true;
			}

			bool ReadBE(int64_t & x) { return ReadBE(reinterpret_cast<uint64_t&>(x)); }

			bool Write(uint64_t x) {
				uint8_t data[8] = { x & 0xFF, (x>>8) & 0xFF, (x>>16) & 0xFF, (x>>24) & 0xFF, (x>>32) & 0xFF, (x>>40) & 0xFF, (x>>48) & 0xFF, (x>>56) & 0xFF };
				return Write(data, 8);
			}

			bool Write(int64_t x) { return Write(reinterpret_cast<uint64_t&>(x)); }


		///\name 32-bit floating point
		///\{
			bool Read(float & x) {
				union {
					float f;
					uint8_t data[4];
				};
				f = 1.0f;
				if(data[0] == 63 && data[1] == 128) {
					if(!Read(data, 4)) return false;
					std::swap(data[0], data[3]);
					std::swap(data[1], data[2]);
					x = f;
					return true;
				} else if(data[3] == 63 && data[2] == 128) {
					if(!Read(data, 4)) return false;
					x = f;
					return true;
				} else {
					assert(!"Error: Couldn't determine floating point endianness");
					return false;
				}
			}
		
			bool Write(float x) {
				union {
					float f;
					uint8_t data[4];
				};
				f = 1.0f;
				if(data[0] == 63 && data[1] == 128) {
					f = x;
					std::swap(data[0], data[3]);
					std::swap(data[1], data[2]);
					return Write(data, 4);
				} else if(data[3] == 63 && data[2] == 128) {
					f = x;
					return Write(data, 4);
				} else {
					assert(!"Error: Couldn't determine floating point endianness");
					return false;
				}
			}
		///\}

		///\name 64-bit floating point
		///\{
			bool Read(double & x) {
				union {
					float f;
					double d;
					uint8_t data[8];
				};
				f = 1.0f;
				if(data[0] == 63 && data[1] == 128) {
					if(!Read(data, 8)) return false;
					std::swap(data[0], data[7]);
					std::swap(data[1], data[6]);
					std::swap(data[2], data[5]);
					std::swap(data[3], data[4]);
					x = d;
					return true;
				} else if(data[3] == 63 && data[2] == 128) {
					if(!Read(data, 8)) return false;
					x = d;
					return true;
				} else {
					assert(!"Error: Couldn't determine floating point endianness");
					return false;
				}
			}

			bool Write(double x) {
				union {
					float f;
					double d;
					uint8_t data[8];
				};
				f = 1.0f;
				if(data[0] == 63 && data[1] == 128) {
					d = x;
					std::swap(data[0], data[7]);
					std::swap(data[1], data[6]);
					std::swap(data[2], data[5]);
					std::swap(data[3], data[4]);
					return Write(data, 8);
				} else if(data[3] == 63 && data[2] == 128) {
					d = x;
					return Write(data, 8);
				} else {
					assert(!"Error: Couldn't determine floating point endianness");
					return false;
				}
			}
		///\}

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
