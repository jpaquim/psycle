// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "psy3.h"
#include "songio.h"
// #include "machines.h"
// #include "pattern.h"
#include "constants.h"
#include <datacompression.h>
#include <dir.h>
#include "machinefactory.h"
#include <stdlib.h>
#include <string.h>
#include <portable.h>

#if !defined DIVERSALIS__OS__MICROSOFT
#define _MAX_PATH 4096
#endif

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE 1
#endif

static void readinfo(SongFile*);
static void readsngi(SongFile*);
static void readseqd(SongFile*, unsigned char* playorder, int32_t* playlength);
static void readpatd(SongFile*);
static void readinsd(SongFile*);
static void readmacd(SongFile*, Properties* workspace);
static void readsmsb(SongFile*);
static void makeplugincatchername(const char* psy3dllname, char* catchername);
static void loadwavesubchunk(SongFile*, int32_t instrIdx, int32_t pan, char * instrum_name, int32_t fullopen, int32_t loadIdx);
static Machine* machineloadfilechunk(SongFile*, int32_t index, Properties* workspace);
static void buildsequence(SongFile*, unsigned char* playorder, int playlength);
static void psy3_setinstrumentnames(SongFile*);

static uint32_t psy3_chunkcount(Song*);
static void psy3_writeheader(SongFile*);
static void psy3_writesonginfo(SongFile*);
static void psy3_writesngi(SongFile*);
static void psy3_writeseqd(SongFile*);
static void psy3_writepatd(SongFile*);
static void psy3_writemacd(SongFile*);
static void psy3_writeinsd(SongFile*);
static void psy3_writesmsb(SongFile*);
static void psy3_writemachine(SongFile*, Machine*, int32_t slot);
static void psy3_savedllnameandindex(PsyFile*, const char* name,
	int32_t shellindex);
static Properties* machine_properties(SongFile*, int32_t slot);
static void psy3_saveinstrumentfilechunk(SongFile*, Instrument* instrument);
static void psy3_savesample(SongFile*, Sample*);
static short* psy3_floatbuffertoshort(float* buffer, uintptr_t numframes);

void psy3_load(struct SongFile* self)
{
	char header[9];	
	uint32_t index = 0;
//	int32_t temp;
	int32_t solo = 0;
	int32_t playlength;
	int32_t chunkcount = 0;		
	int32_t progress = 0;
	uint32_t filesize = psyfile_filesize(self->file);
	unsigned char playorder[MAX_SONG_POSITIONS];	
	Properties* machinesproperties;
	
	machinesproperties = properties_createsection(
		self->workspaceproperties, "machines");	
	psyfile_read(self->file, &self->file->fileversion, sizeof(uint32_t));
	psyfile_read(self->file, &self->file->filesize, sizeof(uint32_t));	
	if(self->file->fileversion > CURRENT_FILE_VERSION)
	{
		//MessageBox(0,"This file is from a newer version of Psycle! This process will try to load it anyway.", "Load Warning", MB_OK | MB_ICONERROR);
	}	
	psyfile_read(self->file, &self->file->chunkcount, sizeof(uint32_t));
	if (self->file->fileversion >= 8) {
		char trackername[256];
		char trackervers[256];
		int32_t bytesread;

		psyfile_readstring(self->file, trackername, 256);
		psyfile_readstring(self->file, trackervers, 256);
		bytesread = 4 + strlen(trackername)+strlen(trackervers)+2;
		psyfile_skip(self->file, self->file->filesize - bytesread);// Size of the current Header DATA // This ensures that any extra data is skipped.
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
			readmacd(self, machinesproperties);			
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
			psyfile_seekchunkend(self->file);
		} else
		if (strcmp(header,"SMID") == 0) {
			psyfile_readchunkbegin(self->file);
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
		}
		psy_signal_emit(&self->song->signal_loadprogress, self, 1,
			progress);
	}	
	buildsequence(self, playorder, playlength);	
	psy3_setinstrumentnames(self);
}

void psy3_setinstrumentnames(SongFile* self)
{
	TableIterator it;

	for (it = table_begin(&self->song->instruments.container);
			!tableiterator_equal(&it, table_end()); tableiterator_inc(&it)) {
		Instrument* instrument;
		Sample* sample;

		instrument = tableiterator_value(&it);
		sample = samples_at(&self->song->samples, tableiterator_key(&it));
		instrument_setname(instrument, sample_name(sample));		
	}
}


void buildsequence(SongFile* self, unsigned char* playorder, int playlength)
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

void readinfo(SongFile* self)
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
		song_setproperties(self->song, &songproperties);
		//bugfix. There were songs with incorrect size.
		if(self->file->currchunk.version == 0) {
			self->file->currchunk.size = 
				(uint32_t)(psyfile_getpos(self->file) - 
					self->file->currchunk.begins);
		}
	}
}

void readsngi(SongFile* self)
{		
	int32_t i;
	int32_t temp;
	int32_t songtracks;		
	int32_t currentoctave;
	int32_t solo;
	int32_t _tracksoloed;
	int32_t seqbus;
	int32_t paramselected;
	int32_t auxcolselected;
	int32_t instselected;
	unsigned char _trackmuted[64];
	int32_t _trackarmedcount;
	unsigned char _trackarmed[64];
	int32_t m_ticksperbeat;
	int32_t m_extraticksperline;
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
		solo = temp;
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
		m_ticksperbeat=24;
		m_extraticksperline = 0;
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
			m_ticksperbeat = temp;
			psyfile_read(self->file, &temp, sizeof temp);
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
}

