/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uibmpreader.h"
/* local */
#include "uibitmap.h"
#include "uigraphics.h"
/* std */
#include <stdlib.h>
#include <stdio.h>
/* platform */
#include "../../detail/os.h"

typedef struct BmpHeader {
	uint16_t type;       /* Magic identifier            */
	uint32_t size;       /* File size in bytes          */
	uint16_t reserved1;
	uint16_t reserved2;
	uint32_t offset;     /* Offset to image data, bytes */
} BmpHeader;

typedef struct BmpInfo {
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
} BmpInfo;

typedef struct BmpColourIndex {
	uint8_t r, g, b, junk;
} BmpColourIndex;

static bool bigendian(void)
{
	union { int i; char c[sizeof(int)]; } u;
	u.i = 1;
	return !u.c[0];
}


void psy_ui_bmpreader_init(psy_ui_BmpReader* self,
	struct psy_ui_Bitmap* bitmap)
{
	self->bitmap = bitmap;
}

void psy_ui_bmpreader_dispose(psy_ui_BmpReader* self)
{

}

int psy_ui_bmpreader_load(psy_ui_BmpReader* self, const char* path)
{
	FILE* fp;
	BmpHeader header;
	BmpInfo infoheader;
	BmpColourIndex colourindex[256];
	uintptr_t bytesRead;
	int32_t j;
	int32_t gotindex = FALSE;
	uint8_t grey, r, g, b;
	psy_ui_Graphics gc;	
	psy_ui_Colour colour;	
	bool verbose;

	assert(self);
	
	if (!self->bitmap) {
		return PSY_ERRRUN;
	}
		
	verbose = 0;
	if ((fp = fopen(path, "rb")) == NULL) {
		return PSY_ERRFILEFORMAT;
	}
	/* Magic identifier            */
	bytesRead = fread(&header.type, sizeof(char), 2, fp);
	if (header.type != 'M' * 256 + 'B') {
		/* todo warning */
	}
	/* File size in bytes          */
	bytesRead = fread(&header.size, sizeof(char), 4, fp);
	/* reserved					   */
	bytesRead = fread(&header.reserved1, sizeof(char), 2, fp);
	bytesRead = fread(&header.reserved2, sizeof(char), 2, fp);
	/* Offset to image data, bytes */
	bytesRead = fread(&header.offset, sizeof(char), 4, fp);
	/* Read and check the information header */
	if (fread(&infoheader, sizeof(BmpInfo), 1, fp) != 1) {
		return PSY_ERRFILE;
	}
	if (verbose) {
		fprintf(stderr, "Image path = %s\n", path);
		fprintf(stderr, "Image size = %d x %d\n", infoheader.width, infoheader.height);
		fprintf(stderr, "Number of colour planes is %d\n", infoheader.planes);
		fprintf(stderr, "Bits per pixel is %d\n", infoheader.bits);
		fprintf(stderr, "Compression type is %d\n", infoheader.compression);
		fprintf(stderr, "Number of colours is %d\n", infoheader.ncolours);
		fprintf(stderr, "Number of required colours is %d\n",
			infoheader.importantcolours);
	}
	/* Read the lookup table if there is one */
	{
		uint32_t i;

		for (i = 0; i < 255; i++) {
			colourindex[i].r = rand() % 256;
			colourindex[i].g = rand() % 256;
			colourindex[i].b = rand() % 256;
			colourindex[i].junk = rand() % 256;
		}
	}
	if (infoheader.ncolours > 0) {
		uint32_t i;
		for (i = 0; i < infoheader.ncolours; i++) {
			if (fread(&colourindex[i].b, sizeof(unsigned char), 1, fp) != 1) {
				fprintf(stderr, "Image read failed\n");
				exit(-1);
			}
			if (fread(&colourindex[i].g, sizeof(unsigned char), 1, fp) != 1) {
				fprintf(stderr, "Image read failed\n");
				exit(-1);
			}
			if (fread(&colourindex[i].r, sizeof(unsigned char), 1, fp) != 1) {
				fprintf(stderr, "Image read failed\n");
				exit(-1);
			}
			if (fread(&colourindex[i].junk, sizeof(unsigned char), 1, fp) != 1) {
				fprintf(stderr, "Image read failed\n");
				exit(-1);
			}
			fprintf(stderr, "%3d\t%3d\t%3d\t%3d\n", i,
				colourindex[i].r, colourindex[i].g, colourindex[i].b);
		}
		gotindex = TRUE;
	}
	/* Seek to the start of the image data */
	fseek(fp, header.offset, SEEK_SET);
	if (verbose) {
		fprintf(stderr, "Image size = %d x %d\n", infoheader.width, infoheader.height);
	}
	psy_ui_bitmap_dispose(self->bitmap);
	psy_ui_bitmap_init_size(self->bitmap,		
		psy_ui_realsize_make(
			infoheader.width,
			infoheader.height));
	psy_ui_graphics_init_bitmap(&gc, self->bitmap);	
	psy_ui_colour_init_rgb(&colour, 255, 255, 255);
	psy_ui_drawsolidrectangle(&gc,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_zero(),
			psy_ui_realsize_make(
				infoheader.width,
				infoheader.height)),
		colour);	
	if (infoheader.height > 0) {
		int32_t padding; 

		/* Read the image */		

		/* 
		** Each scan line is zero padded to the nearest 4-byte boundary. If the image
		** has a width that is not divisible by four, say, 21 bytes, there would be
		** 3 bytes of padding at the end of every scan line.
		*/
		if (infoheader.bits == 24) {
			/* padding width: 4 bytes */
			/* num bytes per line: infoheader.width * 3 */
			padding = (int32_t)(ceil((infoheader.width * 3) / 4.0)) * 4 - (infoheader.width * 3);
		} else {
			padding = 1;
		}
		for (j = infoheader.height - 1; j >= 0; j--) {
			int32_t i;

			for (i = 0; i < infoheader.width; i++) {

				switch (infoheader.bits) {
				case 1:
					break;
				case 4:
					break;
				case 8:					
					if (fread(&grey, sizeof(unsigned char), 1, fp) != 1) {
						fprintf(stderr, "Image read failed\n");
						exit(-1);
					}
					if (gotindex) {
						putchar(colourindex[grey].r);
						putchar(colourindex[grey].g);
						putchar(colourindex[grey].b);
					} else {
						putchar(grey);
					}
					break;
				case 24:
					if (fread(&b, sizeof(unsigned char), 1, fp) != 1) {
						fprintf(stderr, "Image read failed\n");
						psy_ui_graphics_dispose(&gc);	
						fclose(fp);
						return PSY_ERRFILEFORMAT;
					}
					if (fread(&g, sizeof(unsigned char), 1, fp) != 1) {
						fprintf(stderr, "Image read failed\n");
						psy_ui_graphics_dispose(&gc);	
						fclose(fp);
						return PSY_ERRFILEFORMAT;
					}
					if (fread(&r, sizeof(unsigned char), 1, fp) != 1) {
						fprintf(stderr, "Image read failed\n");
						psy_ui_graphics_dispose(&gc);	
						fclose(fp);
						return PSY_ERRFILEFORMAT;
					}					
					psy_ui_colour_init_rgb(&colour, r, g, b);
					psy_ui_drawsolidrectangle(&gc,
						psy_ui_realrectangle_make(
							psy_ui_realpoint_make(i, j),
							psy_ui_realsize_make(1.0, 1.0)),
						colour);
					if (verbose) {
						fprintf(stderr, "Point = %d , %d, %d, %d, %d\n", i, j,
							(int)r, (int)g, (int)b);
					}
					// putchar(r);
				   //  putchar(g);
				   //  putchar(b);
					break;
				}

			} /* i */
			for (i = 0; i < padding; ++i) {
				fread(&r, sizeof(unsigned char), 1, fp);
			}
		} /* j */
	}
	psy_ui_graphics_dispose(&gc);	
	fclose(fp);
	return PSY_OK;
}
