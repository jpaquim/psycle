/** @file
 *  @brief implementation file
 *  $Date: 2004/10/11 06:59:39 $
 *  $Revision: 1.4 $
 */
#include "stdafx.h"
#if defined(_MSC_VER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include "crtdbg.h"
#define malloc(a) _malloc_dbg(a,_NORMAL_BLOCK,__FILE__,__LINE__)
    inline void*  operator new(size_t size, LPCSTR strFileName, INT iLine)
        {return _malloc_dbg(size, _NORMAL_BLOCK, strFileName, iLine);}
    inline void operator delete(void *pVoid, LPCSTR strFileName, INT iLine)
        {_free_dbg(pVoid, _NORMAL_BLOCK);}
#define new  ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#if defined(_WINAMP_PLUGIN_)
//#include <windows.h>
#include <stdio.h>
#endif // _WINAMP_PLUGIN_

#include "FileIO.h"
#include "ErrorString.h"

ULONG RiffFile::FourCC(char *psName)
{
	long retbuf = 0x20202020;   // four spaces (padding)
	char *ps = ((char *)&retbuf);

	// Remember, this is Intel format!
	// The first character goes in the LSB
	for (int i=0; i<4 && psName[i]; i++ )
	{
		*ps++ = psName[i];
	}
	return retbuf;
}


