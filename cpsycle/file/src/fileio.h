// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#ifndef _FILEIO_H
#define _FILEIO_H

#include <stdio.h>

typedef struct TYPEULONGINV {
  unsigned char hihi;
  unsigned char hilo;
  unsigned char lohi;
  unsigned char lolo;
} ULONGINV;

typedef struct
{
	unsigned long _id;
	unsigned long _size; // This one should be ULONGINV (it is, at least, in the files I([JAZ]) have tested)
}
RiffChunkHeader;


typedef struct {	
	int _modified;
	char szName[1024];
	FILE* _file;
} RiffFile; 

int rifffile_open(RiffFile*, const char* psFileName);
int rifffile_create(RiffFile*, const char* psFileName, int overwrite);
int rifffile_close(RiffFile*);
int rifffile_read(RiffFile*, void* pData, unsigned long numBytes);
int rifffile_write(RiffFile*, void* pData, unsigned long numBytes);
int rifffile_expect(RiffFile*, void* pData, unsigned long numBytes);
long rifffile_seek(RiffFile*,long offset);
long rifffile_skip(RiffFile*, long numBytes);
int rifffile_eof(RiffFile*);
long rifffile_filesize(RiffFile*);
int rifffile_readstring(RiffFile*, char* pData, unsigned long maxBytes);
long rifffile_getpos(RiffFile*);	
FILE* rifffile_getfile(RiffFile*);

static unsigned long FourCC(char *psName);
	

#endif
