///\implementation psy::core::RiffFile
#include <psycle/core/psycleCorePch.hpp>

#include "fileio.h"

namespace psy
{
	namespace core
	{

		RiffFile::RiffFile( )
		{
		}

		RiffFile::~ RiffFile( )
		{
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
				int c=EOF;
				for(long index = 0; index < max_length; index++)
				{
					if ( (c=_stream.get())!=EOF)
					{
						data[index] = c;
						if(c == '\0') return true;
					}
				}
				#if 0 ///\todo : put this back! it was for the case where the string was longer than max_length
				{
					char c;
					do {
						if(!Read(c)) return false; //\todo : return false, or return true? the string is read already. it could be EOF.
					} while(c);
				}
				#endif
				if (c==EOF) return true; else return false;
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

		int RiffFile::Seek(std::ptrdiff_t const & bytes)
		{
			if (write_mode) _stream.seekp(bytes, std::ios::beg); else _stream.seekg(bytes, std::ios::beg);
			if (_stream.eof()) throw std::runtime_error("seek failed");
			return GetPos();
		}

		int RiffFile::Skip(std::ptrdiff_t const & bytes)
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