bool RiffFile::Open(
	const TCHAR* psFileName)
{
	DWORD bytesRead;
	_tcscpy(szName,psFileName);

	_modified = false;
	_handle = ::CreateFile(psFileName,
		GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (_handle == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	if (!ReadFile(_handle, &_header, sizeof(_header), &bytesRead, NULL))
	{
		CloseHandle(_handle);
		return false;
	}
	return true;
}

bool RiffFile::Create(
	const TCHAR* psFileName,
	bool overwrite)
{
	DWORD bytesWritten;
	_tcscpy(szName,psFileName);

	_modified = false;
	_handle = ::CreateFile(psFileName,
		GENERIC_READ|GENERIC_WRITE, 0, NULL,
		overwrite ? CREATE_ALWAYS : CREATE_NEW,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (_handle == INVALID_HANDLE_VALUE)
	{
		return false;
	}
	_header._id = FourCC("RIFF");
	_header._size = 0;
	if (!WriteFile(_handle, &_header, sizeof(_header), &bytesWritten, NULL))
	{
		CloseHandle(_handle);
		return false;
	}
	return true;
}

BOOL RiffFile::Close(
	void)
{
	DWORD bytesWritten;
	if (_modified)
	{
		Seek(0);
		WriteFile(_handle, &_header, sizeof(_header), &bytesWritten, NULL);
		_modified = false;
	}
	BOOL b = CloseHandle(_handle);
	_handle = INVALID_HANDLE_VALUE;
	return b;
}

void * RiffFile::Read(
	void* pData,
	ULONG numBytes)
{
	DWORD bytesRead;
	if (ReadFile(_handle, pData, numBytes, &bytesRead, NULL) == FALSE)
	{
		CloseHandle(_handle);
		throw Exception((SF::tformat(SF::CResourceString(IDS_ERR_READ_FILE)) % __FILE__ % __LINE__).str());
	}

	if(bytesRead != numBytes){
		CloseHandle(_handle);
		throw Exception((SF::tformat(SF::CResourceString(IDS_ERR_READ_FILE)) % __FILE__ % __LINE__).str());
	}

	return pData;
}

void RiffFile::Write(
	void* pData,
	ULONG numBytes)
{
	DWORD bytesWritten;
	if (WriteFile(_handle, pData, numBytes, &bytesWritten, NULL) == FALSE)
	{	
		CString _err = ErrorString(GetLastError());
		throw Exception((SF::tformat(SF::CResourceString(IDS_ERR_MSG0096)) % _err.GetBuffer() % __FILE__ % __LINE__).str());
	}
	_modified = true;
	_header._size += bytesWritten;
	if(bytesWritten != numBytes)
	{
		throw Exception((SF::tformat(SF::CResourceString(IDS_ERR_WRITE_FILE)) % __FILE__ % __LINE__).str());
	};
}


bool RiffFile::Expect(
	void* pData,
	ULONG numBytes)
{
	DWORD bytesRead;
	UCHAR c;

	while (numBytes-- != 0)
	{
		if (!ReadFile(_handle, &c, sizeof(c), &bytesRead, NULL))
		{
			return false;
		}
		if (c != *((char*)pData))
		{
			return false;
		}
		pData = (char*)pData + 1;
	}
	return true;
}

long RiffFile::Seek(
	long offset)
{
	return SetFilePointer(_handle, offset, NULL, FILE_BEGIN);
}

long RiffFile::Skip(
	long numBytes)
{
	return SetFilePointer(_handle, numBytes, NULL, FILE_CURRENT); 
}

bool RiffFile::Eof(
	void)
{
	return false;
}

long RiffFile::GetPos()
{
	return SetFilePointer(_handle, 0, NULL, FILE_CURRENT);
}


long RiffFile::FileSize()
{
	const long init = GetPos();
	const long end = SetFilePointer(_handle,0,NULL,FILE_END);
	SetFilePointer(_handle,init,NULL,FILE_BEGIN);
	return end;
}

const char * RiffFile::ReadString(char* pData, const ULONG maxBytes)
{
	if (maxBytes > 0)
	{
		memset(pData,0,maxBytes);

		char c;
		for (ULONG index = 0; index < maxBytes; index++)
		{
			if (Read(&c, sizeof(c)))
			{
				pData[index] = c;
				if (c == 0)
				{
					return pData;
				}
			}
			else 
			{
				throw Exception((SF::tformat(SF::CResourceString(IDS_ERR_READ_FILE)) % __FILE__ % __LINE__).str());
			}
		}
		while (c != 0)
		{
			if (!Read(&c, sizeof(c)))
			{
				throw Exception((SF::tformat(SF::CResourceString(IDS_ERR_READ_FILE)) % __FILE__ % __LINE__).str());
			}
		}
		return pData;
	}
	throw Exception((SF::tformat(SF::CResourceString(IDS_ERR_READ_FILE)) % __FILE__ % __LINE__).str());

}

const TCHAR * RiffFile::ReadStringA2T(TCHAR* pData, const ULONG maxLength)
{
	char* _buffer = new char[maxLength];
	ReadString(_buffer,maxLength);
    _tcscpy(pData,CA2T(_buffer));
	delete [] _buffer;
	return pData;
}

const TCHAR * RiffFile::ReadString(TCHAR* pData, const ULONG maxLength)
{
	if (maxLength > 0)
	{
		memset(pData,0,maxLength * sizeof(TCHAR));

		TCHAR c;
		for (ULONG index = 0; index < maxLength; index++)
		{
			if (Read(&c, sizeof(c)))
			{
				pData[index] = c;
				if (c == 0)
				{
					return pData;
				}
			}
			else 
			{
				throw Exception((SF::tformat(SF::CResourceString(IDS_ERR_READ_FILE)) % __FILE__ % __LINE__).str());
			}
		}
		while (c != 0)
		{
			if (!Read(&c, sizeof(c)))
			{
				throw Exception((SF::tformat(SF::CResourceString(IDS_ERR_READ_FILE)) % __FILE__ % __LINE__).str());
			}
		}
		return pData;
	}
	throw Exception((SF::tformat(SF::CResourceString(IDS_ERR_READ_FILE)) % __FILE__ % __LINE__).str());
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool OldPsyFile::Open(
	const TCHAR* psFileName)
{
	_tcscpy(szName,psFileName);
	_file = _tfopen(psFileName, _T("rb"));
	return (_file != NULL);
}

bool OldPsyFile::Create(
	const TCHAR* psFileName,
	bool overwrite)
{
	_tcscpy(szName,psFileName);
	_file = _tfopen(psFileName, _T("rb"));
	if (_file != NULL)
	{
		fclose(_file);
		if (!overwrite)
		{
			return false;
		}
	}

	_file = _tfopen(psFileName, _T("wb"));
	return (_file != NULL);
}

BOOL OldPsyFile::Close()
{
	if ( _file != NULL )
	{
		fflush(_file);
		BOOL b = !ferror(_file);
		fclose(_file);
		_file = NULL;
		return b;
	}
	return true;
}

BOOL OldPsyFile::Error()
{
	BOOL b = !ferror(_file);
	return b;
}

void * OldPsyFile::Read(
	void* pData,
	ULONG numBytes)
{
	//if(feof(_file) || ferror(_file)){
	//	throw RiffFile::Exception("OldPsyFile::Read()’†‚ÅƒGƒ‰[‚ª”­¶‚µ‚Ü‚µ‚½B");
	//}

	DWORD bytesRead = fread(pData, sizeof(char), numBytes, _file);
	if(bytesRead != numBytes)
	{
		throw Exception((SF::tformat(SF::CResourceString(IDS_ERR_READ_FILE)) % __FILE__ % __LINE__).str());
	}
	
	return pData;
}

void OldPsyFile::Write(
	void* pData,
	const ULONG numBytes)
{
	fflush(_file);
	DWORD bytesWritten = fwrite(pData, sizeof(char), numBytes, _file);
	if(bytesWritten != numBytes)
	{
		throw Exception((SF::tformat(SF::CResourceString(IDS_ERR_WRITE_FILE)) % __FILE__ % __LINE__).str());
	};
}

bool OldPsyFile::Expect(
	void* pData,
	ULONG numBytes)
{
	UCHAR c;

	while (numBytes-- != 0)
	{
		if (fread(&c, sizeof(c), 1, _file) != 1)
		{
			return false;
		}
		if (c != *((char*)pData))
		{
			return false;
		}
		pData = (char*)pData + 1;
	}
	return true;
}

long OldPsyFile::Seek(
	long offset)
{
	if (fseek(_file, offset, SEEK_SET) != 0)
	{
		return -1;
	}
	return ftell(_file);
}

long OldPsyFile::Skip(
	long numBytes)
{
	if (fseek(_file, numBytes, SEEK_CUR) != 0)
	{
		return -1;
	}
	return ftell(_file);
}

bool OldPsyFile::Eof(
	void)
{
	return feof(_file)?true:false;
}

long OldPsyFile::FileSize()
{
	int init = ftell(_file);
	fseek(_file, 0,SEEK_END);
	int end = ftell(_file);
	fseek(_file,init,SEEK_SET);
	return end;
}

long OldPsyFile::GetPos()
{
	return ftell(_file);
}
