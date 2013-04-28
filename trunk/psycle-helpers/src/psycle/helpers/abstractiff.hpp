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
		/// big-endian 64-bit integer.
		class Long64BE
		{
		public:
			union _d{
				struct _byte{
					unsigned char hihihi;
					unsigned char hihilo;
					unsigned char hilohi;
					unsigned char hilolo;
					unsigned char lohihi;
					unsigned char lohilo;
					unsigned char lolohi;
					unsigned char lololo;
				}byte;
				uint64_t originalValue;
			}d;
			Long64BE();
			Long64BE(const uint64_t);
			Long64BE(const int64_t);
			inline void changeValue(const uint64_t);
			inline void changeValue(const int64_t);
			inline uint64_t unsignedValue() const;
			inline int64_t signedValue() const;
		};

		/// big-endian 32-bit integer.
		class LongBE
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
			LongBE();
			LongBE(const uint32_t);
			LongBE(const int32_t);
			inline void changeValue(const uint32_t);
			inline void changeValue(const int32_t);
			inline uint32_t unsignedValue() const;
			inline int32_t signedValue() const;
		};
		/// big-endian 24-bit intenger.
		class Long24LE;
		class Long24BE
		{
		public:
			struct _byte{
				unsigned char hi;
				unsigned char lohi;
				unsigned char lolo;
			}byte;
			Long24BE();
			Long24BE(const uint32_t);
			Long24BE(const int32_t);
			inline void changeValue(const uint32_t);
			inline void changeValue(const int32_t);
			inline uint32_t unsignedValue() const;
			inline int32_t signedValue() const;
		};
		/// big-endian 16-bit integer.
		class ShortBE
		{
		public:
			union _d {
				struct _byte{
					unsigned char hi;
					unsigned char lo;
				}byte;
				uint16_t originalValue;
			}d;
			ShortBE();
			ShortBE(const uint16_t);
			ShortBE(const int16_t);
			inline void changeValue(const uint16_t);
			inline void changeValue(const int16_t);
			inline uint16_t unsignedValue() const;
			inline int16_t signedValue() const;
		};

		// big-endian 32-bit fixed point value. (16.16)
		class FixedPointBE
		{
		public:
			union _d {
				struct _byte{
					char hi;
					unsigned char lo;
				}byte;
				int16_t value;
			}integer;
			union _d2 {
				struct _byte{
					unsigned char hi;
					unsigned char lo;
				}byte;
				uint16_t value;
			}decimal;
			FixedPointBE();
			FixedPointBE(const float);
			inline void changeValue(const float);
			inline float value() const;
		};
		/// big-endian 32-bit float.
		class FloatBE
		{
		public:
			union _d {
				struct _byte {
					unsigned char hihi;
					unsigned char hilo;
					unsigned char lohi;
					unsigned char lolo;
				}byte;
				float originalValue;
			}d;
			FloatBE();
			FloatBE(const float);
			inline void changeValue(const float);
			inline float Value() const;
		};
		/// big-endian 64-bit float.
		class DoubleBE
		{
		public:
			union _d {
				struct _byte {
					unsigned char hihihi;
					unsigned char hihilo;
					unsigned char hilohi;
					unsigned char hilolo;
					unsigned char lohihi;
					unsigned char lohilo;
					unsigned char lolohi;
					unsigned char lololo;
				}byte;
				double originalValue;
			}d;
			DoubleBE();
			DoubleBE(const double);
			inline void changeValue(const double);
			inline double Value() const;
		};

		/// little-endian 64-bit integer.
		class Long64LE
		{
		public:
			union _d{
				struct _byte{
					unsigned char lololo;
					unsigned char lolohi;
					unsigned char lohilo;
					unsigned char lohihi;
					unsigned char hilolo;
					unsigned char hilohi;
					unsigned char hihilo;
					unsigned char hihihi;
				}byte;
				uint64_t originalValue;
			}d;
			Long64LE();
			Long64LE(const uint64_t);
			Long64LE(const int64_t);
			inline void changeValue(const uint64_t);
			inline void changeValue(const int64_t);
			inline uint64_t unsignedValue() const;
			inline int64_t signedValue() const;
		};

		/// little-endian 32-bit integer.
		class LongLE
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
			LongLE();
			LongLE(const uint32_t);
			LongLE(const int32_t);
			inline void changeValue(const uint32_t);
			inline void changeValue(const int32_t);
			inline uint32_t unsignedValue() const;
			inline int32_t signedValue() const;
		};

		/// little-endian 24-bit integer.
		class Long24LE
		{
		public:
			struct _byte{
				unsigned char lolo;
				unsigned char lohi;
				unsigned char hi;
			}byte;
			Long24LE();
			Long24LE(const uint32_t);
			Long24LE(const int32_t);
			inline void changeValue(const uint32_t);
			inline void changeValue(const int32_t);
			inline uint32_t unsignedValue() const;
			inline int32_t signedValue() const;
		};

		/// little-endian 16-bit integer.
		class ShortLE
		{
		public:
			union _d {
				struct _byte{
					unsigned char lo;
					unsigned char hi;
				}byte;
				uint16_t originalValue;
			}d;
			ShortLE();
			ShortLE(const uint16_t);
			ShortLE(const int16_t);
			inline void changeValue(const uint16_t);
			inline void changeValue(const int16_t);
			inline uint16_t unsignedValue() const;
			inline int16_t signedValue() const;
		};
		/// little-endian 32-bit float.
		class FloatLE
		{
		public:
			union _d {
				struct _byte {
					unsigned char lolo;
					unsigned char lohi;
					unsigned char hilo;
					unsigned char hihi;
				}byte;
				float originalValue;
			}d;
			FloatLE();
			FloatLE(const float);
			inline void changeValue(const float);
			inline float Value() const;
		};
		/// little-endian 64-bit float.
		class DoubleLE
		{
		public:
			union _d {
				struct _byte {
					unsigned char lololo;
					unsigned char lolohi;
					unsigned char lohilo;
					unsigned char lohihi;
					unsigned char hilolo;
					unsigned char hilohi;
					unsigned char hihilo;
					unsigned char hihihi;
				}byte;
				double originalValue;
			}d;
			DoubleLE();
			DoubleLE(const double);
			inline void changeValue(const double);
			inline double Value() const;
		};

		/******* Data Structures *******/
		typedef char IffChunkId[4];
		/*abstract*/ class BaseChunkHeader
		{
		public:
			IffChunkId id;
			virtual ~BaseChunkHeader();

			static void copyId(const IffChunkId& idOrigin, IffChunkId& idDest);
			std::string idString() const;
			bool matches(const IffChunkId& id2) const;
			virtual void setLength(uint32_t newlength) = 0;
			virtual uint32_t length() const = 0;
		};


		/******* Base Class for reader ******/
		class AbstractIff
		{
		public:

			AbstractIff();
			virtual ~AbstractIff();

			virtual bool isValidFile() const = 0;

			virtual void Open(const std::string& fname);
			virtual void Create(const std::string& fname, const bool overwrite);
			virtual void close();
			virtual bool Eof() const;
			std::string const inline & file_name() const throw();
			std::streamsize fileSize();

			virtual void addNewChunk(const BaseChunkHeader& header) = 0;
			virtual const BaseChunkHeader& readHeader() = 0;
			virtual const BaseChunkHeader& findChunk(const IffChunkId& id, bool allowWrap=false) = 0;
			virtual void skipThisChunk() = 0;
			virtual void UpdateCurrentChunkSize() = 0;
		
			void ReadString(std::string &);
			void ReadString(char *, std::size_t const max_length);
			void ReadSizedString(char *, std::size_t const read_length);
			template<typename T>
				void ReadArray(T* thearray, std::size_t n);

			inline void Read(uint8_t & x);
			inline void Read(int8_t & x);
			virtual void Read(uint16_t & x)=0;
			virtual void Read(int16_t & x)=0;
			virtual void Read(uint32_t & x)=0;
			virtual void Read(int32_t & x)=0;
			virtual void Read(float & x)=0;
			virtual void Read(double & x)=0;
			inline void Read(char & x);
			inline void Read(bool & x);
			inline void Read(IffChunkId & id);
			inline void Read(Long64BE & val);
			inline void Read(Long64LE & val);
			inline void Read(LongBE & val);
			inline void Read(LongLE & val);
			inline void Read(Long24BE & val);
			inline void Read(Long24LE & val);
			inline void Read(ShortBE & val);
			inline void Read(ShortLE & val);
			inline void Read(FloatBE & val);
			inline void Read(FloatLE & val);
			inline void Read(DoubleBE & val);
			inline void Read(DoubleLE & val);

			inline void WriteString(const std::string &string);
			inline void WriteString(const char * const data);
			inline void WriteSizedString(const char * const data, std::size_t const length);
			template<typename T>
				inline void WriteArray(T const* thearray, std::size_t n);

			inline void Write(const uint8_t x);
			inline void Write(const int8_t x);
			virtual void Write(const uint16_t x)=0;
			virtual void Write(const int16_t x)=0;
			virtual void Write(const uint32_t x)=0;
			virtual void Write(const int32_t x)=0;
			virtual void Write(const uint64_t x)=0;
			virtual void Write(const int64_t x)=0;
			virtual void Write(const float x)=0;
			virtual void Write(const double x)=0;
			inline void Write(const char x);
			inline void Write(const bool x);
			inline void Write(const IffChunkId& id);
			inline void Write(const Long64BE& val);
			inline void Write(const Long64LE& val);
			inline void Write(const LongBE& val);
			inline void Write(const LongLE& val);
			inline void Write(const Long24BE& val);
			inline void Write(const Long24LE& val);
			inline void Write(const ShortBE& val);
			inline void Write(const ShortLE& val);
			inline void Write(const FloatBE& val);
			inline void Write(const FloatLE& val);
			inline void Write(const DoubleBE& val);
			inline void Write(const DoubleLE& val);
		protected:
			inline bool isWriteMode() const {return write_mode;}
			std::streampos GetPos(void);
			std::streampos Seek(std::streampos const bytes);
			std::streampos Skip(std::streampos const bytes);

			inline void ReadBE(uint32_t & x);
			inline void ReadBE(int32_t & x);
			inline void ReadBE(uint16_t & x);
			inline void ReadBE(int16_t & x);
			inline void ReadBE(float & x);
			inline void ReadBE(double & x);
			inline void ReadLE(uint32_t & x);
			inline void ReadLE(int32_t & x);
			inline void ReadLE(uint16_t & x);
			inline void ReadLE(int16_t & x);
			inline void ReadLE(float & x);
			inline void ReadLE(double & x);

			inline void WriteBE(const uint32_t x);
			inline void WriteBE(const int32_t x);
			inline void WriteBE(const uint16_t x);
			inline void WriteBE(const int16_t x);
			inline void WriteBE(const float x);
			inline void WriteBE(const double x);
			inline void WriteLE(const uint32_t x);
			inline void WriteLE(const int32_t x);
			inline void WriteLE(const uint16_t x);
			inline void WriteLE(const int16_t x);
			inline void WriteLE(const float x);
			inline void WriteLE(const double x);

			inline void ReadRaw (void * data, std::size_t const bytes);
			inline void WriteRaw(const void * const data, std::size_t const bytes);
			bool Expect(const IffChunkId& id);
			bool Expect(const void * const data, std::size_t const bytes);
			void UpdateFormSize(std::streamoff headerposition, uint32_t numBytes);

		private:
			bool write_mode;
			std::string file_name_;
			std::fstream _stream;
		};
}}
