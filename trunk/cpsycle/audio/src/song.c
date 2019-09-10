// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "song.h"
#include <fileio.h>
#include "pattern.h"
#include "constants.h"
#include <datacompression.h>

static void loadpsy3(Song* self, RiffFile* file, char header[9]);
static void readsngi(Song*, RiffFile* file, int size, int version);
void readseqd(Song* song, RiffFile* file, int size, int version,
			  unsigned char* playorder, int* playlength);
static void readpatd(Song* song, RiffFile* file, int size, int version);
static void loadpsy2(Song* self, RiffFile* file);
static void initproperties(Song* self);

void song_init(Song* self)
{
	Pattern* pattern;

	self->properties = properties_create();	
	initproperties(self);
	machines_init(&self->machines);
	patterns_init(&self->patterns);
	sequence_init(&self->sequence, &self->patterns);
	samples_init(&self->samples);
	instruments_init(&self->instruments);
	xminstruments_init(&self->xminstruments);	
	pattern = (Pattern*) malloc(sizeof(Pattern));
	patterns_insert(&self->patterns, 0, pattern);
	pattern_init(pattern);	
	sequence_insert(&self->sequence, sequence_begin(&self->sequence, 0), 0);
}

void song_dispose(Song* self)
{
	properties_free(self->properties);
	self->properties = 0;	
	machines_dispose(&self->machines);
	patterns_dispose(&self->patterns);
	sequence_dispose(&self->sequence);		
	samples_dispose(&self->samples);
	instruments_dispose(&self->instruments);
	xminstruments_dispose(&self->xminstruments);	
}

void initproperties(Song* self)
{
	properties_append_string(self->properties, "title", "Untitled");	
	properties_append_string(self->properties, "credits", "Unnamed");
	properties_append_string(self->properties, "comments", "No Comments");
	properties_append_double(self->properties, "bpm", 125.0, 0, 999);
	properties_append_int(self->properties, "lpb", 4, 1, 999);
	properties_append_int(self->properties, "tracks", 16, 1, 64);
}

