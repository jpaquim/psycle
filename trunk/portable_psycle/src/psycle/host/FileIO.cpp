///\file
///\brief implementation file for psycle::host::RiffFile.
#include <project.private.hpp>
#include "FileIO.hpp"

#include <diversalis/operating_system.hpp>
#if defined DIVERSALIS__OPERATING_SYSTEM__MICROSOFT
	#include <diversalis/compiler.hpp>
	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma warning(push)
	#endif
	#include <tchar.h> // because of microsoftisms: _tcscpy
	#include <atlbase.h> // because of microsoftisms: CA2T
	#if defined DIVERSALIS__COMPILER__MICROSOFT
		#pragma warning(pop)
	#endif
#else
	#error "this sorry file is not portable"
#endif

namespace psycle
{
	namespace host
	{
		unsigned __int32 RiffFile::FourCC(char const * null_terminated_string)
		{
			unsigned __int32 result(0x20202020); // four spaces (padding)
			char * chars(reinterpret_cast<char *>(&result));
			for(int i(0) ; i < 4 && null_terminated_string[i] ; ++i) *chars++ = null_terminated_string[i];
			return result;
		}

		bool RiffFile::Open(std::string const & filename)
		{
			szName = filename;
			DWORD bytesRead;
			_modified = false;
			_handle = ::CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			if(_handle == INVALID_HANDLE_VALUE) return false;
			if(!ReadFile(_handle, &_header, sizeof(_header), &bytesRead, 0))
			{
				CloseHandle(_handle);
				return false;
			}
			return true;
		}

		bool RiffFile::Create(std::string const & filename, bool const & overwrite)
		{
			szName = filename;
			DWORD bytesWritten;
			_modified = false;
			_handle = ::CreateFile(filename.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, overwrite ? CREATE_ALWAYS : CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
			if(_handle == INVALID_HANDLE_VALUE) return false;
			_header._id = FourCC("RIFF");
			_header._size = 0;
			if(!WriteFile(_handle, &_header, sizeof(_header), &bytesWritten, 0))
			{
				CloseHandle(_handle);
				return false;
			}
			return true;
		}

		bool RiffFile::Close()
		{
			DWORD bytesWritten;
			if (_modified)
			{
				Seek(0);
				WriteFile(_handle, &_header, sizeof(_header), &bytesWritten, 0);
				_modified = false;
			}
			BOOL b = CloseHandle(_handle);
			_handle = INVALID_HANDLE_VALUE;
			return b;
		}

		bool RiffFile::Read(void * data, std::size_t const & bytes)
		{
			DWORD bytes_read;
			if(!ReadFile(_handle, data, bytes, &bytes_read, 0)) return false;
			return bytes_read == bytes;
		}

		bool RiffFile::Write(void const * data, std::size_t const & bytes)
		{
			DWORD bytes_written;
			if(!WriteFile(_handle, data, bytes, &bytes_written, 0)) return false;
			_modified = true;
			_header._size += bytes_written;
			return bytes_written == bytes;
		}

		bool RiffFile::Expect(void * data, std::size_t const & bytes)
		{
			char * chars(reinterpret_cast<char*>(data));
			std::size_t count(bytes);
			while(count--)
			{
				unsigned char c;
				DWORD bytes_read;
				if(!ReadFile(_handle, &c, sizeof(c), &bytes_read, 0)) return false;
				if(c != *chars) return false;
				++chars;
			}
			return true;
		}

		std::ptrdiff_t RiffFile::Seek(std::ptrdiff_t const & bytes)
		{
			return SetFilePointer(_handle, bytes, 0, FILE_BEGIN);
		}

		std::ptrdiff_t RiffFile::Skip(std::ptrdiff_t const & bytes)
		{
			return SetFilePointer(_handle, bytes, 0, FILE_CURRENT); 
		}

		bool RiffFile::Eof()
		{
			return GetPos() < FileSize();
		}

		std::size_t RiffFile::GetPos()
		{
			return SetFilePointer(_handle, 0, 0, FILE_CURRENT);
		}

		std::size_t RiffFile::FileSize()
		{
			std::size_t const save(GetPos());
			std::size_t const end(SetFilePointer(_handle,0,0,FILE_END));
			Seek(save);
			return end;
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

		TCHAR * RiffFile::ReadStringA2T(TCHAR * data, std::size_t const & max_length)
		{
			char * buffer = new char[max_length];
			ReadString(buffer, max_length);
			_tcscpy(data, CA2T(buffer));
			delete [] buffer;
			return data;
		}



		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// OldPsyFile



		bool OldPsyFile::Open(std::string const & filename)
		{
			szName = filename;
			return _file = std::fopen(filename.c_str(), "rb");
		}

		bool OldPsyFile::Create(std::string const & filename, bool const & overwrite)
		{
			szName = filename;
			_file = std::fopen(filename.c_str(), "rb");
			if(_file)
			{
				std::fclose(_file);
				if(!overwrite) return false;
			}
			return _file = std::fopen(filename.c_str(), "wb");
		}

		bool OldPsyFile::Close()
		{
			if(!_file)
			{
				std::fflush(_file);
				bool const result(Error());
				std::fclose(_file);
				_file = 0;
				return result;
			}
			return true;
		}

		bool OldPsyFile::Error()
		{
			return !/*std::*/ferror(_file);
		}

		bool OldPsyFile::Read(void * data, std::size_t const & bytes)
		{
			return std::fread(data, sizeof(char), bytes, _file) == bytes;
		}

		bool OldPsyFile::Write(void const * data, std::size_t const & bytes)
		{
			///\todo why flushing?
			std::fflush(_file);
			return std::fwrite(data, sizeof(char), bytes, _file) == bytes;
		}

		bool OldPsyFile::Expect(void * data, std::size_t const & bytes)
		{
			char * chars(reinterpret_cast<char*>(data));
			std::size_t count(bytes);
			while(count--)
			{
				unsigned char c;
				if(std::fread(&c, sizeof c, 1, _file) != 1) return false;
				if(c != *chars) return false;
				++chars;
			}
			return true;
		}

		std::ptrdiff_t OldPsyFile::Seek(std::ptrdiff_t const & bytes)
		{
			if(std::fseek(_file, bytes, SEEK_SET)) return -1;
			return std::ftell(_file);
		}

		std::ptrdiff_t OldPsyFile::Skip(std::ptrdiff_t const & bytes)
		{
			if(std::fseek(_file, bytes, SEEK_CUR)) return -1;
			return std::ftell(_file);
		}

		bool OldPsyFile::Eof()
		{
			return static_cast<bool>(/*std::*/feof(_file));
		}

		std::size_t OldPsyFile::FileSize()
		{
			std::size_t const save(std::ftell(_file));
			std::fseek(_file, 0, SEEK_END);
			std::size_t const end(std::ftell(_file));
			std::fseek(_file, save, SEEK_SET);
			return end;
		}

		std::size_t OldPsyFile::GetPos()
		{
			return std::ftell(_file);
		}
	}
}