void readseqd(SongFile* self, unsigned char* playorder, int32_t* playlength)
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

unsigned char * CreateNewPattern(void)
{	
	return malloc(MULTIPLY2);	
}

void readpatd(SongFile* self)
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
		if(index < MAX_PATTERNS)
		{
			uint32_t sizez77 = 0;
			byte* psource;
			byte* pdest;
			int32_t y;
			size_t destsize;
			// num lines
			psyfile_read(self->file, &temp, sizeof temp );
			// clear it out if it already exists
//			removepattern(index);
			patternlines[index] = temp;
			// num tracks per pattern // eventually this may be variable per pattern, like when we get multipattern
			psyfile_read(self->file, &songtracks, sizeof temp );
			psyfile_readstring(self->file, patternname[index], sizeof *patternname);
			psyfile_read(self->file, &sizez77, sizeof sizez77);
			psource = (byte*) malloc(sizez77);
			psyfile_read(self->file, psource, sizez77);
			beerz77decomp2(psource, &pdest, &destsize);			
			free(psource);
			// songtracks = patternlines[index] > 0 ? destsize / ((size_t)patternlines[index] * EVENT_SIZE) : 0;
			{				
				Pattern* pattern;				
				PatternNode* node = 0;

				psource = pdest;
				pattern = pattern_allocinit();
				patterns_insert(&self->song->patterns, index, pattern);				
				for(y = 0 ; y < patternlines[index]; ++y) {					
					unsigned char* ptrack = psource;
					int32_t track;			
					psy_dsp_beat_t offset;

					offset = bpl * y;
					for (track = 0; track < self->song->patterns.songtracks; ++track) {
						PatternEvent* event = (PatternEvent*)(ptrack);
						if (!patternevent_empty(event)) {
							node = pattern_insert(pattern, node, track, offset, event);
						}
						ptrack += sizeof(PatternEvent);
					}										
					psource += self->song->patterns.songtracks * EVENT_SIZE;
				}
				pattern->length = patternlines[index] * bpl;
				free(pdest);
				pdest = 0;				
			}
		}
		//\ fix for a bug existing in the song saver in the 1.7.x series
		if ((self->file->currchunk.version == 0x0000) && psyfile_getpos(self->file) == 
				self->file->currchunk.begins + self->file->currchunk.size + 4) {
			self->file->currchunk.size += 4;
		}
		if (self->file->currchunk.version > 0) {
			if( !self->song->patterns.sharetracknames) {
				int32_t t;
				for(t = 0; t < self->song->patterns.songtracks; t++) {
					char name[40];
					psyfile_readstring(self->file, name, 40);
					// changetrackname(index,t,name);
				}
			}
		}
	}		
}

void readinsd(SongFile* self)
{	
///\name Loop stuff
	///\{
	unsigned char _loop;
	int32_t _lines;
	///\}

	///\verbatim
	/// NNA values overview:
	///
	/// 0 = Note Cut      [Fast Release 'Default']
	/// 1 = Note Release  [Release Stage]
	/// 2 = Note Continue [No NNA]
	///\endverbatim
	unsigned char _NNA;


	int32_t sampler_to_use = -1; // Sampler machine index for lockinst.
	unsigned char _LOCKINST;	// Force this instrument number to change the selected machine to use a specific sampler when editing (i.e. when using the pc or midi keyboards, not the notes already existing in a pattern)

	///\name Amplitude Envelope overview:
	///\{
	/// Attack Time [in Samples at 44.1Khz, independently of the real samplerate]
	int32_t ENV_AT;	
	/// Decay Time [in Samples at 44.1Khz, independently of the real samplerate]
	int32_t ENV_DT;	
	/// Sustain Level [in %]
	int32_t ENV_SL;	
	/// Release Time [in Samples at 44.1Khz, independently of the real samplerate]
	int32_t ENV_RT;	
	///\}
	
	///\name Filter 
	///\{
	/// Attack Time [in Samples at 44.1Khz]
	int32_t ENV_F_AT;	
	/// Decay Time [in Samples at 44.1Khz]
	int32_t ENV_F_DT;	
	/// Sustain Level [0..128]
	int32_t ENV_F_SL;	
	/// Release Time [in Samples at 44.1Khz]
	int32_t ENV_F_RT;	

	/// Cutoff Frequency [0-127]
	int32_t ENV_F_CO;	
	/// Resonance [0-127]
	int32_t ENV_F_RQ;	
	/// EnvAmount [-128,128]
	int32_t ENV_F_EA;	
	/// Filter Type. See psycle::helpers::dsp::FilterType. [0..6]
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
			Instrument* instrument;

			instrument = instrument_allocinit();			
			psyfile_read(self->file, &_loop, sizeof(_loop));
			psyfile_read(self->file, &_lines, sizeof(_lines));
			psyfile_read(self->file, &_NNA, sizeof(_NNA));

			instrument->nna = _NNA;

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

			instrument->_RPAN = _RPAN;
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

void readsmsb(SongFile* self)
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
			Sample* wave;

			wave = sample_allocinit();
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
			wave->defaultvolume = temp16 / (psy_dsp_amp_t) 0x80;
			// wave loop start
			psyfile_read(self->file, &temp, sizeof(temp));
			wave->loopstart = temp;
			// wave loop end
			psyfile_read(self->file, &temp, sizeof(temp));
			wave->loopend = temp;
			// wave loop type				
			psyfile_read(self->file, &temp, sizeof(temp));
			wave->looptype = (LoopType) temp;
			// wave sustain loop start
			psyfile_read(self->file, &temp, sizeof(temp));
			wave->sustainloopstart = temp;
			// wave sustain loop end
			psyfile_read(self->file, &temp, sizeof(temp));
			wave->sustainloopend = temp;
			psyfile_read(self->file, &temp, sizeof(temp));
			wave->sustainlooptype = (LoopType) temp;			
			// "bigger than" insted of "bigger or equal", because that means 
			// interpolate between loopend and loopstart
			if (wave->loopend > wave->numframes) {
				wave->loopend = wave->numframes;
			}
			if (wave->sustainloopend > wave->numframes) {
				wave->sustainloopend = wave->numframes;
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
			if (temp8 <= WAVEFORMS_RANDOM) {
				// wave->vibratotype = (WaveForms) temp8;
			} else { 
				// wave->vibratotype = (WaveForms) WAVEFORMS_SINUS;
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
				wave->channels.samples[0] = malloc(sizeof(float)*wave->numframes);
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
					wave->channels.samples[1] = malloc(sizeof(float)*wave->numframes);
					for (i = 0; i < wave->numframes; i++) {
						short val = (short) pDest[i];
						wave->channels.samples[1][i] = (float) val;					
					}
					free(pDest);
					pData = 0;
					wave->channels.numchannels = 2;
				}
				samples_insert(&self->song->samples, wave, sampleidx);
			}
		}
	}
}

