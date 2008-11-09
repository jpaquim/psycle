/**
	\file
	implementation file for psycle::host::abstractIff
*/
#include <psycle/project.private.hpp>
#include "abstractiff.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
namespace psycle
{
	namespace host
	{
		ULongBE::ULongBE() {
			d.originalValue = 0;
		}
		ULongBE::ULongBE(std::uint32_t val) {
			d.byte.hihi = (val >> 24)&0xFF;
			d.byte.hilo = (val >> 16)&0xFF;
			d.byte.lohi = (val >> 8)&0xFF;
			d.byte.lolo = val&0xFF;
		}

		unsigned long ULongBE::unsignedValue() {
			return d.byte.hihi << 24 + d.byte.hilo << 16 + d.byte.lohi << 8 + d.byte.lolo;
		}
		signed long ULongBE::signedValue() {
			return static_cast<signed long>(unsignedValue());
		}

		UShortBE::UShortBE() {
			d.originalValue = 0;
		}
		UShortBE::UShortBE(std::uint16_t val) {
			d.byte.hi = (val >> 8)&0xFF;
			d.byte.lo = val&0xFF;
		}
		unsigned short UShortBE::unsignedValue() {
			return d.byte.hi << 8 + d.byte.lo;
		}
		signed short UShortBE::signedValue() {
			return static_cast<signed short>(unsignedValue());
		}
		LongBE::LongBE() {
			d.originalValue = 0;
		}
		LongBE::LongBE(std::int32_t val):ULongBE(static_cast<std::uint32_t>(val)){
		}
		ShortBE::ShortBE() {
			d.originalValue = 0;
		}
		ShortBE::ShortBE(std::int16_t val):UShortBE(static_cast<std::uint16_t>(val)){
		}

		float FixedPointBE::value() { 
			return (float)(integer.byte.hi << 8 + integer.byte.lo) + (decimal.byte.hi << 8 + decimal.byte.lo)*0.0000152587890625f;
		}

		ULongLE::ULongLE() {
			d.originalValue = 0;
		}
		ULongLE::ULongLE(std::uint32_t val) {
			d.byte.hihi = (val >> 24)&0xFF;
			d.byte.hilo = (val >> 16)&0xFF;
			d.byte.lohi = (val >> 8)&0xFF;
			d.byte.lolo = val&0xFF;
		}
		unsigned long ULongLE::unsignedValue() {
			return d.byte.hihi << 24 + d.byte.hilo << 16 + d.byte.lohi << 8 + d.byte.lolo;
		}
		signed long ULongLE::signedValue() {
			return static_cast<signed long>(unsignedValue());
		}

		UShortLE::UShortLE() {
			d.originalValue = 0;
		}
		UShortLE::UShortLE(std::uint16_t val) {
			d.byte.hi = (val >> 8)&0xFF;
			d.byte.lo = val&0xFF;
		}
		unsigned short UShortLE::unsignedValue() {
			return d.byte.hi << 8 + d.byte.lo;
		}
		signed short UShortLE::signedValue() {
			return static_cast<signed short>(unsignedValue());
		}
		LongLE::LongLE() {
			d.originalValue = 0;
		}
		LongLE::LongLE(std::int32_t val):ULongLE(static_cast<std::uint32_t>(val)){
		}
		ShortLE::ShortLE() {
			d.originalValue = 0;
		}
		ShortLE::ShortLE(std::int16_t val):UShortLE(static_cast<std::uint16_t>(val)){
		}


		BaseChunkHeader::~BaseChunkHeader() {
		}

		std::string BaseChunkHeader::idString() {
			const char id2[5] = {id[0],id[1],id[2],id[3],'\0'}; return id2;
		}

		boolean BaseChunkHeader::matches(IffChunkId id2) {
			return *(std::uint32_t const*)id == *(std::uint32_t const*)id2;
		}

		AbstractIff::AbstractIff() {
		}

		AbstractIff::~AbstractIff() {
		}

		void AbstractIff::Open(std::string fname) {
			write_mode = false;
			file_name_ = fname;
			_stream.open(file_name_.c_str(), std::ios_base::in | std::ios_base::binary);
			if (!_stream.is_open ()) throw std::runtime_error("stream open failed");
			_stream.seekg (0, std::ios::beg);
		}

