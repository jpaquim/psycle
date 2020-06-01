// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "psy3.h"
#include "song.h"
#include "songio.h"
#include "constants.h"
#include <datacompression.h>
#include <operations.h>
#include <dir.h>
#include "machinefactory.h"
#include "wire.h"

#include <stdlib.h>
#include <string.h>

#include "../../detail/portable.h"
#include "../../detail/psydef.h"
#include "plugin_interface.h"

#if !defined DIVERSALIS__OS__MICROSOFT
#define _MAX_PATH 4096
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE 1
#endif

static void readinfo(psy_audio_SongFile*);
static void readsngi(psy_audio_SongFile*);
static void readseqd(psy_audio_SongFile*, unsigned char* playorder,
	int32_t* playlength);
static void readpatd(psy_audio_SongFile*);
static void readepat(psy_audio_SongFile*);
static void readinsd(psy_audio_SongFile*);
static void readeins(psy_audio_SongFile*);
static void readsmid(psy_audio_SongFile*);
static void readmacd(psy_audio_SongFile*);
static void readsmsb(psy_audio_SongFile*);
static void loadxminstrument(psy_audio_SongFile*, psy_audio_Instrument* instrument,
	bool islegacy, uint32_t legacyversion);
static void xminstrumentenvelopeload(psy_audio_SongFile*, bool legacy, uint32_t legacyversion);
static psy_audio_Sample* xmloadwav(psy_audio_SongFile*);
static void loadwavesubchunk(psy_audio_SongFile*, int32_t instrIdx,
	int32_t pan, char * instrum_name, int32_t fullopen, int32_t loadIdx);
static psy_audio_Machine* machineloadchunk(psy_audio_SongFile*,
	int32_t index);
static psy_audio_Machine* machineloadchunk_createmachine(psy_audio_SongFile*,
	int32_t index, char* modulename, char* catchername, bool* replaced);
static void buildsequence(psy_audio_SongFile*, unsigned char* playorder,
	int playlength);
static void psy3_setinstrumentnames(psy_audio_SongFile*);

static uint32_t psy3_chunkcount(psy_audio_Song*);
static int psy3_write_header(psy_audio_SongFile*);
static int psy3_write_songinfo(psy_audio_SongFile*);
static int psy3_write_sngi(psy_audio_SongFile*);
static int psy3_write_seqd(psy_audio_SongFile*);
static int psy3_write_patd(psy_audio_SongFile*);
static int psy3_write_epat(psy_audio_SongFile*);
static int psy3_write_macd(psy_audio_SongFile*);
static int psy3_write_insd(psy_audio_SongFile*);
static int psy3_write_smsb(psy_audio_SongFile*);
static int psy3_write_machine(psy_audio_SongFile*, psy_audio_Machine*,
	uint32_t slot);
static void psy3_savedllnameandindex(PsyFile*, const char* name,
	int32_t shellindex);
static int psy3_write_connections(psy_audio_SongFile*, uintptr_t slot);
static int psy3_save_instrument(psy_audio_SongFile*, psy_audio_Instrument*);
static int psy3_save_sample(psy_audio_SongFile*, psy_audio_Sample*);
static short* psy3_floatbuffertoshort(float* buffer, uintptr_t numframes);

//	===================
//	PSY3 SONG LOAD
//	===================
int psy_audio_psy3_load(psy_audio_SongFile* self)
{
	char header[9];	
	uint32_t temp32;
	uint32_t index = 0;
	int32_t solo = 0;
	int32_t playlength;
	int32_t chunkcount = 0;		
	int32_t progress = 0;
	uint32_t filesize = psyfile_filesize(self->file);
	unsigned char playorder[MAX_SONG_POSITIONS];	
	
	psyfile_read(self->file, &temp32, sizeof(temp32));
	self->file->fileversion = temp32;
	psyfile_read(self->file, &temp32, sizeof(temp32));	
	self->file->filesize = temp32;
	if(self->file->fileversion > CURRENT_FILE_VERSION) {
		psy_audio_songfile_warn(self, 
			"This file is from a newer version of Psycle! "
			"This process will try to load it anyway.\n");
	}	
	psyfile_read(self->file, &self->file->chunkcount, sizeof(uint32_t));
	if (self->file->fileversion >= 8) {
		char trackername[256];
		char trackervers[256];
		int32_t bytesread;

		psyfile_readstring(self->file, trackername, 256);
		psyfile_readstring(self->file, trackervers, 256);
		bytesread = 4 + strlen(trackername)+strlen(trackervers)+2;
		// Size of the current Header DATA
		// This ensures that any extra data is skipped.
		psyfile_skip(self->file, self->file->filesize - bytesread);
	}	
	// start reading chunks		
	header[4] = 0;
	while(psyfile_read(self->file, header, 4) && self->file->chunkcount) {
		if (strcmp(header,"INFO") == 0) {			
			psyfile_readchunkbegin(self->file);
			readinfo(self);
			psyfile_seekchunkend(self->file);
			progress += 1;
		} else 
		if (strcmp(header,"SNGI")==0) {
			psyfile_readchunkbegin(self->file);
			readsngi(self);
			progress += 1;
			psyfile_seekchunkend(self->file);
		} else 
		if (strcmp(header,"SEQD")==0) {
			psyfile_readchunkbegin(self->file);
			readseqd(self, playorder, &playlength);			
			psyfile_seekchunkend(self->file);
			progress += 1;
		} else 
		if (strcmp(header,"PATD")==0) {
			psyfile_readchunkbegin(self->file);
			readpatd(self);
			psyfile_seekchunkend(self->file);
			progress += 1;
		} else
		if (strcmp(header,"MACD")==0) {
			psyfile_readchunkbegin(self->file);			
			readmacd(self);			
			psyfile_seekchunkend(self->file);
			progress += 1;
		} else
		if (strcmp(header,"INSD")==0) {
			psyfile_readchunkbegin(self->file);
			readinsd(self);
			psyfile_seekchunkend(self->file);
			progress += 1;
		} else
		if (strcmp(header,"EINS")==0) {
			psyfile_readchunkbegin(self->file);
			readeins(self);
			psyfile_seekchunkend(self->file);
		} else
		if (strcmp(header,"SMID") == 0) {
			psyfile_readchunkbegin(self->file);
			readsmid(self);
			psyfile_seekchunkend(self->file);
		} else
		if (strcmp(header,"SMSB") == 0) {
			psyfile_readchunkbegin(self->file);			
			readsmsb(self);			
			psyfile_seekchunkend(self->file);
		} else
		if (strcmp(header,"VIRG") == 0) {
			psyfile_readchunkbegin(self->file);
			psyfile_seekchunkend(self->file);
		} else {
			// we are not at a valid header for some weird reason.  
			// probably there is some extra data.
			// shift back 3 bytes and try again			
			psyfile_skip(self->file, -3);			
			// this makes it impossible to add new chunks
			// todo: read header and skip the chunk
			// psyfile_readchunkbegin(self->file);
			// psyfile_seekchunkend(self->file);			
		}
		psy_signal_emit(&self->song->signal_loadprogress, self, 1,
			progress);
	}	
	buildsequence(self, playorder, playlength);	
	psy3_setinstrumentnames(self);
	return self->err;
}

void psy3_setinstrumentnames(psy_audio_SongFile* self)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->song->instruments.container);
			!psy_tableiterator_equal(&it, psy_table_end()); psy_tableiterator_inc(&it)) {
		psy_audio_Instrument* instrument;
		psy_audio_Sample* sample;

		instrument = psy_tableiterator_value(&it);
		sample = psy_audio_samples_at(&self->song->samples,
			sampleindex_make(psy_tableiterator_key(&it), 0));
		if (sample) {
			instrument_setname(instrument, sample_name(sample));
		}
	}
}


void buildsequence(psy_audio_SongFile* self, unsigned char* playorder, int playlength)
{
	SequencePosition sequenceposition;
	int32_t i;
		
	sequenceposition.track =
		sequence_appendtrack(&self->song->sequence, sequencetrack_allocinit());
	for (i = 0; i < playlength; ++i) {			
		sequenceposition.trackposition =
			sequence_last(&self->song->sequence, sequenceposition.track);
		sequence_insert(&self->song->sequence, sequenceposition, playorder[i]);
	}
}

void readinfo(psy_audio_SongFile* self)
{
	if((self->file->currchunk.version&0xFFFF0000) == VERSION_MAJOR_ZERO)		
	{		
		char name_[129];
		char author_[65];
		char comments_[65536];
		SongProperties songproperties;

		psyfile_readstring(self->file, name_, sizeof name_);
		psyfile_readstring(self->file, author_, sizeof author_);
		psyfile_readstring(self->file, comments_,sizeof comments_);
		songproperties_init(&songproperties, name_, author_, comments_);
		psy_audio_song_setproperties(self->song, &songproperties);
		//bugfix. There were songs with incorrect size.
		if(self->file->currchunk.version == 0) {
			self->file->currchunk.size = 
				(uint32_t)(psyfile_getpos(self->file) - 
					self->file->currchunk.begins);
		}
	}
}