void loadwavesubchunk(SongFile* self, int32_t instrIdx, int32_t pan, char * instrum_name, int32_t fullopen, int32_t loadIdx)
{
	char Header[8];
	uint32_t version;
	uint32_t size;

	psyfile_read(self->file, &Header,4);
	Header[4] = 0;
	psyfile_read(self->file, &version,sizeof(version));
	psyfile_read(self->file, &size,sizeof(size));

	//fileformat supports several waves, but sampler only supports one.
	if (strcmp(Header,"WAVE")==0 && version <= CURRENT_FILE_VERSION_WAVE || loadIdx == 0)
	{
		Sample* wave;
		//This index represented the index position of this wave for the instrument. 0 to 15.
		uint32_t legacyindex;
		unsigned short volume = 0;
		int32_t tmp = 0;
		unsigned char doloop = 0;
		char dummy[32];
		uint32_t packedsize;
		byte* pData;
		short* pDest;
		
		wave = sample_allocinit();
		wave->panfactor = (float) pan / 256.f ; //(value_mapper::map_256_1(pan));
		wave->samplerate = 44100;				
		psyfile_read(self->file, &legacyindex,sizeof(legacyindex));
		psyfile_read(self->file, &wave->numframes, sizeof(wave->numframes));
		psyfile_read(self->file, &volume, sizeof volume);
		wave->globalvolume = volume*0.01f;
		psyfile_read(self->file, &wave->loopstart, sizeof wave->loopstart);
		psyfile_read(self->file, &wave->loopend, sizeof wave->loopend);
				
		psyfile_read(self->file, &tmp, sizeof(tmp));
		wave->tune = tmp;
		psyfile_read(self->file, &tmp, sizeof(tmp));
		//Current sampler uses 100 cents. Older used +-256
		tmp = (int32_t)((float)tmp/2.56f);
		wave->finetune = tmp;		
		psyfile_read(self->file, &doloop, sizeof(doloop));
		wave->looptype = doloop ? LOOP_NORMAL : LOOP_DO_NOT;
		psyfile_read(self->file, &wave->stereo, sizeof(&wave->stereo));
		//Old sample name, never used.
		psyfile_readstring(self->file, dummy,sizeof(dummy));
		sample_setname(wave, instrum_name);
		psyfile_read(self->file, &packedsize,sizeof(packedsize));

		
		if ( fullopen )
		{
			uint32_t i;
			pData = malloc(packedsize+4);// +4 to avoid any attempt at buffer overflow by the code
			psyfile_read(self->file, pData, packedsize);
			sounddesquash(pData, &pDest);		
			free(pData);
			wave->channels.samples[0] = malloc(sizeof(float)*wave->numframes);
			for (i = 0; i < wave->numframes; i++) {
				short val = (short) pDest[i];
				wave->channels.samples[0][i] = (float) val;				
			 }
			free(pDest);
			pData = 0;
			wave->channels.numchannels = 1;
		}
		else
		{
			psyfile_skip(self->file, packedsize);

			wave->channels.samples[0] = 0;
		}

		if (wave->stereo)
		{
			psyfile_read(self->file, &packedsize,sizeof(packedsize));
			if ( fullopen )
			{
				uint32_t i;
				pData = malloc(packedsize+4); // +4 to avoid any attempt at buffer overflow by the code
				psyfile_read(self->file, pData,packedsize);
				sounddesquash(pData, &pDest);
				free(pData);
				wave->channels.samples[1] = malloc(sizeof(float)*wave->numframes);
				for (i = 0; i < wave->numframes; i++) {
					short val = (short) pDest[i];
					wave->channels.samples[1][i] = (float) val;					
				}
				free(pDest);
				pData = 0;
				wave->channels.numchannels = 2;
			}
			else
			{
				psyfile_skip(self->file, packedsize);
				wave->channels.samples[1] = 0;
			}
		}
		samples_insert(&self->song->samples, wave, instrIdx);
	}
	else
	{
		psyfile_skip(self->file, size);
	}
}

