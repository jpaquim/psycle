#include "stdafx.h"


#if defined(_WINAMP_PLUGIN_)
//#include <windows.h>
#include <stdio.h>
#endif // _WINAMP_PLUGIN_

#include "FileIO.h"

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
	char* psFileName)
{
	DWORD bytesRead;

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
	char* psFileName,
	bool overwrite)
{
	DWORD bytesWritten;
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

void RiffFile::Close(
	void)
{
	DWORD bytesWritten;
	if (_modified)
	{
		Seek(0);
		WriteFile(_handle, &_header, sizeof(_header), &bytesWritten, NULL);
		_modified = false;
	}
	CloseHandle(_handle);
	_handle = INVALID_HANDLE_VALUE;
}

bool RiffFile::Read(
	void* pData,
	ULONG numBytes)
{
	DWORD bytesRead;
	if (!ReadFile(_handle, pData, numBytes, &bytesRead, NULL))
	{
		return false;
	}
	return (bytesRead == numBytes);
}

bool RiffFile::Write(
	void* pData,
	ULONG numBytes)
{
	DWORD bytesWritten;
	if (!WriteFile(_handle, pData, numBytes, &bytesWritten, NULL))
	{
		return false;
	}
	_modified = true;
	_header._size += bytesWritten;
	return (bytesWritten == numBytes);
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

ULONG RiffFile::Seek(
	ULONG offset)
{
	return SetFilePointer(_handle, offset, NULL, FILE_BEGIN);
}

ULONG RiffFile::Skip(
	ULONG numBytes)
{
	return SetFilePointer(_handle, numBytes, NULL, FILE_CURRENT); 
}

bool RiffFile::Eof(
	void)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

bool OldPsyFile::Open(
	char* psFileName)
{
	_file = fopen(psFileName, "rb");
	return (_file != NULL);
}

bool OldPsyFile::Create(
	char* psFileName,
	bool overwrite)
{
	_file = fopen(psFileName, "rb");
	if (_file != NULL)
	{
		fclose(_file);
		if (!overwrite)
		{
			return false;
		}
	}

	_file = fopen(psFileName, "wb");
	return (_file != NULL);
}

void OldPsyFile::Close(
	void)
{
	if ( _file != NULL )
	{
		fflush(_file);
		fclose(_file);
		_file = NULL;
	}
}

bool OldPsyFile::Read(
	void* pData,
	ULONG numBytes)
{
	DWORD bytesRead = fread(pData, sizeof(char), numBytes, _file);
	return (bytesRead == numBytes);
}

bool OldPsyFile::Write(
	void* pData,
	ULONG numBytes)
{
	fflush(_file);
	DWORD bytesWritten = fwrite(pData, sizeof(char), numBytes, _file);
	return (bytesWritten == numBytes);
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

ULONG OldPsyFile::Seek(
	ULONG offset)
{
	if (fseek(_file, offset, SEEK_SET) != 0)
	{
		return (ULONG)-1;
	}
	return ftell(_file);
}

ULONG OldPsyFile::Skip(
	ULONG numBytes)
{
	if (fseek(_file, numBytes, SEEK_CUR) != 0)
	{
		return (ULONG)-1;
	}
	return ftell(_file);
}

bool OldPsyFile::Eof(
	void)
{
	return feof(_file)?true:false;
}