		void AbstractIff::Create(std::string fname, bool const & overwrite) {
			write_mode = true;
			file_name_ = fname;
			if(!overwrite)
			{
				std::fstream filetest(file_name_.c_str (), std::ios_base::in | std::ios_base::binary);
				if (filetest.is_open ())
				{
					filetest.close();
					throw std::runtime_error("stream already exists");
				}
			}
			_stream.open(file_name_.c_str (), std::ios_base::out | std::ios_base::trunc |std::ios_base::binary);
		}

		void AbstractIff::close() {
			_stream.close();
		}

		bool AbstractIff::Eof() {
			//_stream.eof ???
			return !_stream.bad();
		}

		std::string const inline & AbstractIff::file_name() const throw() {
			return file_name_;
		}

		std::size_t AbstractIff::fileSize() {
			std::size_t curPos = GetPos();
			_stream.seekg(0, std::ios::end);         // goto end of file
			std::size_t fileSize = _stream.tellg();  // read the filesize
			_stream.seekg(curPos);                   // go back to begin of file
			return fileSize;
		}

		void AbstractIff::ReadString(std::string &) {
			std::string result;
			char c = '\0';

			Read(c);
			while(c != '\0') {
				result += c;
				Read(c);
			}
			result += c;
		}
		void AbstractIff::ReadString(char *data, std::size_t const & max_length) {
			if (max_length <= 0 ) {
				throw std::runtime_error("erroneous max_length passed to ReadString");
			}

			memset(data,0,max_length);
			std::string temp;
			ReadString(temp);
			strncpy(data,temp.c_str(), std::min(temp.length(),max_length)-1);
		}

		void AbstractIff::ReadSizedString(char *data, std::size_t const & max_length) {
			memset(data,0,max_length);
			ReadRaw(data,max_length);
			data[max_length-1]='\0';
		}

		template<typename T>
		void AbstractIff::ReadArray(T* array, int n) {
			for(int i=0;i<n;i++) Read(array[i]);
		}
		void AbstractIff::Read(std::uint8_t & x) {
			ReadRaw(&x,1);
		}
		void AbstractIff::Read(std::int8_t & x) {
			ReadRaw(&x,1);
		}
		void AbstractIff::Read(char & x) {
			ReadRaw(&x,1);
		}

		void AbstractIff::Read(bool & x) {
			std::uint8_t c;
			Read(c);
			x = c;
		}
		void AbstractIff::ReadBE(std::uint32_t & x) {
			ULongBE x1;
			ReadRaw(&x1.d.byte,4);
			x = x1.unsignedValue();
		}
		void AbstractIff::ReadBE(std::int32_t & x) {
			LongBE x1;
			ReadRaw(&x1.d.byte,4);
			x = x1.signedValue();
		}
		void AbstractIff::ReadBE(std::uint16_t & x) {
			UShortBE x1;
			ReadRaw(&x1.d.byte,2);
			x = x1.unsignedValue();
		}
		void AbstractIff::ReadBE(std::int16_t & x) {
			ShortBE x1;
			ReadRaw(&x1.d.byte,2);
			x = x1.signedValue();
		}
		void AbstractIff::ReadLE(std::uint32_t & x) {
			ULongLE x1;
			ReadRaw(&x1.d.byte,4);
			x = x1.unsignedValue();
		}
		void AbstractIff::ReadLE(std::int32_t & x) {
			LongLE x1;
			ReadRaw(&x1.d.byte,4);
			x = x1.signedValue();
		}
		void AbstractIff::ReadLE(std::uint16_t & x) {
			UShortLE x1;
			ReadRaw(&x1.d.byte,2);
			x = x1.unsignedValue();
		}
		void AbstractIff::ReadLE(std::int16_t & x) {
			ShortLE x1;
			ReadRaw(&x1.d.byte,2);
			x = x1.signedValue();
		}

		void AbstractIff::WriteString(std::string &string) {
			WriteRaw(string.c_str(),string.length());
		}

		void AbstractIff::WriteString(const char * const data) {
			unsigned long idx = 0;

			while(data[idx] != '\0')
			{
				idx++;
			}
			WriteRaw(data,idx);
		}