void readsngi(psy_audio_SongFile* self)
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
	
	if((self->file->currchunk.version&0xFFFF0000) == VERSION_MAJOR_ZERO)
	{
		// why all these temps?  to make sure if someone changes the defs of
		// any of these members, the rest of the file reads ok.  assume 
		// everything is an int32_t, when we write we do the same thing.

		// # of tracks for whole song
		psyfile_read(self->file, &temp, sizeof temp);
		songtracks = temp;
		patterns_setsongtracks(&self->song->patterns, songtracks);
		// bpm
		{///\todo: this was a hack added in 1.9alpha to allow decimal bpm values
			int32_t bpmcoarse;
			short temp16 = 0;
			psyfile_read(self->file, &temp16, sizeof temp16);
			bpmcoarse = temp16;
			psyfile_read(self->file, &temp16, sizeof temp16);
			self->song->properties.bpm = bpmcoarse + (temp16 / 100.0f);			
		}
		// linesperbeat
		psyfile_read(self->file, &temp, sizeof temp);
		self->song->properties.lpb = temp;		
		// current octave
		psyfile_read(self->file, &temp, sizeof temp);
		currentoctave = temp;
		// machinesoloed
		// we need to buffer this because destroy machine will clear it
		psyfile_read(self->file, &temp, sizeof temp);
		if (temp >= 0) {
			self->machinesoloed = (uint32_t) temp;			
		} else {
			self->machinesoloed = UINTPTR_MAX;
		}
		// tracksoloed
		psyfile_read(self->file, &temp, sizeof temp);
		_tracksoloed = temp;
		psyfile_read(self->file, &temp, sizeof temp);
		seqbus = temp;
		psyfile_read(self->file, &temp, sizeof temp);
		paramselected = temp;
		psyfile_read(self->file, &temp, sizeof temp);
		auxcolselected = temp;
		psyfile_read(self->file, &temp, sizeof temp);
		instselected = temp;
		// sequence width, for multipattern
		psyfile_read(self->file, &temp,sizeof(temp));
		_trackarmedcount = 0;
		for(i = 0 ; i < songtracks; ++i)
		{
			psyfile_read(self->file, &_trackmuted[i],sizeof(_trackmuted[i]));
			// remember to count them
			psyfile_read(self->file, &_trackarmed[i],sizeof(_trackarmed[i]));
			if(_trackarmed[i]) ++_trackarmedcount;
		}
		if(self->file->currchunk.version == 0) {
			// fix for a bug existing in the song saver in the 1.7.x series
			self->file->currchunk.size = (11 * sizeof(int32_t)) + (songtracks * 2 * 1); //sizeof(bool));
		}
		if(self->file->currchunk.version >= 1) {
			psyfile_read(self->file, &sharetracknames, 1);
			self->song->patterns.sharetracknames = sharetracknames;
			if( sharetracknames) {
				int32_t t;
				for(t = 0; t < songtracks; t++) {
					char txt[40];
					psyfile_readstring(self->file, txt, 40);
					// changetrackname(0,t,name);
				}
			}
		}
		if (self->file->currchunk.version >= 2) {
			psyfile_read(self->file, &temp, sizeof temp);
			self->song->properties.tpb = temp;
			psyfile_read(self->file, &temp, sizeof temp);
			self->song->properties.extraticksperbeat = temp;
		} else {
			self->song->properties.tpb = 24;
			self->song->properties.extraticksperbeat = 0;
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

void readseqd(psy_audio_SongFile* self, unsigned char* playorder, int32_t* playlength)
{		
	int32_t index;	
		
	if((self->file->currchunk.version&0xffff0000) == VERSION_MAJOR_ZERO)
	{
		// index, for multipattern - for now always 0
		psyfile_read(self->file, &index, sizeof index);
		if (index < MAX_SEQUENCES)
		{
			char ptemp[256];
			int32_t i;
			// play length for this sequence
			*playlength = psyfile_read_int32(self->file);			
			// name, for multipattern, for now unused
			psyfile_readstring(self->file, ptemp, sizeof ptemp);
			for (i = 0; i < *playlength; ++i)
			{
				playorder[i] = (uint8_t) psyfile_read_uint32(self->file);
			}
		}
	}	
}

void readpatd(psy_audio_SongFile* self)
{		
	if((self->file->currchunk.version&0xffff0000) == VERSION_MAJOR_ZERO)
	{		
		int32_t index;
		int32_t temp;
		int32_t lpb;
		psy_dsp_beat_t bpl;
		char patternname[MAX_PATTERNS][32];
		/// number of lines of each pattern
		int32_t patternlines[MAX_PATTERNS];
		int32_t songtracks;
		// index

		lpb = self->song->properties.lpb;
		bpl = 1 / (psy_dsp_beat_t) lpb;
		psyfile_read(self->file, &index, sizeof index);
		if (index < MAX_PATTERNS)
		{
			uint32_t sizez77 = 0;
			byte* psource;
			byte* pdest;
			int32_t y;
			size_t destsize;
			// num lines
			psyfile_read(self->file, &temp, sizeof temp);
			// clear it out if it already exists
//			removepattern(index);
			patternlines[index] = temp;
			// num tracks per pattern // eventually this may be variable per pattern, like when we get multipattern
			psyfile_read(self->file, &songtracks, sizeof temp);
			psyfile_readstring(self->file, patternname[index], sizeof * patternname);
			psyfile_read(self->file, &sizez77, sizeof sizez77);
			if (self->file->currchunk.version > 1) {
				psyfile_skip(self->file, sizez77);
			} else {
				psource = (byte*)malloc(sizez77);
				psyfile_read(self->file, psource, sizez77);
				beerz77decomp2(psource, &pdest, &destsize);
				free(psource);
				// songtracks = patternlines[index] > 0 ? destsize / ((size_t)patternlines[index] * EVENT_SIZE) : 0;
				{
					psy_audio_Pattern* pattern;
					PatternNode* node = 0;

					psource = pdest;
					pattern = psy_audio_pattern_allocinit();
					patterns_insert(&self->song->patterns, index, pattern);
					psy_audio_pattern_setname(pattern, patternname[index]);
					for (y = 0; y < patternlines[index]; ++y) {
						unsigned char* ptrack = psource;
						uint32_t track;
						psy_dsp_beat_t offset;

						offset = bpl * y;
						for (track = 0; track < patterns_songtracks(&self->song->patterns);
							++track) {
							psy_audio_PatternEvent event;
							// Psy3 PatternEntry format
							// type				offset
							// uint8_t note;		0
							// uint8_t inst;		1
							// uint8_t mach;		2
							// uint8_t cmd;			3
							// uint8_t parameter;	4												
							patternevent_clear(&event);
							event.note = ptrack[0];
							event.inst = (ptrack[1] == 0xFF)
								? event.inst = NOTECOMMANDS_INST_EMPTY
								: ptrack[1];
							event.mach = (ptrack[2] == 0xFF)
								? event.mach = NOTECOMMANDS_MACH_EMPTY
								: ptrack[2];
							event.cmd = ptrack[3];
							event.parameter = ptrack[4];
							if (!patternevent_empty(&event)) {
								node = psy_audio_pattern_insert(pattern, node, track, offset,
									&event);
							}
							ptrack += EVENT_SIZE;
						}
						psource += self->song->patterns.songtracks * EVENT_SIZE;
					}
					pattern->length = patternlines[index] * bpl;
					free(pdest);
					pdest = 0;
				}
				//\ fix for a bug existing in the song saver in the 1.7.x series
				if ((self->file->currchunk.version == 0x0000) && psyfile_getpos(self->file) ==
					self->file->currchunk.begins + self->file->currchunk.size + 4) {
					self->file->currchunk.size += 4;
				}
			}			
		}
		if (self->file->currchunk.version > 0) {
			if (!self->song->patterns.sharetracknames) {
				uint32_t t;
				for (t = 0; t < self->song->patterns.songtracks; t++) {
					char name[40];
					psyfile_readstring(self->file, name, 40);
					// changetrackname(index,t,name);
				}
			}
		}
		if (self->file->currchunk.version > 1) {
			readepat(self);
		}
	}		
}

void readepat(psy_audio_SongFile* self)
{		
	if((self->file->currchunk.version&0xffff0000) == VERSION_MAJOR_ZERO)
	{
		int32_t index;
		int32_t temp;
		float ftemp;				
		char patternname[MAX_PATTERNS][32];		
		uint32_t numpatterns;
		uint32_t numentries;
		uint32_t c;
		uint32_t i;		
			
		psyfile_read(self->file, &temp, sizeof(temp));
		numpatterns = temp;
		for (i = 0; i < numpatterns; ++i) {
			psy_audio_Pattern* pattern;

			pattern = psy_audio_pattern_allocinit();

			psyfile_read(self->file, &temp, sizeof(temp));
			index = temp;			
			patterns_insert(&self->song->patterns, index, pattern);
			// pattern length
			psyfile_read(self->file, &ftemp, sizeof ftemp);
			psy_audio_pattern_setlength(pattern, ftemp);
			// num tracks per pattern // eventually this may be variable per pattern, like when we get multipattern
			psyfile_read(self->file, &temp, sizeof temp );
			psyfile_readstring(self->file, patternname[index], sizeof *patternname);
			// num entries
			psyfile_read(self->file, &temp, sizeof temp);
			numentries = temp;
			for (c = 0; c < numentries; ++c) {
				psy_audio_PatternEntry* entry;
				uint32_t numevents;
				uint32_t j;

				entry = patternentry_alloc();
				entry->events = 0;
				// read track
				psyfile_read(self->file, &temp, sizeof(temp));
				entry->track = temp;
				// read offset
				psyfile_read(self->file, &ftemp, sizeof(temp));				
				entry->offset = ftemp;				
				// num events
				psyfile_read(self->file, &temp, sizeof(temp));				
				numevents = temp;
				// read events
				for (j = 0; j < numevents; ++j) {
					psy_audio_PatternEvent ev;

					patternevent_clear(&ev);
					psyfile_read(self->file, &temp, sizeof(temp));
					ev.note = temp;
					psyfile_read(self->file, &temp, sizeof(temp));
					ev.inst = temp;
					psyfile_read(self->file, &temp, sizeof(temp));
					ev.mach = temp;
					psyfile_read(self->file, &temp, sizeof(temp));
					ev.vol = temp;
					psyfile_read(self->file, &temp, sizeof(temp));
					ev.cmd = temp;
					psyfile_read(self->file, &temp, sizeof(temp));
					ev.parameter = temp;
					patternentry_addevent(entry, &ev);
				}
				psy_list_append(&pattern->events, entry);
			}			
		}
	}		
}

void readinsd(psy_audio_SongFile* self)
{	


	///\verbatim
	/// NNA values overview:
	///
	/// 0 = Note Cut      [Fast Release 'Default']
	/// 1 = Note Release  [Release Stage]
	/// 2 = Note Continue [No NNA]
	///\endverbatim
	unsigned char _NNA;


	int32_t sampler_to_use = -1; // psy_audio_Sampler machine index for lockinst.
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
	/// psy_dsp_Filter Type. See psycle::helpers::dsp::FilterType. [0..6]
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
	
	if((self->file->currchunk.version&0xffff0000) == VERSION_MAJOR_ZERO)
	{
		psyfile_read(self->file, &index, sizeof index);
		if(index < MAX_INSTRUMENTS)
		{	
			psy_audio_Instrument* instrument;
			// Loop stuff
			unsigned char loop;
			int32_t lines;
			psy_audio_NewNoteAction nna;

			instrument = instrument_allocinit();
			psyfile_read(self->file, &loop, sizeof(loop));
			instrument->loop = loop;
			psyfile_read(self->file, &lines, sizeof(lines));
			instrument->lines = lines;
			psyfile_read(self->file, &_NNA, sizeof(_NNA));
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
			instrument_setnna(instrument, nna);

			psyfile_read(self->file, &ENV_AT, sizeof(ENV_AT));
			psyfile_read(self->file, &ENV_DT, sizeof(ENV_DT));
			psyfile_read(self->file, &ENV_SL, sizeof(ENV_SL));
			psyfile_read(self->file, &ENV_RT, sizeof(ENV_RT));

			adsr_settings_setattack(
				&instrument->volumeenvelope, ENV_AT * 1.f/44100);
			adsr_settings_setdecay(
				&instrument->volumeenvelope, ENV_DT * 1.f/44100);
			adsr_settings_setsustain(
				&instrument->volumeenvelope, ENV_SL / 100.f);
			adsr_settings_setrelease(
				&instrument->volumeenvelope, ENV_RT * 1.f/44100);
			
			psyfile_read(self->file, &ENV_F_AT, sizeof(ENV_F_AT));
			psyfile_read(self->file, &ENV_F_DT, sizeof(ENV_F_DT));
			psyfile_read(self->file, &ENV_F_SL, sizeof(ENV_F_SL));
			psyfile_read(self->file, &ENV_F_RT, sizeof(ENV_F_RT));

			adsr_settings_setattack(
				&instrument->filterenvelope, ENV_AT * 1.f/44100);
			adsr_settings_setdecay(
				&instrument->filterenvelope, ENV_F_DT * 1.f/44100);
			adsr_settings_setsustain(
				&instrument->filterenvelope, ENV_F_SL / 128.f);
			adsr_settings_setrelease(
				&instrument->filterenvelope, ENV_F_RT * 1.f/44100);

			psyfile_read(self->file, &ENV_F_CO, sizeof(ENV_F_CO));
			psyfile_read(self->file, &ENV_F_RQ, sizeof(ENV_F_RQ));
			psyfile_read(self->file, &ENV_F_EA, sizeof(ENV_F_EA));

			instrument->filtercutoff = ENV_F_CO / 127.f;
			instrument->filterres = ENV_F_RQ / 127.f;
			instrument->filtermodamount = ENV_F_EA / 255.f + 0.5f;
			
			psyfile_read(self->file, &val, sizeof(val));
			ENV_F_TP = val;
			instrument->filtertype = (FilterType) val;

			psyfile_read(self->file, &pan, sizeof(pan));
			psyfile_read(self->file, &_RPAN, sizeof(_RPAN));
			psyfile_read(self->file, &_RCUT, sizeof(_RCUT));
			psyfile_read(self->file, &_RRES, sizeof(_RRES));

			instrument->randompan = _RPAN;
			instrument->_RCUT = _RCUT;
			instrument->_RRES = _RRES;			
			
			psyfile_readstring(self->file, instrum_name,sizeof(instrum_name));

			// now we have to read waves			
			psyfile_read(self->file, &numwaves, sizeof(numwaves));
			for (i = 0; i < numwaves; i++)
			{
				loadwavesubchunk(self, index, pan, instrum_name, 1, i );
			}

			if ((self->file->currchunk.version & 0xFF) >= 1) 
			{ //revision 1 or greater
				psyfile_read(self->file, &sampler_to_use, sizeof(sampler_to_use));
				psyfile_read(self->file, &_LOCKINST, sizeof(_LOCKINST));
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
			instrument_setname(instrument, instrum_name);
			instruments_insert(&self->song->instruments, instrument, index);			
		}
	}	
}

void readeins(psy_audio_SongFile* self)
{   
	// Legacy for sampulse previous to Psycle 1.12	
	// Version zero was the development version (1.7 alphas, psycle-core).
	// Version one is the published one (1.8.5 onwards).
	size_t filepos = psyfile_getpos(self->file);
	uint32_t  numSamples;
	uint32_t i;
	if ((self->file->currchunk.version&0xFFFF0000) == VERSION_MAJOR_ONE)
	{
		char temp[8];
		// lowversion 0 is 1.8.5, lowversion 1 is 1.8.6
		uint32_t lowversion = (self->file->currchunk.version&0xFFFF);
		int idx;		
		// Instrument Data Load
		uint32_t numInstruments;
		psyfile_read(self->file, &numInstruments, sizeof(numInstruments));
		for(i = 0; i < numInstruments && filepos < self->file->currchunk.begins +
				self->file->currchunk.size; i++) {
			uint32_t sizeINST=0;

			psyfile_read(self->file, &idx, sizeof(idx));
			psyfile_read(self->file, &temp, 4);
			temp[4]='\0';
			psyfile_read(self->file, &sizeINST, sizeof(sizeINST));
			filepos = psyfile_getpos(self->file);
			if (strcmp(temp,"INST")== 0) {
				uint32_t versionINST;
				psyfile_read(self->file, &versionINST, sizeof(versionINST));
				if (versionINST == 1) {
					bool legacyenabled;
					psyfile_read(self->file, &legacyenabled, sizeof(legacyenabled));
				} else {
					//versionINST 0 was not stored, so seek back.
					psyfile_seek(self->file, filepos);
					versionINST = 0;
				}
//				XMInstrument inst;
				loadxminstrument(self, 0, TRUE, lowversion);
//				inst.Load(*pFile, versionINST, true, lowversion);
//				xminstruments.SetInst(inst,idx);
			}
			if (lowversion > 0) {
				//Version 0 doesn't write the chunk size correctly
				//so we cannot correct it in case of error
				psyfile_seek(self->file, filepos + sizeINST);
				filepos = psyfile_getpos(self->file);
			}
		}
		

		psyfile_read(self->file, &numSamples, sizeof(numSamples));
		for(i = 0;i < numSamples && filepos <
			self->file->currchunk.begins + self->file->currchunk.size;i++)
		{
			char temp[8];
			uint32_t versionSMPD;
			uint32_t sizeSMPD=0;

			psyfile_read(self->file, &idx, sizeof(idx));
			psyfile_read(self->file, &temp, 4);
			temp[4]='\0';
			psyfile_read(self->file, &sizeSMPD, sizeof(sizeSMPD));
			filepos = psyfile_getpos(self->file);
			if (strcmp(temp,"SMPD")== 0)
			{
				psy_audio_Sample* wave;
				psyfile_read(self->file, &versionSMPD, sizeof(versionSMPD));
				//versionSMPD 0 was not stored, so seek back.
				if (versionSMPD != 1) {
					psyfile_seek(self->file, filepos);					
					versionSMPD = 0;
				}
				wave = xmloadwav(self);
				psy_audio_samples_insert(&self->song->samples, wave,
					sampleindex_make(idx, 0));
				// XMInstrument::WaveData<> wave;
				// wave.Load(*pFile, versionSMPD, true);
				// samples.SetSample(wave, idx);
			}
			if (lowversion > 0) {
				//Version 0 doesn't write the chunk size correctly
				//so we cannot correct it in case of error
				psyfile_seek(self->file, filepos + sizeSMPD);
				filepos = psyfile_getpos(self->file);
			}
		}
	}	
}

void readsmid(psy_audio_SongFile* self)
{	
	if((self->file->currchunk.version & 0xFFFF0000) == VERSION_MAJOR_ZERO) {
		uint32_t index;

		psyfile_read(self->file, &index, sizeof(index));
		if (index < MAX_INSTRUMENTS) {
			psy_audio_Instrument* instrument;

			if (!instruments_at(&self->song->instruments, index)) {
				instrument = instrument_allocinit();
				instruments_insert(&self->song->instruments, instrument,
					index);
			}
			instrument = instruments_at(&self->song->instruments, index);
			loadxminstrument(self, instrument, 0, self->file->currchunk.version &
				0xFFFF);
		}
	}	
}

psy_audio_Sample* xmloadwav(psy_audio_SongFile* self)
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

	wave = sample_allocinit(2);
	psyfile_readstring(self->file, wavename, sizeof(wavename));
	sample_setname(wave, wavename);
	// wavelength
	psyfile_read(self->file, &temp, sizeof(temp));
	wave->numframes = temp;
	// global volume
	psyfile_read(self->file, &ftemp, sizeof(ftemp));
	wave->globalvolume = ftemp;
	// default volume
	psyfile_read(self->file, &temp16, sizeof(temp16));
	wave->defaultvolume = temp16 / (psy_dsp_amp_t) 255;
	// wave loop start
	psyfile_read(self->file, &temp, sizeof(temp));
	wave->loop.start = temp;
	// wave loop end
	psyfile_read(self->file, &temp, sizeof(temp));
	wave->loop.end = temp;
	// wave loop type				
	psyfile_read(self->file, &temp, sizeof(temp));
	wave->loop.type = (psy_audio_SampleLoopType) temp;
	// wave sustain loop start
	psyfile_read(self->file, &temp, sizeof(temp));
	wave->sustainloop.start = temp;
	// wave sustain loop end
	psyfile_read(self->file, &temp, sizeof(temp));
	wave->sustainloop.end = temp;
	psyfile_read(self->file, &temp, sizeof(temp));
	wave->sustainloop.type = (psy_audio_SampleLoopType) temp;			
	// "bigger than" insted of "bigger or equal", because that means 
	// interpolate between loopend and loopstart
	if (wave->loop.end > wave->numframes) {
		wave->loop.end = wave->numframes;
	}
	if (wave->sustainloop.end > wave->numframes) {
		wave->sustainloop.end = wave->numframes;
	} 
	if (self->file->currchunk.version == 0) {
		wave->samplerate = 8363;
	}
	else {
		psyfile_read(self->file, &temp, sizeof(temp));
		wave->samplerate = temp;				
	}
	// wave tune
	psyfile_read(self->file, &temp16, sizeof(temp16));
	wave->tune = temp16;
	// wave fine tune
	psyfile_read(self->file, &temp16, sizeof(temp16));
	wave->finetune = temp16;			
	// wave stereo
	psyfile_read(self->file, &btemp, sizeof(btemp));
	wave->stereo = btemp;
	// pan enabled
	psyfile_read(self->file, &btemp, sizeof(btemp));
	wave->panenabled = btemp;
	// pan factor
	psyfile_read(self->file, &ftemp, sizeof(ftemp));
	wave->panfactor = ftemp;			
	if(self->file->currchunk.version >= 1) {
		// surround
		psyfile_read(self->file, &btemp, sizeof(btemp));
		wave->surround = btemp;				
	}
	else if (wave->panfactor > 1.0f) {
		wave->surround = 1;
		wave->panfactor -= 1.0f;
	} else { 
		wave->surround = 0;
	}
	// vibrato attack
	psyfile_read(self->file, &temp8, sizeof(temp8));
	wave->vibrato.attack = temp8;			
	// vibrato speed
	psyfile_read(self->file, &temp8, sizeof(temp8));
	wave->vibrato.speed = temp8;
	// vibrato depth
	psyfile_read(self->file, &temp8, sizeof(temp8));
	wave->vibrato.depth = temp8;			
	// vibrato type
	psyfile_read(self->file, &temp8, sizeof(temp8));				
	if (temp8 <= psy_audio_WAVEFORMS_RANDOM) {
		// wave->vibratotype = (psy_audio_WaveForms) temp8;
	} else { 
		// wave->vibratotype = (psy_audio_WaveForms) psy_audio_WAVEFORMS_SINUS;
	}														
	{ // wave data
		byte* pData;
		short* pDest;
		uint32_t i;
		psyfile_read(self->file, &size1, sizeof(size1));				
		pData = malloc(size1);
		psyfile_read(self->file, pData, size1);
		sounddesquash(pData, &pDest);
		free(pData);
		wave->channels.samples[0] = dsp.memory_alloc(wave->numframes, sizeof(float));
		for (i = 0; i < wave->numframes; i++) {
			short val = (short) pDest[i];
			wave->channels.samples[0][i] = (float) val;				
		}
		free(pDest);
		pData = 0;
		wave->channels.numchannels = 1;
		if (wave->stereo)
		{
			uint32_t i;
			psyfile_read(self->file, &size2, sizeof(size1));							
			pData = malloc(size2);
			psyfile_read(self->file, pData, size1);
			sounddesquash(pData, &pDest);
			free(pData);
			wave->channels.samples[1] = dsp.memory_alloc(wave->numframes, sizeof(float));
			for (i = 0; i < wave->numframes; i++) {
				short val = (short) pDest[i];
				wave->channels.samples[1][i] = (float) val;					
			}
			free(pDest);
			pData = 0;
			wave->channels.numchannels = 2;
		}		
	}
	return wave;
}

void loadxminstrument(psy_audio_SongFile* self, psy_audio_Instrument* instrument, bool islegacy, uint32_t legacyversion)
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

	psyfile_readstring(self->file, name, 256);
	psyfile_read(self->file, &m_Lines, sizeof(m_Lines));
	if (islegacy) m_Lines = 0;

	psyfile_read(self->file, &m_GlobVol, sizeof(m_GlobVol));
	if (instrument) {
		instrument->globalvolume = m_GlobVol;	
	}
	psyfile_read(self->file, &m_VolumeFadeSpeed, sizeof(m_VolumeFadeSpeed));

	psyfile_read(self->file, &m_InitPan, sizeof(m_InitPan));
	psyfile_read(self->file, &m_PanEnabled, sizeof(m_PanEnabled));
	if (self->file->currchunk.version == 0) {
		m_Surround = FALSE;
	}
	else {
		psyfile_read(self->file, &m_Surround, sizeof(m_Surround));
	}

	psyfile_read(self->file, &m_NoteModPanCenter, sizeof(m_NoteModPanCenter));
	psyfile_read(self->file, &m_NoteModPanSep, sizeof(m_NoteModPanSep));

	psyfile_read(self->file, &m_FilterCutoff, sizeof(m_FilterCutoff));
	psyfile_read(self->file, &m_FilterResonance, sizeof(m_FilterResonance));
	{ 
		uint16_t unused = 0;
		psyfile_read(self->file, &unused, sizeof(unused));
	}
	{
		uint32_t i = 0;
		psyfile_read(self->file, &i, sizeof(i));
		m_FilterType = i;
	}

	psyfile_read(self->file, &m_RandomVolume, sizeof(m_RandomVolume));
	psyfile_read(self->file, &m_RandomPanning, sizeof(m_RandomPanning));
	psyfile_read(self->file, &m_RandomCutoff, sizeof(m_RandomCutoff));
	psyfile_read(self->file, &m_RandomResonance, sizeof(m_RandomResonance));

	{
		uint32_t i = 0;		

		psyfile_read(self->file, &i, sizeof(i));
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
			instrument_setnna(instrument, nna);
		}
		psyfile_read(self->file, &i, sizeof(i));
		m_DCT = i; // static_cast<DupeCheck::Type>(i);
		psyfile_read(self->file, &i, sizeof(i));
		m_DCA = i; //static_cast<NewNoteAction::Type>(i);
	}

	{
		int32_t i;		
		int first = 1;		
		uint8_t targetnote;
		uint8_t sampleslot;
		psy_audio_InstrumentEntry instentry;		

		instrumententry_init(&instentry);
		instentry.sampleindex = sampleindex_make(0, 0);
		if (instrument) {
			instrument_clearentries(instrument);
		}
		for(i = 0; i < note_map_size; i++) {
			int note;

			note = i;
			psyfile_read(self->file, &targetnote, sizeof(targetnote));
			psyfile_read(self->file, &sampleslot, sizeof(sampleslot));			
			if (instrument) {
				if (first) {
					instentry.sampleindex.slot = sampleslot;
					instentry.keyrange.low = note;
					first = 0;
				} else
				if (sampleslot != instentry.sampleindex.slot) {
					instentry.keyrange.high = note - 1;				
					instrument_addentry(instrument, &instentry);
					instentry.keyrange.low = note;
					instentry.sampleindex.slot = sampleslot;
				}
			}
		}
		if (instrument) {
			instentry.keyrange.high = 119;
			instrument_addentry(instrument, &instentry);
		}
	}
	xminstrumentenvelopeload(self, 0, 0);

	if (islegacy && legacyeins==0) {
		//Workaround for a bug in that version
		xminstrumentenvelopeload(self, islegacy, self->file->currchunk.version);  // islegacy, version
		xminstrumentenvelopeload(self, islegacy, self->file->currchunk.version); // riffFile,islegacy, version);
	}
	else {
		xminstrumentenvelopeload(self, islegacy, self->file->currchunk.version); // m_PanEnvelope.Load(riffFile,islegacy, version);
		xminstrumentenvelopeload(self, islegacy, self->file->currchunk.version); // m_FilterEnvelope.Load(riffFile,islegacy, version);
	}
	xminstrumentenvelopeload(self, islegacy, self->file->currchunk.version); // m_PitchEnvelope.Load(riffFile,islegacy, version);

	// ValidateEnabled();		
}
			
