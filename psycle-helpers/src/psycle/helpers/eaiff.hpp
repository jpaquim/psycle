/**
	\file
	interface file for psycle::helpers::EA's iffFile.
	based on information from http://www.borg.com/~jglatt/tech/aboutiff.htm
*/
#pragma once
#include "abstractiff.hpp"
namespace psycle { namespace helpers {

using namespace universalis::stdlib;

		/******* Data Structures *******/
		class IffChunkHeader : public BaseChunkHeader
		{
			friend class EaIff;
		protected:
			LongBE ulength;
		public:
			IffChunkHeader();
			IffChunkHeader(const IffChunkId& newid, uint32_t newsize){
				copyId(newid, id);
				ulength.changeValue(newsize);
			};
			virtual ~IffChunkHeader();
			virtual void setLength(uint32_t newlength);
			virtual uint32_t length() const;
		};

		class FormChunkHeader
		{
		public:
			IffChunkHeader header;
			IffChunkId contentId;
		};

		/*********  IFF file reader comforming to EA's specifications ****/
		class EaIff : public AbstractIff
		{
		public:
			EaIff():headerPosition(0){}
			virtual ~EaIff(){}

			virtual bool isValidFile() const;

			virtual void Open(const std::string& fname);
			virtual void Create(const std::string& fname, const bool overwrite);
			virtual void close();
			virtual bool Eof() const;

			virtual void addNewChunk(const BaseChunkHeader& header);
			virtual const IffChunkHeader& readHeader();
			virtual const IffChunkHeader& findChunk(const IffChunkId& id, bool allowWrap=false);
			virtual void skipThisChunk();
			virtual void UpdateCurrentChunkSize();

			virtual void addFormChunk(const IffChunkId& id);
			virtual void addCatChunk(const IffChunkId& id, bool endprevious=true);
			virtual void addListChunk(const IffChunkId& id, bool endprevious=true);
			virtual void addListProperty(const IffChunkId& contentId, const IffChunkId& propId);
			virtual void addListProperty(const IffChunkId& contentId, const IffChunkId& propId, void const *data, uint32_t dataSize);

			inline void Read(uint8_t & x) { AbstractIff::Read(x); }
			inline void Read(int8_t & x) { AbstractIff::Read(x); }
			inline void Read(uint16_t & x) { ReadBE(x); }
			inline void Read(int16_t & x) { ReadBE(x); }
			inline void Read(uint32_t & x) { ReadBE(x); }
			inline void Read(int32_t & x) { ReadBE(x); }
			inline void Read(float & x) { ReadBE(x); }
			inline void Read(double & x) { ReadBE(x); }
			inline void Read(char & x) { AbstractIff::Read(x); }
			inline void Read(bool & x) { AbstractIff::Read(x); }
			inline void Read(IffChunkId & id) { AbstractIff::Read(id); }
			inline void Read(Long64BE & val){ AbstractIff::Read(val); }
			inline void Read(Long64LE & val){ AbstractIff::Read(val); }
			inline void Read(LongBE & val){ AbstractIff::Read(val); }
			inline void Read(LongLE & val){ AbstractIff::Read(val); }
			inline void Read(Long24BE & val){ AbstractIff::Read(val); }
			inline void Read(Long24LE & val){ AbstractIff::Read(val); }
			inline void Read(ShortBE & val){ AbstractIff::Read(val); }
			inline void Read(ShortLE & val){ AbstractIff::Read(val); }
			inline void Read(FloatBE & val){ AbstractIff::Read(val); }
			inline void Read(FloatLE & val){ AbstractIff::Read(val); }
			inline void Read(DoubleBE & val){ AbstractIff::Read(val); }
			inline void Read(DoubleLE & val){ AbstractIff::Read(val); }

			inline void Write(const uint8_t x) { AbstractIff::Write(x); }
			inline void Write(const int8_t x) { AbstractIff::Write(x); }
			inline void Write(const uint16_t x) { WriteBE(x); }
			inline void Write(const int16_t x) { WriteBE(x); }
			inline void Write(const uint32_t x) { WriteBE(x); }
			inline void Write(const int32_t x) { WriteBE(x); }
			inline void Write(const float x) { WriteBE(x); }
			inline void Write(const double x) { WriteBE(x); }
			inline void Write(const char x) { AbstractIff::Write(x); }
			inline void Write(const bool x) { AbstractIff::Write(x); }
			inline void Write(const Long64BE& val) { AbstractIff::Write(val); }
			inline void Write(const Long64LE& val) { AbstractIff::Write(val); }
			inline void Write(const IffChunkId& id){ AbstractIff::Write(id); }
			inline void Write(const LongBE& val){ AbstractIff::Write(val); }
			inline void Write(const LongLE& val){ AbstractIff::Write(val); }
			inline void Write(const Long24BE& val){ AbstractIff::Write(val); }
			inline void Write(const Long24LE& val){ AbstractIff::Write(val); }
			inline void Write(const ShortBE& val){ AbstractIff::Write(val); }
			inline void Write(const ShortLE& val){ AbstractIff::Write(val); }
			inline void Write(const FloatBE& val) { AbstractIff::Write(val); }
			inline void Write(const FloatLE& val) { AbstractIff::Write(val); }
			inline void Write(const DoubleBE& val) { AbstractIff::Write(val); }
			inline void Write(const DoubleLE& val) { AbstractIff::Write(val); }

			static const IffChunkId grabbag;

		protected:
			void WriteHeader(const IffChunkHeader& header);

			IffChunkHeader currentHeader;
			uint32_t headerPosition;
		};
}}