void readmacd(SongFile* self, Properties* machinesproperties)
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
			Machine* machine;
			Properties* machineproperties;
			
			machineproperties = properties_createsection(machinesproperties, "machine");
			properties_append_int(machineproperties, "index", index, 0, MAX_MACHINES);
			machine = machineloadfilechunk(self, index, machineproperties);
			if (machine) {
				machines_insert(&self->song->machines, index, machine);
			}
		}
	}	
}

Machine* machineloadfilechunk(SongFile* self, int32_t index, Properties* properties)
{
	// assume version 0 for now	
	Machine* machine;
	int32_t type;
	char modulename[256];
	char plugincatchername[256];
	char editname[32];
	int32_t i;
	
	psyfile_read(self->file, &type,sizeof(type));
	psyfile_readstring(self->file, modulename, 256);
	makeplugincatchername(modulename, plugincatchername);
	machine = machinefactory_makemachine(self->song->machinefactory, type,
		plugincatchername);
	if (!machine) {
		machine = machinefactory_makemachine(self->song->machinefactory, MACH_DUMMY, 
			plugincatchername);
		type = MACH_DUMMY;
	}
	
	{
		unsigned char bypass;
		unsigned char mute;
		int32_t panning;
		int32_t x;
		int32_t y;
		
		psyfile_read(self->file, &bypass, sizeof(bypass));
		psyfile_read(self->file, &mute, sizeof(mute));
		psyfile_read(self->file, &panning, sizeof(panning));
		psyfile_read(self->file, &x, sizeof(x));
		psyfile_read(self->file, &y, sizeof(y));
		psyfile_skip(self->file, 2*sizeof(int32_t));	// numInputs, numOutputs				
		properties_append_int(properties, "x", x, 0, 0);
		properties_append_int(properties, "y", y, 0, 0);
		if (bypass) {
			machine->vtable->bypass(machine);
		}
		if (mute) {
			machine->vtable->mute(machine);
		}
		machine->vtable->setpanning(machine, panning / 128.f);
	}	
	
	for(i = 0; i < MAX_CONNECTIONS; i++)
	{
		int32_t input;
		int32_t output;
		float inconvol;
		float wiremultiplier;
		unsigned char connection;
		unsigned char incon;
																					
		// Incoming connections Machine number
		psyfile_read(self->file, &input ,sizeof(input));		
		// Outgoing connections Machine number
		psyfile_read(self->file, &output, sizeof(output));
		// Incoming connections Machine vol
		psyfile_read(self->file, &inconvol, sizeof(inconvol));
		// Value to multiply _inputConVol[] to have a 0.0...1.0 range
		psyfile_read(self->file, &wiremultiplier, sizeof(wiremultiplier));
		// Outgoing connections activated
		psyfile_read(self->file, &connection, sizeof(connection));
		// Incoming connections activated
		psyfile_read(self->file, &incon, sizeof(incon));
		if (connection && output != -1) {					
			machines_connect(&self->song->machines, index, output);
		}
		if (incon && input != -1) {
			machines_connect(&self->song->machines, input, index);
			connections_setwirevolume(&self->song->machines.connections,
				input, index, inconvol);
		}
	}

	psyfile_readstring(self->file, editname, 32);
	if (type == MACH_DUMMY) {
		char txt[40];
		strcpy(txt, "X!");
		strcat(txt, editname);		
	}
	machine->vtable->seteditname(machine, editname);
	machine->vtable->loadspecific(machine, self, index);
	return machine;	
}

static char* replace_char(char* str, char c, char r)
{
	char* p;
	
	for (p = strchr(str, c); p != 0; p = strchr(p + 1, c)) *p = r;
	return p;
}

void makeplugincatchername(const char* modulename, char* catchername)
{
	char prefix[_MAX_PATH];
	char ext[_MAX_PATH];

	extract_path(modulename, prefix, catchername, ext);
	_strlwr(catchername);
	replace_char(catchername, ' ', '-');
	if (strstr(catchername, "blitz")) {
		// strcpy(catchername, "blitzn");
	}
}

void psy3_save(struct SongFile* self)
{
	uint32_t chunkcount;

	chunkcount = psy3_chunkcount(self->song);
	psy3_writeheader(self);
	// the rest of the modules can be arranged in any order
	psy3_writesonginfo(self);
	psy3_writesngi(self);
	psy3_writeseqd(self);
	psy3_writepatd(self);
	psy3_writemacd(self);
	psy3_writeinsd(self);
	psy3_writesmsb(self);
}

uint32_t psy3_chunkcount(Song* song)
{
	// 3 chunks (INFO, SNGI, SEQD. SONG is not counted as a chunk) plus:
	uint32_t rv = 3;

	// PATD
	rv += (uint32_t)patterns_size(&song->patterns);
	// MACD
	rv += (uint32_t)machines_size(&song->machines);
	// INSD
	rv += (uint32_t)instruments_size(&song->instruments);
	// SMSB
	rv += (uint32_t)samples_size(&song->samples);
	return rv;
}

