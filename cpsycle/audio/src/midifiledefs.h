// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_MIDIFILEDEFS_H
#define psy_audio_MIDIFILEDEFS_H

#include "../../detail/stdint.h"

#pragma pack(push, 1)

typedef struct MCHUNK
{
	char        id[4];
	uint32_t	length;
} MCHUNK;

typedef struct MTHD
{	
	uint16_t format;
	uint16_t numtracks;
	uint16_t division;
} MTHD;

typedef struct MTRK
{
	/* Here's the 8 byte header that all chunks must have */
	char           id[4];   /* This will be 'M','T','r','k' */
	uint32_t       length;  /* This will be the actual size of Data[] */

	/* Here are the data bytes */
	unsigned char  data[];  /* Its actual size is Data[Length] */
} MTRK;

#pragma pack(pop)

#endif /* psy_audio_MIDIFILEDEFS_H */
