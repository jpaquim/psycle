/**
	\file
	implementation file for psycle::host::eaiff
*/

#include "Eaiff.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace psycle
{
	namespace host
	{
		const IffChunkId EaIff::grabbag = {' ',' ',' ',' '};

		std::uint32_t IffChunkHeader::length() {
			return ulength.unsignedValue();
		}

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

		void EaIff::WriteChunkHeader(const IffChunkHeader& header) {
			if (GetPos()%2 > 0) Write('\0');
			WriteRaw(&header,sizeof(header));
			currentHeader=header;
		}

		const IffChunkHeader& EaIff::readHeader() {
			if(GetPos()%2 > 0) Skip(1);
			ReadRaw(&currentHeader,sizeof(currentHeader));
			return currentHeader;
		}
		const IffChunkHeader& EaIff::findChunk(IffChunkId id, bool allowWrap) {

			return currentHeader;
		}
		void EaIff::skipThisChunk() {
			Seek(headerPosition + 8 + currentHeader.length());
			if (GetPos() % 2 > 0) Skip(1);
		}

	}
}
