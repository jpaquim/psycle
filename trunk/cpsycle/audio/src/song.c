// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "song.h"
#include <fileio.h>
#include "pattern.h"

static void loadpsy3(Song* self, RiffFile* file, char header[9]);
static void loadpsy2(Song* self, RiffFile* file);

void song_init(Song* self)
{
	Pattern* pattern;

	self->properties = properties_create();	
	machines_init(&self->machines);
	patterns_init(&self->patterns);
	sequence_init(&self->sequence, &self->patterns);
	pattern = (Pattern*) malloc(sizeof(Pattern));
	patterns_insert(&self->patterns, 0, pattern);
	pattern_init(pattern);
	sequence_insert(&self->sequence, 0, 0);
}

void song_dispose(Song* self)
{
	properties_free(self->properties);
	self->properties = 0;	
	machines_dispose(&self->machines);
	patterns_dispose(&self->patterns);
	sequence_dispose(&self->sequence);		
}

void song_load(Song* self, const char* path)
{	
	RiffFile file;

	if (rifffile_open(&file, (char*) path))
	{
		char header[9];
		rifffile_read(&file, header, 8);
		header[8]=0;		
		if (strcmp(header,"PSY3SONG")==0) {			
			loadpsy3(self, &file, header);
		} else
		if (strcmp(header,"PSY2SONG")==0) {
			loadpsy2(self, &file);
		} else {

		}
		rifffile_close(&file);
	}
}

void loadpsy3(Song* self, RiffFile* file, char header[9])
{
	unsigned int version = 0;
	unsigned int size = 0;
	unsigned int index = 0;
//	int temp;
	int solo = 0;
	int chunkcount = 0;		
	long filesize = rifffile_filesize(file);
	rifffile_read(file, &version,sizeof(version));
	rifffile_read(file, &size,sizeof(size));
	rifffile_read(file, &chunkcount,sizeof(chunkcount));
	rifffile_skip(file, size-sizeof(chunkcount));

	header[4]=0;
	while(rifffile_read(file, header, 4)) {
		if (strcmp(header,"INFO")==0) {
			int curpos=0; 
			size_t begins;
			curpos; // not used
			rifffile_read(file, &version,sizeof(version));
			rifffile_read(file, &size,sizeof(size));
			begins = rifffile_getpos(file);
		//	if((version&0xFFFF0000) == VERSION_MAJOR_ZERO)
		//	{
			{
				char name_[129]; char author_[65]; char comments_[65536];
				rifffile_readstring(file, name_, sizeof name_);
				rifffile_readstring(file, author_, sizeof author_);
				rifffile_readstring(file, comments_,sizeof comments_);				
				//bugfix. There were songs with incorrect size.
				if(version == 0) {
				//	size= (UINT)(rifffile_getpos(file) - begins);
				}
			}
		//	}
			chunkcount--;
		//	rifffile_skip(file, begins + size);
		} else 
		if (strcmp(header,"SNGI")==0) {
			int curpos=0; curpos; // not used
			rifffile_read(file, &version,sizeof(version));
			rifffile_read(file, &size,sizeof(size));
			chunkcount--;
			rifffile_skip(file, size);
		} else 
		if (strcmp(header,"SEQD")==0) {
			int curpos=0; curpos; // not used
			rifffile_read(file, &version,sizeof(version));
			rifffile_read(file, &size,sizeof(size));
			chunkcount--;
			rifffile_skip(file, size);
		} else 
		if (strcmp(header,"PATD")==0) {
			int curpos=0; curpos; // not used
			rifffile_read(file, &version,sizeof(version));
			rifffile_read(file, &size,sizeof(size));
			chunkcount--;
			rifffile_skip(file, size);
		} else
		if (strcmp(header,"MACD")==0) {
			int curpos=0; curpos; // not used
			rifffile_read(file, &version,sizeof(version));
			rifffile_read(file, &size,sizeof(size));
			chunkcount--;
			rifffile_skip(file, size);
		} else
		if (strcmp(header,"INSD")==0) {
			int curpos=0; curpos; // not used
			rifffile_read(file, &version,sizeof(version));
			rifffile_read(file, &size,sizeof(size));
			chunkcount--;
			rifffile_skip(file, size);
		} else {
			// we are not at a valid header for some weird reason.  
			// probably there is some extra data.
			// shift back 3 bytes and try again
			rifffile_skip(file, -3);
		}
	}
}

void loadpsy2(Song* self, RiffFile* file)
{

}