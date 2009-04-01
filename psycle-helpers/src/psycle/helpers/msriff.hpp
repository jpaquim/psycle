/**
	\file
	interface file for psycle::helpers::RiffFile.
	
*/
#pragma once
#include "AbstractIff.hpp"
namespace psycle
{
	namespace helpers
	{

		/******* Data Structures *******/
		class RiffChunkHeader : public BaseChunkHeader
		{
		protected:
			ULongLE ulength;
		public:
			RiffChunkHeader();
			virtual ~RiffChunkHeader();
			virtual std::uint32_t length();
		};

		class RiffFormChunkHeader
		{
		public:
			RiffChunkHeader header;
			IffChunkId contentId;
		};

		/*********  IFF file reader comforming to Microsoft RIFF specifications ****/
		class MsRiff : public AbstractIff
		{
		public:
			MsRiff():headerPosition(0){}
			virtual ~MsRiff(){}

			virtual bool isValidFile() const;

			virtual void Open(std::string fname);
			virtual void Create(std::string fname, bool const & overwrite);
			virtual void close();
			virtual bool Eof();
/*
			virtual void addFormChunk(IffChunkId id);
			virtual void addCatChunk(IffChunkId id, bool endprevious=true);
			virtual void addListChunk(IffChunkId id, bool endprevious=true);
			virtual void addListProperty(IffChunkId contentId, IffChunkId propId);
			virtual void addListProperty(IffChunkId contentId, IffChunkId propId, void const *data, std::uint32_t dataSize);
*/
			virtual const RiffChunkHeader& readHeader();
			virtual const RiffChunkHeader& findChunk(IffChunkId id, bool allowWrap=false);
			virtual void skipThisChunk();

			inline void Read(std::uint8_t & x) { AbstractIff::Read(x); }
			inline void Read(std::int8_t & x) { AbstractIff::Read(x); }
			inline void Read(std::uint16_t & x) { ReadLE(x); }
			inline void Read(std::int16_t & x) { ReadLE(x); }
			inline void Read(std::uint32_t & x) { ReadLE(x); }
			inline void Read(std::int32_t & x) { ReadLE(x); }
			inline void Read(char & x) { AbstractIff::Read(x); }
			inline void Read(bool & x) { AbstractIff::Read(x); }

			inline void Write(const std::uint8_t & x) { AbstractIff::Write(x); }
			inline void Write(const std::int8_t & x) { AbstractIff::Write(x); }
			inline void Write(const std::uint16_t& x) { WriteLE(x); }
			inline void Write(const std::int16_t& x) { WriteLE(x); }
			inline void Write(const std::uint32_t& x) { WriteLE(x); }
			inline void Write(const std::int32_t& x) { WriteLE(x); }
			inline void Write(const char & x) { AbstractIff::Write(x); }
			inline void Write(const bool & x) { AbstractIff::Write(x); }
/*
			static const RiffChunkId grabbag;
*/
		protected:
			void WriteChunkHeader(const RiffChunkHeader& header);
			void WriteChunkId(IffChunkId id);

			RiffChunkHeader currentHeader;
			std::uint32_t headerPosition;
		};
	}
}
