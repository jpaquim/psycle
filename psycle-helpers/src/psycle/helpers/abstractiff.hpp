/**
	\file
	interface file for psycle::helpers::abstractIff psycle::helpers::BaseChunkHeader, and the different big-endian and little-endian types.
*/
#pragma once

#include <universalis.hpp>
#include <string>
#include <fstream>


// ************************* FIXME ***************************
// GCC emits a flood of warnings here due to inline functions lacking a definition in the header.
// Probably MSVC doesn't show this.
#ifdef DIVERSALIS__COMPILER__MICROSOFT
	#define FIXME_inline inline
#else
	#define FIXME_inline
#endif
// ***********************************************************

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
			FIXME_inline void changeValue(const uint64_t);
			FIXME_inline void changeValue(const int64_t);
			FIXME_inline uint64_t unsignedValue() const;
			FIXME_inline int64_t signedValue() const;
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
			FIXME_inline void changeValue(const uint32_t);
			FIXME_inline void changeValue(const int32_t);
			FIXME_inline uint32_t unsignedValue() const;
			FIXME_inline int32_t signedValue() const;
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
			FIXME_inline void changeValue(const uint32_t);
			FIXME_inline void changeValue(const int32_t);
			FIXME_inline uint32_t unsignedValue() const;
			FIXME_inline int32_t signedValue() const;
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
			FIXME_inline void changeValue(const uint16_t);
			FIXME_inline void changeValue(const int16_t);
			FIXME_inline uint16_t unsignedValue() const;
			FIXME_inline int16_t signedValue() const;
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
			FIXME_inline void changeValue(const float);
			FIXME_inline float value() const;
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
			FIXME_inline void changeValue(const float);
			FIXME_inline float Value() const;
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
			FIXME_inline void changeValue(const double);
			FIXME_inline double Value() const;
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
			FIXME_inline void changeValue(const uint64_t);
			FIXME_inline void changeValue(const int64_t);
			FIXME_inline uint64_t unsignedValue() const;
			FIXME_inline int64_t signedValue() const;
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
			FIXME_inline void changeValue(const uint32_t);
			FIXME_inline void changeValue(const int32_t);
			FIXME_inline uint32_t unsignedValue() const;
			FIXME_inline int32_t signedValue() const;
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
			FIXME_inline void changeValue(const uint32_t);
			FIXME_inline void changeValue(const int32_t);
			FIXME_inline uint32_t unsignedValue() const;
			FIXME_inline int32_t signedValue() const;
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
			FIXME_inline void changeValue(const uint16_t);
			FIXME_inline void changeValue(const int16_t);
			FIXME_inline uint16_t unsignedValue() const;
			FIXME_inline int16_t signedValue() const;
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
			FIXME_inline void changeValue(const float);
			FIXME_inline float Value() const;
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
			FIXME_inline void changeValue(const double);
			FIXME_inline double Value() const;
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
			std::string const FIXME_inline & file_name() const throw();
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

			FIXME_inline void Read(uint8_t & x);
			FIXME_inline void Read(int8_t & x);
			virtual void Read(uint16_t & x)=0;
			virtual void Read(int16_t & x)=0;
			virtual void Read(uint32_t & x)=0;
			virtual void Read(int32_t & x)=0;
			virtual void Read(float & x)=0;
			virtual void Read(double & x)=0;
			FIXME_inline void Read(char & x);
			FIXME_inline void Read(bool & x);
			FIXME_inline void Read(IffChunkId & id);
			FIXME_inline void Read(Long64BE & val);
			FIXME_inline void Read(Long64LE & val);
			FIXME_inline void Read(LongBE & val);
			FIXME_inline void Read(LongLE & val);
			FIXME_inline void Read(Long24BE & val);
			FIXME_inline void Read(Long24LE & val);
			FIXME_inline void Read(ShortBE & val);
			FIXME_inline void Read(ShortLE & val);
			FIXME_inline void Read(FloatBE & val);
			FIXME_inline void Read(FloatLE & val);
			FIXME_inline void Read(DoubleBE & val);
			FIXME_inline void Read(DoubleLE & val);

			FIXME_inline void WriteString(const std::string &string);
			FIXME_inline void WriteString(const char * const data);
			FIXME_inline void WriteSizedString(const char * const data, std::size_t const length);
			template<typename T>
				FIXME_inline void WriteArray(T const* thearray, std::size_t n);

			FIXME_inline void Write(const uint8_t x);
			FIXME_inline void Write(const int8_t x);
			virtual void Write(const uint16_t x)=0;
			virtual void Write(const int16_t x)=0;
			virtual void Write(const uint32_t x)=0;
			virtual void Write(const int32_t x)=0;
			virtual void Write(const uint64_t x)=0;
			virtual void Write(const int64_t x)=0;
			virtual void Write(const float x)=0;
			virtual void Write(const double x)=0;
			FIXME_inline void Write(const char x);
			FIXME_inline void Write(const bool x);
			FIXME_inline void Write(const IffChunkId& id);
			FIXME_inline void Write(const Long64BE& val);
			FIXME_inline void Write(const Long64LE& val);
			FIXME_inline void Write(const LongBE& val);
			FIXME_inline void Write(const LongLE& val);
			FIXME_inline void Write(const Long24BE& val);
			FIXME_inline void Write(const Long24LE& val);
			FIXME_inline void Write(const ShortBE& val);
			FIXME_inline void Write(const ShortLE& val);
			FIXME_inline void Write(const FloatBE& val);
			FIXME_inline void Write(const FloatLE& val);
			FIXME_inline void Write(const DoubleBE& val);
			FIXME_inline void Write(const DoubleLE& val);
		protected:
			FIXME_inline bool isWriteMode() const {return write_mode;}
			std::streampos GetPos(void);
			std::streampos Seek(std::streampos const bytes);
			std::streampos Skip(std::streampos const bytes);

			FIXME_inline void ReadBE(uint32_t & x);
			FIXME_inline void ReadBE(int32_t & x);
			FIXME_inline void ReadBE(uint16_t & x);
			FIXME_inline void ReadBE(int16_t & x);
			FIXME_inline void ReadBE(float & x);
			FIXME_inline void ReadBE(double & x);
			FIXME_inline void ReadLE(uint32_t & x);
			FIXME_inline void ReadLE(int32_t & x);
			FIXME_inline void ReadLE(uint16_t & x);
			FIXME_inline void ReadLE(int16_t & x);
			FIXME_inline void ReadLE(float & x);
			FIXME_inline void ReadLE(double & x);

			FIXME_inline void WriteBE(const uint32_t x);
			FIXME_inline void WriteBE(const int32_t x);
			FIXME_inline void WriteBE(const uint16_t x);
			FIXME_inline void WriteBE(const int16_t x);
			FIXME_inline void WriteBE(const float x);
			FIXME_inline void WriteBE(const double x);
			FIXME_inline void WriteLE(const uint32_t x);
			FIXME_inline void WriteLE(const int32_t x);
			FIXME_inline void WriteLE(const uint16_t x);
			FIXME_inline void WriteLE(const int16_t x);
			FIXME_inline void WriteLE(const float x);
			FIXME_inline void WriteLE(const double x);

			FIXME_inline void ReadRaw (void * data, std::size_t const bytes);
			FIXME_inline void WriteRaw(const void * const data, std::size_t const bytes);
			bool Expect(const IffChunkId& id);
			bool Expect(const void * const data, std::size_t const bytes);
			void UpdateFormSize(std::streamoff headerposition, uint32_t numBytes);

		private:
			bool write_mode;
			std::string file_name_;
			std::fstream _stream;
		};
		
		template<typename T>
		void AbstractIff::ReadArray(T* thearray, std::size_t n) {
			for(std::size_t i=0;i<n;i++) Read(thearray[i]);
		}
		
}}