/// The meaning of the first value (int), is time, and the unit depends on the context.
typedef struct {
	int first;
	float second;
} PointValue;

void xminstrumentenvelopeload(psy_audio_SongFile* self, bool legacy, uint32_t legacyversion)
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
		psyfile_read(self->file, &temp, 4);
		temp[4]='\0';
		psyfile_read(self->file, &version, sizeof(version));
		psyfile_read(self->file, &size, sizeof(size));
		filepos = psyfile_getpos(self->file);
		if (strcmp("SMIE", temp) !=0 ) {
			psyfile_skip(self->file, size);
			return;
		}
	}
	else {
		version = legacyversion;
	}

	// Information starts here

	psyfile_read(self->file, &m_Enabled, sizeof(m_Enabled));
	psyfile_read(self->file, &m_Carry, sizeof(m_Carry));
	{
		uint32_t i32 = 0;

		psyfile_read(self->file, &i32, sizeof(i32)); m_LoopStart = i32;
		psyfile_read(self->file, &i32, sizeof(i32)); m_LoopEnd = i32;
		psyfile_read(self->file, &i32, sizeof(i32)); m_SustainBegin = i32;
		psyfile_read(self->file, &i32, sizeof(i32)); m_SustainEnd = i32;
	}
	{
		uint32_t num_of_points = 0;
		uint32_t i;

		psyfile_read(self->file, &num_of_points, sizeof(num_of_points));
		for(i = 0; i < num_of_points; i++){
			PointValue value;
			// The time in which this point is placed. The unit depends on the
			// mode.
			psyfile_read(self->file, &value.first, sizeof(value.first));
			// The value that this point has. Depending on the type of
			// envelope, this can range between 0.0 and 1.0 or between -1.0 and
			// 1.0
			psyfile_read(self->file, &value.second, sizeof(value.second));
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
			psyfile_read(self->file, &read, sizeof(read));
			// m_Mode = read;
		}
		psyfile_read(self->file, &m_Adsr, sizeof(m_Adsr));
	}

	// Information ends here
	if (!legacy) {
		psyfile_seek(self->file, filepos + size);
	}
}

