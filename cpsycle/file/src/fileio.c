// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "fileio.h"
#include <string.h> 
#include <stdio.h>

typedef int BOOL;

#if !defined(TRUE)
#define TRUE 1
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

unsigned long FourCC(char *psName)
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

BOOL psyfile_open(PsyFile* self,
				   const char* psFileName)
{
	strcpy(self->szName,psFileName);
	self->_file = fopen(psFileName, "rb");
	return (self->_file != NULL);
}

BOOL psyfile_create(PsyFile* self,
					 const char* psFileName,
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

BOOL psyfile_close(PsyFile* self)
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

BOOL psyfile_read(PsyFile* self,
				   void* pData,
				   unsigned long numBytes)
{
	unsigned long bytesRead = fread(pData, sizeof(char), numBytes, self->_file);
	return (bytesRead == numBytes);
	
}

BOOL psyfile_write(PsyFile* self,
					void* pData,
					unsigned long numBytes)
{
	unsigned long bytesWritten;
	fflush(self->_file);
	bytesWritten = fwrite(pData, sizeof(char), numBytes, self->_file);
	return (bytesWritten == numBytes);
	
	
}

BOOL psyfile_expect(PsyFile* self,
					 void* pData,
					 unsigned long numBytes)
{
	unsigned char c;
	
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

long psyfile_seek(PsyFile* self,
				   long offset)
{
	if (fseek(self->_file, offset, SEEK_SET) != 0)
	{
		return -1;
	}
	return ftell(self->_file);
	
}

long psyfile_skip(PsyFile* self,
				   long numBytes)
{
	if (fseek(self->_file, numBytes, SEEK_CUR) != 0) return -1;
	return ftell(self->_file);
}

BOOL psyfile_eof(PsyFile* self)
{
	return feof(self->_file);
	
}

long psyfile_getpos(PsyFile* self)
{
	return ftell(self->_file);
}


long psyfile_filesize(PsyFile* self)
{	
	int init = ftell(self->_file);
	int end;
	fseek(self->_file, 0,SEEK_END);
	end = ftell(self->_file);
	fseek(self->_file,init,SEEK_SET);
	return end;
	
}

BOOL psyfile_readstring(PsyFile* self, char* pData, unsigned long maxBytes)
{
	if (maxBytes > 0)
	{		
		char c;
		unsigned long index;
		memset(pData,0,maxBytes);
		for (index = 0; index < maxBytes; index++)
		{
			if (psyfile_read(self, &c, sizeof(c)))
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
			if (!psyfile_read(self, &c, sizeof(c)))
			{
				return FALSE;
			}
		} while (c != 0);
		return TRUE;
	}
	return FALSE;
	
}

FILE* psyfile_getfile(PsyFile* self)
{
	return self->_file;
}

