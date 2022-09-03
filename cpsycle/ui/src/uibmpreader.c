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

/* prototype */
static int psy_ui_bmpreader_read_header(psy_ui_BmpReader*, FILE*);
static int psy_ui_bmpreader_read_info_header(psy_ui_BmpReader*, FILE*);
static bool psy_ui_bmpreader_has_colour_map(const psy_ui_BmpReader*);
static int psy_ui_bmpreader_read_colour_map(psy_ui_BmpReader*, FILE*);
static int psy_ui_bmpreader_read_data_24(psy_ui_BmpReader*, FILE*,
	psy_ui_Graphics*);


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
	self->verbose = 0;
}

void psy_ui_bmpreader_dispose(psy_ui_BmpReader* self)
{

}

int psy_ui_bmpreader_load(psy_ui_BmpReader* self, const char* path)
{
	FILE* fp;	
	int status;

	assert(self);
	
	if (!self->bitmap) {
		return PSY_ERRRUN;
	}			
	if (self->verbose) {
		fprintf(stderr, "Image path = %s\n", path);
	}
	if ((fp = fopen(path, "rb")) == NULL) {
		return PSY_ERRFILEFORMAT;
	}
	psy_ui_bmpreader_read_header(self, fp);
	psy_ui_bmpreader_read_info_header(self, fp);
	if (psy_ui_bmpreader_has_colour_map(self)) {
		psy_ui_bmpreader_read_colour_map(self, fp);
	}
	/* seek to the start of the image data */
	fseek(fp, self->header.offset, SEEK_SET);
	if (self->verbose) {
		fprintf(stderr, "Image size = %d x %d\n", self->infoheader.width,
			self->infoheader.height);
	}
	status = PSY_OK;
	if (self->infoheader.height > 0) {
		psy_ui_Graphics gc;
				
		psy_ui_bitmap_dispose(self->bitmap);
		psy_ui_bitmap_init_size(self->bitmap, psy_ui_realsize_make(
			self->infoheader.width, self->infoheader.height));
		psy_ui_graphics_init_bitmap(&gc, self->bitmap);		
		psy_ui_drawsolidrectangle(&gc, psy_ui_realrectangle_make(
			psy_ui_realpoint_zero(), psy_ui_realsize_make(
				self->infoheader.width, self->infoheader.height)),
			psy_ui_colour_white());
		switch (self->infoheader.bits) {
		case 1:
			status = PSY_ERRFILEFORMAT;
			break;
		case 4:
			status = PSY_ERRFILEFORMAT;
			break;
		case 8:
			status = PSY_ERRFILEFORMAT;
			break;
		case 24:
			status = psy_ui_bmpreader_read_data_24(self, fp, &gc);
			break;
		default:
			status = PSY_ERRFILEFORMAT;
			break;
		}
		psy_ui_graphics_dispose(&gc);		
	}
	fclose(fp);
	return status;	
}

int psy_ui_bmpreader_read_header(psy_ui_BmpReader* self, FILE* fp)
{
	uintptr_t bytesRead;
	
	/* Magic identifier            */
	bytesRead = fread(&self->header.type, sizeof(char), 2, fp);
	if (self->header.type != 'M' * 256 + 'B') {
		/* todo warning */
	}
	/* File size in bytes          */
	bytesRead = fread(&self->header.size, sizeof(char), 4, fp);
	/* reserved					   */
	bytesRead = fread(&self->header.reserved1, sizeof(char), 2, fp);
	bytesRead = fread(&self->header.reserved2, sizeof(char), 2, fp);
	/* Offset to image data, bytes */
	bytesRead = fread(&self->header.offset, sizeof(char), 4, fp);
	return PSY_OK;
}

int psy_ui_bmpreader_read_info_header(psy_ui_BmpReader* self, FILE* fp)
{
	/* read and check the information header */
	if (fread(&self->infoheader, sizeof(psy_ui_BmpInfo), 1, fp) != 1) {
		return PSY_ERRFILE;
	}
	if (self->verbose) {		
		fprintf(stderr, "Image size = %d x %d\n", self->infoheader.width,
			self->infoheader.height);
		fprintf(stderr, "Number of colour planes is %d\n",
			self->infoheader.planes);
		fprintf(stderr, "Bits per pixel is %d\n", self->infoheader.bits);
		fprintf(stderr, "Compression type is %d\n",
			self->infoheader.compression);
		fprintf(stderr, "Number of colours is %d\n", self->infoheader.ncolours);
		fprintf(stderr, "Number of required colours is %d\n",
			self->infoheader.importantcolours);
	}
	return PSY_OK;
}

