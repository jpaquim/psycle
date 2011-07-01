/**
	\file
	implementation file for psycle::helpers::eaiff
*/
#include "msriff.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace psycle { namespace helpers {

		//const RiffChunkId MsRiff::grabbag = {' ',' ',' ',' '};

		uint32_t RiffChunkHeader::length() const {
			return ulength.unsignedValue();
		}

		RiffChunkHeader::RiffChunkHeader(){}
		RiffChunkHeader::~RiffChunkHeader(){}

		bool MsRiff::isValidFile() const { return false;}

		void MsRiff::Open(std::string fname) { AbstractIff::Open(fname); }
		void MsRiff::Create(std::string fname, bool const & overwrite) { AbstractIff::Create(fname, overwrite); }
		void MsRiff::close() { AbstractIff::close(); }
		bool MsRiff::Eof() { return AbstractIff::Eof(); }
		void MsRiff::addNewChunk(const RiffChunkHeader& header) {
			WriteChunkHeader(header);
		}
/*
		void MsRiff::addFormChunk(IffChunkId id) {
		}
		void MsRiff::addListChunk(IffChunkId id, bool endprevious) {
		}
		void MsRiff::addCatChunk(IffChunkId id, bool endprevious) {
		}
		void MsRiff::addListProperty(IffChunkId contentId, IffChunkId propId) {
		}
		void MsRiff::addListProperty(IffChunkId contentId, IffChunkId propId, void const *data, std::uint32_t dataSize) {
		}
*/
		void MsRiff::WriteChunkHeader(const RiffChunkHeader& header) {
			if (GetPos()%2 > 0) Write('\0');
			Write(header.id);
			Write(header.length());
			currentHeader=header;
		}

		const RiffChunkHeader& MsRiff::readHeader() {
			if(GetPos()%2 > 0) Skip(1);
			ReadRaw(&currentHeader,sizeof(currentHeader));
			return currentHeader;
		}
		const RiffChunkHeader& MsRiff::findChunk(IffChunkId id, bool allowWrap) {

			return currentHeader;
		}
		void MsRiff::skipThisChunk() {
			Seek(headerPosition + 8 + currentHeader.length());
			if (GetPos() % 2 > 0) Skip(1);
		}
}}