void psy3_writeheader(SongFile* self)
{	
	// id = "PSY3SONG"; // PSY2 was 0.96 to 1.7.2
	uint32_t pos;
	uint32_t chunkcount;

	psyfile_write(self->file, "PSY3",4);
	pos = psyfile_writeheader(self->file, "SONG", CURRENT_FILE_VERSION, 0);
	chunkcount = psy3_chunkcount(self->song);
	psyfile_write(self->file, &chunkcount, sizeof(chunkcount));
	psyfile_writestring(self->file, PSYCLE__TITLE);
	psyfile_writestring(self->file, PSYCLE__VERSION);
	psyfile_updatesize(self->file, pos);
}

void psy3_writesonginfo(SongFile* self)
{		
	uint32_t sizepos;	
	
	sizepos = psyfile_writeheader(self->file, "INFO", CURRENT_FILE_VERSION_INFO, 0);
	psyfile_writestring(self->file, self->song->properties.title);
	psyfile_writestring(self->file, self->song->properties.credits);
	psyfile_writestring(self->file, self->song->properties.comments);		
	psyfile_updatesize(self->file, sizepos);
}

void psy3_writesngi(SongFile* self)
{	
	// ===================
	// 	SONG INFO
	// 	===================
	// 	id = "SNGI";

	uint32_t sizepos;
	int32_t temp;
	unsigned char bTemp = 0;
	int32_t i;

	sizepos = psyfile_writeheader(self->file, "SNGI", CURRENT_FILE_VERSION_SNGI, 0);

	temp = self->song->patterns.songtracks;
	psyfile_write(self->file, &temp, sizeof(temp));
	temp = (int32_t) self->song->properties.bpm;
	psyfile_write(self->file, &temp, sizeof(temp));
	temp = (int32_t) self->song->properties.lpb;
	psyfile_write(self->file, &temp, sizeof(temp));
	temp = self->song->properties.octave;
	psyfile_write(self->file, &temp, sizeof(temp));
	temp = 0; // machineSoloed;
	psyfile_write(self->file, &temp, sizeof(temp));
	temp = 0; // _trackSoloed;
	psyfile_write(self->file, &temp, sizeof(temp));

	temp = 0 ; // seqBus;
	psyfile_write(self->file, &temp, sizeof(temp));

	temp = 0; // paramSelected;
	psyfile_write(self->file, &temp, sizeof(temp));
	temp = 0; // auxcolSelected;
	psyfile_write(self->file, &temp, sizeof(temp));
	temp = 0; // instSelected;
	psyfile_write(self->file, &temp, sizeof(temp));

	temp = 1; // sequence width
	psyfile_write(self->file, &temp, sizeof(temp));

	bTemp = 0;
	for (i = 0; i < self->song->patterns.songtracks; ++i) {
		psyfile_write(self->file, &bTemp, sizeof(bTemp));
		psyfile_write(self->file, &bTemp, sizeof(bTemp)); // remember to count them
	}

	bTemp =  0; // shareTrackNames
	psyfile_write(self->file, &bTemp, sizeof(bTemp));
	if (bTemp) {
		int32_t t;

		for(t = 0; t < self->song->patterns.songtracks; ++t) {
			psyfile_writestring(self->file, ""); //_trackNames[0][t]);
		}
	}
	temp = self->song->properties.tpb;
	psyfile_write(self->file, &temp, sizeof(temp));
	temp = self->song->properties.extraticksperbeat;
	psyfile_write(self->file, &temp, sizeof(temp));
	psyfile_updatesize(self->file, sizepos);
}			


void psy3_writeseqd(SongFile* self)
{	
	//		===================
	//		SEQUENCE DATA
	//		===================
	//		id = "SEQD"; 
	
	int32_t index;	

	for (index=0;index < MAX_SEQUENCES; ++index)
	{
		uint32_t sizepos;
		static char* sequenceName = "seq0"; // This needs to be replaced when converting to Multisequence.
		int32_t temp;		
		List* t;
		SequenceTrack* track;

		sizepos = psyfile_writeheader(self->file, "SEQD", CURRENT_FILE_VERSION_SEQD, 0);
		psyfile_write(self->file, &index, sizeof(index)); // Sequence Track number
		temp = sequence_size(&self->song->sequence, self->song->sequence.tracks);
		psyfile_write(self->file, &temp, sizeof(temp)); // Sequence length
		psyfile_writestring(self->file, sequenceName); // Sequence Name
		
		track = (SequenceTrack*)self->song->sequence.tracks->entry;
		for (t = track->entries ; t != 0; t = t->next) {
			SequenceEntry* entry;

			entry = (SequenceEntry*) t->entry;
			temp = entry->pattern;
			psyfile_write(self->file, &temp, sizeof(temp));	// Sequence data.
		}
		psyfile_updatesize(self->file, sizepos);
	}
}

