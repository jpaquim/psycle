/*
 * zipwriter
 * a library for writing zipfiles
 *
 * copyright (c) 2006 Mrs. Brisby <mrs.brisby@nimh.org>
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
#ifndef __ZIPwriter_h
#define __ZIPwriter_h

#include <sys/types.h>
#include <time.h>

#ifdef __cplusplus
 extern "C" {
#endif

#ifndef ZW_BUFSIZE
#define ZW_BUFSIZE 65536
#endif

typedef struct zipwriter zipwriter;
typedef struct zipwriter_method zipwriter_method;
typedef struct zipwriter_file zipwriter_file;

struct zipwriter {
	int fd;
	int err;
	zipwriter_file *top, *cur;

	unsigned char *tail;
	void *comment;
	size_t comment_length;

	unsigned char buffer[ZW_BUFSIZE];

	/* may grow in the future... */
	void *compressor_state[5];
};

struct zipwriter_method {
	unsigned int num;
	void (*start)(zipwriter_file *f);
	void (*chunk)(zipwriter_file *f, const void *buf, unsigned int len);
	void (*finish)(zipwriter_file *f);
};

struct zipwriter_file {
	time_t mtime;
	unsigned char *head;
	unsigned char *head2;
	unsigned int head2_size;

	off_t head_off;
	char *filename;
	unsigned int crc32;
	unsigned int len, wlen;
	unsigned int flags;

	zipwriter_file *next, *prev;
	zipwriter_method *proc;
	zipwriter *top;
};

/* outfd must be opened for reading+writing, be seekable, and must be empty.
 * this means that you probably should have opened it with:
 * 	open(..., O_CREAT|O_EXCL|O_RDWR)
 * it should also probably be a temporary file so that you can rename() it
 * over the target filename thus making your file-save operations crashproof.
 *
 * outfd must not be used (directly) again.
 */
zipwriter *zipwriter_start(int outfd);

/* set the global zipfile comment.
 *
 * this MAY be disturbed by various zipfile reading/writing tools, so you
 * should not put anything in here that you need to get back.
 *
 * if it fails to put the comment in, zipwriter_finish() will return an
 * error.
 */
void zipwriter_comment(zipwriter *z, const void *buf, size_t length);

/* starts adding a file. the directory entry isn't actually written until the
 * end of the curent file.
 *
 * zipwriter_file->mtime will be initialized to "now" you can change this.
 *
 * the name must be the path relative to the root of the zipfile directory,
 * separated by forward-slashes -- EVEN on platforms that use a backslash.
 *
 */
zipwriter_file *zipwriter_addfile(zipwriter *d, const char *name,
		unsigned int flags);
#define ZIPWRITER_STORE		0
#define ZIPWRITER_DEFLATE	8
#define ZIPWRITER_COMPRESS_MASK	0xffff

/* writes a chunk of data to the current file. returns zero if f isn't
 * opened, if it isn't the current file, or if write() fails. when
 * possible, f->top->err will be set, and it can be detected at the
 * end of the zipwriter process
 */
int zipwriter_write(zipwriter_file *f, const void *buf, size_t len);

/* finishes the current zipfile, adding a comment if requested, and
 * writing the zip index. this closes the file descriptor, and fsync()s
 * it, so when this function returns nonzero, the file is on the disk.
 * if this function returns zero, either it failed to do this, or
 * a zipwriter_write() failed earlier.
 */
int zipwriter_finish(zipwriter *d);

/* copies a file into a zip
*/

void zipwriter_copy(int in, zipwriter_file *out);

#ifdef __cplusplus
 };
#endif

#endif