		void AbstractIff::WriteSizedString(const char * const data, std::size_t const & length) {
			if (length <= 0 ) {
				throw std::runtime_error("erroneous max_length passed to WriteString");
			}
			unsigned long idx = 0;

			while(data[idx] != '\0' && idx < length)
			{
				idx++;
			}
			WriteRaw(data,idx);

			char c = '\0';
			while(idx < length) {
				Write(c);
			}
		}

		template<typename T>
		void AbstractIff::WriteArray(T const* thearray, int n) {
			for(int i=0;i<n;i++) Write(thearray[i])
		}

		void AbstractIff::Write(const std::uint8_t & x) {
			WriteRaw(&x,1);
		}

		void AbstractIff::Write(const std::int8_t & x) {
			WriteRaw(&x,1);
		}

		void AbstractIff::Write(const char & x) {
			WriteRaw(&x,1);
		} 

		void AbstractIff::Write(const bool & x) {
			std::uint8_t c = x; Write(c);
		}
		void AbstractIff::WriteBE(const std::uint32_t & x) {
			ULongBE x2(x);
			Write(x2.unsignedValue());			
		}
		void AbstractIff::WriteBE(const std::int32_t & x) {
			LongBE x2(x);
			Write(x2.signedValue());			
		}
		void AbstractIff::WriteBE(const std::uint16_t & x) {
			UShortBE x2(x);
			Write(x2.unsignedValue());			
		}
		void AbstractIff::WriteBE(const std::int16_t & x) {
			ShortBE x2(x);
			Write(x2.signedValue());			
		}
		void AbstractIff::WriteLE(const std::uint32_t & x) {
			ULongLE x2(x);
			Write(x2.unsignedValue());			
		}
		void AbstractIff::WriteLE(const std::int32_t & x) {
			LongLE x2(x);
			Write(x2.signedValue());			
		}
		void AbstractIff::WriteLE(const std::uint16_t & x) {
			UShortLE x2(x);
			Write(x2.unsignedValue());			
		}
		void AbstractIff::WriteLE(const std::int16_t & x) {
			ShortLE x2(x);
			Write(x2.signedValue());			
		}

		std::size_t AbstractIff::GetPos(void) {
			return  (write_mode) ? _stream.tellp() : _stream.tellg();
		}
		std::size_t AbstractIff::Seek(std::ptrdiff_t const & bytes){
			if (write_mode) {
				_stream.seekp(bytes, std::ios::beg);
			} else {
				_stream.seekg(bytes, std::ios::beg);
			}
			if (_stream.eof()) throw std::runtime_error("seek failed");
			return GetPos();
		}

		std::size_t AbstractIff::Skip(std::ptrdiff_t const & bytes) {
			if (write_mode) {
				_stream.seekp(bytes, std::ios::cur);
			} else { 
				_stream.seekg(bytes, std::ios::cur);
			}
			if (_stream.eof()) throw std::runtime_error("skip failed");
			return GetPos();
		}

		void AbstractIff::ReadRaw (void * data, std::size_t const & bytes)
		{
			if (_stream.eof()) throw std::runtime_error("Read failed");
			_stream.read(reinterpret_cast<char*>(data) ,bytes);
			if (_stream.eof() || _stream.bad()) throw std::runtime_error("Read failed");
		}

		void AbstractIff::WriteRaw(const void * const data, std::size_t const & bytes)
		{
			_stream.write(reinterpret_cast<const char*>(data), bytes);
			if (_stream.bad()) throw std::runtime_error("write failed");
		}

		bool AbstractIff::Expect(IffChunkId& id) 
		{
			return Expect(id,4);
		}

		bool AbstractIff::Expect(void * data, std::size_t const & bytes)
		{
			char * chars(reinterpret_cast<char*>(data));
			std::size_t count(bytes);
			while(count--)
			{
				unsigned char c;
				Read(c);
				if(c != *chars) return false;
				++chars;
			}
			return true;
		}

		void AbstractIff::Backpatch(long fileOffset, const void *data, unsigned numBytes) {
			std::size_t currentPos = GetPos();
			Seek(fileOffset);
			WriteRaw(data,numBytes);
			Seek(currentPos);
		}
	}
}