void readsmsb(psy_audio_SongFile* self)
{
	if((self->file->currchunk.version&0xFFFF0000) == VERSION_MAJOR_ZERO) {
		uint32_t sampleidx;
		psyfile_read(self->file, &sampleidx, sizeof(sampleidx));
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

			wave = sample_allocinit(2);
			psyfile_readstring(self->file, wavename, sizeof(wavename));
			sample_setname(wave, wavename);
			// wavelength
			psyfile_read(self->file, &temp, sizeof(temp));
			wave->numframes = temp;
			// global volume
			psyfile_read(self->file, &ftemp, sizeof(ftemp));
			wave->globalvolume = ftemp;
			// default volume
			psyfile_read(self->file, &temp16, sizeof(temp16));
			wave->defaultvolume = temp16 / (psy_dsp_amp_t) 255;
			// wave loop start
			psyfile_read(self->file, &temp, sizeof(temp));
			wave->loop.start = temp;
			// wave loop end
			psyfile_read(self->file, &temp, sizeof(temp));
			wave->loop.end = temp;
			// wave loop type				
			psyfile_read(self->file, &temp, sizeof(temp));
			wave->loop.type = (psy_audio_SampleLoopType) temp;
			// wave sustain loop start
			psyfile_read(self->file, &temp, sizeof(temp));
			wave->sustainloop.start = temp;
			// wave sustain loop end
			psyfile_read(self->file, &temp, sizeof(temp));
			wave->sustainloop.end = temp;
			psyfile_read(self->file, &temp, sizeof(temp));
			wave->sustainloop.type = (psy_audio_SampleLoopType) temp;			
			// "bigger than" insted of "bigger or equal", because that means 
			// interpolate between loopend and loopstart
			if (wave->loop.end > wave->numframes) {
				wave->loop.end = wave->numframes;
			}
			if (wave->sustainloop.end > wave->numframes) {
				wave->sustainloop.end = wave->numframes;
			} 
			if (self->file->currchunk.version == 0) {
				wave->samplerate = 8363;
			}
			else {
				psyfile_read(self->file, &temp, sizeof(temp));
				wave->samplerate = temp;				
			}
			// wave tune
			psyfile_read(self->file, &temp16, sizeof(temp16));
			wave->tune = temp16;
			// wave fine tune
			psyfile_read(self->file, &temp16, sizeof(temp16));
			wave->finetune = temp16;			
			// wave stereo
			psyfile_read(self->file, &btemp, sizeof(btemp));
			wave->stereo = btemp;
			// pan enabled
			psyfile_read(self->file, &btemp, sizeof(btemp));
			wave->panenabled = btemp;
			// pan factor
			psyfile_read(self->file, &ftemp, sizeof(ftemp));
			wave->panfactor = ftemp;			
			if(self->file->currchunk.version >= 1) {
				// surround
				psyfile_read(self->file, &btemp, sizeof(btemp));
				wave->surround = btemp;				
			}
			else if (wave->panfactor > 1.0f) {
				wave->surround = 1;
				wave->panfactor -= 1.0f;
			} else { 
				wave->surround = 0;
			}
			// vibrato attack
			psyfile_read(self->file, &temp8, sizeof(temp8));
			wave->vibrato.attack = temp8;			
			// vibrato speed
			psyfile_read(self->file, &temp8, sizeof(temp8));
			wave->vibrato.speed = temp8;
			// vibrato depth
			psyfile_read(self->file, &temp8, sizeof(temp8));
			wave->vibrato.depth = temp8;			
			// vibrato type
			psyfile_read(self->file, &temp8, sizeof(temp8));				
			if (temp8 <= psy_audio_WAVEFORMS_RANDOM) {
				// wave->vibratotype = (psy_audio_WaveForms) temp8;
			} else { 
				// wave->vibratotype = (psy_audio_WaveForms) psy_audio_WAVEFORMS_SINUS;
			}														
			{ // wave data
				byte* pData;
				short* pDest;
				uint32_t i;
				psyfile_read(self->file, &size1, sizeof(size1));				
				pData = malloc(size1);
				psyfile_read(self->file, pData, size1);
				sounddesquash(pData, &pDest);
				free(pData);
				wave->channels.samples[0] = dsp.memory_alloc(wave->numframes, sizeof(float));
				for (i = 0; i < wave->numframes; i++) {
					short val = (short) pDest[i];
					wave->channels.samples[0][i] = (float) val;				
				}
				free(pDest);
				pData = 0;
				wave->channels.numchannels = 1;
				if (wave->stereo)
				{
					uint32_t i;
					psyfile_read(self->file, &size2, sizeof(size2));
					pData = malloc(size2);
					psyfile_read(self->file, pData, size2);
					sounddesquash(pData, &pDest);
					free(pData);
					wave->channels.samples[1] = dsp.memory_alloc(wave->numframes, sizeof(float));
					for (i = 0; i < wave->numframes; i++) {
						short val = (short) pDest[i];
						wave->channels.samples[1][i] = (float) val;					
					}
					free(pDest);
					pData = 0;
					wave->channels.numchannels = 2;
				}
				psy_audio_samples_insert(&self->song->samples, wave,
					sampleindex_make(sampleidx, 0));
			}
		}
	}
}

