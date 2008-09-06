// -*- mode:c++; indent-tabs-mode:t -*-

/***************************************************************************************
 Copyright 2007-2008 members of the psycle project http://psycle.sourceforge.net

 This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.
 This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
***************************************************************************************/

///\implementation psy::core::RiffFile

#include "fileio.h"
#include <stdexcept>
#include <cstring>

namespace psy { namespace core {

RiffFile::RiffFile() {}

RiffFile::~RiffFile() {
	///\todo close the stream if it's opened!!!
}

bool RiffFile::ReadString(std::string & result) {
	result = "";
	for(;;) {
		char c;
		if(!ReadChunk(&c, sizeof c)) return false;
		if(!c) return true;
		result += c;
	}
}

bool RiffFile::ReadString(char * data, std::size_t const & max_length) {
	if(max_length <= 0) return false;
	std::memset(data, 0, max_length);
	int c = EOF;
	for(long index = 0; index < max_length; ++index) if((c = _stream.get()) != EOF) {
		data[index] = c;
		if(c == '\0') return true;
	}
	#if 0 ///\todo : put this back! it was for the case where the string was longer than max_length
	{
		char c;
		do {
			if(!Read(c)) return false; //\todo : return false, or return true? the string is read already. it could be EOF.
		} while(c);
	}
	#endif
	return c == EOF;
}

bool RiffFile::Open(std::string const & filename) {
	write_mode = false;
	file_name_ = filename;
	_stream.open(file_name_.c_str(), std::ios_base::in | std::ios_base::binary);
	if(!_stream.is_open ()) return false;
	_stream.seekg (0, std::ios::beg);
	return 1;
}

bool RiffFile::Create(std::string const & filename, bool const & overwrite) {
	write_mode = true;
	file_name_ = filename;
	if(!overwrite) {
		std::fstream filetest(file_name_.c_str (), std::ios_base::in | std::ios_base::binary);
		if(filetest.is_open()) {
			filetest.close();
			return false;
		}
	}
	_stream.open(file_name_.c_str (), std::ios_base::out | std::ios_base::trunc |std::ios_base::binary);
	return _stream.is_open ();
}

bool RiffFile::Close() {
	_stream.close();
	return true;
}

bool RiffFile::Error() {
	return !_stream.bad();
}

bool RiffFile::ReadChunk(void * data, std::size_t const & bytes) {
	if(_stream.eof()) return false;
	_stream.read(reinterpret_cast<char*>(data), bytes);
	if(_stream.eof()) return false;
	if(_stream.bad()) return false;
	return 1;
}

bool RiffFile::WriteChunk(void const * data, std::size_t const & bytes) {
	_stream.write(reinterpret_cast<char const *>(data), bytes);
	if(_stream.bad()) return 0;
	return 1;
}

bool RiffFile::Expect(void * data, std::size_t const & bytes) {
	char * chars(reinterpret_cast<char*>(data));
	std::size_t count(bytes);
	while(count--) {
		unsigned char c;
		_stream.read(reinterpret_cast<char*>(&c), sizeof c);
		if(_stream.eof()) return false;
		if(c != *chars) return false;
		++chars;
	}
	return true;
}

int RiffFile::Seek(std::ptrdiff_t const & bytes) {
	if(write_mode) _stream.seekp(bytes, std::ios::beg); else _stream.seekg(bytes, std::ios::beg);
	if(_stream.eof()) throw std::runtime_error("seek failed");
	return GetPos();
}

int RiffFile::Skip(std::ptrdiff_t const & bytes) {
	if(write_mode) _stream.seekp(bytes, std::ios::cur); else _stream.seekg(bytes, std::ios::cur);
	if(_stream.eof()) throw std::runtime_error("seek failed");
	return GetPos();
}

bool RiffFile::Eof() {
	return _stream.eof();
}

std::size_t RiffFile::FileSize() {
	std::size_t curPos = GetPos();
	// goto end of file
	_stream.seekg(0, std::ios::end);
	// read the filesize
	std::size_t fileSize = _stream.tellg();
	// go back to begin of file
	_stream.seekg(curPos);
	return fileSize;
}

std::size_t RiffFile::GetPos() {
	return  write_mode ? _stream.tellp() : _stream.tellg();
}




/*********************************************************************************/
// MemoryFile

MemoryFile::MemoryFile() {}
MemoryFile::~MemoryFile(){}

bool MemoryFile::OpenMem(std::ptrdiff_t blocksize) { return false; }
bool MemoryFile::CloseMem() { return false; }
std::size_t MemoryFile::FileSize() { return 0; }
std::size_t MemoryFile::GetPos() { return 0; }
int MemoryFile::Seek(std::ptrdiff_t const & bytes) { return 0; }
int MemoryFile::Skip(std::ptrdiff_t const & bytes) { return 0; }
bool MemoryFile::ReadString(char *, std::size_t const & max_length) { return false; }
bool MemoryFile::WriteChunk(void const *, std::size_t const &){ return false; }
bool MemoryFile::ReadChunk (void       *, std::size_t const &){ return false; }
bool MemoryFile::Expect    (void       *, std::size_t const &){ return false; }


}}
