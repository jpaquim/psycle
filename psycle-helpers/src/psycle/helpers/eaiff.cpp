/**
	\file
	implementation file for psycle::helpers::eaiff
*/
#include "eaiff.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace psycle { namespace helpers {

		const IffChunkId EaIff::grabbag = {' ',' ',' ',' '};

		IffChunkHeader::IffChunkHeader(){}
		IffChunkHeader::~IffChunkHeader(){}
		void IffChunkHeader::setLength(uint32_t newlength) {
			ulength.changeValue(newlength);
		}
		uint32_t IffChunkHeader::length() const {
			return ulength.unsignedValue();
		}

///////////////////////////////////////////////////////
		bool EaIff::isValidFile() const { return false;}

		void EaIff::Open(const std::string& fname) { AbstractIff::Open(fname); }
		void EaIff::Create(const std::string& fname, const bool overwrite) { AbstractIff::Create(fname, overwrite); }
		void EaIff::close() { 
			if (isWriteMode()) {
				std::streamsize size = fileSize();
				UpdateFormSize(0,size-sizeof(currentHeader));
			}
			AbstractIff::close(); 
		}
		bool EaIff::Eof() const { return AbstractIff::Eof(); }

		void EaIff::addNewChunk(const BaseChunkHeader& header)
		{
			if (GetPos()%2 > 0) Write('\0');
			headerPosition=static_cast<uint32_t>(GetPos());
			Write(header.id);
			Write(header.length());
			BaseChunkHeader::copyId(header.id, currentHeader.id);
			currentHeader.setLength(header.length());
		}
        void EaIff::addFormChunk(const IffChunkId& /*id*/) {
		}
        void EaIff::addListChunk(const IffChunkId& /*id*/, bool /*endprevious*/) {
		}
        void EaIff::addCatChunk(const IffChunkId& /*id*/, bool /*endprevious*/) {
		}
        void EaIff::addListProperty(const IffChunkId& /*contentId*/, const IffChunkId& /*propId*/) {
		}
        void EaIff::addListProperty(const IffChunkId& /*contentId*/, const IffChunkId& /*propId*/, void const * /*data*/, uint32_t /*dataSize*/) {
		}

		const IffChunkHeader& EaIff::readHeader() {
			if(GetPos()%2 > 0) Skip(1);
			headerPosition = static_cast<uint32_t>(GetPos());
			Read(currentHeader.id);
			Read(currentHeader.ulength);
			return currentHeader;
		}
		const IffChunkHeader& EaIff::findChunk(const IffChunkId& id, bool allowWrap) {
			do {
				const IffChunkHeader& header = readHeader();
				if (header.matches(id)) return header;
				Skip(header.length());
				if (header.length()%2)Skip(1);
			}while(!Eof());
			if (allowWrap && Eof()) {
				Seek(8);//Skip RIFF/RIFX chunk
				return findChunk(id,false);
			}
			throw std::runtime_error(std::string("couldn't find chunk ") + id);
		}
		void EaIff::skipThisChunk() {
			Seek(headerPosition + static_cast<std::streamoff>(sizeof(currentHeader) + currentHeader.length()));
			if (GetPos() % 2 > 0) Skip(1);
		}

		void EaIff::WriteHeader(const IffChunkHeader& header) {
			if (GetPos()%2 > 0) Write('\0');
			headerPosition=static_cast<uint32_t>(GetPos());
			Write(header.id);
			Write(header.ulength);
			currentHeader=header;
		}

		void EaIff::UpdateCurrentChunkSize()
		{
			std::streampos chunksize = GetPos();
			chunksize-=headerPosition+static_cast<std::streamoff>(sizeof(currentHeader));
			UpdateFormSize(headerPosition, chunksize);
		}

}}