void psy3_writepatd(SongFile* self)
{

	// ===================
	// PATTERN DATA
	// ===================
	// id = "PATD"; 
	
	int32_t i;
	int32_t temp;
	unsigned char shareTrackNames;

	for (i = 0; i < MAX_PATTERNS; ++i) {
		// check every pattern for validity
		if (sequence_patternused(&self->song->sequence, i)) {
			// ok save it
			Pattern* pattern;
			int32_t patternLines;
			int32_t lpb;
			unsigned char* pSource;
			unsigned char* pCopy;
			int32_t y;
			int32_t t;
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
			pSource = malloc(patsize);			
			pCopy = pSource;			

			for (y = 0; y < patternLines; ++y) {
				for (t = 0; t < self->song->patterns.songtracks; ++t) {
					unsigned char* pData;
					PatternEvent* event;					

					pData = pCopy + y * self->song->patterns.songtracks * EVENT_SIZE +
						t * EVENT_SIZE;
					event = (PatternEvent*) pData;
					patternevent_clear(event);
				}
			}
			

			for (node = pattern->events; node != 0; node = node->next) {
				PatternEvent* event;
				unsigned char* pData;
				PatternEntry* entry;
				int32_t y;
				int32_t t;
					
				entry = (PatternEntry*) node->entry;
				y = (int32_t) (entry->offset * lpb);
				t = entry->track;
				pData = pCopy + y * self->song->patterns.songtracks * EVENT_SIZE +
						t * EVENT_SIZE;
				event = (PatternEvent*) pData;
				*event = entry->event;
			}			
			size77 = beerz77comp2(pSource, 
				&pCopy, self->song->patterns.songtracks*patternLines*EVENT_SIZE);
			free(pSource);
			pSource = 0;			
			sizepos = psyfile_writeheader(self->file, "PATD",
				CURRENT_FILE_VERSION_PATD, 0);
			index = i; // index
			psyfile_write(self->file, &index, sizeof(index));
			temp = patternLines;
			psyfile_write(self->file, &temp, sizeof(temp));
			temp = self->song->patterns.songtracks; // eventually this may be variable per pattern
			psyfile_write(self->file, &temp, sizeof(temp));

			psyfile_writestring(self->file, pattern->label);

			psyfile_write(self->file, &size77, sizeof(size77));
			psyfile_write(self->file, pCopy, size77);
			free(pCopy);
			pCopy = 0;			
			
			shareTrackNames = 0;
			if( !shareTrackNames) {
				int32_t t;
				for(t = 0; t < self->song->patterns.songtracks; ++t) {
					psyfile_writestring(self->file, ""); //_trackNames[i][t]);
				}
			}
			psyfile_updatesize(self->file, sizepos);
		}
	}
}

void psy3_writemacd(SongFile* self)
{

//	===================
//	MACHINE DATA
//	===================
//	id = "MACD"; 

	int32_t i;

	for (i = 0; i < MAX_MACHINES; ++i) {
		Machine* machine;

		machine = machines_at(&self->song->machines, i);
		if (machine) {
			int32_t index;
			uint32_t sizepos;

			sizepos = psyfile_writeheader(self->file, "MACD",
				CURRENT_FILE_VERSION_MACD, 0);
			index = i; // index
			psyfile_write(self->file, &index, sizeof(index));
			psy3_writemachine(self, machine, index);
			psyfile_updatesize(self->file, sizepos);
		}
	}			
}

void psy3_writemachine(SongFile* self, Machine* machine, int32_t slot)
{
	const MachineInfo* info;

	info = machine->vtable->info(machine);
	if (info) {
		int32_t i;
		int32_t unused;
		int32_t temp;
		unsigned char bTemp;
		MachineSockets* sockets;
		const char* tmpName;
		Properties* p;

		unused = -1;
		p = machine_properties(self, slot);
		temp = info->type;
		psyfile_write(self->file, &temp, sizeof(temp));
		psy3_savedllnameandindex(self->file, info->modulepath, info->shellidx);
		bTemp = (unsigned char) machine->vtable->bypassed(machine);
		psyfile_write(self->file, &bTemp, sizeof(bTemp));
		bTemp = (unsigned char) machine->vtable->muted(machine);
		psyfile_write(self->file, &bTemp, sizeof(bTemp));
		temp = (int32_t)(machine->vtable->panning(machine) * 256);
		psyfile_write(self->file, &temp, sizeof(temp));
		temp = p ? properties_int(p, "x", 100) : 100;
		psyfile_write(self->file, &temp, sizeof(temp));
		temp = p ? properties_int(p, "y", 100) : 100;
		psyfile_write(self->file, &temp, sizeof(temp));
		// Connections
		sockets = connections_at(&self->song->machines.connections, slot);
		temp = sockets && sockets->inputs ? list_size(sockets->inputs) : 0;
		psyfile_write(self->file, &temp, sizeof(temp));
		temp = sockets && sockets->outputs ? list_size(sockets->outputs) : 0;
		psyfile_write(self->file, &temp, sizeof(temp));
		for (i = 0; i < MAX_CONNECTIONS; ++i) {
			unsigned char in = 0;
			unsigned char out = 0;

			if (sockets && sockets->inputs) {
				WireSocket* socket = connection_at(sockets->inputs, i);
				if (socket) {
					WireSocketEntry* entry = (WireSocketEntry*)socket->entry;
					temp = (int32_t)entry->slot;
					psyfile_write(self->file, &temp, sizeof(temp));
					in = 1;
				}
				else {
					psyfile_write(self->file, &unused, sizeof(unused));
				}
			}
			else {
				psyfile_write(self->file, &unused, sizeof(unused));
			}
			if (sockets && sockets->outputs) {
				WireSocket* socket = connection_at(sockets->outputs, i);
				if (socket) {
					WireSocketEntry* entry = (WireSocketEntry*)socket->entry;
					temp = (int32_t)entry->slot;
					psyfile_write(self->file, &temp, sizeof(temp));
					out = 1;
				}
				else {
					psyfile_write(self->file, &unused, sizeof(unused));
				}
			}
			else {
				psyfile_write(self->file, &unused, sizeof(unused));
			}
			// float volume; volume = inWires[i].GetVolume();
			// float volMultiplier; volMultiplier = inWires[i].GetVolMultiplier();
			// volume /= volMultiplier;
			{
				float fTemp = 1.f;
				// volume
				psyfile_write(self->file, &fTemp, sizeof(fTemp));	// Incoming connections Machine vol
				// volMultiplier
				psyfile_write(self->file, &fTemp, sizeof(fTemp));	// Value to multiply _inputConVol[] to have a 0.0...1.0 range
				bTemp = out;
				psyfile_write(self->file, &bTemp, sizeof(bTemp)); // Outgoing connections activated
				bTemp = in;
				psyfile_write(self->file, &bTemp, sizeof(bTemp)); // Incoming connections activated
			}
		}
		tmpName = machine->vtable->editname(machine) 
			? machine->vtable->editname(machine) : "";
		psyfile_writestring(self->file, tmpName);
		{
			machine->vtable->savespecific(machine, self, slot);
		}
	}

	/*int32_t type;	
		
	std::string tmpName = _editName;
	pFile->WriteString(tmpName);
	SaveSpecificChunk(pFile);
	SaveWireMapping(pFile);
	SaveParamMapping(pFile);*/
}

