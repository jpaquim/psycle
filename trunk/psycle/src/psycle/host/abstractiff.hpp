// -*- mode:c++; indent-tabs-mode:t -*-
/**
	\file
	interface file for psycle::host::abstractIff psycle::Host::BaseChunkHeader, and the different big-endian and little-endian types.
*/
#pragma once
namespace psycle
{
	namespace host
	{
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
				unsigned long originalValue;
			}d;
			ULongBE();
			ULongBE(std::uint32_t);
			virtual unsigned long unsignedValue();
			virtual signed long signedValue();
		};

		class LongBE : public ULongBE
		{
		public:
			LongBE();
			LongBE(std::int32_t);
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
				unsigned short originalValue;
			}d;
			UShortBE();
			UShortBE(std::uint16_t);
			virtual unsigned short unsignedValue();
			virtual signed short signedValue();
		};
		class ShortBE : public UShortBE
		{
		public:
			ShortBE();
			ShortBE(std::int16_t);
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
				unsigned short value;
			}decimal;
			float value();
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
				unsigned long originalValue;
			}d;
			ULongLE();
			ULongLE(std::uint32_t);
			virtual unsigned long unsignedValue();
			virtual signed long signedValue();
		};

		class LongLE : public ULongLE
		{
		public:
			LongLE();
			LongLE(std::int32_t);
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
				unsigned short originalValue;
			}d;
			UShortLE();
			UShortLE(std::uint16_t);
			virtual unsigned short unsignedValue();
			virtual signed short signedValue();
		};
		class ShortLE : public UShortLE
		{
		public:
			ShortLE();
			ShortLE(std::int16_t);
		};

		/******* Data Structures *******/
		typedef char IffChunkId[4];
		class BaseChunkHeader
		{
		public:
			IffChunkId id;
			virtual ~BaseChunkHeader();
			std::string idString();
			boolean matches(IffChunkId id2);
			virtual std::uint32_t length() = 0;
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
			virtual bool Eof();
			std::string const inline & file_name() const throw();
			std::size_t fileSize();

			virtual void addNewChunk(const BaseChunkHeader& header) = 0;
			virtual const BaseChunkHeader& readHeader() = 0;
			virtual const BaseChunkHeader& findChunk(IffChunkId id) = 0;
			virtual void skipThisChunk() = 0;
		
			void ReadString(std::string &);
			void ReadString(char *, std::size_t const & max_length);
			void ReadSizedString(char *, std::size_t const & max_length);
			template<typename T>
				void ReadArray(T* array, int n);
			virtual void Read(std::uint8_t & x);
			virtual void Read(std::int8_t & x);
			virtual void Read(std::uint16_t & x)=0;
			virtual void Read(std::int16_t & x)=0;
			virtual void Read(std::uint32_t & x)=0;
			virtual void Read(std::int32_t & x)=0;
			virtual void Read(char & x);
			virtual void Read(bool & x);

			void WriteString(std::string &string);
			void WriteString(const char * const data);
			void WriteSizedString(const char * const data, std::size_t const & length);

			template<typename T>
				void WriteArray(T const* array, int n);
			virtual void Write(const std::uint8_t & x);
			virtual void Write(const std::int8_t & x);
			virtual void Write(const std::uint16_t & x)=0;
			virtual void Write(const std::int16_t & x)=0;
			virtual void Write(const std::uint32_t & x)=0;
			virtual void Write(const std::int32_t & x)=0;
			virtual void Write(const char & x);
			virtual void Write(const bool & x);

		protected:
			std::size_t GetPos(void);
			std::size_t Seek(std::ptrdiff_t const & bytes);
			std::size_t Skip(std::ptrdiff_t const & bytes);

			void ReadBE(std::uint32_t & x);
			void ReadBE(std::int32_t & x);
			void ReadBE(std::uint16_t & x);
			void ReadBE(std::int16_t & x);
			void ReadLE(std::uint32_t & x);
			void ReadLE(std::int32_t & x);
			void ReadLE(std::uint16_t & x);
			void ReadLE(std::int16_t & x);

			void WriteBE(const std::uint32_t & x);
			void WriteBE(const std::int32_t & x);
			void WriteBE(const std::uint16_t & x);
			void WriteBE(const std::int16_t & x);
			void WriteLE(const std::uint32_t & x);
			void WriteLE(const std::int32_t & x);
			void WriteLE(const std::uint16_t & x);
			void WriteLE(const std::int16_t & x);

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
	}
}
