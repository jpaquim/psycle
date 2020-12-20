// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "psy3loader.h"
// local
#include "constants.h"
#include "plugin_interface.h"
#include "song.h"
#include "songio.h"
#include "machinefactory.h"
#include "wire.h"
// dsp
#include <envelope.h>
#include <datacompression.h>
#include <operations.h>
// file
#include <dir.h>
// platform
#include "../../detail/portable.h"

#if !defined DIVERSALIS__OS__MICROSOFT
#define _MAX_PATH 4096
#endif

static void psy_audio_psy3loader_read_info(psy_audio_PSY3Loader*);
static void psy_audio_psy3loader_read_sngi(psy_audio_PSY3Loader*);
static void psy_audio_psy3loader_read_seqd(psy_audio_PSY3Loader*);
static void psy_audio_psy3loader_read_patd(psy_audio_PSY3Loader*);
static void psy_audio_psy3loader_read_epat(psy_audio_PSY3Loader*);
static void psy_audio_psy3loader_read_insd(psy_audio_PSY3Loader*);
static void psy_audio_psy3loader_read_eins(psy_audio_PSY3Loader*);
static void psy_audio_psy3loader_read_smid(psy_audio_PSY3Loader*);
static void psy_audio_psy3loader_read_macd(psy_audio_PSY3Loader*);
static void psy_audio_psy3loader_read_smsb(psy_audio_PSY3Loader*);
static void psy_audio_psy3loader_read_virg(psy_audio_PSY3Loader*);
static void psy_audio_psy3loader_loadxminstrument(psy_audio_PSY3Loader*, psy_audio_Instrument* instrument,
	bool islegacy, uint32_t legacyversion);
static void psy_audio_psy3loader_xminstrumentenvelopeload(psy_audio_PSY3Loader*, bool legacy, uint32_t legacyversion);
static psy_audio_Sample* psy_audio_psy3loader_xmloadwav(psy_audio_PSY3Loader*);
static void psy_audio_psy3loader_loadwavesubchunk(psy_audio_PSY3Loader*, int32_t instrIdx,
	int32_t pan, char * instrum_name, int32_t fullopen, int32_t loadIdx);
static psy_audio_Machine* psy_audio_psy3loader_machineloadchunk(psy_audio_PSY3Loader*,
	int32_t index);
static psy_audio_Machine* psy_audio_psy3loader_machineloadchunk_createmachine(psy_audio_PSY3Loader*,
	int32_t index, char* modulename, char* catchername, bool* replaced);
static void psy_audio_psy3loader_setinstrumentnames(psy_audio_PSY3Loader*);
static void psy_audio_psy3loader_postload(psy_audio_PSY3Loader*);

void psy_audio_psy3loader_init(psy_audio_PSY3Loader* self, psy_audio_SongFile* songfile)
{
	assert(self && songfile && songfile->song);
	psy_audio_legacywires_init(&self->legacywires);
	self->songfile = songfile;
}

void psy_audio_psy3loader_dispose(psy_audio_PSY3Loader* self)
{
	psy_audio_legacywires_dispose(&self->legacywires);
}

//	===================
//	PSY3 SONG LOAD
//	===================
int psy_audio_psy3loader_load(psy_audio_PSY3Loader* self)
{
	char header[9];	
	uint32_t temp32;
	uint32_t index = 0;
	int32_t solo = 0;
	int32_t chunkcount = 0;		
	int32_t progress = 0;
	uint32_t filesize = psyfile_filesize(self->songfile->file);		
	self->songfile->legacywires = &self->legacywires;

	psyfile_read(self->songfile->file, &temp32, sizeof(temp32));
	self->songfile->file->fileversion = temp32;
	psyfile_read(self->songfile->file, &temp32, sizeof(temp32));
	self->songfile->file->filesize = temp32;
	if(self->songfile->file->fileversion > CURRENT_FILE_VERSION) {
		psy_audio_songfile_warn(self->songfile, 
			"This file is from a newer version of Psycle! "
			"This process will try to load it anyway.\n");
	}	
	psyfile_read(self->songfile->file, &self->songfile->file->chunkcount, sizeof(uint32_t));
	if (self->songfile->file->fileversion >= 8) {
		char trackername[256];
		char trackervers[256];
		int32_t bytesread;

		psyfile_readstring(self->songfile->file, trackername, 256);
		psyfile_readstring(self->songfile->file, trackervers, 256);
		bytesread = 4 + strlen(trackername)+strlen(trackervers)+2;
		// Size of the current Header DATA
		// This ensures that any extra data is skipped.
		psyfile_skip(self->songfile->file, self->songfile->file->filesize - bytesread);
	}	
	// start reading chunks		
	header[4] = 0;
	while(psyfile_read(self->songfile->file, header, 4) && self->songfile->file->chunkcount) {
		if (strcmp(header,"INFO") == 0) {			
			psyfile_readchunkbegin(self->songfile->file);
			psy_audio_psy3loader_read_info(self);
			psyfile_seekchunkend(self->songfile->file);
			progress += 1;
		} else 
		if (strcmp(header,"SNGI")==0) {
			psyfile_readchunkbegin(self->songfile->file);
			psy_audio_psy3loader_read_sngi(self);
			progress += 1;
			psyfile_seekchunkend(self->songfile->file);
		} else 
		if (strcmp(header,"SEQD")==0) {
			psyfile_readchunkbegin(self->songfile->file);
			psy_audio_psy3loader_read_seqd(self);
			psyfile_seekchunkend(self->songfile->file);
			progress += 1;
		} else 
		if (strcmp(header,"PATD")==0) {
			psyfile_readchunkbegin(self->songfile->file);
			psy_audio_psy3loader_read_patd(self);
			psyfile_seekchunkend(self->songfile->file);
			progress += 1;
		} else
		if (strcmp(header,"MACD")==0) {
			psyfile_readchunkbegin(self->songfile->file);
			psy_audio_psy3loader_read_macd(self);
			psyfile_seekchunkend(self->songfile->file);
			progress += 1;
		} else
		if (strcmp(header,"INSD")==0) {
			psyfile_readchunkbegin(self->songfile->file);
			psy_audio_psy3loader_read_insd(self);
			psyfile_seekchunkend(self->songfile->file);
			progress += 1;
		} else
		if (strcmp(header,"EINS")==0) {
			psyfile_readchunkbegin(self->songfile->file);
			psy_audio_psy3loader_read_eins(self);
			psyfile_seekchunkend(self->songfile->file);
		} else
		if (strcmp(header,"SMID") == 0) {
			psyfile_readchunkbegin(self->songfile->file);
			psy_audio_psy3loader_read_smid(self);
			psyfile_seekchunkend(self->songfile->file);
		} else
		if (strcmp(header,"SMSB") == 0) {
			psyfile_readchunkbegin(self->songfile->file);
			psy_audio_psy3loader_read_smsb(self);
			psyfile_seekchunkend(self->songfile->file);
		} else
		if (strcmp(header,"VIRG") == 0) {
			psyfile_readchunkbegin(self->songfile->file);
			psy_audio_psy3loader_read_virg(self);
			psyfile_seekchunkend(self->songfile->file);
		} else {
			// we are not at a valid header for some weird reason.  
			// probably there is some extra data.
			// shift back 3 bytes and try again			
			psyfile_skip(self->songfile->file, -3);
			// this makes it impossible to add new chunks
			// todo: read header and skip the chunk
			// psyfile_readchunkbegin(self->file);
			// psyfile_seekchunkend(self->file);			
		}
		psy_signal_emit(&self->songfile->song->signal_loadprogress, self, 1,
			progress);
	}	
	psy_audio_reposition(&self->songfile->song->sequence);
	psy_audio_psy3loader_setinstrumentnames(self);
	psy_audio_psy3loader_postload(self);
	return self->songfile->err;
}