void loadwavesubchunk(psy_audio_SongFile* self, int32_t instrIdx, int32_t pan, char * instrum_name, int32_t fullopen, int32_t loadIdx)
{
	char Header[8];
	uint32_t version;
	uint32_t size;

	psyfile_read(self->file, &Header,4);
	Header[4] = 0;
	psyfile_read(self->file, &version,sizeof(version));
	psyfile_read(self->file, &size,sizeof(size));

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
		
		sample = sample_allocinit(2);
		sample->panfactor = (float) pan / 256.f ; //(value_mapper::map_256_1(pan));
		sample->samplerate = 44100;				
		legacyindex = psyfile_read_uint32(self->file);
		sample->numframes = psyfile_read_uint32(self->file);
		volume = psyfile_read_uint16(self->file);
		sample->globalvolume = volume * 0.01f;
		sample->loop.start = psyfile_read_uint32(self->file);
		sample->loop.end = psyfile_read_uint32(self->file);		
		sample->tune = psyfile_read_uint32(self->file);		
		psyfile_read(self->file, &tmp, sizeof(tmp));
		//Current sampler uses 100 cents. Older used +-256		
		sample->finetune = (int16_t)(tmp / 2.56f);		
		doloop = psyfile_read_uint8(self->file);
		sample->loop.type = doloop ? psy_audio_SAMPLE_LOOP_NORMAL : psy_audio_SAMPLE_LOOP_DO_NOT;
		stereo = psyfile_read_uint8(self->file);
		sample->stereo = stereo != 0;
		// Old sample name, never used.
		psyfile_readstring(self->file, dummy,sizeof(dummy));
		sample_setname(sample, instrum_name);
		psyfile_read(self->file, &packedsize,sizeof(packedsize));		
		if (fullopen) {
			uint32_t i;
			pData = malloc(packedsize+4);// +4 to avoid any attempt at buffer overflow by the code
			psyfile_read(self->file, pData, packedsize);
			sounddesquash(pData, &pDest);		
			free(pData);
			sample->channels.samples[0] = dsp.memory_alloc(
				sample->numframes, sizeof(float));
			for (i = 0; i < sample->numframes; i++) {
				short val = (short) pDest[i];
				sample->channels.samples[0][i] = (float) val;				
			 }
			free(pDest);
			pData = 0;
			sample->channels.numchannels = 1;
		} else {
			psyfile_skip(self->file, packedsize);
			sample->channels.samples[0] = 0;
		}
		if (sample->stereo) {
			psyfile_read(self->file, &packedsize,sizeof(packedsize));
			if ( fullopen ) {
				uint32_t i;

				// +4 to avoid any attempt at buffer overflow by the code
				pData = malloc(packedsize+4); 
				psyfile_read(self->file, pData,packedsize);
				sounddesquash(pData, &pDest);
				free(pData);
				sample->channels.samples[1] = dsp.memory_alloc(
					sample->numframes, sizeof(float));
				for (i = 0; i < sample->numframes; ++i) {
					short val = (short) pDest[i];
					sample->channels.samples[1][i] = (float) val;					
				}
				free(pDest);
				pData = 0;
				sample->channels.numchannels = 2;
			} else {
				psyfile_skip(self->file, packedsize);
				sample->channels.samples[1] = 0;
			}
		}
		psy_audio_samples_insert(&self->song->samples, sample,
			sampleindex_make(instrIdx, 0));
	} else {
		psyfile_skip(self->file, size);
	}
}

void readmacd(psy_audio_SongFile* self)
{	
	if((self->file->currchunk.version&0xFFFF0000) == VERSION_MAJOR_ZERO)
	{
		int32_t index;
		psyfile_read(self->file, &index, sizeof index);
		if (index == 128) {
			index = index;
		}
		if(index < MAX_MACHINES)
		{			
			psy_audio_Machine* machine;
			
			machine = machineloadchunk(self, index);
			if (machine) {
				psy_audio_machines_insert(&self->song->machines, index, machine);
			}
		}
	}	
}

psy_audio_Machine* machineloadchunk(psy_audio_SongFile* self, int32_t index)
{
	psy_audio_Machine* machine;
	bool replaced;
	char catchername[512];
	char modulename[256];
	char editname[32];	
	int32_t i;
	psy_Table* legacywiretable;
	
	machine = machineloadchunk_createmachine(self, index, modulename, catchername, &replaced);
	{
		unsigned char bypass;
		unsigned char mute;
		int32_t panning;
		int32_t x;
		int32_t y;
		psy_audio_MachineUi* machineui;
		
		psyfile_read(self->file, &bypass, sizeof(bypass));
		psyfile_read(self->file, &mute, sizeof(mute));
		psyfile_read(self->file, &panning, sizeof(panning));
		psyfile_read(self->file, &x, sizeof(x));
		psyfile_read(self->file, &y, sizeof(y));
		psyfile_skip(self->file, 2*sizeof(int32_t));	// numInputs, numOutputs
		
		machineui = psy_audio_songfile_machineui(self, index);
		machineui->x = x;
		machineui->y = y;
		if (bypass) {
			psy_audio_machine_bypass(machine);
		}
		if (mute) {
			psy_audio_machine_mute(machine);
		}
		psy_audio_machine_setpanning(machine, panning / 128.f);
	}
	legacywiretable = (psy_Table*)malloc(sizeof(psy_Table));
	psy_table_init(legacywiretable);
	for (i = 0; i < MAX_CONNECTIONS; ++i) {
		int32_t input;
		int32_t output;
		float inconvol;
		float wiremultiplier;
		unsigned char connection;
		unsigned char incon;
		psy_audio_LegacyWire* legacywire;
						
		// Incoming connections psy_audio_Machine number
		psyfile_read(self->file, &input ,sizeof(input));		
		// Outgoing connections psy_audio_Machine number
		psyfile_read(self->file, &output, sizeof(output));
		// Incoming connections psy_audio_Machine vol
		psyfile_read(self->file, &inconvol, sizeof(inconvol));
		// Value to multiply _inputConVol[] to have a 0.0...1.0 range
		psyfile_read(self->file, &wiremultiplier, sizeof(wiremultiplier));
		// Outgoing connections activated
		psyfile_read(self->file, &connection, sizeof(connection));
		// Incoming connections activated
		psyfile_read(self->file, &incon, sizeof(incon));		
		legacywire = psy_audio_legacywire_allocinit_all(input, incon, inconvol,
			wiremultiplier, output, connection);
		if (legacywire) {
			psy_table_insert(legacywiretable, (uintptr_t)i, (void*)legacywire);
		}
	}
	psy_table_insert(&self->legacywires.legacywires, index, legacywiretable);
	psyfile_readstring(self->file, editname, 32);
	if (replaced) {
		char text[256];

		strcpy(text, "X!");
		strcat(text, editname);
		psy_audio_machine_seteditname(machine, text);
		psy_audio_songfile_warn(self, "replaced missing module ");
		psy_audio_songfile_warn(self, modulename);
		psy_audio_songfile_warn(self, " aka ");
		psy_audio_songfile_warn(self, editname);
		psy_audio_songfile_warn(self, " with dummy-plug\n");
	} else {
		psy_audio_machine_seteditname(machine, editname);
	}	
	psy_audio_machine_loadspecific(machine, self, index);
	if (self->file->currchunk.version >= 1) {
		//TODO: What to do on possibly wrong wire load?
		psy_audio_machine_loadwiremapping(machine, self, index);
	}
	return machine;	
}

