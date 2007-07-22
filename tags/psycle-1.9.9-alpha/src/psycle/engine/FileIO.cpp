///\implementation psycle::host::RiffFile
#include <packageneric/pre-compiled.private.hpp>
#include <packageneric/module.private.hpp>
#include "FileIO.hpp"
#include <stdexcept>

namespace psycle
{
	namespace host
	{
		std::uint32_t RiffFile::FourCC(char const * null_terminated_string)
		{
			std::uint32_t result(0x20202020); // four spaces (padding)
			char * chars(reinterpret_cast<char *>(&result));
			for(int i(0) ; i < 4 && null_terminated_string[i] ; ++i) *chars++ = null_terminated_string[i];
			return result;
		}

		bool RiffFile::ReadString(std::string & result)
		{
			result="";
			for(;;)
			{
				char c;
				if(!ReadChunk(&c, sizeof c)) return false;
				if(!c) return true;
				result += c;
			}
		}

		bool RiffFile::ReadString(char * data, std::size_t const & max_length)
		{
			if(max_length > 0) {
				memset(data,0,max_length);
				char c=EOF;
				for(long index = 0; index < max_length; index++)
				{
					if ( (c=_stream.get())!=EOF)
					{
						data[index] = c;
						if(c == '\0') return true;
					}
				}
//\todo : put this back! it was for the case where the string was longer than max_length
//				{
//					char c;
//					do
//					{
//						if(!ReadChunk(&c, sizeof c)) return false; //\todo : return false, or return true? the string is read already. it could be EOF.
//					} while(c);
//				}
				if (c==EOF) return true; else false;
			}
			return false;
		}

		bool RiffFile::Open(std::string const & filename)
		{
			write_mode = false;
			file_name_ = filename;
			_stream.open(file_name_.c_str(), std::ios_base::in | std::ios_base::binary);
			if (!_stream.is_open ()) return false;
			_stream.seekg (0, std::ios::beg);
			return 1;
		}

		bool RiffFile::Create(std::string const & filename, bool const & overwrite)
		{
			write_mode = true;
			file_name_ = filename;
			if(!overwrite)
			{
				std::fstream filetest(file_name_.c_str (), std::ios_base::in | std::ios_base::binary);
				if (filetest.is_open ())
				{
					filetest.close();
					return false;
				}
			}
			_stream.open(file_name_.c_str (), std::ios_base::out | std::ios_base::trunc |std::ios_base::binary);
			return _stream.is_open ();
		}

		bool RiffFile::Close()
		{
			_stream.close();
			return true;
		}

		bool RiffFile::Error()
		{
			return !_stream.bad();
		}

		bool RiffFile::ReadChunk(void * data, std::size_t const & bytes)
		{
			if (_stream.eof()) return false;
			_stream.read(reinterpret_cast<char*>(data) ,bytes);
			if (_stream.eof()) return false;
			if (_stream.bad()) return false;
			return 1;
		}

		bool RiffFile::WriteChunk(void const * data, std::size_t const & bytes)
		{
			_stream.write(reinterpret_cast<const char*>(data), bytes);
			if (_stream.bad()) return 0;
			return 1;
		}

		bool RiffFile::Expect(void * data, std::size_t const & bytes)
		{
			char * chars(reinterpret_cast<char*>(data));
			std::size_t count(bytes);
			while(count--)
			{
				unsigned char c;
				_stream.read (reinterpret_cast<char *> (&c), sizeof (c));
				if (_stream.eof()) return false;
				if(c != *chars) return false;
				++chars;
			}
			return true;
		}

		int RiffFile::Seek(int const & bytes)
		{
			if (write_mode) _stream.seekp(bytes, std::ios::beg); else _stream.seekg(bytes, std::ios::beg);
			if (_stream.eof()) throw std::runtime_error("seek failed");
			return GetPos();
		}

		int RiffFile::Skip(int const & bytes)
		{
			if (write_mode) _stream.seekp(bytes, std::ios::cur); else _stream.seekg(bytes, std::ios::cur);
			if (_stream.eof()) throw std::runtime_error("seek failed");
			return GetPos();
		}

		bool RiffFile::Eof()
		{
			return _stream.eof();
		}
		
		std::size_t RiffFile::FileSize()
		{
			std::size_t curPos = GetPos();
			_stream.seekg(0, std::ios::end);         // goto end of file
			std::size_t fileSize = _stream.tellg();  // read the filesize
			_stream.seekg(curPos);                   // go back to begin of file
			return fileSize;
		}

		std::size_t RiffFile::GetPos()
		{
			return  (write_mode) ? _stream.tellp() : _stream.tellg();
		}
		
	}
}