void psy3_savedllnameandindex(PsyFile* file, const char* name,
	int32_t shellindex)
{
	char str[256];

	str[0] = '\0';
	if (name) {
		psy_snprintf(str, 256, "%s", name);			
	}
	psyfile_writestring(file, str);

/*	CString str = GetDllName();
	char str2[256];
	if ( str.IsEmpty()) str2[0]=0;
	else strcpy(str2,str.Mid(str.ReverseFind('\\')+1));

	if (index != 0)
	{
		char idxtext[8];
		int32_t divisor=16777216;
		idxtext[4]=0;
		for (int32_t i=0; i < 4; i++)
		{
			int32_t residue = index%divisor;
			idxtext[3-i]=index/divisor;
			index = residue;
			divisor=divisor/256;
		}
		strcat(str2,idxtext);
	}
	pFile->Write(&str2,strlen(str2)+1);*/
}

Properties* machine_properties(SongFile* self, int32_t slot)
{
	Properties* rv = 0;

	if (self->workspaceproperties) {
		Properties* p;

		p = properties_findsection(self->workspaceproperties, "machines");
		if (p) {
			p = p->children;

			while (p != 0) {
				Properties* q;
				
				q = properties_read(p, "index");
				if (q) {
					if (properties_value(q) == slot) {
						rv = p;
						break;
					}
				}
				p = p->next;
			}
		}
	}
	return rv;
}

	
// ===================
// Instrument DATA
// ===================
// id = "INSD";	
void psy3_writeinsd(SongFile* self)
{
	TableIterator it;
	uint32_t sizepos;
	int32_t index;

	for (it = table_begin(&self->song->instruments.container);
			!tableiterator_equal(&it, table_end()); tableiterator_inc(&it)) {
		Instrument* instrument;
		
		instrument = (Instrument*)tableiterator_value(&it);
		sizepos = psyfile_writeheader(self->file, "INSD",
			CURRENT_FILE_VERSION_INSD, 0);
		index = (int32_t) tableiterator_key(&it);
		psyfile_write(self->file, &index, sizeof(index));
		psy3_saveinstrumentfilechunk(self, instrument);
		psyfile_updatesize(self->file, sizepos);
	}	
}

