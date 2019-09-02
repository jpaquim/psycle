// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "fileio.h"

#if defined(_WINAMP_PLUGIN_)
//#include <windows.h>
#include <stdio.h>
#endif // _WINAMP_PLUGIN_



ULONG FourCC(char *psName)
{
	long retbuf = 0x20202020;   // four spaces (padding)
	char *ps = ((char *)&retbuf);
	int i;
	
	// Remember, this is Intel format!
	// The first character goes in the LSB
	for (i=0; i<4 && psName[i]; i++ )
	{
		*ps++ = psName[i];
	}
	return retbuf;
}


BOOL rifffile_open(RiffFile* self,
				   const char* psFileName)
{
	strcpy(self->szName,psFileName);
	self->_file = fopen(psFileName, "rb");
	return (self->_file != NULL);
}

BOOL rifffile_create(RiffFile* self,
					 char* psFileName,
					 BOOL overwrite)
{
	strcpy(self->szName,psFileName);
	self->_file = fopen(psFileName, "rb");
	if (self->_file != NULL)
	{
		fclose(self->_file);
		if (!overwrite)
		{
			return FALSE;
		}
	}
	
	self->_file = fopen(psFileName, "wb");
	return (self->_file != NULL);
}

BOOL rifffile_close(RiffFile* self)
{
	if ( self->_file != NULL )
	{
		BOOL b;
		fflush(self->_file);
		b = !ferror(self->_file);
		fclose(self->_file);
		self->_file = NULL;
		return b;
	}
	return TRUE;
	
	
}

BOOL rifffile_read(RiffFile* self,
				   void* pData,
				   ULONG numBytes)
{
	DWORD bytesRead = fread(pData, sizeof(char), numBytes, self->_file);
	return (bytesRead == numBytes);
	
}

BOOL rifffile_write(RiffFile* self,
					void* pData,
					ULONG numBytes)
{
	DWORD bytesWritten;
	fflush(self->_file);
	bytesWritten = fwrite(pData, sizeof(char), numBytes, self->_file);
	return (bytesWritten == numBytes);
	
	
}

BOOL rifffile_expect(RiffFile* self,
					 void* pData,
					 ULONG numBytes)
{
	UCHAR c;
	
	while (numBytes-- != 0)
	{
		if (fread(&c, sizeof(c), 1, self->_file) != 1)
		{
			return FALSE;
		}
		if (c != *((char*)pData))
		{
			return FALSE;
		}
		pData = (char*)pData + 1;
	}
	return TRUE;
	
	
}

long rifffile_seek(RiffFile* self,
				   long offset)
{
	if (fseek(self->_file, offset, SEEK_SET) != 0)
	{
		return -1;
	}
	return ftell(self->_file);
	
}

long rifffile_skip(RiffFile* self,
				   long numBytes)
{
	if (fseek(self->_file, numBytes, SEEK_CUR) != 0) return -1;
	return ftell(self->_file);
}

BOOL rifffile_eof(RiffFile* self)
{
	return feof(self->_file);
	
}

long rifffile_getpos(RiffFile* self)
{
	return ftell(self->_file);
}


long rifffile_filesize(RiffFile* self)
{	
	int init = ftell(self->_file);
	int end;
	fseek(self->_file, 0,SEEK_END);
	end = ftell(self->_file);
	fseek(self->_file,init,SEEK_SET);
	return end;
	
}

BOOL rifffile_readstring(RiffFile* self, char* pData, ULONG maxBytes)
{
	if (maxBytes > 0)
	{		
		char c;
		ULONG index;
		memset(pData,0,maxBytes);
		for (index = 0; index < maxBytes; index++)
		{
			if (rifffile_read(self, &c, sizeof(c)))
			{
				pData[index] = c;
				if (c == 0)
				{
					return TRUE;
				}
			}
			else 
			{
				return FALSE;
			}
		}
		do
		{
			if (!rifffile_read(self, &c, sizeof(c)))
			{
				return FALSE;
			}
		} while (c != 0);
		return TRUE;
	}
	return FALSE;
	
}

FILE* rifffile_getfile(RiffFile* self)
{
	return self->_file;
}

