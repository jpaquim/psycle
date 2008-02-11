// -*- mode:c++; indent-tabs-mode:t -*-
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// For nice and portable code, get the file from psycle-core
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

///\file
///\brief implementation file for psycle::host::RiffFile.
#include <psycle/project.private.hpp>
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

		bool RiffFile::Open(std::string const & FileName)
		{
			///\todo WinAPI code removed in trunk
			DWORD bytesRead;
			szName = FileName;
			_modified = false;
			_handle = ::CreateFile(FileName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			if(_handle == INVALID_HANDLE_VALUE) return false;
			if(!ReadFile(_handle, &_header, sizeof(_header), &bytesRead, 0))
			{
				CloseHandle(_handle);
				return false;
			}
			return true;
		}

		bool RiffFile::Create(std::string const & FileName, bool overwrite)
		{
			DWORD bytesWritten;
			szName = FileName;
			_modified = false;
			_handle = ::CreateFile(FileName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, overwrite ? CREATE_ALWAYS : CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
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
			bool b = CloseHandle(_handle);
			_handle = INVALID_HANDLE_VALUE;
			return b;
		}

		bool RiffFile::Read(void* pData, std::size_t numBytes)
		{
			DWORD bytesRead;
			if(!ReadFile(_handle, pData, numBytes, &bytesRead, 0)) return false;
			return bytesRead == numBytes;
		}

		bool RiffFile::Write(void const * pData, std::size_t numBytes)
		{
			DWORD bytesWritten;
			if(!WriteFile(_handle, pData, numBytes, &bytesWritten, 0)) return false;
			_modified = true;
			_header._size += bytesWritten;
			return bytesWritten == numBytes;
		}

		bool RiffFile::Expect(void* pData, std::size_t numBytes)
		{
			DWORD bytesRead;
			unsigned char c;
			while(numBytes-- != 0)
			{
				if(!ReadFile(_handle, &c, sizeof(c), &bytesRead, 0)) return false;
				if(c != *((char*)pData)) return false;
				pData = (char*)pData + 1;
			}
			return true;
		}

		int RiffFile::Seek(std::size_t offset)
		{
			return SetFilePointer(_handle, offset, 0, FILE_BEGIN);
		}

		int RiffFile::Skip(std::size_t numBytes)
		{
			return SetFilePointer(_handle, numBytes, 0, FILE_CURRENT); 
		}

		bool RiffFile::Eof()
		{
			return false;
		}

		std::size_t RiffFile::GetPos()
		{
			return SetFilePointer(_handle, 0, 0, FILE_CURRENT);
		}

		std::size_t RiffFile::FileSize()
		{
			const std::size_t init = GetPos();
			const std::size_t end = SetFilePointer(_handle,0,0,FILE_END);
			SetFilePointer(_handle,init,0,FILE_BEGIN);
			return end;
		}

		bool RiffFile::ReadString(std::string & result)
		{
			result="";
			char c;
			while(true)
			{
				if (Read(&c, sizeof(c)))
				{
					if(c == 0) {
						return true;
					}
					result+=c;
				}
				else return false;
			}
		}

		bool RiffFile::ReadString(char* pData, std::size_t maxBytes)
		{
			if(maxBytes > 0)
			{
				std::memset(pData,0,maxBytes);

				char c;
				for(std::size_t index = 0; index < maxBytes; index++)
				{
					if (Read(&c, sizeof c))
					{
						pData[index] = c;
						if(c == 0) return true;
					}
					else return false;
				}
				do
				{
					if(!Read(&c, sizeof c)) return false; //\todo : return false, or return true? the string is read already. it could be EOF.
				} while(c != 0);
				return true;
			}
			return false;
		}

		const TCHAR * RiffFile::ReadStringA2T(TCHAR* pData, const std::size_t maxLength)
		{
			char* _buffer = new char[maxLength];
			ReadString(_buffer,maxLength);
			/* \todo URGH, HA, OUCH, GLURPS!!!!!*/_tcscpy(pData,CA2T(_buffer));
			delete [] _buffer;
			return pData;
		}


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// OldPsyFile



		bool OldPsyFile::Open(std::string const & FileName)
		{
			szName = FileName;
			_file = std::fopen(FileName.c_str(), "rb");
			return _file != 0;
		}

		bool OldPsyFile::Create(std::string const & FileName, bool overwrite)
		{
			szName = FileName;
			_file = std::fopen(FileName.c_str(), "rb");
			if(_file != 0)
			{
				std::fclose(_file);
				if(!overwrite) return false;
			}
			_file = std::fopen(FileName.c_str(), "wb");
			return _file != 0;
		}

		bool OldPsyFile::Close()
		{
			if( _file != 0)
			{
				std::fflush(_file);
				bool b = !ferror(_file);
				std::fclose(_file);
				_file = 0;
				return b;
			}
			return true;
		}

		bool OldPsyFile::Read(void* pData, std::size_t numBytes)
		{
			std::size_t bytesRead = std::fread(pData, sizeof(char), numBytes, _file);
			return bytesRead == numBytes;
		}

		bool OldPsyFile::Write(void const * pData, std::size_t numBytes)
		{
			std::fflush(_file); ///\todo why flushing?
			std::size_t bytesWritten = std::fwrite(pData, sizeof(char), numBytes, _file);
			return bytesWritten == numBytes;
		}

		bool OldPsyFile::Expect(void* pData, std::size_t numBytes)
		{
			unsigned char c;
			while (numBytes-- != 0)
			{
				if(std::fread(&c, sizeof c, 1, _file) != 1) return false;
				if(c != *((char*)pData)) return false;
				pData = (char*)pData + 1;
			}
			return true;
		}

		int OldPsyFile::Seek(std::size_t offset)
		{
			if(std::fseek(_file, offset, SEEK_SET) != 0) return -1;
			return std::ftell(_file);
		}

		int OldPsyFile::Skip(std::size_t numBytes)
		{
			if(std::fseek(_file, numBytes, SEEK_CUR) != 0) return -1;
			return std::ftell(_file);
		}

		bool OldPsyFile::Eof()
		{
			return static_cast<bool>(feof(_file));
		}

		std::size_t OldPsyFile::FileSize()
		{
			int init(std::ftell(_file));
			std::fseek(_file, 0, SEEK_END);
			int end(std::ftell(_file));
			std::fseek(_file, init, SEEK_SET);
			return end;
		}

		std::size_t OldPsyFile::GetPos()
		{
			return std::ftell(_file);
		}

		bool OldPsyFile::Error()
		{
			return !ferror(_file);
		}
	}
}
