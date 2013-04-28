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
	const IffChunkId MsRiff::RIFF = {'R','I','F','F'};
	const IffChunkId MsRiff::RIFX = {'R','I','F','X'};
//////////////////////////////////////////////////////////////////////////
	template<typename long_type>
	void RiffChunkHeader<long_type>::setLength(uint32_t newlength) {
		ulength.changeValue(newlength);
	}
	template<typename long_type>
	uint32_t RiffChunkHeader<long_type>::length() const {
		return ulength.unsignedValue();
	}

//////////////////////////////////////////////////////////////////////////
		bool MsRiff::isValidFile() const { return isValid;}

		void MsRiff::Open(const std::string& fname) {
			isValid=false;
			AbstractIff::Open(fname);
			readHeader();
			if (currentHeader.matches(RIFF)) {
				isLittleEndian=true;
				isValid=true;
			}
			else if (currentHeader.matches(RIFX)) {
				isLittleEndian=false;
				isValid=true;
			}
		}
		void MsRiff::Create(const std::string& fname, bool const  overwrite, bool const littleEndian) {
			AbstractIff::Create(fname, overwrite);
			isLittleEndian=littleEndian;
			isValid=true;
			if(littleEndian) {
				RiffChunkHeader<LongLE> mainhead(RIFF,0);
				addNewChunk(mainhead);
			}
			else {
				RiffChunkHeader<LongBE> mainhead(RIFX,0);
				addNewChunk(mainhead);
			}
		}
		void MsRiff::close() { 
			if (isWriteMode()) {
				std::streamsize size = fileSize();
				UpdateFormSize(0,size-sizeof(currentHeader));
			}
			AbstractIff::close(); 
		}
		bool MsRiff::Eof() const { return AbstractIff::Eof(); }

		void MsRiff::addNewChunk(const BaseChunkHeader& header) {
			if (GetPos()%2 > 0) Write('\0');
			headerPosition=GetPos();
			Write(header.id);
			Write(header.length());
			BaseChunkHeader::copyId(header.id, currentHeader.id);
			currentHeader.setLength(header.length());
		}
/*
		void MsRiff::addFormChunk(IffChunkId id) {
		}
*/

#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
		const RiffChunkHeader<LongLE>& MsRiff::readHeader() {
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
		const RiffChunkHeader<LongBE>& MsRiff::readHeader() {
#endif
			if(GetPos()%2 > 0) Skip(1);
			headerPosition = static_cast<uint32_t>(GetPos());
			Read(currentHeader.id);
			uint32_t length;
			Read(length);
			currentHeader.setLength(length);
			return currentHeader;
		}

#if defined DIVERSALIS__CPU__ENDIAN__LITTLE
		const RiffChunkHeader<LongLE>& MsRiff::findChunk(const IffChunkId& id, bool allowWrap) {
#elif defined DIVERSALIS__CPU__ENDIAN__BIG
		const RiffChunkHeader<LongBE>& MsRiff::findChunk(const IffChunkId& id, bool allowWrap) {
#endif
			do {
				readHeader();
				if (currentHeader.matches(id)) return currentHeader;
				Skip(currentHeader.length());
				if (currentHeader.length()%2)Skip(1);
			}while(!Eof());
			if (allowWrap && Eof()) {
				Seek(8);//Skip RIFF/RIFX chunk
				return findChunk(id,false);
			}
			throw std::runtime_error(std::string("couldn't find chunk ") + id);
		}
		void MsRiff::skipThisChunk() {
			Seek(headerPosition + static_cast<std::streamoff>(sizeof(currentHeader) + currentHeader.length()));
			if (GetPos() % 2 > 0) Skip(1);
		}
		void MsRiff::UpdateCurrentChunkSize()
		{
			std::streampos chunksize = GetPos();
			chunksize-=headerPosition+static_cast<std::streamoff>(sizeof(currentHeader));
			UpdateFormSize(headerPosition, chunksize);
		}
}}
