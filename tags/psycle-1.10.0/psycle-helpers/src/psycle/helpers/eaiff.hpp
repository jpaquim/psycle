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
			ULongBE ulength;
		public:
			IffChunkHeader();
			virtual ~IffChunkHeader();
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

			virtual void Open(std::string fname);
			virtual void Create(std::string fname, bool const & overwrite);
			virtual void close();
			virtual bool Eof() const;

			virtual void addFormChunk(IffChunkId id);
			virtual void addCatChunk(IffChunkId id, bool endprevious=true);
			virtual void addListChunk(IffChunkId id, bool endprevious=true);
			virtual void addListProperty(IffChunkId contentId, IffChunkId propId);
			virtual void addListProperty(IffChunkId contentId, IffChunkId propId, void const *data, uint32_t dataSize);

			virtual const IffChunkHeader& readHeader();
			virtual const IffChunkHeader& findChunk(IffChunkId id, bool allowWrap=false);
			virtual void skipThisChunk();

			inline void Read(uint8_t & x) { AbstractIff::Read(x); }
			inline void Read(int8_t & x) { AbstractIff::Read(x); }
			inline void Read(uint16_t & x) { ReadBE(x); }
			inline void Read(int16_t & x) { ReadBE(x); }
			inline void Read(uint32_t & x) { ReadBE(x); }
			inline void Read(int32_t & x) { ReadBE(x); }
			inline void Read(char & x) { AbstractIff::Read(x); }
			inline void Read(bool & x) { AbstractIff::Read(x); }

			inline void Write(const uint8_t & x) { AbstractIff::Write(x); }
			inline void Write(const int8_t & x) { AbstractIff::Write(x); }
			inline void Write(const uint16_t& x) { WriteBE(x); }
			inline void Write(const int16_t& x) { WriteBE(x); }
			inline void Write(const uint32_t& x) { WriteBE(x); }
			inline void Write(const int32_t& x) { WriteBE(x); }
			inline void Write(const char & x) { AbstractIff::Write(x); }
			inline void Write(const bool & x) { AbstractIff::Write(x); }

			static const IffChunkId grabbag;

		protected:
			void WriteHeader(const IffChunkHeader& header);
			void Read(IffChunkId id) { AbstractIff::Read(id); }
			void Read(ULongBE& ulong){ AbstractIff::Read(ulong); }
			void Read(ULongLE& ulong){ AbstractIff::Read(ulong); }
			void Write(const IffChunkId id){ AbstractIff::Write(id); }
			void Write(const ULongBE& ulong){ AbstractIff::Write(ulong); }
			void Write(const ULongLE& ulong){ AbstractIff::Write(ulong); }

			IffChunkHeader currentHeader;
			uint32_t headerPosition;
		};
}}