psy_audio_Machine* machineloadchunk_createmachine(psy_audio_SongFile* self,
	int32_t index, char* modulename, char* catchername, bool* replaced)
{
	psy_audio_Machine* machine;
	int32_t type;	

	*replaced = FALSE;
	modulename[0] = '\0';
	catchername[0] = '\0';
	psyfile_read(self->file, &type, sizeof(type));
	psyfile_readstring(self->file, modulename, 256);
	plugincatcher_catchername(self->song->machinefactory->catcher,
		modulename, catchername, 0);
	// todo shellidx;
	machine = psy_audio_machinefactory_makemachine(self->song->machinefactory, type,
		catchername);
	if (!machine) {
		machine = psy_audio_machinefactory_makemachine(self->song->machinefactory, MACH_DUMMY,
			catchername);
		*replaced = TRUE;
	}
	return machine;
}

//	===================
//	psy3 song save
//	===================
int psy_audio_psy3_save(psy_audio_SongFile* self)
{
	uint32_t chunkcount;
	int status = PSY_OK;

	chunkcount = psy3_chunkcount(self->song);
	if (status = psy3_write_header(self)) {
		return status;
	}
	// the rest of the modules can be arranged in any order
	if (status = psy3_write_songinfo(self)) {
		return status;
	}
	if (status = psy3_write_sngi(self)) {
		return status;
	}
	if (status = psy3_write_seqd(self)) {
		return status;
	} 
	if (status = psy3_write_patd(self)) {
		return status;
	}
	//	
	// if (status = psy3_write_epat(self)) {
		// return status;
	//}	
	if (status = psy3_write_macd(self)) {
		return status;
	}
	if (status = psy3_write_insd(self)) {
		return status;
	}
	if (status = psy3_write_smsb(self)) {
		return status;
	}
	return status;
}

uint32_t psy3_chunkcount(psy_audio_Song* song)
{
	// 3 chunks (INFO, SNGI, SEQD. SONG is not counted as a chunk) plus:
	uint32_t rv = 3;

	// PATD
	rv += (uint32_t) patterns_size(&song->patterns);
	// MACD
	rv += (uint32_t) psy_audio_machines_size(&song->machines);
	// INSD
	rv += (uint32_t) instruments_size(&song->instruments);
	// SMSB
	rv += (uint32_t) psy_audio_samples_groupsize(&song->samples);
	return rv;
}

int psy3_write_header(psy_audio_SongFile* self)
{	
	// id = "PSY3SONG"; // PSY2 was 0.96 to 1.7.2
	uint32_t pos;
	uint32_t chunkcount;
	int status = PSY_OK;

	if (status = psyfile_write(self->file, "PSY3",4)) {
		return status;
	}
	if (status = psyfile_writeheader(self->file, "SONG", CURRENT_FILE_VERSION,
			0, &pos)) {
		return status;
	}
	chunkcount = psy3_chunkcount(self->song);
	if (status = psyfile_write(self->file, &chunkcount, sizeof(chunkcount))) {
		return status;
	}
	if (status = psyfile_writestring(self->file, PSYCLE__TITLE)) {
		return status;
	}
	if (status = psyfile_writestring(self->file, PSYCLE__VERSION)) {
		return status;
	}
	psyfile_updatesize(self->file, pos);
	return status;
}

int psy3_write_songinfo(psy_audio_SongFile* self)
{		
	uint32_t sizepos;
	int status = PSY_OK;
	
	if (status = psyfile_writeheader(self->file, "INFO",
			CURRENT_FILE_VERSION_INFO, 0, &sizepos)) {
		return status;
	}
	if (status = psyfile_writestring(self->file,
			self->song->properties.title)) {
		return status;
	}
	if (status = psyfile_writestring(self->file,
			self->song->properties.credits)) {
		return status;
	}
	if (status = psyfile_writestring(self->file,
			self->song->properties.comments)) {
		return status;
	}
	psyfile_updatesize(self->file, sizepos);
	return status;
}

//	===================
//	song info
//	===================
//	id = "SNGI";
int psy3_write_sngi(psy_audio_SongFile* self)
{
	uint32_t sizepos;	
	uint32_t i;
	int status = PSY_OK;

	if (status = psyfile_writeheader(self->file, "SNGI",
		CURRENT_FILE_VERSION_SNGI, 0, &sizepos)) {
			return status;
	}
	if (status = psyfile_write_int32(self->file, (int32_t)
			self->song->patterns.songtracks)) {
		return status;
	}
	if (status = psyfile_write_int32(self->file, (int32_t)
		self->song->properties.bpm)) {
		return status;
	}
	if (status = psyfile_write_int32(self->file, (int32_t)
		self->song->properties.lpb)) {
		return status;
	}
	if (status = psyfile_write_int32(self->file, (int32_t)
		self->song->properties.octave)) {
		return status;
	}
	// machinesoloed
	if (psy_audio_machines_soloed(&self->song->machines) != UINTPTR_MAX) {

	}
	if (status = psyfile_write_int32(self->file,
		(psy_audio_machines_soloed(&self->song->machines) != UINTPTR_MAX)
		? (int)psy_audio_machines_soloed(&self->song->machines)
		: -1)) {
		return status;
	}
	// tracksoloed
	if (status = psyfile_write_int32(self->file, 0)) {
		return status;
	}
	// seqbus
	if (status = psyfile_write_int32(self->file, 0)) {
		return status;
	}
	// paramselected
	if (status = psyfile_write_int32(self->file, 0)) {
		return status;
	}
	// auxcolselected
	if (status = psyfile_write_int32(self->file, 0)) {
		return status;
	}
	// instselected
	if (status = psyfile_write_int32(self->file, 0)) {
		return status;
	}
	// sequence width
	if (status = psyfile_write_int32(self->file, 0)) {
		return status;
	}
	for (i = 0; i < self->song->patterns.songtracks; ++i) {
		if (status = psyfile_write_uint8(self->file, 0)) {
			return status;
		}
		 // remember to count them
		if (status = psyfile_write_uint8(self->file, 0)) {
			return status;
		}
	}
	// shareTrackNames
	if (status = psyfile_write_uint8(self->file, 0)) {
		return status;
	}
	if (0) {
		uint32_t t;

		for(t = 0; t < self->song->patterns.songtracks; ++t) {
			//_trackNames[0][t]);
			if (status = psyfile_writestring(self->file, "")) {
				return status;
			}; 
		}
	}	
	if (status = psyfile_write_int32(self->file, (int32_t)
		self->song->properties.tpb)) {
		return status;
	}
	if (status = psyfile_write_int32(self->file, (int32_t)
		self->song->properties.extraticksperbeat)) {
		return status;
	}
	psyfile_updatesize(self->file, sizepos);	
	return status;
}			

//	===================
//	sequence data
//	===================
//	id = "SEQD"; 
int psy3_write_seqd(psy_audio_SongFile* self)
{		
	int32_t index;
	int status = PSY_OK;

	for (index = 0; index < MAX_SEQUENCES; ++index)
	{
		uint32_t sizepos;
		// This needs to be replaced to store the Multisequence.
		static char* sequencename = "seq0";
		psy_List* t;
		SequenceTrack* track;

		if (status = psyfile_writeheader(self->file, "SEQD",
				CURRENT_FILE_VERSION_SEQD, 0, &sizepos)) {
			return status;
		}
		// sequence track number
		if (status = psyfile_write_int32(self->file, (int32_t) index)) {
			return status;
		}
		// sequence length
		if (status = psyfile_write_int32(self->file, (int32_t)
				sequence_size(&self->song->sequence,
				self->song->sequence.tracks))) {
			return status;
		}
		// sequence name
		if (status = psyfile_writestring(self->file, sequencename)) {
			return status;
		}
		track = (SequenceTrack*) self->song->sequence.tracks->entry;
		for (t = track->entries ; t != 0; t = t->next) {
			SequenceEntry* entry;

			entry = (SequenceEntry*) t->entry;
			// sequence data
			if (status = psyfile_write_int32(self->file, (int32_t)
					entry->pattern)) {
				return status;
			}
		}
		psyfile_updatesize(self->file, sizepos);
	}
	return status;
}

