// -*- mode:c++; indent-tabs-mode:t -*-
/**
	\file
	interface file for psycle::host::ExtRiffFile and psycle::host::WaveFile.
	based on information from http://www.borg.com/~jglatt/tech/aboutiff.htm
*/
#pragma once
#include "abstractiff.hpp"
namespace psycle
{
	namespace host
	{

		/******* Data Structures *******/
		class IffChunkHeader : public BaseChunkHeader
		{
		protected:
			ULongBE ulength;
		public:
			IffChunkHeader();
			virtual ~IffChunkHeader();
			virtual std::uint32_t length();
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

			virtual void Open(std::string fname);
			virtual void Create(std::string fname, bool const & overwrite);
			virtual void close();
			virtual bool Eof();

			virtual void addFormChunk(IffChunkId id);
			virtual void addCatChunk(IffChunkId id, bool endprevious=true);
			virtual void addListChunk(IffChunkId id, bool endprevious=true);
			virtual void addListProperty(IffChunkId contentId, IffChunkId propId);
			virtual void addListProperty(IffChunkId contentId, IffChunkId propId, void const *data, std::uint32_t dataSize);

			virtual const IffChunkHeader& readHeader();
			virtual const IffChunkHeader& findChunk(IffChunkId id, bool allowWrap=false);
			virtual void skipThisChunk();

			inline void Read(std::uint8_t & x) { AbstractIff::Read(x); }
			inline void Read(std::int8_t & x) { AbstractIff::Read(x); }
			inline void Read(std::uint16_t & x) { ReadBE(x); }
			inline void Read(std::int16_t & x) { ReadBE(x); }
			inline void Read(std::uint32_t & x) { ReadBE(x); }
			inline void Read(std::int32_t & x) { ReadBE(x); }
			inline void Read(char & x) { AbstractIff::Read(x); }
			inline void Read(bool & x) { AbstractIff::Read(x); }

			inline void Write(const std::uint8_t & x) { AbstractIff::Write(x); }
			inline void Write(const std::int8_t & x) { AbstractIff::Write(x); }
			inline void Write(const std::uint16_t& x) { WriteBE(x); }
			inline void Write(const std::int16_t& x) { WriteBE(x); }
			inline void Write(const std::uint32_t& x) { WriteBE(x); }
			inline void Write(const std::int32_t& x) { WriteBE(x); }
			inline void Write(const char & x) { AbstractIff::Write(x); }
			inline void Write(const bool & x) { AbstractIff::Write(x); }

			static const IffChunkId grabbag;

		protected:
			void WriteChunkHeader(const IffChunkHeader& header);
			void WriteChunkId(IffChunkId id);

			IffChunkHeader currentHeader;
			std::uint32_t headerPosition;
		};
	}
}
