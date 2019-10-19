// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#ifndef FILEIO_H
#define FILEIO_H

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
} PsyFile; 

int psyfile_open(PsyFile*, const char* path);
int psyfile_create(PsyFile*, const char* path, int overwrite);
int psyfile_close(PsyFile*);
int psyfile_read(PsyFile*, void* data, unsigned long numBytes);
int psyfile_write(PsyFile*, void* data, unsigned long numBytes);
int psyfile_expect(PsyFile*, void* data, unsigned long numBytes);
long psyfile_seek(PsyFile*,long offset);
long psyfile_skip(PsyFile*, long numBytes);
int psyfile_eof(PsyFile*);
long psyfile_filesize(PsyFile*);
int psyfile_readstring(PsyFile*, char* pData, unsigned long maxBytes);
long psyfile_getpos(PsyFile*);	
FILE* psyfile_getfile(PsyFile*);

static unsigned long FourCC(char *psName);
	

#endif
