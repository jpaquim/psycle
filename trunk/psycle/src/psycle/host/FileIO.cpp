///\implementation psycle::host::RiffFile
#include <packageneric/pre-compiled.private.hpp>
#include "FileIO.hpp"

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
				if(!Read(&c, sizeof c)) return false;
				if(!c) return true;
				result += c;
			}
		}

		bool RiffFile::ReadString(char * data, std::size_t const & max_length)
		{
			if(max_length > 0)
			{
				std::memset(data, 0, max_length);
				for(std::size_t index(0) ; index < max_length ; ++index)
				{
					char c;
					if(!Read(&c, sizeof c)) return false;
					{
						data[index] = c;
						if(!c) return true;
					}
				}
				{
					char c;
					do
					{
						if(!Read(&c, sizeof c)) return false; //\todo : return false, or return true? the string is read already. it could be EOF.
					} while(c);
				}
				return true;
			}
			return false;
		}

		bool RiffFile::Open(std::string const & filename)
		{
			file_name_ = filename;
			return file_ = std::fopen(filename.c_str(), "rb");
		}

		bool RiffFile::Create(std::string const & filename, bool const & overwrite)
		{
			file_name_ = filename;
			file_ = std::fopen(filename.c_str(), "rb");
			if(file_)
			{
				std::fclose(file_);
				if(!overwrite) return false;
			}
			return file_ = std::fopen(filename.c_str(), "wb");
		}

		bool RiffFile::Close()
		{
			if(file_)
			{
				std::fflush(file_);
				bool const result(Error());
				std::fclose(file_);
				file_ = 0;
				return result;
			}
			return true;
		}

		bool RiffFile::Error()
		{
			return !/*std::*/ferror(file_);
		}

		bool RiffFile::Read(void * data, std::size_t const & bytes)
		{
			return std::fread(data, sizeof(char), bytes, file_) == bytes;
		}

		bool RiffFile::Write(void const * data, std::size_t const & bytes)
		{
			///\todo why flushing?
			std::fflush(file_);
			return std::fwrite(data, sizeof(char), bytes, file_) == bytes;
		}

		bool RiffFile::Expect(void * data, std::size_t const & bytes)
		{
			char * chars(reinterpret_cast<char*>(data));
			std::size_t count(bytes);
			while(count--)
			{
				unsigned char c;
				if(std::fread(&c, sizeof c, 1, file_) != 1) return false;
				if(c != *chars) return false;
				++chars;
			}
			return true;
		}

		std::ptrdiff_t RiffFile::Seek(std::ptrdiff_t const & bytes)
		{
			if(std::fseek(file_, bytes, SEEK_SET)) return -1;
			return std::ftell(file_);
		}

		std::ptrdiff_t RiffFile::Skip(std::ptrdiff_t const & bytes)
		{
			if(std::fseek(file_, bytes, SEEK_CUR)) return -1;
			return std::ftell(file_);
		}

		bool RiffFile::Eof()
		{
			return static_cast<bool>(/*std::*/feof(file_));
		}

		std::size_t RiffFile::FileSize()
		{
			std::size_t const save(std::ftell(file_));
			std::fseek(file_, 0, SEEK_END);
			std::size_t const end(std::ftell(file_));
			std::fseek(file_, save, SEEK_SET);
			return end;
		}

		std::size_t RiffFile::GetPos()
		{
			return std::ftell(file_);
		}
	}
}
