/**
	\file
	interface file for psycle::helpers::MsRiff.
	
*/
#pragma once
#include "abstractiff.hpp"

namespace psycle { namespace helpers {

	using namespace universalis::stdlib;

	/******* Data Structures *******/
	template<typename long_type>
	class RiffChunkHeader : public BaseChunkHeader
	{
	public:
		long_type ulength;
		RiffChunkHeader(){};
		RiffChunkHeader(const IffChunkId& newid, uint32_t newsize){
			copyId(newid, id);
			ulength.changeValue(newsize);
		};
		virtual ~RiffChunkHeader(){};
		virtual void setLength(uint32_t newlength);
		virtual uint32_t length() const;
	};


	/*********  IFF file reader comforming to Microsoft RIFF/RIFX specifications ****/
	class MsRiff : public AbstractIff
	{
	public:
		MsRiff():headerPosition(0), isLittleEndian(true),isValid(false){}
		virtual ~MsRiff(){}

		virtual bool isValidFile() const;

		virtual void Open(const std::string& fname);
		virtual void Create(const std::string& fname, bool const overwrite, 
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
			bool const littleEndian=true
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
			bool const littleEndian=false
#endif
		);
		virtual void close();
		virtual bool Eof() const;
/*
		virtual void addFormChunk(IffChunkId id);
*/

		virtual void addNewChunk(const BaseChunkHeader& header);
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
		virtual const RiffChunkHeader<LongLE>& readHeader();
		virtual const RiffChunkHeader<LongLE>& findChunk(const IffChunkId& id, bool allowWrap=false);
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
		virtual const RiffChunkHeader<LongBE>& readHeader();
		virtual const RiffChunkHeader<LongBE>& findChunk(const IffChunkId& id, bool allowWrap=false);
#endif
		virtual void skipThisChunk();
		virtual void UpdateCurrentChunkSize();

		inline void Read(uint8_t & x) { AbstractIff::Read(x); }
		inline void Read(int8_t & x) { AbstractIff::Read(x); }
		inline void Read(uint16_t & x) { if (isLittleEndian) {ReadLE(x);} else {ReadBE(x);} }
		inline void Read(int16_t & x) { if (isLittleEndian) {ReadLE(x);} else {ReadBE(x);} }
		inline void Read(uint32_t & x) { if (isLittleEndian) {ReadLE(x);} else {ReadBE(x);} }
		inline void Read(int32_t & x) { if (isLittleEndian) {ReadLE(x);} else {ReadBE(x);} }
		inline void Read(float & x) { if (isLittleEndian) {ReadLE(x);} else {ReadBE(x);} }
		inline void Read(double & x) { if (isLittleEndian) {ReadLE(x);} else {ReadBE(x);} }
		inline void Read(char & x) { AbstractIff::Read(x); }
		inline void Read(bool & x) { AbstractIff::Read(x); }
		inline void Read(IffChunkId & id) {AbstractIff::Read(id);}
		inline void Read(Long64BE & val) {AbstractIff::Read(val);}
		inline void Read(Long64LE & val) {AbstractIff::Read(val);}
		inline void Read(LongBE & val) {AbstractIff::Read(val);}
		inline void Read(LongLE & val) {AbstractIff::Read(val);}
		inline void Read(Long24BE & val){ AbstractIff::Read(val); }
		inline void Read(Long24LE & val){ AbstractIff::Read(val); }
		inline void Read(ShortBE & val){ AbstractIff::Read(val); }
		inline void Read(ShortLE & val){ AbstractIff::Read(val); }
		inline void Read(FloatBE & val){ AbstractIff::Read(val); }
		inline void Read(FloatLE & val){ AbstractIff::Read(val); }
		inline void Read(DoubleBE & val){ AbstractIff::Read(val); }
		inline void Read(DoubleLE & val){ AbstractIff::Read(val); }

        inline void Write(const uint8_t & x) { AbstractIff::Write(x); }
        inline void Write(const int8_t & x) { AbstractIff::Write(x); }
        inline void Write(const uint16_t& x) { if (isLittleEndian) {WriteLE(x);} else {WriteBE(x);} }
		inline void Write(const int16_t& x) { if (isLittleEndian) {WriteLE(x);} else {WriteBE(x);} }
		inline void Write(const uint32_t& x) { if (isLittleEndian) {WriteLE(x);} else {WriteBE(x);} }
		inline void Write(const int32_t& x) { if (isLittleEndian) {WriteLE(x);} else {WriteBE(x);} }
		inline void Write(const float x) { if (isLittleEndian) {WriteLE(x);} else {WriteBE(x);} }
		inline void Write(const double x) { if (isLittleEndian) {WriteLE(x);} else {WriteBE(x);} }
		inline void Write(const char & x) { AbstractIff::Write(x); }
		inline void Write(const bool & x) { AbstractIff::Write(x); }
		inline void Write(const IffChunkId & id) { AbstractIff::Write(id); }
		inline void Write(const Long64BE & val) { AbstractIff::Write(val); }
		inline void Write(const Long64LE & val) { AbstractIff::Write(val); }
		inline void Write(const LongBE & val) { AbstractIff::Write(val); }
		inline void Write(const LongLE & val) { AbstractIff::Write(val); }
		inline void Write(const Long24BE& val){ AbstractIff::Write(val); }
		inline void Write(const Long24LE& val){ AbstractIff::Write(val); }
		inline void Write(const ShortBE& val){ AbstractIff::Write(val); }
		inline void Write(const ShortLE& val){ AbstractIff::Write(val); }
		inline void Write(const FloatBE& val){ AbstractIff::Write(val); }
		inline void Write(const FloatLE& val){ AbstractIff::Write(val); }
		inline void Write(const DoubleBE& val){ AbstractIff::Write(val); }
		inline void Write(const DoubleLE& val){ AbstractIff::Write(val); }

		static const IffChunkId RIFF;
		static const IffChunkId RIFX;
	protected:
		
#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
		RiffChunkHeader<LongLE> currentHeader;
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
		RiffChunkHeader<LongBE> currentHeader;
#endif
		//RIFF only supports up to 32bits. for 64bits there's the RIFF64 format (and the W64 format...)
		std::streampos headerPosition;
		bool isLittleEndian;
		bool isValid;
	};
}}
