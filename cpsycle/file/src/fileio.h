/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_FILEIO_H
#define psy_FILEIO_H

#include "dir.h"

#include <stdio.h>
#include "../../detail/stdint.h"
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TYPEULONGINV {
  unsigned char hihi;
  unsigned char hilo;
  unsigned char lohi;
  unsigned char lolo;
} ULONGINV;

typedef struct {
	uint32_t version;
	uint32_t size;
	uint32_t begins;
} RiffChunk;

typedef struct PsyFile {	
	int _modified;
	char szName[1024];
	FILE* _file;	
	RiffChunk currchunk;
	uint32_t fileversion;
	uint32_t filesize;
	uint32_t chunkcount;
	int flip16;
} PsyFile; 

int psyfile_open(PsyFile*, const char* filename);
int psyfile_create(PsyFile*, const char* filename, bool overwrite);
int psyfile_close(PsyFile*);
int psyfile_read(PsyFile*, void* data, uintptr_t numbytes);
int psyfile_write(PsyFile*, const void* data, uintptr_t numbytes);
int psyfile_write_int8(PsyFile*, int8_t value);
int psyfile_write_uint8(PsyFile*, uint8_t value);
int psyfile_write_int16(PsyFile*, int16_t value);
int psyfile_write_uint16(PsyFile*, uint16_t value);
int psyfile_write_int32(PsyFile*, int32_t value);
int psyfile_write_uint32(PsyFile*, uint32_t value);
int psyfile_write_float(PsyFile*, float value);
int psyfile_expect(PsyFile*, void* data, uintptr_t numbytes);
int32_t psyfile_seek(PsyFile*, uint32_t offset);
int32_t psyfile_skip(PsyFile*, uint32_t numbytes);
int psyfile_eof(PsyFile*);
uint32_t psyfile_filesize(PsyFile*);
int psyfile_readstring(PsyFile*, char* pData, uint32_t maxbytes);
uint32_t psyfile_getpos(PsyFile*);
FILE* psyfile_getfile(PsyFile*);
int psyfile_writeheader(PsyFile*, char* pData, uint32_t version,
	uint32_t size, uint32_t* pos);
int psyfile_writestring(PsyFile*, const char* str);
int psyfile_updatesize(PsyFile*, uint32_t startpos, uint32_t* rv_size);
int psyfile_readchunkbegin(PsyFile*);
void psyfile_seekchunkend(PsyFile*);
int psyfile_error(PsyFile*);

INLINE uint32_t psyfile_fileversion(const PsyFile* self)
{
	return self->fileversion;
}

INLINE uint32_t psyfile_currchunkversion(const PsyFile* self)
{
	return self->currchunk.version;
}



static uint32_t FourCC(char *psname);

#ifdef __cplusplus
}
#endif

#endif /* psy_FILEIO_H */
