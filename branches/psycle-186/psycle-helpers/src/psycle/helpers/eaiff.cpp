/**
	\file
	implementation file for psycle::helpers::eaiff
*/
#include "eaiff.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace psycle
{
	namespace helpers
	{
		const IffChunkId EaIff::grabbag = {' ',' ',' ',' '};

		std::uint32_t IffChunkHeader::length() const {
			return ulength.unsignedValue();
		}

		IffChunkHeader::IffChunkHeader(){}
		IffChunkHeader::~IffChunkHeader(){}

		bool EaIff::isValidFile() const { return false;}

		void EaIff::Open(std::string fname) { AbstractIff::Open(fname); }
		void EaIff::Create(std::string fname, bool const & overwrite) { AbstractIff::Create(fname, overwrite); }
		void EaIff::close() { AbstractIff::close(); }
		bool EaIff::Eof() const { return AbstractIff::Eof(); }


		void EaIff::addFormChunk(IffChunkId id) {
		}
		void EaIff::addListChunk(IffChunkId id, bool endprevious) {
		}
		void EaIff::addCatChunk(IffChunkId id, bool endprevious) {
		}
		void EaIff::addListProperty(IffChunkId contentId, IffChunkId propId) {
		}
		void EaIff::addListProperty(IffChunkId contentId, IffChunkId propId, void const *data, std::uint32_t dataSize) {
		}

		const IffChunkHeader& EaIff::readHeader() {
			if(GetPos()%2 > 0) Skip(1);
			Read(currentHeader.id);
			Read(currentHeader.ulength);
			return currentHeader;
		}
		const IffChunkHeader& EaIff::findChunk(IffChunkId id, bool allowWrap) {

			return currentHeader;
		}
		void EaIff::skipThisChunk() {
			Seek(headerPosition + 8 + currentHeader.length());
			if (GetPos() % 2 > 0) Skip(1);
		}

		void EaIff::WriteHeader(const IffChunkHeader& header) {
			if (GetPos()%2 > 0) Write('\0');
			Write(header.id);
			Write(header.ulength);
			currentHeader=header;
		}


	}
}