void psy_audio_psy3loader_postload(psy_audio_PSY3Loader* self)
{					
	psy_TableIterator it;
	
	for (it = psy_audio_machines_begin(&self->songfile->song->machines);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {		
		psy_audio_machine_postload(
			(psy_audio_Machine*)psy_tableiterator_value(&it),
			self->songfile,
			psy_tableiterator_key(&it));
	}		
}

void psy_audio_psy3loader_setinstrumentnames(psy_audio_PSY3Loader* self)
{
	psy_TableIterator it;

	for (it = psy_audio_instruments_groupbegin(&self->songfile->song->instruments, 0);
			!psy_tableiterator_equal(&it, psy_table_end()); psy_tableiterator_inc(&it)) {
		psy_audio_Instrument* instrument;
		psy_audio_Sample* sample;

		instrument = psy_tableiterator_value(&it);
		sample = psy_audio_samples_at(&self->songfile->song->samples,
			sampleindex_make(psy_tableiterator_key(&it), 0));
		if (sample) {
			psy_audio_instrument_setname(instrument, psy_audio_sample_name(sample));
		}
	}
}

void psy_audio_psy3loader_read_info(psy_audio_PSY3Loader* self)
{
	if((self->songfile->file->currchunk.version&0xFFFF0000) == VERSION_MAJOR_ZERO)
	{		
		char name_[129];
		char author_[65];
		char comments_[65536];
		psy_audio_SongProperties songproperties;

		psyfile_readstring(self->songfile->file, name_, sizeof name_);
		psyfile_readstring(self->songfile->file, author_, sizeof author_);
		psyfile_readstring(self->songfile->file, comments_,sizeof comments_);
		psy_audio_songproperties_init(&songproperties, name_, author_, comments_);
		psy_audio_song_setproperties(self->songfile->song, &songproperties);
		//bugfix. There were songs with incorrect size.
		if(self->songfile->file->currchunk.version == 0) {
			self->songfile->file->currchunk.size =
				(uint32_t)(psyfile_getpos(self->songfile->file) -
					self->songfile->file->currchunk.begins);
		}
	}
}

void psy_audio_psy3loader_read_sngi(psy_audio_PSY3Loader* self)
{		
	int32_t i;
	int32_t temp;
	int32_t songtracks;		
	int32_t currentoctave;
	int32_t _tracksoloed;
	int32_t seqbus;
	int32_t paramselected;
	int32_t auxcolselected;
	int32_t instselected;
	unsigned char _trackmuted[64];
	int32_t _trackarmedcount;
	unsigned char _trackarmed[64];	
	unsigned char sharetracknames;
	
	if((self->songfile->file->currchunk.version&0xFFFF0000) == VERSION_MAJOR_ZERO)
	{
		// why all these temps?  to make sure if someone changes the defs of
		// any of these members, the rest of the file reads ok.  assume 
		// everything is an int32_t, when we write we do the same thing.

		// # of tracks for whole song
		psyfile_read(self->songfile->file, &temp, sizeof temp);
		songtracks = temp;
		psy_audio_patterns_setsongtracks(&self->songfile->song->patterns, songtracks);
		// bpm
		{///\todo: this was a hack added in 1.9alpha to allow decimal bpm values
			int32_t bpmcoarse;
			short temp16 = 0;
			psyfile_read(self->songfile->file, &temp16, sizeof temp16);
			bpmcoarse = temp16;
			psyfile_read(self->songfile->file, &temp16, sizeof temp16);
			self->songfile->song->properties.bpm = bpmcoarse + (temp16 / 100.0f);
		}
		// linesperbeat
		psyfile_read(self->songfile->file, &temp, sizeof temp);
		self->songfile->song->properties.lpb = temp;
		// current octave
		psyfile_read(self->songfile->file, &temp, sizeof temp);
		currentoctave = temp;
		// machinesoloed
		// we need to buffer this because destroy machine will clear it
		psyfile_read(self->songfile->file, &temp, sizeof temp);
		if (temp >= 0) {
			self->songfile->machinesoloed = (uint32_t) temp;
		} else {
			self->songfile->machinesoloed = UINTPTR_MAX;
		}
		// tracksoloed
		psyfile_read(self->songfile->file, &temp, sizeof temp);
		_tracksoloed = temp;
		psyfile_read(self->songfile->file, &temp, sizeof temp);
		seqbus = temp;
		psyfile_read(self->songfile->file, &temp, sizeof temp);
		paramselected = temp;
		psyfile_read(self->songfile->file, &temp, sizeof temp);
		auxcolselected = temp;
		psyfile_read(self->songfile->file, &temp, sizeof temp);
		instselected = temp;
		// sequence width, for multipattern
		psyfile_read(self->songfile->file, &temp,sizeof(temp));
		_trackarmedcount = 0;
		for(i = 0 ; i < songtracks; ++i)
		{
			psyfile_read(self->songfile->file, &_trackmuted[i],sizeof(_trackmuted[i]));
			// remember to count them
			psyfile_read(self->songfile->file, &_trackarmed[i],sizeof(_trackarmed[i]));
			if(_trackarmed[i]) ++_trackarmedcount;
		}
		if(self->songfile->file->currchunk.version == 0) {
			// fix for a bug existing in the song saver in the 1.7.x series
			self->songfile->file->currchunk.size = (11 * sizeof(int32_t)) + (songtracks * 2 * 1); //sizeof(bool));
		}
		if(self->songfile->file->currchunk.version >= 1) {
			psyfile_read(self->songfile->file, &sharetracknames, 1);
			self->songfile->song->patterns.sharetracknames = sharetracknames;
			if( sharetracknames) {
				int32_t t;
				for(t = 0; t < songtracks; t++) {
					char txt[40];
					psyfile_readstring(self->songfile->file, txt, 40);
					// changetrackname(0,t,name);
				}
			}
		}
		if (self->songfile->file->currchunk.version >= 2) {
			psyfile_read(self->songfile->file, &temp, sizeof temp);
			self->songfile->song->properties.tpb = temp;
			psyfile_read(self->songfile->file, &temp, sizeof temp);
			self->songfile->song->properties.extraticksperbeat = temp;
		} else {
			self->songfile->song->properties.tpb = 24;
			self->songfile->song->properties.extraticksperbeat = 0;
		}
//		if (fullopen)
		{
			///\todo: warning! this is done here, because the plugins, when loading, need an up-to-date information.
			/// it should be coded in some way to get this information from the loading song, since doing it here
			/// is bad for the winamp plugin (or any other multi-document situation).
//			global::player().setbpm(beatspermin(), linesperbeat(), extraticksperline());
		}
	}	
}

void psy_audio_psy3loader_read_seqd(psy_audio_PSY3Loader* self)
{					
	if ((self->songfile->file->currchunk.version & 0xffff0000) == VERSION_MAJOR_ZERO)
	{
		psy_audio_SequenceTrackNode* t;		
		int32_t index;
		uint32_t i;
		char ptemp[256];
		int32_t playlength;

		// index, for multipattern - for now always 0
		psyfile_read(self->songfile->file, &index, sizeof(index));
		if (index < 0) {
			return; // skip
		}
		// create new tracks (0..index), if not already existing
		for (i = 0, t = self->songfile->song->sequence.tracks; i <= (uint32_t)index; ++i) {
			if (!t) {
				t = psy_audio_sequence_appendtrack(&self->songfile->song->sequence,
					psy_audio_sequencetrack_allocinit());
			} else if (i < (uint32_t)index) {
				psy_list_next(&t);
			}
		}
		assert(t);
		// play length for this sequence
		playlength = psyfile_read_int32(self->songfile->file);
		if (playlength < 0) {
			return; // skip
		}
		// name, for multipattern, for now unused
		psyfile_readstring(self->songfile->file, ptemp, sizeof ptemp);
		for (i = 0; i < (uint32_t)playlength; ++i) {
			uint8_t patternslot;
			psy_audio_SequencePosition sequenceposition;

			sequenceposition.tracknode = t;
			sequenceposition.trackposition =
				psy_audio_sequence_last(&self->songfile->song->sequence, t);
			patternslot = (uint8_t)psyfile_read_uint32(self->songfile->file);
			psy_audio_sequence_insert(&self->songfile->song->sequence, sequenceposition,
				patternslot);
		}
		if (self->songfile->file->currchunk.version > 0) {
			// this extends the seqd chunk file format to handle 
			// beat positions
			psy_audio_SequenceEntryNode* p;
			psy_audio_SequenceTrack* track;

			track = (psy_audio_SequenceTrack*)psy_list_entry(t);
			for (p = track->entries, i = 0; i < (uint32_t)playlength && p != NULL;
					++i, psy_list_next(&p)) {
				float repositionoffset;
				psy_audio_SequenceEntry* sequenceentry;
				
				sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
				repositionoffset = (float)psyfile_read_float(self->songfile->file);
				sequenceentry->repositionoffset = repositionoffset;
			}			
		}
	}	
}

void psy_audio_psy3loader_read_patd(psy_audio_PSY3Loader* self)
{		
	if((self->songfile->file->currchunk.version&0xffff0000) == VERSION_MAJOR_ZERO)
	{		
		int32_t index;
		int32_t temp;
		int32_t lpb;
		psy_dsp_big_beat_t bpl;
		char patternname[MAX_PATTERNS][32];
		/// number of lines of each pattern
		int32_t patternlines[MAX_PATTERNS];
		int32_t songtracks;
		// index

		lpb = self->songfile->song->properties.lpb;
		bpl = 1 / (psy_dsp_big_beat_t) lpb;
		psyfile_read(self->songfile->file, &index, sizeof index);
		if (index < MAX_PATTERNS)
		{
			uint32_t sizez77 = 0;
			byte* psource;
			byte* pdest;
			int32_t y;
			size_t destsize;
			// num lines
			psyfile_read(self->songfile->file, &temp, sizeof temp);
			// clear it out if it already exists
//			removepattern(index);
			patternlines[index] = temp;
			// num tracks per pattern // eventually this may be variable per pattern, like when we get multipattern
			psyfile_read(self->songfile->file, &songtracks, sizeof temp);
			psyfile_readstring(self->songfile->file, patternname[index], sizeof * patternname);
			psyfile_read(self->songfile->file, &sizez77, sizeof sizez77);
			if (self->songfile->file->currchunk.version > 1) {
				psyfile_skip(self->songfile->file, sizez77);
			} else {
				psource = (byte*)malloc(sizez77);
				psyfile_read(self->songfile->file, psource, sizez77);
				beerz77decomp2(psource, &pdest, &destsize);
				free(psource);
				// songtracks = patternlines[index] > 0 ? destsize / ((size_t)patternlines[index] * EVENT_SIZE) : 0;
				{
					psy_audio_Pattern* pattern;
					psy_audio_PatternNode* node = 0;

					psource = pdest;
					pattern = psy_audio_pattern_allocinit();
					psy_audio_patterns_insert(&self->songfile->song->patterns, index, pattern);
					psy_audio_pattern_setname(pattern, patternname[index]);
					for (y = 0; y < patternlines[index]; ++y) {
						unsigned char* ptrack = psource;
						uint32_t track;
						psy_dsp_big_beat_t offset;

						offset = bpl * y;
						for (track = 0; track < psy_audio_patterns_songtracks(&self->songfile->song->patterns);
							++track) {
							psy_audio_PatternEvent event;
							// Psy3 PatternEntry format
							// type				offset
							// uint8_t note;		0
							// uint8_t inst;		1
							// uint8_t mach;		2
							// uint8_t cmd;			3
							// uint8_t parameter;	4												
							psy_audio_patternevent_clear(&event);
							event.note = ptrack[0];
							event.inst = (ptrack[1] == 0xFF)
								? event.inst = psy_audio_NOTECOMMANDS_INST_EMPTY
								: ptrack[1];
							event.mach = (ptrack[2] == 0xFF)
								? event.mach = psy_audio_NOTECOMMANDS_MACH_EMPTY
								: ptrack[2];
							event.cmd = ptrack[3];
							event.parameter = ptrack[4];
							if (!psy_audio_patternevent_empty(&event)) {
								node = psy_audio_pattern_insert(pattern, node, track, offset,
									&event);
							}
							ptrack += EVENT_SIZE;
						}
						psource += self->songfile->song->patterns.songtracks * EVENT_SIZE;
					}
					pattern->length = patternlines[index] * bpl;
					free(pdest);
					pdest = 0;
				}
				//\ fix for a bug existing in the song saver in the 1.7.x series
				if ((self->songfile->file->currchunk.version == 0x0000) && psyfile_getpos(self->songfile->file) ==
					self->songfile->file->currchunk.begins + self->songfile->file->currchunk.size + 4) {
					self->songfile->file->currchunk.size += 4;
				}
			}			
		}
		if (self->songfile->file->currchunk.version > 0) {
			if (!self->songfile->song->patterns.sharetracknames) {
				uint32_t t;
				for (t = 0; t < self->songfile->song->patterns.songtracks; t++) {
					char name[40];
					psyfile_readstring(self->songfile->file, name, 40);
					// changetrackname(index,t,name);
				}
			}
		}
		if (self->songfile->file->currchunk.version > 1) {
			psy_audio_psy3loader_read_epat(self);
		}
	}		
}

void psy_audio_psy3loader_read_epat(psy_audio_PSY3Loader* self)
{		
	if((self->songfile->file->currchunk.version&0xffff0000) == VERSION_MAJOR_ZERO)
	{
		int32_t index;
		int32_t temp;
		float ftemp;				
		char patternname[MAX_PATTERNS][32];		
		uint32_t numpatterns;
		uint32_t numentries;
		uint32_t c;
		uint32_t i;		
			
		psyfile_read(self->songfile->file, &temp, sizeof(temp));
		numpatterns = temp;
		for (i = 0; i < numpatterns; ++i) {
			psy_audio_Pattern* pattern;

			pattern = psy_audio_pattern_allocinit();

			psyfile_read(self->songfile->file, &temp, sizeof(temp));
			index = temp;			
			psy_audio_patterns_insert(&self->songfile->song->patterns, index, pattern);
			// pattern length
			psyfile_read(self->songfile->file, &ftemp, sizeof ftemp);
			psy_audio_pattern_setlength(pattern, ftemp);
			// num tracks per pattern // eventually this may be variable per pattern, like when we get multipattern
			psyfile_read(self->songfile->file, &temp, sizeof temp );
			psyfile_readstring(self->songfile->file, patternname[index], sizeof *patternname);
			// num entries
			psyfile_read(self->songfile->file, &temp, sizeof temp);
			numentries = temp;
			for (c = 0; c < numentries; ++c) {
				psy_audio_PatternEntry* entry;
				uint32_t numevents;
				uint32_t j;

				entry = psy_audio_patternentry_alloc();
				entry->events = 0;
				// read track
				psyfile_read(self->songfile->file, &temp, sizeof(temp));
				entry->track = temp;
				// read offset
				psyfile_read(self->songfile->file, &ftemp, sizeof(temp));
				entry->offset = ftemp;				
				// num events
				psyfile_read(self->songfile->file, &temp, sizeof(temp));
				numevents = temp;
				// read events
				for (j = 0; j < numevents; ++j) {
					psy_audio_PatternEvent ev;

					psy_audio_patternevent_clear(&ev);
					psyfile_read(self->songfile->file, &temp, sizeof(temp));
					ev.note = temp;
					psyfile_read(self->songfile->file, &temp, sizeof(temp));
					ev.inst = temp;
					psyfile_read(self->songfile->file, &temp, sizeof(temp));
					ev.mach = temp;
					psyfile_read(self->songfile->file, &temp, sizeof(temp));
					ev.vol = temp;
					psyfile_read(self->songfile->file, &temp, sizeof(temp));
					ev.cmd = temp;
					psyfile_read(self->songfile->file, &temp, sizeof(temp));
					ev.parameter = temp;
					psy_audio_patternentry_addevent(entry, &ev);
				}
				psy_list_append(&pattern->events, entry);
			}			
		}
	}		
}

void psy_audio_psy3loader_read_insd(psy_audio_PSY3Loader* self)
{	


	///\verbatim
	/// NNA values overview:
	///
	/// 0 = Note Cut      [Fast Release 'Default']
	/// 1 = Note Release  [Release Stage]
	/// 2 = Note Continue [No NNA]
	///\endverbatim
	unsigned char _NNA;


	int32_t sampler_to_use = -1; // psy_audio_XMSampler machine index for lockinst.
	unsigned char _LOCKINST;	// Force this instrument number to change the selected machine to use a specific sampler when editing (i.e. when using the pc or midi keyboards, not the notes already existing in a pattern)

	///\name Amplitude Envelope overview:
	///\{
	/// Attack Time [in psy_audio_Samples at 44.1Khz, independently of the real samplerate]
	int32_t ENV_AT;	
	/// Decay Time [in psy_audio_Samples at 44.1Khz, independently of the real samplerate]
	int32_t ENV_DT;	
	/// Sustain Level [in %]
	int32_t ENV_SL;	
	/// Release Time [in psy_audio_Samples at 44.1Khz, independently of the real samplerate]
	int32_t ENV_RT;	
	///\}
	
	///\name psy_dsp_Filter 
	///\{
	/// Attack Time [in psy_audio_Samples at 44.1Khz]
	int32_t ENV_F_AT;	
	/// Decay Time [in psy_audio_Samples at 44.1Khz]
	int32_t ENV_F_DT;	
	/// Sustain Level [0..128]
	int32_t ENV_F_SL;	
	/// Release Time [in psy_audio_Samples at 44.1Khz]
	int32_t ENV_F_RT;	

	/// Cutoff Frequency [0-127]
	int32_t ENV_F_CO;	
	/// Resonance [0-127]
	int32_t ENV_F_RQ;	
	/// EnvAmount [-128,128]
	int32_t ENV_F_EA;	
	/// psy_dsp_Filter Type. See psycle::helpers::dsp::psy_dsp_FilterType. [0..6]
	int32_t ENV_F_TP;	
	///\}
	unsigned char _RPAN;
	unsigned char _RCUT;
	unsigned char _RRES;
	char instrum_name[32];

	int32_t val;
	int32_t pan=128;
	int32_t numwaves;
	int32_t i;
	int32_t index;
	
	if((self->songfile->file->currchunk.version&0xffff0000) == VERSION_MAJOR_ZERO)
	{
		psyfile_read(self->songfile->file, &index, sizeof index);
		if(index < MAX_INSTRUMENTS)
		{	
			psy_audio_Instrument* instrument;			
			// Loop stuff
			unsigned char loop;
			int32_t lines;
			psy_audio_NewNoteAction nna;

			instrument = psy_audio_instrument_allocinit();
			psyfile_read(self->songfile->file, &loop, sizeof(loop));
			instrument->loop = loop;
			psyfile_read(self->songfile->file, &lines, sizeof(lines));
			instrument->lines = lines;
			psyfile_read(self->songfile->file, &_NNA, sizeof(_NNA));
			///\verbatim
			/// NNA values overview:
			///
			/// 0 = Note Cut      [Fast Release 'Default']
			/// 1 = Note Release  [Release Stage]
			/// 2 = Note Continue [No NNA]
			///\endverbatim
			switch (_NNA) {
				case 0:
					nna = psy_audio_NNA_STOP;
				break;
				case 1:
					nna = psy_audio_NNA_NOTEOFF;
				break;
				case 2:
					nna = psy_audio_NNA_CONTINUE;
				break;
				default:
					nna = psy_audio_NNA_STOP;
				break;
			}			
			psy_audio_instrument_setnna(instrument, nna);

			// read envelopes
			// ENV_VOL
			psyfile_read(self->songfile->file, &ENV_AT, sizeof(ENV_AT));
			psyfile_read(self->songfile->file, &ENV_DT, sizeof(ENV_DT));
			psyfile_read(self->songfile->file, &ENV_SL, sizeof(ENV_SL));
			psyfile_read(self->songfile->file, &ENV_RT, sizeof(ENV_RT));
			// ENV_AT
			psy_dsp_envelopesettings_settimeandvalue(&instrument->volumeenvelope,
				1, ENV_AT * 1.f / 44100, 1.f);
			// ENV_DT, ENV_SL
			psy_dsp_envelopesettings_settimeandvalue(&instrument->volumeenvelope,
				2, (ENV_AT + ENV_DT) * 1.f / 44100, ENV_SL / 100.f);			
			// ENV_RT
			psy_dsp_envelopesettings_settimeandvalue(&instrument->volumeenvelope,
				3, (ENV_AT + ENV_DT + ENV_RT) * 1.f / 44100, 0.f);
						
			// ENV_F
			psyfile_read(self->songfile->file, &ENV_F_AT, sizeof(ENV_F_AT));
			psyfile_read(self->songfile->file, &ENV_F_DT, sizeof(ENV_F_DT));
			psyfile_read(self->songfile->file, &ENV_F_SL, sizeof(ENV_F_SL));
			psyfile_read(self->songfile->file, &ENV_F_RT, sizeof(ENV_F_RT));
			// ENV_F_AT			
			psy_dsp_envelopesettings_settimeandvalue(&instrument->volumeenvelope,
				1, ENV_AT * 1.f / 44100, 1.f);
			// ENV_DT, ENV_SL
			// note: SL map range(128) differs from volume envelope(100)
			psy_dsp_envelopesettings_settimeandvalue(&instrument->volumeenvelope,
				2, (ENV_AT + ENV_DT) * 1.f / 44100, ENV_SL / 128.f);
			// ENV_RT
			psy_dsp_envelopesettings_settimeandvalue(&instrument->volumeenvelope,
				3, (ENV_AT + ENV_DT + ENV_RT) * 1.f / 44100, 0.f);
			
			psyfile_read(self->songfile->file, &ENV_F_CO, sizeof(ENV_F_CO));
			psyfile_read(self->songfile->file, &ENV_F_RQ, sizeof(ENV_F_RQ));
			psyfile_read(self->songfile->file, &ENV_F_EA, sizeof(ENV_F_EA));

			instrument->filtercutoff = ENV_F_CO / 127.f;
			instrument->filterres = ENV_F_RQ / 127.f;
			instrument->filtermodamount = ENV_F_EA / 255.f + 0.5f;
			
			psyfile_read(self->songfile->file, &val, sizeof(val));
			ENV_F_TP = val;
			instrument->filtertype = (psy_dsp_FilterType) val;

			psyfile_read(self->songfile->file, &pan, sizeof(pan));
			psyfile_read(self->songfile->file, &_RPAN, sizeof(_RPAN));
			psyfile_read(self->songfile->file, &_RCUT, sizeof(_RCUT));
			psyfile_read(self->songfile->file, &_RRES, sizeof(_RRES));

			instrument->randompan = _RPAN;
			instrument->_RCUT = _RCUT;
			instrument->_RRES = _RRES;			
			
			psyfile_readstring(self->songfile->file, instrum_name,sizeof(instrum_name));

			// now we have to read waves			
			psyfile_read(self->songfile->file, &numwaves, sizeof(numwaves));
			for (i = 0; i < numwaves; i++)
			{
				psy_audio_psy3loader_loadwavesubchunk(self, index, pan, instrum_name, 1, i );
			}

			if ((self->songfile->file->currchunk.version & 0xFF) >= 1)
			{ //revision 1 or greater
				psyfile_read(self->songfile->file, &sampler_to_use, sizeof(sampler_to_use));
				psyfile_read(self->songfile->file, &_LOCKINST, sizeof(_LOCKINST));
			}

			//Ensure validity of values read
			if (sampler_to_use < 0 || sampler_to_use >= MAX_BUSES) { _LOCKINST=FALSE; sampler_to_use = -1; }
			//Truncate to 220 samples boundaries, and ensure it is not zero.
			ENV_AT = (ENV_AT/220)*220; if (ENV_AT <=0) ENV_AT=1;
			ENV_DT = (ENV_DT/220)*220; if (ENV_DT <=0) ENV_DT=1;
			if (ENV_RT == 16) ENV_RT = 220;
			else { ENV_RT = (ENV_RT/220)*220; if (ENV_RT <=0) ENV_RT=1; }
			ENV_F_AT = (ENV_F_AT/220)*220; if (ENV_F_AT <=0) ENV_F_AT=1;
			ENV_F_DT = (ENV_F_DT/220)*220; if (ENV_F_DT <=0) ENV_F_DT=1;
			ENV_F_RT = (ENV_F_RT/220)*220; if (ENV_F_RT <=0) ENV_F_RT=1;			
			psy_audio_instrument_setname(instrument, instrum_name);
			psy_audio_instrument_setindex(instrument, index);
			psy_audio_instruments_insert(&self->songfile->song->instruments, instrument,
				psy_audio_instrumentindex_make(0, index));
		}
	}	
}

void psy_audio_psy3loader_read_eins(psy_audio_PSY3Loader* self)
{   
	// Legacy for sampulse previous to Psycle 1.12	
	// Version zero was the development version (1.7 alphas, psycle-core).
	// Version one is the published one (1.8.5 onwards).
	size_t filepos = psyfile_getpos(self->songfile->file);
	uint32_t  numSamples;
	uint32_t i;
	if ((self->songfile->file->currchunk.version&0xFFFF0000) == VERSION_MAJOR_ONE)
	{
		char temp[8];
		// lowversion 0 is 1.8.5, lowversion 1 is 1.8.6
		uint32_t lowversion = (self->songfile->file->currchunk.version&0xFFFF);
		int idx;		
		// Instrument Data Load
		uint32_t numInstruments;
		psyfile_read(self->songfile->file, &numInstruments, sizeof(numInstruments));
		for(i = 0; i < numInstruments && filepos < self->songfile->file->currchunk.begins +
				self->songfile->file->currchunk.size; i++) {
			uint32_t sizeINST=0;

			psyfile_read(self->songfile->file, &idx, sizeof(idx));
			psyfile_read(self->songfile->file, &temp, 4);
			temp[4]='\0';
			psyfile_read(self->songfile->file, &sizeINST, sizeof(sizeINST));
			filepos = psyfile_getpos(self->songfile->file);
			if (strcmp(temp,"INST")== 0) {
				uint32_t versionINST;
				psyfile_read(self->songfile->file, &versionINST, sizeof(versionINST));
				if (versionINST == 1) {
					bool legacyenabled;
					psyfile_read(self->songfile->file, &legacyenabled, sizeof(legacyenabled));
				} else {
					//versionINST 0 was not stored, so seek back.
					psyfile_seek(self->songfile->file, filepos);
					versionINST = 0;
				}
//				XMInstrument inst;
				psy_audio_psy3loader_loadxminstrument(self, 0, TRUE, lowversion);
//				inst.Load(*pFile, versionINST, true, lowversion);
//				xminstruments.SetInst(inst,idx);
			}
			if (lowversion > 0) {
				//Version 0 doesn't write the chunk size correctly
				//so we cannot correct it in case of error
				psyfile_seek(self->songfile->file, filepos + sizeINST);
				filepos = psyfile_getpos(self->songfile->file);
			}
		}		
		psyfile_read(self->songfile->file, &numSamples, sizeof(numSamples));
		for(i = 0;i < numSamples && filepos <
			self->songfile->file->currchunk.begins + self->songfile->file->currchunk.size;i++)
		{
			char temp[8];
			uint32_t versionSMPD;
			uint32_t sizeSMPD=0;

			psyfile_read(self->songfile->file, &idx, sizeof(idx));
			psyfile_read(self->songfile->file, &temp, 4);
			temp[4]='\0';
			psyfile_read(self->songfile->file, &sizeSMPD, sizeof(sizeSMPD));
			filepos = psyfile_getpos(self->songfile->file);
			if (strcmp(temp,"SMPD")== 0)
			{
				psy_audio_Sample* wave;
				psyfile_read(self->songfile->file, &versionSMPD, sizeof(versionSMPD));
				//versionSMPD 0 was not stored, so seek back.
				if (versionSMPD != 1) {
					psyfile_seek(self->songfile->file, filepos);
					versionSMPD = 0;
				}
				wave = psy_audio_psy3loader_xmloadwav(self);
				psy_audio_samples_insert(&self->songfile->song->samples, wave,
					sampleindex_make(idx, 0));
				// XMInstrument::WaveData<> wave;
				// wave.Load(*pFile, versionSMPD, true);
				// samples.SetSample(wave, idx);
			}
			if (lowversion > 0) {
				//Version 0 doesn't write the chunk size correctly
				//so we cannot correct it in case of error
				psyfile_seek(self->songfile->file, filepos + sizeSMPD);
				filepos = psyfile_getpos(self->songfile->file);
			}
		}
	}	
}

void psy_audio_psy3loader_read_smid(psy_audio_PSY3Loader* self)
{	
	if((self->songfile->file->currchunk.version & 0xFFFF0000) == VERSION_MAJOR_ZERO) {
		uint32_t index;

		psyfile_read(self->songfile->file, &index, sizeof(index));
		if (index < MAX_INSTRUMENTS) {
			psy_audio_Instrument* instrument;

			if (!psy_audio_instruments_at(&self->songfile->song->instruments,
					psy_audio_instrumentindex_make(1, index))) {
				instrument = psy_audio_instrument_allocinit();
				psy_audio_instrument_setindex(instrument, index);
				psy_audio_instruments_insert(&self->songfile->song->instruments, instrument,
					psy_audio_instrumentindex_make(1, index));
			}
			instrument = psy_audio_instruments_at(&self->songfile->song->instruments,
				psy_audio_instrumentindex_make(1, index));
			psy_audio_psy3loader_loadxminstrument(self, instrument, 0, self->songfile->file->currchunk.version &
				0xFFFF);
		}
	}	
}

psy_audio_Sample* psy_audio_psy3loader_xmloadwav(psy_audio_PSY3Loader* self)
{
	uint32_t size1;
	uint32_t size2;
	char wavename[256];			
	uint32_t temp;
	uint8_t temp8;
	uint16_t temp16;			
	float ftemp;
	unsigned char btemp;
	psy_audio_Sample* wave;

	wave = psy_audio_sample_allocinit(1);
	psyfile_readstring(self->songfile->file, wavename, sizeof(wavename));
	psy_audio_sample_setname(wave, wavename);
	// wavelength
	psyfile_read(self->songfile->file, &temp, sizeof(temp));
	wave->numframes = temp;
	// global volume
	psyfile_read(self->songfile->file, &ftemp, sizeof(ftemp));
	wave->globalvolume = ftemp;
	// default volume
	psyfile_read(self->songfile->file, &temp16, sizeof(temp16));
	wave->defaultvolume = temp16;
	// wave loop start
	psyfile_read(self->songfile->file, &temp, sizeof(temp));
	wave->loop.start = temp;
	// wave loop end
	psyfile_read(self->songfile->file, &temp, sizeof(temp));
	wave->loop.end = temp;
	// wave loop type				
	psyfile_read(self->songfile->file, &temp, sizeof(temp));
	wave->loop.type = (psy_audio_SampleLoopType) temp;
	// wave sustain loop start
	psyfile_read(self->songfile->file, &temp, sizeof(temp));
	wave->sustainloop.start = temp;
	// wave sustain loop end
	psyfile_read(self->songfile->file, &temp, sizeof(temp));
	wave->sustainloop.end = temp;
	psyfile_read(self->songfile->file, &temp, sizeof(temp));
	wave->sustainloop.type = (psy_audio_SampleLoopType) temp;			
	// "bigger than" insted of "bigger or equal", because that means 
	// interpolate between loopend and loopstart
	if (wave->loop.end > wave->numframes) {
		wave->loop.end = wave->numframes;
	}
	if (wave->sustainloop.end > wave->numframes) {
		wave->sustainloop.end = wave->numframes;
	} 
	if (self->songfile->file->currchunk.version == 0) {
		wave->samplerate = 8363;
	} else {
		psyfile_read(self->songfile->file, &temp, sizeof(temp));
		wave->samplerate = temp;				
	}
	// wave tune
	psyfile_read(self->songfile->file, &temp16, sizeof(temp16));
	wave->tune = temp16;
	// wave fine tune
	psyfile_read(self->songfile->file, &temp16, sizeof(temp16));
	wave->finetune = temp16;			
	// wave stereo
	psyfile_read(self->songfile->file, &btemp, sizeof(btemp));
	wave->stereo = btemp;
	// pan enabled
	psyfile_read(self->songfile->file, &btemp, sizeof(btemp));
	wave->panenabled = btemp;
	// pan factor
	psyfile_read(self->songfile->file, &ftemp, sizeof(ftemp));
	wave->panfactor = ftemp;			
	if(self->songfile->file->currchunk.version >= 1) {
		// surround
		psyfile_read(self->songfile->file, &btemp, sizeof(btemp));
		wave->surround = btemp;				
	}
	else if (wave->panfactor > 1.0f) {
		wave->surround = 1;
		wave->panfactor -= 1.0f;
	} else { 
		wave->surround = 0;
	}
	// vibrato attack
	psyfile_read(self->songfile->file, &temp8, sizeof(temp8));
	wave->vibrato.attack = temp8;			
	// vibrato speed
	psyfile_read(self->songfile->file, &temp8, sizeof(temp8));
	wave->vibrato.speed = temp8;
	// vibrato depth
	psyfile_read(self->songfile->file, &temp8, sizeof(temp8));
	wave->vibrato.depth = temp8;			
	// vibrato type
	psyfile_read(self->songfile->file, &temp8, sizeof(temp8));
	if (temp8 <= psy_audio_WAVEFORMS_RANDOM) {
		// wave->vibratotype = (psy_audio_WaveForms) temp8;
	} else { 
		// wave->vibratotype = (psy_audio_WaveForms) psy_audio_WAVEFORMS_SINUS;
	}														
	{ // wave data
		byte* pData;
		short* pDest;
		uint32_t i;
		psyfile_read(self->songfile->file, &size1, sizeof(size1));
		pData = malloc(size1);
		psyfile_read(self->songfile->file, pData, size1);
		sounddesquash(pData, &pDest);
		free(pData);
		psy_audio_sample_allocwavedata(wave);
		for (i = 0; i < wave->numframes; i++) {
			short val = (short) pDest[i];
			wave->channels.samples[0][i] = (float) val;				
		}
		free(pDest);
		pData = 0;		
		if (wave->stereo)
		{
			uint32_t i;
			psyfile_read(self->songfile->file, &size2, sizeof(size1));
			pData = malloc(size2);
			psyfile_read(self->songfile->file, pData, size1);
			sounddesquash(pData, &pDest);
			free(pData);
			psy_audio_sample_resize(wave, 2);			
			for (i = 0; i < wave->numframes; i++) {
				short val = (short) pDest[i];
				wave->channels.samples[1][i] = (float) val;					
			}
			free(pDest);
			pData = 0;			
		}		
	}
	return wave;
}

void psy_audio_psy3loader_loadxminstrument(psy_audio_PSY3Loader* self, psy_audio_Instrument* instrument, bool islegacy, uint32_t legacyversion)
{		
	// SMID chunk
	char name[256];
	uint16_t m_Lines;
	/// [0..1.0f] Global volume affecting all samples of the instrument.
	float m_GlobVol;
	/// [0..1.0f] Fadeout speed. Decreasing amount for each tracker tick.
	float m_VolumeFadeSpeed;
	// Paninng
	bool m_PanEnabled;
	/// Initial panFactor (if enabled) [-1..1]
	float m_InitPan;
	bool m_Surround;
	/// Note number for center pan position
	uint8_t m_NoteModPanCenter;
	/// -32..32. 1/256th of panFactor change per seminote.
	int8_t m_NoteModPanSep;

	/// Cutoff Frequency [0..127]
	uint8_t m_FilterCutoff;
	/// Resonance [0..127]
	uint8_t m_FilterResonance;
	int32_t m_FilterType;

	// Randomness. Applies on new notes.

	/// Random Volume % [ 0.0 -> No randomize. 1.0 = randomize full scale.]
	float m_RandomVolume;
	/// Random Panning (same)
	float m_RandomPanning;
	/// Random CutOff (same)
	float m_RandomCutoff;
	/// Random Resonance (same)
	float m_RandomResonance;
		
	/// Check to do when a new event comes in the channel.
	int32_t m_DCT;
	/// Action to take on the playing voice when the action defined by m_DCT comes in the same channel 
	/// (like the same note value).
	int32_t m_DCA;
	int legacyeins;
	static const int note_map_size = 120; // C-0 .. B-9

	legacyeins = 0;

	psyfile_readstring(self->songfile->file, name, 256);
	psy_audio_instrument_setname(instrument, name);
	psyfile_read(self->songfile->file, &m_Lines, sizeof(m_Lines));
	if (islegacy) m_Lines = 0;

	psyfile_read(self->songfile->file, &m_GlobVol, sizeof(m_GlobVol));
	if (instrument) {
		instrument->globalvolume = m_GlobVol;	
	}
	psyfile_read(self->songfile->file, &m_VolumeFadeSpeed, sizeof(m_VolumeFadeSpeed));
	if (instrument) {
		instrument->volumefadespeed = m_VolumeFadeSpeed;
	}

	psyfile_read(self->songfile->file, &m_InitPan, sizeof(m_InitPan));
	if (instrument) {
		instrument->initpan = m_InitPan;
	}
	psyfile_read(self->songfile->file, &m_PanEnabled, sizeof(m_PanEnabled));
	if (instrument) {
		instrument->panenabled = m_PanEnabled;
	}
	if (self->songfile->file->currchunk.version == 0) {
		m_Surround = FALSE;
	} else {
		psyfile_read(self->songfile->file, &m_Surround, sizeof(m_Surround));
	}
	if (instrument) {
		instrument->surround = m_Surround;
	}
	psyfile_read(self->songfile->file, &m_NoteModPanCenter, sizeof(m_NoteModPanCenter));
	if (instrument) {
		instrument->notemodpancenter = m_NoteModPanCenter;
	}
	psyfile_read(self->songfile->file, &m_NoteModPanSep, sizeof(m_NoteModPanSep));
	if (instrument) {
		instrument->notemodpansep = m_NoteModPanSep;
	}	

	psyfile_read(self->songfile->file, &m_FilterCutoff, sizeof(m_FilterCutoff));
	if (instrument) {
		instrument->filtercutoff = m_FilterCutoff / 127.f;
	}
	psyfile_read(self->songfile->file, &m_FilterResonance, sizeof(m_FilterResonance));
	if (instrument) {
		instrument->filterres = m_FilterResonance / 127.f;
	}
	{ 
		uint16_t unused = 0;
		psyfile_read(self->songfile->file, &unused, sizeof(unused));
	}
	{
		uint32_t i = 0;
		psy_dsp_FilterType ft;

		psyfile_read(self->songfile->file, &i, sizeof(i));
		if (instrument) {
			m_FilterType = i;
			switch (i) {
			case 0: ft = F_LOWPASS12;
				break;
			case 1: ft = F_HIGHPASS12;
				break;
			case 2: ft = F_BANDPASS12;
				break;
			case 3: ft = F_BANDREJECT12;
				break;
			case 4:	ft = F_NONE;//This one is kept here because it is used in load/save. Also used in Sampulse instrument filter as "use channel default"
				break;
			case 5: ft = F_ITLOWPASS;
				break;
			case 6: ft = F_MPTLOWPASSE;
				break;
			case 7:	ft = F_MPTHIGHPASSE;
				break;
			case 8:	ft = F_LOWPASS12E;
				break;
			case 9:	ft = F_HIGHPASS12E;
				break;
			case 10:ft = F_BANDPASS12E;
				break;
			case 11:ft = F_BANDREJECT12E;
				break;
			default:
				ft = F_NONE;
				break;
			}
			instrument->filtertype = ft;
		}
	}
	psyfile_read(self->songfile->file, &m_RandomVolume, sizeof(m_RandomVolume));
	if (instrument) {
		instrument->randomvolume = m_RandomVolume;
	}
	psyfile_read(self->songfile->file, &m_RandomPanning, sizeof(m_RandomPanning));
	if (instrument) {
		instrument->randompanning = m_RandomPanning;
	}
	psyfile_read(self->songfile->file, &m_RandomCutoff, sizeof(m_RandomCutoff));
	if (instrument) {
		instrument->randomcutoff = m_RandomCutoff;
	}
	psyfile_read(self->songfile->file, &m_RandomResonance, sizeof(m_RandomResonance));
	if (instrument) {
		instrument->randomresonance = m_RandomResonance;
	}
	{
		uint32_t i = 0;		

		psyfile_read(self->songfile->file, &i, sizeof(i));
		if (instrument) {
			/// Action to take on the playing voice when any new note comes 
			/// in the same channel.
			psy_audio_NewNoteAction nna;
			// NNA_STOP = 0x0,		///  [Note Cut]	
			// (This one actually does a very fast fadeout)
			// NNA_CONTINUE = 0x1,	///  [Ignore]
			// NNA_NOTEOFF = 0x2,	///  [Note off]
			// NNA_FADEOUT = 0x3	///  [Note fade]
			switch (i) {
				case 0: nna = psy_audio_NNA_STOP;
				break;
				case 1: nna = psy_audio_NNA_CONTINUE;
				break;
				case 2: nna = psy_audio_NNA_NOTEOFF;
				break;
				case 3: nna = psy_audio_NNA_FADEOUT;
				break;
				default:
					nna = psy_audio_NNA_STOP;
				break;
			}
			psy_audio_instrument_setnna(instrument, nna);
		}
		psyfile_read(self->songfile->file, &i, sizeof(i));
		m_DCT = i; // static_cast<DupeCheck::Type>(i);
		psyfile_read(self->songfile->file, &i, sizeof(i));
		m_DCA = i; //static_cast<NewNoteAction::Type>(i);
	}

	{
		int32_t i;		
		int first = 1;		
		uint8_t targetnote;
		uint8_t sampleslot;
		psy_audio_InstrumentEntry instentry;		

		psy_audio_instrumententry_init(&instentry);
		instentry.sampleindex = sampleindex_make(0, 0);
		if (instrument) {
			psy_audio_instrument_clearentries(instrument);
		}
		for(i = 0; i < note_map_size; i++) {
			int note;

			note = i;
			psyfile_read(self->songfile->file, &targetnote, sizeof(targetnote));
			psyfile_read(self->songfile->file, &sampleslot, sizeof(sampleslot));
			if (instrument) {
				if (first) {
					instentry.sampleindex.slot = sampleslot;
					instentry.keyrange.low = note;
					first = 0;
				} else
				if (sampleslot != instentry.sampleindex.slot) {
					instentry.keyrange.high = note - 1;				
					psy_audio_instrument_addentry(instrument, &instentry);
					instentry.keyrange.low = note;
					instentry.sampleindex.slot = sampleslot;
				}
			}
		}
		if (instrument) {
			instentry.keyrange.high = 119;
			psy_audio_instrument_addentry(instrument, &instentry);
		}
	}
	psy_audio_psy3loader_xminstrumentenvelopeload(self, 0, 0);
	if (islegacy && legacyeins==0) {
		//Workaround for a bug in that version
		psy_audio_psy3loader_xminstrumentenvelopeload(self, islegacy, self->songfile->file->currchunk.version);  // islegacy, version
		psy_audio_psy3loader_xminstrumentenvelopeload(self, islegacy, self->songfile->file->currchunk.version); // riffFile,islegacy, version);
	}
	else {
		psy_audio_psy3loader_xminstrumentenvelopeload(self, islegacy, self->songfile->file->currchunk.version); // m_PanEnvelope.Load(riffFile,islegacy, version);
		psy_audio_psy3loader_xminstrumentenvelopeload(self, islegacy, self->songfile->file->currchunk.version); // m_FilterEnvelope.Load(riffFile,islegacy, version);
	}
	psy_audio_psy3loader_xminstrumentenvelopeload(self, islegacy, self->songfile->file->currchunk.version); // m_PitchEnvelope.Load(riffFile,islegacy, version);

	// ValidateEnabled();		
}
			
/// The meaning of the first value (int), is time, and the unit depends on the context.
typedef struct {
	int first;
	float second;
} PointValue;

void psy_audio_psy3loader_xminstrumentenvelopeload(psy_audio_PSY3Loader* self, bool legacy, uint32_t legacyversion)
{
	char temp[8];
	uint32_t version=0;
	uint32_t size=0;
	size_t filepos=0;
	/// Envelope is enabled or disabled
	bool m_Enabled;
	/// if m_Carry and a new note enters, the envelope position is set to that of the previous note *on the same channel*
	bool m_Carry;
	/// Array of Points of the envelope.
	/// first : time at which to set the value. Unit can be different things depending on the context.
	/// second : 0 .. 1.0f . (or -1.0 1.0 or whatever else) Use it as a multiplier.
//	PointValue m_Points[1024];
	/// Loop Start Point
	unsigned int m_LoopStart;
	/// Loop End Point
	unsigned int m_LoopEnd; 
	/// Sustain Start Point
	unsigned int m_SustainBegin;
	/// Sustain End Point
	unsigned int m_SustainEnd;
	/// Envelope mode (meaning of the time value)
	int32_t m_Mode;
	/// Indicates that this envelope is operated as an ADSR (it is an option for the visual component).
	bool m_Adsr;

	if (!legacy) {
		psyfile_read(self->songfile->file, &temp, 4);
		temp[4]='\0';
		psyfile_read(self->songfile->file, &version, sizeof(version));
		psyfile_read(self->songfile->file, &size, sizeof(size));
		filepos = psyfile_getpos(self->songfile->file);
		if (strcmp("SMIE", temp) !=0 ) {
			psyfile_skip(self->songfile->file, size);
			return;
		}
	}
	else {
		version = legacyversion;
	}

	// Information starts here

	psyfile_read(self->songfile->file, &m_Enabled, sizeof(m_Enabled));
	psyfile_read(self->songfile->file, &m_Carry, sizeof(m_Carry));
	{
		uint32_t i32 = 0;

		psyfile_read(self->songfile->file, &i32, sizeof(i32)); m_LoopStart = i32;
		psyfile_read(self->songfile->file, &i32, sizeof(i32)); m_LoopEnd = i32;
		psyfile_read(self->songfile->file, &i32, sizeof(i32)); m_SustainBegin = i32;
		psyfile_read(self->songfile->file, &i32, sizeof(i32)); m_SustainEnd = i32;
	}
	{
		uint32_t num_of_points = 0;
		uint32_t i;

		psyfile_read(self->songfile->file, &num_of_points, sizeof(num_of_points));
		for(i = 0; i < num_of_points; i++){
			PointValue value;
			// The time in which this point is placed. The unit depends on the
			// mode.
			psyfile_read(self->songfile->file, &value.first, sizeof(value.first));
			// The value that this point has. Depending on the type of
			// envelope, this can range between 0.0 and 1.0 or between -1.0 and
			// 1.0
			psyfile_read(self->songfile->file, &value.second, sizeof(value.second));
			// m_Points.push_back(value);
		}
	}
	if (version == 0) {
		m_Mode = 0; // Mode::TICK;
		m_Adsr = FALSE;
	}
	else {
		{
			uint32_t read;
			psyfile_read(self->songfile->file, &read, sizeof(read));
			// m_Mode = read;
		}
		psyfile_read(self->songfile->file, &m_Adsr, sizeof(m_Adsr));
	}

	// Information ends here
	if (!legacy) {
		psyfile_seek(self->songfile->file, filepos + size);
	}
}

void psy_audio_psy3loader_read_smsb(psy_audio_PSY3Loader* self)
{
	if((self->songfile->file->currchunk.version&0xFFFF0000) == VERSION_MAJOR_ZERO) {
		uint32_t sampleidx;
		psyfile_read(self->songfile->file, &sampleidx, sizeof(sampleidx));
		if (sampleidx < MAX_INSTRUMENTS) {
			uint32_t size1;
			uint32_t size2;
			char wavename[256];			
			uint32_t temp;
			uint8_t temp8;
			uint16_t temp16;			
			float ftemp;
			unsigned char btemp;
			psy_audio_Sample* wave;

			wave = psy_audio_sample_allocinit(1);
			psyfile_readstring(self->songfile->file, wavename, sizeof(wavename));
			psy_audio_sample_setname(wave, wavename);
			// wavelength
			psyfile_read(self->songfile->file, &temp, sizeof(temp));
			wave->numframes = temp;
			// global volume
			psyfile_read(self->songfile->file, &ftemp, sizeof(ftemp));
			wave->globalvolume = ftemp;
			// default volume
			psyfile_read(self->songfile->file, &temp16, sizeof(temp16));
			wave->defaultvolume = temp16;
			// wave loop start
			psyfile_read(self->songfile->file, &temp, sizeof(temp));
			wave->loop.start = temp;
			// wave loop end
			psyfile_read(self->songfile->file, &temp, sizeof(temp));
			wave->loop.end = temp;
			// wave loop type				
			psyfile_read(self->songfile->file, &temp, sizeof(temp));
			wave->loop.type = (psy_audio_SampleLoopType) temp;
			// wave sustain loop start
			psyfile_read(self->songfile->file, &temp, sizeof(temp));
			wave->sustainloop.start = temp;
			// wave sustain loop end
			psyfile_read(self->songfile->file, &temp, sizeof(temp));
			wave->sustainloop.end = temp;
			psyfile_read(self->songfile->file, &temp, sizeof(temp));
			wave->sustainloop.type = (psy_audio_SampleLoopType) temp;			
			// "bigger than" insted of "bigger or equal", because that means 
			// interpolate between loopend and loopstart
			if (wave->loop.end > wave->numframes) {
				wave->loop.end = wave->numframes;
			}
			if (wave->sustainloop.end > wave->numframes) {
				wave->sustainloop.end = wave->numframes;
			} 
			if (self->songfile->file->currchunk.version == 0) {
				wave->samplerate = 8363;
			}
			else {
				psyfile_read(self->songfile->file, &temp, sizeof(temp));
				wave->samplerate = temp;				
			}
			// wave tune
			psyfile_read(self->songfile->file, &temp16, sizeof(temp16));
			wave->tune = temp16;
			// wave fine tune
			psyfile_read(self->songfile->file, &temp16, sizeof(temp16));
			wave->finetune = temp16;			
			// wave stereo
			psyfile_read(self->songfile->file, &btemp, sizeof(btemp));
			wave->stereo = btemp;
			// pan enabled
			psyfile_read(self->songfile->file, &btemp, sizeof(btemp));
			wave->panenabled = btemp;
			// pan factor
			psyfile_read(self->songfile->file, &ftemp, sizeof(ftemp));
			wave->panfactor = ftemp;			
			if(self->songfile->file->currchunk.version >= 1) {
				// surround
				psyfile_read(self->songfile->file, &btemp, sizeof(btemp));
				wave->surround = btemp;				
			}
			else if (wave->panfactor > 1.0f) {
				wave->surround = 1;
				wave->panfactor -= 1.0f;
			} else { 
				wave->surround = 0;
			}
			// vibrato attack
			psyfile_read(self->songfile->file, &temp8, sizeof(temp8));
			wave->vibrato.attack = temp8;			
			// vibrato speed
			psyfile_read(self->songfile->file, &temp8, sizeof(temp8));
			wave->vibrato.speed = temp8;
			// vibrato depth
			psyfile_read(self->songfile->file, &temp8, sizeof(temp8));
			wave->vibrato.depth = temp8;			
			// vibrato type
			psyfile_read(self->songfile->file, &temp8, sizeof(temp8));
			if (temp8 <= psy_audio_WAVEFORMS_RANDOM) {
				 wave->vibrato.type = (psy_audio_WaveForms)temp8;
			} else { 
				 wave->vibrato.type = (psy_audio_WaveForms) psy_audio_WAVEFORMS_SINUS;
			}														
			{ // wave data
				byte* pData;
				short* pDest;
				uint32_t i;
				psyfile_read(self->songfile->file, &size1, sizeof(size1));
				pData = malloc(size1);
				psyfile_read(self->songfile->file, pData, size1);
				sounddesquash(pData, &pDest);
				free(pData);
				psy_audio_sample_allocwavedata(wave);				
				for (i = 0; i < wave->numframes; i++) {
					short val = (short) pDest[i];
					wave->channels.samples[0][i] = (float) val;				
				}
				free(pDest);
				pData = 0;				
				if (wave->stereo)
				{
					uint32_t i;

					psy_audio_sample_resize(wave, 2);
					psyfile_read(self->songfile->file, &size2, sizeof(size2));
					pData = malloc(size2);
					psyfile_read(self->songfile->file, pData, size2);
					sounddesquash(pData, &pDest);
					free(pData);					
					for (i = 0; i < wave->numframes; i++) {
						short val = (short) pDest[i];
						wave->channels.samples[1][i] = (float) val;					
					}
					free(pDest);
					pData = 0;					
				}
				psy_audio_samples_insert(&self->songfile->song->samples, wave,
					sampleindex_make(sampleidx, 0));
			}
		}
	}
}

void psy_audio_psy3loader_loadwavesubchunk(psy_audio_PSY3Loader* self, int32_t instrIdx, int32_t pan, char * instrum_name, int32_t fullopen, int32_t loadIdx)
{
	char Header[8];
	uint32_t version;
	uint32_t size;

	psyfile_read(self->songfile->file, &Header,4);
	Header[4] = 0;
	psyfile_read(self->songfile->file, &version,sizeof(version));
	psyfile_read(self->songfile->file, &size,sizeof(size));

	//fileformat supports several waves, but sampler only supports one.
	if (strcmp(Header,"WAVE") == 0 &&
		version <= CURRENT_FILE_VERSION_WAVE || loadIdx == 0) {
		psy_audio_Sample* sample;
		//This index represented the index position of this wave for the instrument. 0 to 15.
		uint32_t legacyindex;
		uint16_t volume = 0;		
		int32_t tmp = 0;		
		uint8_t doloop = 0;
		uint8_t stereo = 0;
		char dummy[32];
		uint32_t packedsize;
		byte* pData;
		int16_t* pDest;
		
		sample = psy_audio_sample_allocinit(1);
		sample->panfactor = (float) pan / 256.f ; //(value_mapper::map_256_1(pan));
		sample->samplerate = 44100;				
		legacyindex = psyfile_read_uint32(self->songfile->file);
		sample->numframes = psyfile_read_uint32(self->songfile->file);
		volume = psyfile_read_uint16(self->songfile->file);
		sample->globalvolume = volume * 0.01f;
		sample->loop.start = psyfile_read_uint32(self->songfile->file);
		sample->loop.end = psyfile_read_uint32(self->songfile->file);
		sample->tune = psyfile_read_uint32(self->songfile->file);
		psyfile_read(self->songfile->file, &tmp, sizeof(tmp));
		//Current sampler uses 100 cents. Older used +-256		
		sample->finetune = (int16_t)(tmp / 2.56f);		
		doloop = psyfile_read_uint8(self->songfile->file);
		sample->loop.type = doloop ? psy_audio_SAMPLE_LOOP_NORMAL : psy_audio_SAMPLE_LOOP_DO_NOT;
		stereo = psyfile_read_uint8(self->songfile->file);
		sample->stereo = stereo != 0;
		// Old sample name, never used.
		psyfile_readstring(self->songfile->file, dummy,sizeof(dummy));
		psy_audio_sample_setname(sample, instrum_name);
		psyfile_read(self->songfile->file, &packedsize,sizeof(packedsize));
		if (fullopen) {
			uint32_t i;
			pData = malloc(packedsize+4);// +4 to avoid any attempt at buffer overflow by the code
			psyfile_read(self->songfile->file, pData, packedsize);
			sounddesquash(pData, &pDest);		
			free(pData);
			psy_audio_sample_allocwavedata(sample);			
			for (i = 0; i < sample->numframes; i++) {
				short val = (short) pDest[i];
				sample->channels.samples[0][i] = (float) val;				
			 }
			free(pDest);
			pData = 0;
			sample->channels.numchannels = 1;
		} else {
			psyfile_skip(self->songfile->file, packedsize);
			sample->channels.samples[0] = 0;
		}
		if (sample->stereo) {
			psyfile_read(self->songfile->file, &packedsize,sizeof(packedsize));
			if ( fullopen ) {
				uint32_t i;

				// +4 to avoid any attempt at buffer overflow by the code
				pData = malloc(packedsize+4); 
				psyfile_read(self->songfile->file, pData,packedsize);
				sounddesquash(pData, &pDest);
				free(pData);
				psy_audio_sample_resize(sample, 2);				
				for (i = 0; i < sample->numframes; ++i) {
					short val = (short) pDest[i];
					sample->channels.samples[1][i] = (float) val;					
				}
				free(pDest);
				pData = 0;
				sample->channels.numchannels = 2;
			} else {
				psyfile_skip(self->songfile->file, packedsize);
				sample->channels.samples[1] = 0;
			}
		}
		psy_audio_samples_insert(&self->songfile->song->samples, sample,
			sampleindex_make(instrIdx, 0));
	} else {
		psyfile_skip(self->songfile->file, size);
	}
}

void psy_audio_psy3loader_read_macd(psy_audio_PSY3Loader* self)
{	
	if((self->songfile->file->currchunk.version&0xFFFF0000) == VERSION_MAJOR_ZERO)
	{
		int32_t index;

		psyfile_read(self->songfile->file, &index, sizeof index);
		if (index < MAX_MACHINES) {			
			psy_audio_Machine* machine;
			
			machine = psy_audio_psy3loader_machineloadchunk(self, index);
			if (machine) {
				psy_audio_machines_insert(&self->songfile->song->machines, index,
					machine);
			}
		}
	}	
}

psy_audio_Machine* psy_audio_psy3loader_machineloadchunk(psy_audio_PSY3Loader* self, int32_t index)
{
	psy_audio_Machine* machine;
	bool replaced;
	char catchername[512];
	char modulename[256];
	char editname[32];	
	int32_t i;
	psy_audio_MachineWires* machinewires;
	
	machine = psy_audio_psy3loader_machineloadchunk_createmachine(self, index, modulename, catchername, &replaced);
	{
		unsigned char bypass;
		unsigned char mute;
		int32_t panning;
		int32_t x;
		int32_t y;
		
		psyfile_read(self->songfile->file, &bypass, sizeof(bypass));
		psyfile_read(self->songfile->file, &mute, sizeof(mute));
		psyfile_read(self->songfile->file, &panning, sizeof(panning));
		psyfile_read(self->songfile->file, &x, sizeof(x));
		psyfile_read(self->songfile->file, &y, sizeof(y));
		psyfile_skip(self->songfile->file, 2*sizeof(int32_t));	// numInputs, numOutputs
		
		psy_audio_machine_setposition(machine, x, y);		
		if (bypass) {
			psy_audio_machine_bypass(machine);
		}
		if (mute) {
			psy_audio_machine_mute(machine);
		}
		psy_audio_machine_setpanning(machine, panning / 128.f);
	}
	machinewires = psy_audio_machinewires_allocinit();
	
	for (i = 0; i < MAX_CONNECTIONS; ++i) {
		int32_t input;
		int32_t output;
		float inconvol;
		float wiremultiplier;
		unsigned char connection;
		unsigned char incon;
		psy_audio_LegacyWire* legacywire;
						
		// Incoming connections psy_audio_Machine number
		psyfile_read(self->songfile->file, &input ,sizeof(input));		
		// Outgoing connections psy_audio_Machine number
		psyfile_read(self->songfile->file, &output, sizeof(output));
		// Incoming connections psy_audio_Machine vol
		psyfile_read(self->songfile->file, &inconvol, sizeof(inconvol));
		// Value to multiply _inputConVol[] to have a 0.0...1.0 range
		psyfile_read(self->songfile->file, &wiremultiplier, sizeof(wiremultiplier));
		// Outgoing connections activated
		psyfile_read(self->songfile->file, &connection, sizeof(connection));
		// Incoming connections activated
		psyfile_read(self->songfile->file, &incon, sizeof(incon));
		legacywire = psy_audio_legacywire_allocinit_all(input, incon, inconvol,
			wiremultiplier, output, connection);
		if (legacywire) {
			psy_audio_machinewires_insert(machinewires, (uintptr_t)i, legacywire);			
		}
	}
	psy_audio_legacywires_insert(&self->legacywires, index, machinewires);	
	psyfile_readstring(self->songfile->file, editname, 32);
	if (replaced) {
		char text[256];

		strcpy(text, "X!");
		strcat(text, editname);
		psy_audio_machine_seteditname(machine, text);
		psy_audio_songfile_warn(self->songfile, "replaced missing module ");
		psy_audio_songfile_warn(self->songfile, modulename);
		psy_audio_songfile_warn(self->songfile, " aka ");
		psy_audio_songfile_warn(self->songfile, editname);
		psy_audio_songfile_warn(self->songfile, " with dummy-plug\n");
	} else {
		psy_audio_machine_seteditname(machine, editname);
	}	
	psy_audio_machine_loadspecific(machine, self->songfile, index);
	if (psyfile_currchunkversion(self->songfile->file) >= 1) {
		//TODO: What to do on possibly wrong wire load?
		psy_audio_machine_loadwiremapping(machine, self->songfile, index);
	}
	return machine;	
}

psy_audio_Machine* psy_audio_psy3loader_machineloadchunk_createmachine(
	psy_audio_PSY3Loader* self, int32_t index, char* modulename,
	char* catchername, bool* replaced)
{
	psy_audio_Machine* machine;
	int32_t type;	

	*replaced = FALSE;
	modulename[0] = '\0';
	catchername[0] = '\0';
	psyfile_read(self->songfile->file, &type, sizeof(type));
	psyfile_readstring(self->songfile->file, modulename, 256);
	plugincatcher_catchername(self->songfile->song->machinefactory->catcher,
		modulename, catchername, 0);
	// todo shellidx;
	machine = psy_audio_machinefactory_makemachine(self->songfile->song->machinefactory, type,
		catchername, UINTPTR_MAX);
	if (!machine) {
		machine = psy_audio_machinefactory_makemachine(self->songfile->song->machinefactory, MACH_DUMMY,
			catchername, UINTPTR_MAX);
		*replaced = TRUE;
	}
	return machine;
}

void psy_audio_psy3loader_read_virg(psy_audio_PSY3Loader* self)
{
	if ((self->songfile->file->fileversion & 0xFFFF0000) == VERSION_MAJOR_ZERO)
	{
		int32_t virtual_inst;
		int32_t mac_idx;
		int32_t inst_idx;
		psy_audio_Machine* machine;

		psyfile_read(self->songfile->file, &virtual_inst, sizeof(virtual_inst));
		psyfile_read(self->songfile->file, &mac_idx, sizeof(mac_idx));
		psyfile_read(self->songfile->file, &inst_idx, sizeof(inst_idx));
		
		if (virtual_inst >= MAX_MACHINES && virtual_inst < MAX_VIRTUALINSTS) {
			// && mac != NULL && (mac->_type == MACH_SAMPLER || mac->_type == MACH_XMSAMPLER))
			machine = psy_audio_machinefactory_makemachinefrompath(self->songfile->song->machinefactory,
				MACH_VIRTUALGENERATOR,
				NULL, mac_idx, inst_idx);		
			psy_audio_machines_insert(&self->songfile->song->machines, virtual_inst, machine);			
		}				
	}	
}
