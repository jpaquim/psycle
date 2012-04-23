/**
	\file
	interface file for psycle::helpers::abstractIff psycle::helpers::BaseChunkHeader, and the different big-endian and little-endian types.
*/
#pragma once

#include <universalis.hpp>
#include <string>
#include <fstream>

namespace psycle { namespace helpers {

using namespace universalis::stdlib;

		/***** DATA TYPES *********/

		/// big-endian 32-bit integer.
		class ULongBE
		{
		public:
			union _d{
				struct _byte{
					unsigned char hihi;
					unsigned char hilo;
					unsigned char lohi;
					unsigned char lolo;
				}byte;
				uint32_t originalValue;
			}d;
			ULongBE();
			ULongBE(uint32_t);
			virtual uint32_t unsignedValue() const;
			virtual int32_t signedValue() const;
		};

		class LongBE : public ULongBE
		{
		public:
			LongBE();
			LongBE(int32_t);
		};

		/// big-endian 16-bit unsigned integer.
		class UShortBE
		{
		public:
			union _d {
				struct _byte{
					unsigned char hi;
					unsigned char lo;
				}byte;
				uint16_t originalValue;
			}d;
			UShortBE();
			UShortBE(uint16_t);
			virtual uint16_t unsignedValue() const;
			virtual int16_t signedValue() const;
		};
		class ShortBE : public UShortBE
		{
		public:
			ShortBE();
			ShortBE(int16_t);
		};

		// big-endian 32-bit fixed point value.
		class FixedPointBE
		{
			union _d {
				struct _byte{
					char hi;
					unsigned char lo;
				}byte;
				short value;
			}integer;
			union _d2 {
				struct _byte{
					unsigned char hi;
					unsigned char lo;
				}byte;
				uint16_t value;
			}decimal;
			float value() const;
		};

		/// little-endian 32-bit unsigned integer.
		class ULongLE
		{
		public:
			union _d{
				struct _byte{
					unsigned char lolo;
					unsigned char lohi;
					unsigned char hilo;
					unsigned char hihi;
				}byte;
				uint32_t originalValue;
			}d;
			ULongLE();
			ULongLE(uint32_t);
			virtual uint32_t unsignedValue() const;
			virtual int32_t signedValue() const;
		};

		class LongLE : public ULongLE
		{
		public:
			LongLE();
			LongLE(int32_t);
		};

		/// little-endian 16-bit unsigned integer.
		class UShortLE
		{
		public:
			union _d {
				struct _byte{
					unsigned char lo;
					unsigned char hi;
				}byte;
				uint16_t originalValue;
			}d;
			UShortLE();
			UShortLE(uint16_t);
			virtual uint16_t unsignedValue() const;
			virtual int16_t signedValue() const;
		};
		class ShortLE : public UShortLE
		{
		public:
			ShortLE();
			ShortLE(int16_t);
		};

		/******* Data Structures *******/
		typedef char IffChunkId[4];
		class BaseChunkHeader
		{
		public:
			IffChunkId id;
			virtual ~BaseChunkHeader();
			std::string idString() const;
			bool matches(IffChunkId id2) const;
			virtual uint32_t length() const = 0;
		};

		/******* Base Class for reader ******/
		class AbstractIff
		{
		public:

			AbstractIff();
			virtual ~AbstractIff();

			virtual bool isValidFile() const = 0;

			virtual void Open(std::string fname);
			virtual void Create(std::string fname, bool const & overwrite);
			virtual void close();
			virtual bool Eof() const;
			std::string const inline & file_name() const throw();
			std::size_t fileSize();

			virtual void addNewChunk(const BaseChunkHeader& header) = 0;
			virtual const BaseChunkHeader& readHeader() = 0;
			virtual const BaseChunkHeader& findChunk(IffChunkId id) = 0;
			virtual void skipThisChunk() = 0;
		
			void ReadString(std::string &);
			void ReadString(char *, std::size_t const & max_length);
			void ReadSizedString(char *, std::size_t const & read_length);
			template<typename T>
				void ReadArray(T* array, int n);

			virtual void Read(uint8_t & x);
			virtual void Read(int8_t & x);
			virtual void Read(uint16_t & x)=0;
			virtual void Read(int16_t & x)=0;
			virtual void Read(uint32_t & x)=0;
			virtual void Read(int32_t & x)=0;
			virtual void Read(char & x);
			virtual void Read(bool & x);

			void WriteString(std::string &string);
			void WriteString(const char * const data);
			void WriteSizedString(const char * const data, std::size_t const & length);
			template<typename T>
				void WriteArray(T const* array, int n);

			virtual void Write(const uint8_t & x);
			virtual void Write(const int8_t & x);
			virtual void Write(const uint16_t & x)=0;
			virtual void Write(const int16_t & x)=0;
			virtual void Write(const uint32_t & x)=0;
			virtual void Write(const int32_t & x)=0;
			virtual void Write(const char & x);
			virtual void Write(const bool & x);

		protected:
			std::size_t GetPos(void);
			std::size_t Seek(std::ptrdiff_t const & bytes);
			std::size_t Skip(std::ptrdiff_t const & bytes);

			void ReadBE(uint32_t & x);
			void ReadBE(int32_t & x);
			void ReadBE(uint16_t & x);
			void ReadBE(int16_t & x);
			void ReadLE(uint32_t & x);
			void ReadLE(int32_t & x);
			void ReadLE(uint16_t & x);
			void ReadLE(int16_t & x);

			void WriteBE(const uint32_t & x);
			void WriteBE(const int32_t & x);
			void WriteBE(const uint16_t & x);
			void WriteBE(const int16_t & x);
			void WriteLE(const uint32_t & x);
			void WriteLE(const int32_t & x);
			void WriteLE(const uint16_t & x);
			void WriteLE(const int16_t & x);

			void Read(IffChunkId id);
			void Read(ULongBE& ulong);
			void Read(ULongLE& ulong);
			void Write(const IffChunkId id);
			void Write(const ULongBE& ulong);
			void Write(const ULongLE& ulong);

			void ReadRaw (void * data, std::size_t const & bytes);
			void WriteRaw(const void * const data, std::size_t const & bytes);
			bool Expect(IffChunkId& id);
			bool Expect  (void * data, std::size_t const & bytes);

			void Backpatch(long fileOffset, const void *data, unsigned numBytes);

		private:
			bool write_mode;
			std::string file_name_;
			std::fstream _stream;
		};
}}