//	===================
//	pattern data
//	===================
//	id = "PATD"; 
int psy3_write_patd(psy_audio_SongFile* self)
{	
	int32_t i;
	int32_t temp;
	unsigned char shareTrackNames;
	int status = PSY_OK;

	for (i = 0; i < MAX_PATTERNS; ++i) {
		// check every pattern for validity
		if (sequence_patternused(&self->song->sequence, i)) {
			// ok save it
			psy_audio_Pattern* pattern;
			int32_t patternLines;
			int32_t lpb;
			unsigned char* source;
			unsigned char* copy;
			int32_t y;
			uint32_t t;
			int32_t index;
			PatternNode* node;
			int32_t size77;
			size_t patsize;
			uint32_t sizepos;
			
			pattern = patterns_at(&self->song->patterns, i);
			lpb = self->song->properties.lpb;
			patternLines = (int32_t) (pattern->length * lpb + 0.5);
			patsize = self->song->patterns.songtracks *
				patternLines * EVENT_SIZE;			

			// clear source
			source = malloc(patsize);			
			copy = source;
			for (y = 0; y < patternLines; ++y) {
				for (t = 0; t < self->song->patterns.songtracks; ++t) {
					unsigned char* data;					

					data = copy + y * self->song->patterns.songtracks * EVENT_SIZE +
						t * EVENT_SIZE;
					// Psy3 PatternEntry format
					// type				offset
					// uint8_t note;		0
					// uint8_t inst;		1
					// uint8_t mach;		2
					// uint8_t cmd;			3
					// uint8_t parameter;	4

					// empty entry					
					data[0] = 255;
					data[1] = 255;
					data[2] = 255;
					data[3] = 0;
					data[4] = 0;
				}
			}
			
			for (node = pattern->events; node != 0; node = node->next) {
				unsigned char* data;
				psy_audio_PatternEntry* entry;
				int32_t y;
				int32_t t;
					
				entry = (psy_audio_PatternEntry*) node->entry;
				y = (int32_t) (entry->offset * lpb);
				t = entry->track;
				data = copy + y * self->song->patterns.songtracks * EVENT_SIZE
						+ t * EVENT_SIZE;
				// Psy3 PatternEntry format
				// type				offset
				// uint8_t note;		0
				// uint8_t inst;		1
				// uint8_t mach;		2
				// uint8_t cmd;			3
				// uint8_t parameter;	4
				data[0] = patternentry_front(entry)->note;
				data[1] = (uint8_t)(patternentry_front(entry)->inst & 0xFF);
				data[2] = patternentry_front(entry)->mach;
				data[3] = patternentry_front(entry)->cmd;
				data[4] = patternentry_front(entry)->parameter;
			}			
			size77 = beerz77comp2(source, 
				&copy, self->song->patterns.songtracks * patternLines *
				EVENT_SIZE);
			free(source);
			source = 0;			
			if (status = psyfile_writeheader(self->file, "PATD",
				CURRENT_FILE_VERSION_PATD, 0, &sizepos)) {
				return status;
			}
			index = i; // index
			psyfile_write(self->file, &index, sizeof(index));
			temp = patternLines;
			psyfile_write(self->file, &temp, sizeof(temp));
			temp = self->song->patterns.songtracks; // eventually this may be variable per pattern
			psyfile_write(self->file, &temp, sizeof(temp));

			psyfile_writestring(self->file, psy_audio_pattern_name(pattern));

			psyfile_write(self->file, &size77, sizeof(size77));
			psyfile_write(self->file, copy, size77);
			free(copy);
			copy = 0;			
			
			shareTrackNames = 0;
			if( !shareTrackNames) {
				uint32_t t;
				for(t = 0; t < self->song->patterns.songtracks; ++t) {
					psyfile_writestring(self->file, ""); //_trackNames[i][t]);
				}
			}
			psy3_write_epat(self);
			psyfile_updatesize(self->file, sizepos);
		}
	}
	return status;
}

//	===================
//	extended pattern data
//	===================
int psy3_write_epat(psy_audio_SongFile* self)
{	
	psy_TableIterator it;
	int status = PSY_OK;
	int c;

	c = 0;
	// count number of valid patterns
	for (it = psy_table_begin(&self->song->patterns.slots);
			!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
		// check every pattern for validity
		if (sequence_patternused(&self->song->sequence,
				psy_tableiterator_key(&it))) {
			++c;
		}
	}
	// write number of valid patterns;
	if (status = psyfile_write_int32(self->file,
		(uint32_t) c)) {
		return status;
	}	
	for (it = psy_table_begin(&self->song->patterns.slots);
			!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
		// check every pattern for validity
		if (sequence_patternused(&self->song->sequence,
				psy_tableiterator_key(&it))) {			
			psy_audio_Pattern* pattern;										
			int32_t index;
			PatternNode* node;						
		
			// ok save it
			pattern = (psy_audio_Pattern*) psy_tableiterator_value(&it);
			index = psy_tableiterator_key(&it);			
			if (status = psyfile_write_int32(self->file, index)) {
				return status;
			}
			// length
			if (status = psyfile_write_float(self->file, pattern->length)) {
				return status;
			}
			// num songtracks, eventually this may be variable per pattern
			if (status = psyfile_write_int32(self->file,
					(uint32_t) self->song->patterns.songtracks)) {
				return status;
			}
			// pattern label
			psyfile_writestring(self->file, psy_audio_pattern_name(pattern));
			// num pattern entries
			if (status = psyfile_write_int32(self->file,
					psy_list_size(pattern->events))) {
				return status;
			}			
			// Write Events
			for (node = pattern->events; node != 0; node = node->next) {
				psy_audio_PatternEntry* entry;
				psy_List* p;
				
				entry = (psy_audio_PatternEntry*) node->entry;
				if (status = psyfile_write_int32(self->file, entry->track)) {
					return status;
				}
				if (status = psyfile_write_float(self->file, entry->offset)) {
					return status;
				}				
				// num events
				if (status = psyfile_write_int32(self->file,
						psy_list_size(entry->events))) {
					return status;
				}				
				entry = (psy_audio_PatternEntry*) node->entry;				
				for (p = entry->events; p != NULL; p = p->next) {
					psy_audio_PatternEvent* ev;

					ev = (psy_audio_PatternEvent*) p->entry;
					if (status = psyfile_write_int32(self->file, ev->note)) {
						return status;
					}
					if (status = psyfile_write_int32(self->file, ev->inst)) {
						return status;
					}
					if (status = psyfile_write_int32(self->file, ev->mach)) {
						return status;
					}
					if (status = psyfile_write_int32(self->file, ev->vol)) {
						return status;
					}
					if (status = psyfile_write_int32(self->file, ev->cmd)) {
						return status;
					}
					if (status = psyfile_write_int32(self->file, ev->parameter)) {
						return status;
					}
				}
			}						
		}
	}
	return status;
}

//	===================
//	machine data
//	===================
//	id = "MACD"; 

int psy3_write_macd(psy_audio_SongFile* self)
{
	int32_t i;
	int status = PSY_OK;

	for (i = 0; i < MAX_MACHINES; ++i) {
		psy_audio_Machine* machine;

		machine = psy_audio_machines_at(&self->song->machines, i);
		if (machine) {
			int32_t index;
			uint32_t sizepos;

			if (status = psyfile_writeheader(self->file, "MACD",
					CURRENT_FILE_VERSION_MACD, 0, &sizepos)) {
				return status;
			}
			index = i; // index
			psyfile_write(self->file, &index, sizeof(index));
			psy3_write_machine(self, machine, index);
			psyfile_updatesize(self->file, sizepos);
		}
	}
	return status;
}

int psy3_write_machine(psy_audio_SongFile* self, psy_audio_Machine* machine,
	uint32_t slot)
{
	const psy_audio_MachineInfo* info;
	int status = PSY_OK;

	info = psy_audio_machine_info(machine);
	if (info) {
		psy_audio_MachineUi* machineui;
			
		if (status =psyfile_write_int32(self->file, (int32_t)info->type)) {
			return status;
		}
		psy3_savedllnameandindex(self->file, info->modulepath, info->shellidx);
		if (status = psyfile_write_uint8(self->file, (uint8_t)
				psy_audio_machine_bypassed(machine))) {
			return status;
		}
		if (status = psyfile_write_uint8(self->file, (uint8_t)
				psy_audio_machine_muted(machine))) {
			return status;
		}
		if (status = psyfile_write_int32(self->file, (int32_t)
			(psy_audio_machine_panning(machine) * 128.f))) {
			return status;
		}
		machineui = psy_audio_songfile_machineui(self, slot);
		if (status = psyfile_write_int32(self->file, machineui->x)) {
			return status;
		}
		if (status = psyfile_write_int32(self->file, machineui->y)) {
			return status;
		}
		if (status = psy3_write_connections(self, slot)) {
			return status;
		}
		psyfile_writestring(self->file, psy_audio_machine_editname(machine)
			? psy_audio_machine_editname(machine) : "");
		psy_audio_machine_savespecific(machine, self, slot);
		psy_audio_machine_savewiremapping(machine, self, slot);
		// SaveParamMapping(pFile);
	}
	return status;
}

int psy3_write_connections(psy_audio_SongFile* self, uintptr_t slot)
{
	psy_audio_MachineSockets* sockets;
	psy_List* in;
	psy_List* out;			
	int incon;
	int outcon;
	int i;
	int c;
	int status = PSY_OK;

	sockets = connections_at(&self->song->machines.connections, slot);						
	if (status = psyfile_write_int32(self->file, sockets && sockets->inputs
			? psy_list_size(sockets->inputs) : 0)) {
		return status;
	}
	if (status = psyfile_write_int32(self->file, sockets && sockets->outputs
			? psy_list_size(sockets->outputs) : 0)) {
		return status;
	}
	c = 0;	
	if (sockets) {
		for (in = sockets->inputs, out = sockets->outputs;
				(in || out) && (c < MAX_CONNECTIONS);
				in = in ? in->next : 0, out = out ? out->next : 0, ++c) {
			float invol = 1.f;			
			if (in) {
				psy_audio_WireSocketEntry* entry;

				incon = 1;
				entry = (psy_audio_WireSocketEntry*) in->entry;
				invol = entry->volume;				
				if (status = psyfile_write_int32(self->file, (int32_t)
						(entry->slot))) {
					return status;
				}
			} else {
				incon = 0;
				if (status = psyfile_write_int32(self->file, -1)) {
				return status;
			}
			}
			if (out) {
				psy_audio_WireSocketEntry* entry;

				outcon = 1;
				entry = (psy_audio_WireSocketEntry*) out->entry;
				if (status = psyfile_write_int32(self->file, (int32_t)
						(entry->slot))) {
					return status;
				}
			} else {
				outcon = 0;
				if (status = psyfile_write_int32(self->file, -1)) {
					return status;
				}
			}			
		}
		if (status = psyfile_write_float(self->file, 1.f)) {
			return status;
		}
		if (status = psyfile_write_float(self->file, 1.f)) {
			return status;
		}
		if (status = psyfile_write_uint8(self->file, (uint8_t) outcon)) {
			return status;
		}
		if (status = psyfile_write_uint8(self->file, (uint8_t) incon)) {
			return status;
		}
	}
	// fill not used connections
	for (i = c; i < MAX_CONNECTIONS; ++i) {
		if (status = psyfile_write_int32(self->file, -1)) {
			return status;
		}
		if (status = psyfile_write_int32(self->file, -1)) {
			return status;
		}
		if (status = psyfile_write_float(self->file, 1.f)) {
			return status;
		}
		if (status = psyfile_write_float(self->file, 1.f)) {
			return status;
		}
		if (status = psyfile_write_uint8(self->file, 0)) {
			return status;
		}
		if (status = psyfile_write_uint8(self->file, 0)) {
			return status;
		}
	}
	return status;
}