bool psy_ui_bmpreader_has_colour_map(const psy_ui_BmpReader* self)
{
	return (self->infoheader.height > 0);
}

int psy_ui_bmpreader_read_colour_map(psy_ui_BmpReader* self, FILE* fp)
{
	uint32_t i;

	for (i = 0; i < 255; i++) {
		self->colourindex[i].r = 0;
		self->colourindex[i].g = 0;
		self->colourindex[i].b = 0;
		self->colourindex[i].junk = 0;	
	}	
	for (i = 0; i < self->infoheader.ncolours; i++) {
		if (fread(&self->colourindex[i].b, sizeof(unsigned char), 1, fp) != 1) {
			fprintf(stderr, "Image read failed\n");
			exit(-1);
		}
		if (fread(&self->colourindex[i].g, sizeof(unsigned char), 1, fp) != 1) {
			fprintf(stderr, "Image read failed\n");
			return PSY_ERRFILEFORMAT;
		}
		if (fread(&self->colourindex[i].r, sizeof(unsigned char), 1, fp) != 1) {
			fprintf(stderr, "Image read failed\n");
			return PSY_ERRFILEFORMAT;
		}
		if (fread(&self->colourindex[i].junk, sizeof(unsigned char), 1, fp) != 1) {
			fprintf(stderr, "Image read failed\n");
			return PSY_ERRFILEFORMAT;
		}
		fprintf(stderr, "%3d\t%3d\t%3d\t%3d\n", i,
			self->colourindex[i].r, self->colourindex[i].g,
			self->colourindex[i].b);
	}
	return PSY_OK;
}

int psy_ui_bmpreader_read_data_24(psy_ui_BmpReader* self, FILE* fp,
	psy_ui_Graphics* gc)
{
	int32_t padding;
	int32_t j;	
	uint8_t grey, r, g, b;
	
	/* 
	** Each scan line is zero padded to the nearest 4-byte boundary. If the image
	** has a width that is not divisible by four, say, 21 bytes, there would be
	** 3 bytes of padding at the end of every scan line.
	*/
	/* padding width: 4 bytes */
	/* num bytes per line: infoheader.width * 3 */
	padding = (int32_t)(ceil((self->infoheader.width * 3) / 4.0)) * 4 -
		(self->infoheader.width * 3);	
	for (j = self->infoheader.height - 1; j >= 0; j--) {
		int32_t i;

		for (i = 0; i < self->infoheader.width; i++) {
			psy_ui_Colour colour;
				
			if (fread(&b, sizeof(unsigned char), 1, fp) != 1) {
				fprintf(stderr, "Image read failed\n");				
				return PSY_ERRFILEFORMAT;
			}
			if (fread(&g, sizeof(unsigned char), 1, fp) != 1) {
				fprintf(stderr, "Image read failed\n");				
				return PSY_ERRFILEFORMAT;
			}
			if (fread(&r, sizeof(unsigned char), 1, fp) != 1) {
				fprintf(stderr, "Image read failed\n");				
				return PSY_ERRFILEFORMAT;
			}					
			psy_ui_colour_init_rgb(&colour, r, g, b);
			psy_ui_drawsolidrectangle(gc, psy_ui_realrectangle_make(
				psy_ui_realpoint_make(i, j), psy_ui_realsize_make(1.0, 1.0)),
				colour);
			if (self->verbose) {
				fprintf(stderr, "Point = %d , %d, %d, %d, %d\n", i, j,
					(int)r, (int)g, (int)b);
			}			
		} /* i */
		for (i = 0; i < padding; ++i) {
			fread(&r, sizeof(unsigned char), 1, fp);
		}
	} /* j */
	return PSY_OK;	
}

/*
	if (fread(&grey, sizeof(unsigned char), 1, fp) != 1) {
		fprintf(stderr, "Image read failed\n");
		exit(-1);
	}
	if (psy_ui_bmpreader_has_colour_map(self)) {
		putchar(self->colourindex[grey].r);
		putchar(self->colourindex[grey].g);
		putchar(self->colourindex[grey].b);
	} else {
		putchar(grey);
	}
	break;
}*/
