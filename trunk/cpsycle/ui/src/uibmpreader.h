/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_BMPREADER_H
#define psy_ui_BMPREADER_H

#include "../../detail/psydef.h"
/* local */
#include "uidef.h"
/* std */
#include <assert.h>

/*
** psy_ui_BmpReader
**
** psy_ui_BmpReader <>-------- psy_ui_Bitmap
*/

#ifdef __cplusplus
extern "C" {
#endif	

struct psy_ui_Bitmap;


typedef struct psy_ui_BmpHeader {
	uint16_t type;       /* Magic identifier            */
	uint32_t size;       /* File size in bytes          */
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t offset;     /* Offset to image data, bytes */
} psy_ui_BmpHeader;

typedef struct psy_ui_BmpInfo {
	uint32_t size;             /* Header size in bytes      */
	int32_t width;             /* Width and height of image */
	int32_t height;
	uint16_t planes;           /* Number of colour planes   */
	uint16_t bits;             /* Bits per pixel            */
	uint32_t compression;      /* Compression type          */
	uint32_t imagesize;        /* Image size in bytes       */
	int32_t xresolution;
	int32_t yresolution;       /* Pixels per meter          */
	uint32_t ncolours;         /* Number of colours         */
	uint32_t importantcolours; /* Important colours         */
} psy_ui_BmpInfo;

typedef struct psy_ui_BmpColourIndex {
	uint8_t r, g, b, junk;
} psy_ui_BmpColourIndex;

typedef struct psy_ui_BmpReader {
	struct psy_ui_Bitmap* bitmap;
	psy_ui_BmpHeader header;
	psy_ui_BmpInfo infoheader;
	psy_ui_BmpColourIndex colourindex[256];
	bool verbose;
} psy_ui_BmpReader;

void psy_ui_bmpreader_init(psy_ui_BmpReader*,
	struct psy_ui_Bitmap* bitmap);
void psy_ui_bmpreader_dispose(psy_ui_BmpReader*);

int psy_ui_bmpreader_load(psy_ui_BmpReader*, const char* path);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_BMPREADER_H */
