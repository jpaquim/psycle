// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#ifndef _FILEIO_H
#define _FILEIO_H

#include <stdio.h>

#if !defined(BOOL)
#define BOOL int
#endif

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
	BOOL _modified;
	char szName[1024];
	FILE* _file;
} RiffFile; 

BOOL rifffile_open(RiffFile*, const char* psFileName);
BOOL rifffile_create(RiffFile*, char* psFileName, BOOL overwrite);
BOOL rifffile_close(RiffFile*);
BOOL rifffile_read(RiffFile*, void* pData, unsigned long numBytes);
BOOL rifffile_write(RiffFile*, void* pData, unsigned long numBytes);
BOOL rifffile_expect(RiffFile*, void* pData, unsigned long numBytes);
long rifffile_seek(RiffFile*,long offset);
long rifffile_skip(RiffFile*, long numBytes);
BOOL rifffile_eof(RiffFile*);
long rifffile_filesize(RiffFile*);
BOOL rifffile_readstring(RiffFile*, char* pData, unsigned long maxBytes);
long rifffile_getpos(RiffFile*);	
FILE* rifffile_getfile(RiffFile*);

static unsigned long FourCC(char *psName);
	

#endif

