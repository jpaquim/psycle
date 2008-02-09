/* -*- mode:c++, indent-tabs-mode:t -*- */
/*
	* zipreader
	* a library for reading zipfiles
	*
	* copyright (c) 2007 Mrs. Brisby <mrs.brisby@nimh.org>
	*
	* This program is free software; you can redistribute it and/or modify
	* it under the terms of the GNU General Public License as published by
	* the Free Software Foundation; either version 2 of the License, or
	* (at your option) any later version.
	*
	* This program is distributed in the hope that it will be useful,
	* but WITHOUT ANY WARRANTY; without even the implied warranty of
	* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	* GNU General Public License for more details.
	*
	* You should have received a copy of the GNU General Public License
	* along with this program; if not, write to the Free Software
	* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
	*/
#ifndef __ZIPread_h
#define __ZIPread_h

#include <sys/types.h>

typedef struct zipreader zipreader;
typedef struct zipreader_file zipreader_file;

struct zipreader_file {
	off_t pos;
	unsigned int csize;
	unsigned int esize;
	unsigned int gpbits;
	unsigned int crc32;
	unsigned int method;

	unsigned char *filename_ptr;

	zipreader *top;
};

struct zipreader {
	int fd;
	unsigned int files;
	unsigned int _fnp;
	off_t *headers; /* pointers to local headers */
	zipreader_file file[1]; /* array */
};

#ifdef __cplusplus
extern "C" {
#endif

/* open a zipreader for a file; the fd must be seekable.
	*/
zipreader *zipreader_open(int fd);

/* attempts to find filename in the zipfile, and returns the
	* appropriate zipreader_file. this also positions z->fd at
	* the start of the compressed file
	*/
zipreader_file *zipreader_seek(zipreader *z, const char *filename);

/* performs an extraction on the file pointed to by f, to the
	* output filedescriptor "outfd"
	*
	* if this routine returns zero, outfd probably doesn't contain
	* the file that was zipped up originally...
	*/
int zipreader_extract(zipreader_file *f, int outfd);

/* close the zipfile; this doesn't close() the underlying fd, but
	* frees all the zipreader_file* structures returned by zipreader_seek()
	* as well as any other allocations that this zipreader performed for
	* previous extractions...
	*/
void zipreader_close(zipreader *z);

#ifdef __cplusplus
};
#endif

#endif