void psy3_saveinstrumentfilechunk(SongFile* self, Instrument* instrument)
{	
	uint32_t temp;
	uint8_t temp8;
	char legacyname;
	
	// _loop
	temp8 = 0;
	psyfile_write(self->file, &temp8, sizeof(temp8));	
	// _lines
	temp = 0;
	psyfile_write(self->file, &temp, sizeof(temp));
	temp8 = instrument->nna;
	psyfile_write(self->file, &temp8, sizeof(temp8));
	// ENV_AT
	temp = adsr_settings_attack(&instrument->volumeenvelope);
	psyfile_write(self->file, &temp, sizeof(temp));
	// ENV_DT
	temp = adsr_settings_decay(&instrument->volumeenvelope);
	psyfile_write(self->file, &temp, sizeof(temp));
	// ENV_SL
	temp = adsr_settings_sustain(&instrument->volumeenvelope);
	psyfile_write(self->file, &temp, sizeof(temp));
	// ENV_RT
	temp = adsr_settings_release(&instrument->volumeenvelope);
	psyfile_write(self->file, &temp, sizeof(temp));

	// ENV_F_AT
	temp = adsr_settings_attack(&instrument->filterenvelope);
	psyfile_write(self->file, &temp, sizeof(temp));
	// ENV_F_DT
	temp = adsr_settings_decay(&instrument->filterenvelope);
	psyfile_write(self->file, &temp, sizeof(temp));
	// ENV_F_SL
	temp = adsr_settings_sustain(&instrument->filterenvelope);
	psyfile_write(self->file, &temp, sizeof(temp));
	// ENV_F_RT
	temp = adsr_settings_release(&instrument->filterenvelope);
	psyfile_write(self->file, &temp, sizeof(temp));
	// ENV_F_CO
	temp = instrument->filtercutoff;
	psyfile_write(self->file, &temp, sizeof(temp));
	// ENV_F_RQ
	temp = instrument->filterres;
	psyfile_write(self->file, &temp, sizeof(temp));
	// ENV_F_EA
	temp = 0;
	psyfile_write(self->file, &temp, sizeof(temp));
	// ENV_F_TP
	temp = 0;
	psyfile_write(self->file, &temp, sizeof(temp));

	//No longer saving pan in version 2
	// legacypan
	temp = 128;
	psyfile_write(self->file, &temp, sizeof(temp));

	temp8 = instrument->_RPAN;
	psyfile_write(self->file, &temp8, sizeof(temp8));
	temp8 = instrument->_RCUT;
	psyfile_write(self->file, &temp8, sizeof(temp8));
	temp8 = instrument->_RRES;
	psyfile_write(self->file, &temp8, sizeof(temp8));

	//No longer saving name in version 2
	legacyname = '\0';
	psyfile_write(self->file, &legacyname, sizeof(legacyname));

	//No longer saving wave subchunk in version 2
	// legacynumwaves;
	temp = 0;
	psyfile_write(self->file, &temp, sizeof(temp));
	// sampler_to_use
	psyfile_write(self->file, &temp, sizeof(temp));
	// _LOCKINST
	temp8 = 0;
	psyfile_write(self->file, &temp8, sizeof(temp8));
}


// ===================
// Sampulse Instrument data
// ===================
// id = "SMSB"; 
void psy3_writesmsb(SongFile* self)
{
	TableIterator it;
	uint32_t sizepos;
	int32_t index;

	for (it = table_begin(&self->song->samples.container);
			!tableiterator_equal(&it, table_end()); tableiterator_inc(&it)) {
		Sample* sample;
		
		sample = (Sample*)tableiterator_value(&it);					
		sizepos = psyfile_writeheader(self->file, "SMSB",
			CURRENT_FILE_VERSION_SMSB, 0);
		index = (int32_t) tableiterator_key(&it);
		psyfile_write(self->file, &index, sizeof(index));
		psy3_savesample(self, sample);
		psyfile_updatesize(self->file, sizepos);		
	}
}

void psy3_savesample(SongFile* self, Sample* sample)
{			
	unsigned char * pData1;
	unsigned char * pData2;
	uint32_t size1;
	uint32_t size2;
	short * wavedatal = 0;
	short * wavedatar = 0;
	
	if (sample->channels.numchannels > 0) {
		wavedatal = psy3_floatbuffertoshort(sample->channels.samples[0],	
			sample->numframes);
	}
	if (sample->channels.numchannels > 1) {
		wavedatar = psy3_floatbuffertoshort(sample->channels.samples[1],
		sample->numframes);
	}
		
	size1 = (uint32_t)soundsquash(wavedatal, &pData1, sample->numframes);
	if (sample->stereo)
	{		
		size2 = (uint32_t)soundsquash(wavedatar, &pData2, sample->numframes);
	}
	psyfile_writestring(self->file, sample->name);	
	psyfile_write_uint32(self->file, sample->numframes);	
	psyfile_write_float(self->file, sample->globalvolume);	
	psyfile_write_uint16(self->file, (uint16_t)(sample->defaultvolume * 0x80));
	psyfile_write_uint32(self->file, sample->loopstart);	
	psyfile_write_uint32(self->file, sample->loopend);	
	psyfile_write_int32(self->file, sample->looptype);	
	psyfile_write_uint32(self->file, sample->sustainloopstart);	
	psyfile_write_uint32(self->file, sample->sustainloopend);	
	psyfile_write_int32(self->file, sample->sustainlooptype);	
	psyfile_write_uint32(self->file, sample->samplerate);	
	psyfile_write_int16(self->file, sample->tune);	
	psyfile_write_int16(self->file, sample->finetune);	
	psyfile_write_uint8(self->file, sample->stereo);	
	psyfile_write_uint8(self->file, (uint8_t) sample->panenabled);	
	psyfile_write_float(self->file, sample->panfactor);	
	psyfile_write_uint8(self->file, sample->surround);	
	psyfile_write_uint8(self->file, sample->vibrato.attack);	
	psyfile_write_uint8(self->file, sample->vibrato.speed);	
	psyfile_write_uint8(self->file, sample->vibrato.depth);	
	psyfile_write_uint8(self->file, (uint8_t) sample->vibrato.type);
	psyfile_write_uint32(self->file, size1);
	psyfile_write(self->file, (void*)pData1, size1);
	free(pData1);
	free(wavedatal);	
	if (sample->stereo)
	{
		psyfile_write_uint32(self->file, size2);
		psyfile_write(self->file, (void*)pData2, size2);
		free(pData2);
		free(wavedatar);
	}
}


short* psy3_floatbuffertoshort(float* buffer, uintptr_t numframes)
{	
	short * rv;
	uintptr_t i;
		
	rv = malloc(numframes * sizeof(short));
	for (i = 0; i < numframes; i++) {		
		rv[i] = (short) buffer[i];		
	}
	return rv;
}
	