void song_load(Song* self, const char* path)
{	
	RiffFile file;

	if (rifffile_open(&file, (char*) path))
	{
		char header[9];

		sequence_clear(&self->sequence);
		patterns_clear(&self->patterns);
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
	int playlength;
	int chunkcount = 0;		
	long filesize = rifffile_filesize(file);
	unsigned char playorder[MAX_SONG_POSITIONS];

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
			readsngi(self, file, size, version);
		//	rifffile_skip(file, size);
		} else 
		if (strcmp(header,"SEQD")==0) {
			int curpos=0; curpos; // not used
			rifffile_read(file, &version,sizeof(version));
			rifffile_read(file, &size,sizeof(size));
			chunkcount--;			
			readseqd(self, file, size, version, playorder, &playlength);
		//	rifffile_skip(file, size);
		} else 
		if (strcmp(header,"PATD")==0) {
			int curpos=0; curpos; // not used
			rifffile_read(file, &version,sizeof(version));
			rifffile_read(file, &size,sizeof(size));
			chunkcount--;
			readpatd(self, file, size, version);
			// rifffile_skip(file, size);
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
	{
		int i;
		for (i = 0; i < playlength; ++i) {
			sequence_insert(&self->sequence, sequence_last(&self->sequence),
				playorder[i]);			
		}
	}
}

void readsngi(Song* song, RiffFile* file, int size, int version)
{		
	int i;
	int temp;
	int songtracks;
	float m_beatspermin;
	int m_linesperbeat;
	int currentoctave;
	int solo;
	int _tracksoloed;
	int seqbus;
	int paramselected;
	int auxcolselected;
	int instselected;
	unsigned char _trackmuted[64];
	int _trackarmedcount;
	unsigned char _trackarmed[64];
	int m_ticksperbeat;
	int m_extraticksperline;
	unsigned char sharetracknames;

	size_t begins = rifffile_getpos(file);
	if((version&0xFFFF0000) == VERSION_MAJOR_ZERO)
	{
		// why all these temps?  to make sure if someone changes the defs of
		// any of these members, the rest of the file reads ok.  assume 
		// everything is an int, when we write we do the same thing.

		// # of tracks for whole song
		rifffile_read(file, &temp, sizeof temp);
		songtracks = temp;
		song->patterns.songtracks = songtracks;
		// bpm
		{///\todo: this was a hack added in 1.9alpha to allow decimal bpm values
			int bpmcoarse;
			short temp16 = 0;
			rifffile_read(file, &temp16, sizeof temp16);
			bpmcoarse = temp16;
			rifffile_read(file, &temp16, sizeof temp16);
			m_beatspermin = bpmcoarse + (temp16 / 100.0f);			
			properties_write_double(song->properties, "bpm", m_beatspermin);
		}
		// linesperbeat
		rifffile_read(file, &temp, sizeof temp);
		m_linesperbeat = temp;
		properties_write_int(song->properties, "lpb", m_linesperbeat);
		// current octave
		rifffile_read(file, &temp, sizeof temp);
		currentoctave = temp;
		// machinesoloed
		// we need to buffer this because destroy machine will clear it
		rifffile_read(file, &temp, sizeof temp);
		solo = temp;
		// tracksoloed
		rifffile_read(file, &temp, sizeof temp);
		_tracksoloed = temp;
		rifffile_read(file, &temp, sizeof temp);  
		seqbus = temp;
		rifffile_read(file, &temp, sizeof temp);  
		paramselected = temp;
		rifffile_read(file, &temp, sizeof temp);  
		auxcolselected = temp;
		rifffile_read(file, &temp, sizeof temp);  
		instselected = temp;
		// sequence width, for multipattern
		rifffile_read(file, &temp,sizeof(temp));
		_trackarmedcount = 0;
		for(i = 0 ; i < songtracks; ++i)
		{
			rifffile_read(file, &_trackmuted[i],sizeof(_trackmuted[i]));
			// remember to count them
			rifffile_read(file, &_trackarmed[i],sizeof(_trackarmed[i]));
			if(_trackarmed[i]) ++_trackarmedcount;
		}
		m_ticksperbeat=24;
		m_extraticksperline=0;
		if(version == 0) {
			// fix for a bug existing in the song saver in the 1.7.x series
			size = (11 * sizeof(int)) + (songtracks * 2 * 1); //sizeof(bool));
		}
		if(version >= 1) {
			rifffile_read(file, &sharetracknames, 1);
			song->patterns.sharetracknames = sharetracknames;
			if( sharetracknames) {
				int t;
				for(t = 0; t < songtracks; t++) {
					char txt[40];
					rifffile_readstring(file, txt, 40);
					// changetrackname(0,t,name);
				}
			}
		}
		if (version >= 2) {
			rifffile_read(file, &temp, sizeof temp);
			m_ticksperbeat = temp;
			rifffile_read(file, &temp, sizeof temp);
			m_extraticksperline = temp;
		}
//		if (fullopen)
		{
			///\todo: warning! this is done here, because the plugins, when loading, need an up-to-date information.
			/// it should be coded in some way to get this information from the loading song, since doing it here
			/// is bad for the winamp plugin (or any other multi-document situation).
//			global::player().setbpm(beatspermin(), linesperbeat(), extraticksperline());
		}
	}
	rifffile_seek(file, begins + size);
}

void readseqd(Song* song, RiffFile* file, int size, int version,
			  unsigned char* playorder, int* playlength)
{	
	int temp;
	int index;	
	
	size_t begins = rifffile_getpos(file);
	if((version&0xffff0000) == VERSION_MAJOR_ZERO)
	{
		// index, for multipattern - for now always 0
		rifffile_read(file, &index, sizeof index);
		if (index < MAX_SEQUENCES)
		{
			char ptemp[256];
			int i;
			// play length for this sequence
			rifffile_read(file, &temp, sizeof temp);
			*playlength = temp;
			// name, for multipattern, for now unused
			rifffile_readstring(file, ptemp, sizeof ptemp);
			for (i = 0; i < *playlength; ++i)
			{
				rifffile_read(file, &temp, sizeof temp);
				playorder[i] = temp;
			}
		}
	}
	rifffile_seek(file, begins + size);
}

unsigned char * CreateNewPattern(void)
{	
	return malloc(MULTIPLY2);	
}

void readpatd(Song* song, RiffFile* file, int size, int version)
{
	size_t begins = rifffile_getpos(file);
	if((version&0xffff0000) == VERSION_MAJOR_ZERO)
	{
		int index;
		int temp;
		char patternname[MAX_PATTERNS][32];
		/// number of lines of each pattern
		int patternlines[MAX_PATTERNS];
		// index
		rifffile_read(file, &index, sizeof index);
		if(index < MAX_PATTERNS)
		{
			unsigned int sizez77 = 0;
			byte* psource;
			byte* pdest;
			int y;
			// num lines
			rifffile_read(file, &temp, sizeof temp );
			// clear it out if it already exists
//			removepattern(index);
			patternlines[index] = temp;
			// num tracks per pattern // eventually this may be variable per pattern, like when we get multipattern
			rifffile_read(file, &temp, sizeof temp );
			rifffile_readstring(file, patternname[index], sizeof *patternname);
			rifffile_read(file, &sizez77, sizeof sizez77);
			psource = (byte*) malloc(sizez77);
			rifffile_read(file, psource, sizez77);			
			beerz77decomp2(psource, &pdest);
			free(psource);
			{				
				Pattern* pattern = (Pattern*) malloc(sizeof(Pattern));				
				PatternNode* node = 0;
				float offset = 0.f;

				psource = pdest;
				pattern_init(pattern);
				patterns_insert(&song->patterns, index, pattern);									
				for(y = 0 ; y < patternlines[index] ; ++y)
				{
					unsigned char* pdata = CreateNewPattern() + (y * MULTIPLY);
					unsigned char* ptrack = pdata;
					int track;
					memcpy(pdata, psource, song->patterns.songtracks * EVENT_SIZE);
					for (track = 0; track < song->patterns.songtracks; ++track) {
						PatternEvent* event = (PatternEvent*)(ptrack);
						if (event->note != 255) {
							node = pattern_insert(pattern, node, track, offset, event);
						}
						ptrack += sizeof(PatternEvent);
					}					
					offset += 0.25;
					psource += song->patterns.songtracks * EVENT_SIZE;
				}
				pattern->length = 64 * 0.25;								
				free(pdest);
				pdest = 0;			
			}
		}
		//\ fix for a bug existing in the song saver in the 1.7.x series
		if((version == 0x0000) &&( rifffile_getpos(file) == begins+size+4)) size += 4;
		if(version > 0) {
			if( !song->patterns.sharetracknames) {
				int t;
				for(t = 0; t < song->patterns.songtracks; t++) {
					char name[40];
					rifffile_readstring(file, name, 40);
					// changetrackname(index,t,name);
				}
			}
		}
	}
	rifffile_seek(file, begins + size);
}

void loadpsy2(Song* self, RiffFile* file)
{

}