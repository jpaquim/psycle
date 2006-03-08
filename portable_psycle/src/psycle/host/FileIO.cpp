///\file
///\brief implementation file for psycle::host::RiffFile.
#include <project.private.hpp>
#include "FileIO.hpp"
namespace psycle
{
	namespace host
	{
		ULONG RiffFile::FourCC(char const *psName)
		{
			long retbuf = 0x20202020; // four spaces (padding)
			char *ps = ((char *)&retbuf);
			// Remember, this is Intel format!
			// The first character goes in the LSB
			for(int i(0); i < 4 && psName[i]; ++i) *ps++ = psName[i];
			return retbuf;
		}

		bool RiffFile::Open(std::string psFileName)
		{
			DWORD bytesRead;
			szName = psFileName;
			_modified = false;
			_handle = ::CreateFile(psFileName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
			if(_handle == INVALID_HANDLE_VALUE) return false;
			if(!ReadFile(_handle, &_header, sizeof(_header), &bytesRead, 0))
			{
				CloseHandle(_handle);
				return false;
			}
			return true;
		}

		bool RiffFile::Create(std::string psFileName, bool overwrite)
		{
			DWORD bytesWritten;
			szName = psFileName;
			_modified = false;
			_handle = ::CreateFile(psFileName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, 0, overwrite ? CREATE_ALWAYS : CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0);
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

		BOOL RiffFile::Close()
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

		bool RiffFile::Read(void* pData, ULONG numBytes)
		{
			DWORD bytesRead;
			if(!ReadFile(_handle, pData, numBytes, &bytesRead, 0)) return false;
			return (bytesRead == numBytes);
		}

		bool RiffFile::Write(const void * const pData, ULONG numBytes)
		{
			DWORD bytesWritten;
			if(!WriteFile(_handle, pData, numBytes, &bytesWritten, 0)) return false;
			_modified = true;
			_header._size += bytesWritten;
			return (bytesWritten == numBytes);
		}

		bool RiffFile::Expect(void* pData, ULONG numBytes)
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

		long RiffFile::Seek(long offset)
		{
			return SetFilePointer(_handle, offset, 0, FILE_BEGIN);
		}

		long RiffFile::Skip(long numBytes)
		{
			return SetFilePointer(_handle, numBytes, 0, FILE_CURRENT); 
		}

		bool RiffFile::Eof()
		{
			return false;
		}

		long RiffFile::GetPos()
		{
			return SetFilePointer(_handle, 0, 0, FILE_CURRENT);
		}

		long RiffFile::FileSize()
		{
			const long init = GetPos();
			const long end = SetFilePointer(_handle,0,0,FILE_END);
			SetFilePointer(_handle,init,0,FILE_BEGIN);
			return end;
		}

		bool RiffFile::ReadString(std::string &result)
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

		bool RiffFile::ReadString(char* pData, ULONG maxBytes)
		{
			if(maxBytes > 0)
			{
				memset(pData,0,maxBytes);

				char c;
				for(ULONG index = 0; index < maxBytes; index++)
				{
					if (Read(&c, sizeof(c)))
					{
						pData[index] = c;
						if(c == 0) return true;
					}
					else return false;
				}
				do
				{
					if(!Read(&c, sizeof(c))) return false; //\todo : return false, or return true? the string is read already. it could be EOF.
				} while(c != 0);
				return true;
			}
			return false;
		}

		const TCHAR * RiffFile::ReadStringA2T(TCHAR* pData, const ULONG maxLength)
		{
			char* _buffer = new char[maxLength];
			ReadString(_buffer,maxLength);
			_tcscpy(pData,CA2T(_buffer));
			delete [] _buffer;
			return pData;
		}


		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		// OldPsyFile



		bool OldPsyFile::Open(std::string psFileName)
		{
			szName = psFileName;
			_file = fopen(psFileName.c_str(), "rb");
			return (_file != 0);
		}

		bool OldPsyFile::Create(std::string psFileName, bool overwrite)
		{
			szName = psFileName;
			_file = fopen(psFileName.c_str(), "rb");
			if(_file != 0)
			{
				fclose(_file);
				if(!overwrite) return false;
			}
			_file = fopen(psFileName.c_str(), "wb");
			return (_file != 0);
		}

		BOOL OldPsyFile::Close()
		{
			if( _file != 0)
			{
				fflush(_file);
				BOOL b = !ferror(_file);
				fclose(_file);
				_file = 0;
				return b;
			}
			return true;
		}

		BOOL OldPsyFile::Error()
		{
			BOOL b = !ferror(_file);
			return b;
		}

		bool OldPsyFile::Read(void* pData, ULONG numBytes)
		{
			DWORD bytesRead = fread(pData, sizeof(char), numBytes, _file);
			return (bytesRead == numBytes);
		}

		bool OldPsyFile::Write(const void* pData, ULONG numBytes)
		{
			fflush(_file);
			DWORD bytesWritten = fwrite(pData, sizeof(char), numBytes, _file);
			return (bytesWritten == numBytes);
		}

		bool OldPsyFile::Expect(void* pData, ULONG numBytes)
		{
			UCHAR c;
			while (numBytes-- != 0)
			{
				if(fread(&c, sizeof(c), 1, _file) != 1) return false;
				if(c != *((char*)pData)) return false;
				pData = (char*)pData + 1;
			}
			return true;
		}

		long OldPsyFile::Seek(long offset)
		{
			if(fseek(_file, offset, SEEK_SET) != 0) return -1;
			return ftell(_file);
		}

		long OldPsyFile::Skip(long numBytes)
		{
			if(fseek(_file, numBytes, SEEK_CUR) != 0) return -1;
			return ftell(_file);
		}

		bool OldPsyFile::Eof()
		{
			return static_cast<bool>(feof(_file));
		}

		long OldPsyFile::FileSize()
		{
			int init(ftell(_file));
			fseek(_file, 0, SEEK_END);
			int end(ftell(_file));
			fseek(_file, init, SEEK_SET);
			return end;
		}

		long OldPsyFile::GetPos()
		{
			return ftell(_file);
		}
	}
}