void psy3_savedllnameandindex(PsyFile* file, const char* path,
	int32_t shellindex)
{
	char str[256];
	char prefix[_MAX_PATH];
	char name[_MAX_PATH];
	char ext[_MAX_PATH];
	char idxtext[8];
	int32_t index;

	str[0] = '\0';
	if (path) {
		idxtext[0] = '\0';
		psy_dir_extract_path(path, prefix, name, ext);
		if (strcmp(ext, "so") == 0) {
			psy_snprintf(ext, 256, "dll");
		}
		index = shellindex;
		if (index != 0) {
			int32_t divisor = 16777216;
			int32_t i;
			idxtext[4] = 0;
			for (i = 0; i < 4; ++i) {
				int32_t residue = index % divisor;
				idxtext[3 - i] = index / divisor;
				index = residue;
				divisor = divisor / 256;
			}
		}
		psy_snprintf(str, 256, "%s.%s%s", name, ext, idxtext);
	}
	psyfile_write(file, str, strlen(str) + 1);
}

//	===================
//	instrument data
//	===================
//	id = "INSD";	
int psy3_write_insd(psy_audio_SongFile* self)
{
	psy_TableIterator it;
	uint32_t sizepos;	
	int status = PSY_OK;

	for (it = psy_table_begin(&self->song->instruments.container);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_Instrument* instrument;
				
		if (status = psyfile_writeheader(self->file, "INSD",
				CURRENT_FILE_VERSION_INSD, 0, &sizepos)) {
			return status;
		}
		if (status = psyfile_write_int32(self->file, (int32_t)
				psy_tableiterator_key(&it))) {
			return status;
		}
		instrument = (psy_audio_Instrument*) psy_tableiterator_value(&it);
		psy3_save_instrument(self, instrument);
		psyfile_updatesize(self->file, sizepos);
	}
	return status;
}

int psy3_save_instrument(psy_audio_SongFile* self,
	psy_audio_Instrument* instrument)
{	
	int status = PSY_OK;
	
	// loop	
	if (status = psyfile_write_uint8(self->file, instrument->loop != FALSE)) {
		return status;
	}
	// lines	
	if (status = psyfile_write_int32(self->file,
			(int32_t) instrument->lines)) {
		return status;
	}	
	if (status = psyfile_write_uint8(self->file, (uint8_t) instrument->nna)) {
		return status;
	}
	// env_at
	if (status = psyfile_write_int32(self->file, (int32_t)
		(adsr_settings_attack(&instrument->volumeenvelope) * 44100 + 0.5f))) {
		return status;
	}
	// env_dt	
	if (status = psyfile_write_int32(self->file, (int32_t)
		(adsr_settings_decay(&instrument->volumeenvelope) * 44100 + 0.5f))) {
		return status;
	}
	// env_sl	
	if (status = psyfile_write_int32(self->file, (int32_t)
			(adsr_settings_sustain(&instrument->volumeenvelope) * 100))) {
		return status;
	}
	// env_rt	
	if (status = psyfile_write_int32(self->file, (int32_t)
		(adsr_settings_release(&instrument->volumeenvelope) * 44100 + 0.5f))) {
		return status;
	}
	// env_f_at
	if (status = psyfile_write_int32(self->file, (int32_t)
		(adsr_settings_attack(&instrument->filterenvelope) * 44100 + 0.5f))) {
		return status;
	}
	// env_f_dt	
	if (status = psyfile_write_int32(self->file, (int32_t)
		(adsr_settings_decay(&instrument->filterenvelope) * 44100 + 0.5f))) {
		return status;
	}
	// env_f_sl	
	if (status = psyfile_write_int32(self->file,
		(int32_t)(adsr_settings_sustain(&instrument->filterenvelope) * 128))) {
		return status;
	}
	// env_f_rt	
	if (status = psyfile_write_int32(self->file, (int32_t)
		(adsr_settings_release(&instrument->filterenvelope) * 44100 + 0.5f))) {
		return status;
	}
	// env_f_co	
	if (status = psyfile_write_int32(self->file,
		(int32_t) instrument->filtercutoff)) {
		return status;
	}
	// env_f_rq	
	if (status = psyfile_write_int32(self->file,
		(int32_t) instrument->filterres)) {
		return status;
	}
	// env_f_ea	
	if (status = psyfile_write_int32(self->file, 0)) {
		return status;
	}
	// env_f_tp	
	if (status = psyfile_write_int32(self->file, 0)) {
		return status;
	}
	// No longer saving pan in version 2
	// legacypan	
	if (status = psyfile_write_int32(self->file, 128)) {
		return status;
	}
	if (status = psyfile_write_uint8(self->file, (uint8_t)
			instrument->randompan)) {
		return status;
	}
	if (status = psyfile_write_uint8(self->file, (uint8_t)
			instrument->_RCUT)) {
		return status;
	}
	if (status = psyfile_write_uint8(self->file, (uint8_t)
			instrument->_RRES)) {
		return status;
	}
	// No longer saving name in version 2	
	if (status = psyfile_write_uint8(self->file, 0)) {
		return status;
	}
	// No longer saving wave subchunk in version 2
	// legacynumwaves;	
	if (status = psyfile_write_int32(self->file, 0)) {
		return status;
	}
	// sampler_to_use
	if (status = psyfile_write_int32(self->file, 0)) {
		return status;
	}
	// lockinst
	if (status = psyfile_write_int32(self->file, 0)) {
		return status;
	}
	return status;
}

//	===================
//	sampulse instrument data
//	===================
//	id = "SMSB"; 

int psy3_write_smsb(psy_audio_SongFile* self)
{
	psy_TableIterator it;
	uint32_t sizepos;	
	int status = PSY_OK;

	for (it = psy_audio_samples_begin(&self->song->samples);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_Sample* sample;
						
		if (status = psyfile_writeheader(self->file, "SMSB",
				CURRENT_FILE_VERSION_SMSB, 0, &sizepos)) {
			return status;
		}
		if (status = psyfile_write_int32(self->file,
				psy_tableiterator_key(&it))) {
			return status;
		}
		sample = psy_audio_samples_at(&self->song->samples,
			sampleindex_make(psy_tableiterator_key(&it), 0));
		if (sample) {
			if (status = psy3_save_sample(self, sample)) {
				return status;
			}
		} else {
			return PSY_ERRFILE;
		}
		psyfile_updatesize(self->file, sizepos);		
	}
	return status;
}

int psy3_save_sample(psy_audio_SongFile* self, psy_audio_Sample* sample)
{			
	unsigned char* data1;
	unsigned char* data2;
	uint32_t size1;
	uint32_t size2;
	short* wavedata_left = 0;
	short* wavedata_right = 0;
	int status = PSY_OK;
	
	if (psy_audio_buffer_numchannels(&sample->channels) > 0) {
		wavedata_left = psy3_floatbuffertoshort(
			psy_audio_buffer_at(&sample->channels, 0), sample->numframes);
		if (!wavedata_left) {
			return PSY_ERRFILE;
		}
	}
	if (psy_audio_buffer_numchannels(&sample->channels) > 1) {
		wavedata_right = psy3_floatbuffertoshort(
			psy_audio_buffer_at(&sample->channels, 1), sample->numframes);
		if (!wavedata_right) {
			free(wavedata_left);
			return PSY_ERRFILE;
		}
	}		
	size1 = (uint32_t)soundsquash(wavedata_left, &data1, sample->numframes);
	if (sample->stereo) {		
		size2 = (uint32_t)soundsquash(wavedata_right, &data2,
			sample->numframes);
	}
	if (status = psyfile_writestring(self->file, sample->name)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint32(self->file, (uint32_t)
			sample->numframes)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_float(self->file, (float)
			sample->globalvolume)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint16(self->file, (uint16_t)
			(sample->defaultvolume * 255))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint32(self->file, (uint32_t)
			sample->loop.start)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint32(self->file, (uint32_t)
			sample->loop.end)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_int32(self->file,
			sample->loop.type)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint32(self->file, (uint32_t)
			sample->sustainloop.start)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint32(self->file, (uint32_t)
			sample->sustainloop.end)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_int32(self->file, sample->sustainloop.type)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint32(self->file, sample->samplerate)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_int16(self->file, sample->tune)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_int16(self->file, sample->finetune)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint8(self->file, sample->stereo)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint8(self->file, (uint8_t)
			sample->panenabled)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_float(self->file, sample->panfactor)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint8(self->file, sample->surround)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint8(self->file, sample->vibrato.attack)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint8(self->file, sample->vibrato.speed)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint8(self->file, sample->vibrato.depth)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint8(self->file, (uint8_t)
			sample->vibrato.type)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint32(self->file, size1)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write(self->file, (void*)data1, size1)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	free(data1);
	free(wavedata_left);	
	if (sample->stereo) {
		if (status = psyfile_write_uint32(self->file, size2)) {
			free(data2);
			free(wavedata_right);
			return status;
		}
		if (status = psyfile_write(self->file, (void*)data2, size2)) {
			free(data2);
			free(wavedata_right);
			return status;
		}
		free(data2);
		free(wavedata_right);
	}
	return status;
}

short* psy3_floatbuffertoshort(float* buffer, uintptr_t numframes)
{		
	short* rv;	
	
	if (buffer && numframes > 0) {
		rv = (short*) malloc(numframes * sizeof(short));
		if (rv) {
			uintptr_t frame;

			for (frame = 0; frame < numframes; ++frame) {
				rv[frame] = (short) buffer[frame];
			}
		}
	} else {
		rv = 0;	
	}
	return rv;
}
