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

#if !defined(FALSE)
#define FALSE 0
#endif

#if !defined(TRUE)
#define TRUE 1
#endif

static void readsngi(SongFile*, uint32_t size, uint32_t version);
static void readseqd(SongFile*, uint32_t size, uint32_t version,
			  unsigned char* playorder, int32_t* playlength);
static void readpatd(SongFile*, uint32_t size, uint32_t version);
static void readinsd(SongFile*, uint32_t size, uint32_t version);
static void readmacd(SongFile*, uint32_t size, uint32_t version, Properties* workspace);
static void makeplugincatchername(const char* psy3dllname, char* catchername);
static void loadwavesubchunk(SongFile*, int32_t instrIdx, int32_t pan, char * instrum_name, int32_t fullopen, int32_t loadIdx);
static Machine* machineloadfilechunk(SongFile*, int32_t index, uint32_t version, Properties* workspace);

static uint32_t psy3_chunkcount(Song*);
static void psy3_writefileheader(SongFile*);
static void psy3_writesonginfo(SongFile*);
static void psy3_writesngi(SongFile*);
static void psy3_writeseqd(SongFile*);
static void psy3_writepatd(SongFile*);
static void psy3_writemacd(SongFile*);
static void psy3_writemachine(SongFile*, Machine*, int32_t slot);
static void psy3_savedllnameandindex(PsyFile*, const char* name,
	int32_t shellindex);
static Properties* machine_properties(SongFile*, int32_t slot);

void psy3_load(SongFile* self)
{
	char header[9];
	uint32_t version = 0;
	uint32_t size = 0;
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
	psyfile_read(self->file, &version,sizeof(version));
	psyfile_read(self->file, &size,sizeof(size));
	psyfile_read(self->file, &chunkcount,sizeof(chunkcount));
	psyfile_skip(self->file, size-sizeof(chunkcount));

	header[4] = 0;
	while(psyfile_read(self->file, header, 4)) {
		if (strcmp(header,"INFO")==0) {
			int32_t curpos=0; 
			uint32_t begins;
			curpos; // not used
			psyfile_read(self->file, &version,sizeof(version));
			psyfile_read(self->file, &size,sizeof(size));
			begins = psyfile_getpos(self->file);
		//	if((version&0xFFFF0000) == VERSION_MAJOR_ZERO)
		//	{
			{
				char name_[129]; char author_[65]; char comments_[65536];
				psyfile_readstring(self->file, name_, sizeof name_);
				psyfile_readstring(self->file, author_, sizeof author_);
				psyfile_readstring(self->file, comments_,sizeof comments_);
				properties_write_string(self->song->properties, "title", name_);
				properties_write_string(self->song->properties, "credits", author_);
				properties_write_string(self->song->properties, "comments", comments_);
				//bugfix. There were songs with incorrect size.
				if(version == 0) {
				//	size= (uint32_t)(psyfile_getpos(self->file) - begins);
				}
			}
		//	}
			progress += 1;
			chunkcount--;
		//	psyfile_skip(self->file, begins + size);
		} else 
		if (strcmp(header,"SNGI")==0) {
			int32_t curpos=0; curpos; // not used
			psyfile_read(self->file, &version,sizeof(version));
			psyfile_read(self->file, &size,sizeof(size));
			chunkcount--;
			readsngi(self, size, version);
			progress += 1;
		//	psyfile_skip(self->file, size);
		} else 
		if (strcmp(header,"SEQD")==0) {
			int32_t curpos=0; curpos; // not used
			psyfile_read(self->file, &version,sizeof(version));
			psyfile_read(self->file, &size,sizeof(size));
			chunkcount--;			
			readseqd(self, size, version, playorder, &playlength);
			progress += 1;
		//	psyfile_skip(self->file, size);
		} else 
		if (strcmp(header,"PATD")==0) {
			int32_t curpos=0; curpos; // not used
			psyfile_read(self->file, &version,sizeof(version));
			psyfile_read(self->file, &size,sizeof(size));
			chunkcount--;
			readpatd(self, size, version);
			progress += 1;
			// psyfile_skip(self->file, size);
		} else
		if (strcmp(header,"MACD")==0) {
			int32_t curpos = 0; // curpos; // not used		
			psyfile_read(self->file, &version,sizeof(version));
			psyfile_read(self->file, &size,sizeof(size));
			chunkcount--;
			readmacd(self, size, version, machinesproperties);
			progress += 1;
			// psyfile_skip(self->file, size);
		} else
		if (strcmp(header,"INSD")==0) {
			int32_t curpos=0; curpos; // not used
			chunkcount--;
			psyfile_read(self->file, &version,sizeof(version));
			psyfile_read(self->file, &size,sizeof(size));			
			readinsd(self, size, version);
			progress += 1;			
			// psyfile_skip(self->file, size);
		} else {
			// we are not at a valid header for some weird reason.  
			// probably there is some extra data.
			// shift back 3 bytes and try again
			psyfile_skip(self->file, -3);
		}
		signal_emit(&self->song->signal_loadprogress, self, 1, progress);
	}
	{
		int32_t i;		
		SequencePosition sequenceposition;
		
		sequenceposition.track =
			sequence_appendtrack(&self->song->sequence, sequencetrack_allocinit());
		for (i = 0; i < playlength; ++i) {			
			sequenceposition.trackposition =
				sequence_last(&self->song->sequence, sequenceposition.track);
			sequence_insert(&self->song->sequence, sequenceposition, playorder[i]);
		}
	}	
}

void readsngi(SongFile* songfile, uint32_t size, uint32_t version)
{		
	int32_t i;
	int32_t temp;
	int32_t songtracks;
	float m_beatspermin;
	int32_t m_linesperbeat;
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

	uint32_t begins = psyfile_getpos(songfile->file);
	if((version&0xFFFF0000) == VERSION_MAJOR_ZERO)
	{
		// why all these temps?  to make sure if someone changes the defs of
		// any of these members, the rest of the file reads ok.  assume 
		// everything is an int32_t, when we write we do the same thing.

		// # of tracks for whole song
		psyfile_read(songfile->file, &temp, sizeof temp);
		songtracks = temp;
		songfile->song->patterns.songtracks = songtracks;
		// bpm
		{///\todo: this was a hack added in 1.9alpha to allow decimal bpm values
			int32_t bpmcoarse;
			short temp16 = 0;
			psyfile_read(songfile->file, &temp16, sizeof temp16);
			bpmcoarse = temp16;
			psyfile_read(songfile->file, &temp16, sizeof temp16);
			m_beatspermin = bpmcoarse + (temp16 / 100.0f);			
			properties_write_double(songfile->song->properties, "bpm", m_beatspermin);
		}
		// linesperbeat
		psyfile_read(songfile->file, &temp, sizeof temp);
		m_linesperbeat = temp;
		properties_write_int(songfile->song->properties, "lpb", m_linesperbeat);
		// current octave
		psyfile_read(songfile->file, &temp, sizeof temp);
		currentoctave = temp;
		// machinesoloed
		// we need to buffer this because destroy machine will clear it
		psyfile_read(songfile->file, &temp, sizeof temp);
		solo = temp;
		// tracksoloed
		psyfile_read(songfile->file, &temp, sizeof temp);
		_tracksoloed = temp;
		psyfile_read(songfile->file, &temp, sizeof temp);
		seqbus = temp;
		psyfile_read(songfile->file, &temp, sizeof temp);
		paramselected = temp;
		psyfile_read(songfile->file, &temp, sizeof temp);
		auxcolselected = temp;
		psyfile_read(songfile->file, &temp, sizeof temp);
		instselected = temp;
		// sequence width, for multipattern
		psyfile_read(songfile->file, &temp,sizeof(temp));
		_trackarmedcount = 0;
		for(i = 0 ; i < songtracks; ++i)
		{
			psyfile_read(songfile->file, &_trackmuted[i],sizeof(_trackmuted[i]));
			// remember to count them
			psyfile_read(songfile->file, &_trackarmed[i],sizeof(_trackarmed[i]));
			if(_trackarmed[i]) ++_trackarmedcount;
		}
		m_ticksperbeat=24;
		m_extraticksperline = 0;
		if(version == 0) {
			// fix for a bug existing in the song saver in the 1.7.x series
			size = (11 * sizeof(int32_t)) + (songtracks * 2 * 1); //sizeof(bool));
		}
		if(version >= 1) {
			psyfile_read(songfile->file, &sharetracknames, 1);
			songfile->song->patterns.sharetracknames = sharetracknames;
			if( sharetracknames) {
				int32_t t;
				for(t = 0; t < songtracks; t++) {
					char txt[40];
					psyfile_readstring(songfile->file, txt, 40);
					// changetrackname(0,t,name);
				}
			}
		}
		if (version >= 2) {
			psyfile_read(songfile->file, &temp, sizeof temp);
			m_ticksperbeat = temp;
			psyfile_read(songfile->file, &temp, sizeof temp);
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
	psyfile_seek(songfile->file, begins + size);
}

void readseqd(SongFile* self, uint32_t size, uint32_t version,
			  unsigned char* playorder, int32_t* playlength)
{	
	int32_t temp;
	int32_t index;	
	
	uint32_t begins = psyfile_getpos(self->file);
	if((version&0xffff0000) == VERSION_MAJOR_ZERO)
	{
		// index, for multipattern - for now always 0
		psyfile_read(self->file, &index, sizeof index);
		if (index < MAX_SEQUENCES)
		{
			char ptemp[256];
			int32_t i;
			// play length for this sequence
			psyfile_read(self->file, &temp, sizeof temp);
			*playlength = temp;
			// name, for multipattern, for now unused
			psyfile_readstring(self->file, ptemp, sizeof ptemp);
			for (i = 0; i < *playlength; ++i)
			{
				psyfile_read(self->file, &temp, sizeof temp);
				playorder[i] = temp;
			}
		}
	}
	psyfile_seek(self->file, begins + size);
}

unsigned char * CreateNewPattern(void)
{	
	return malloc(MULTIPLY2);	
}

void readpatd(SongFile* self, uint32_t size, uint32_t version)
{
	uint32_t begins = psyfile_getpos(self->file);
	if((version&0xffff0000) == VERSION_MAJOR_ZERO)
	{
		int32_t index;
		int32_t temp;
		int32_t lpb;
		beat_t bpl;
		char patternname[MAX_PATTERNS][32];
		/// number of lines of each pattern
		int32_t patternlines[MAX_PATTERNS];
		int32_t songtracks;
		// index

		lpb = properties_int(self->song->properties,  "lpb", 4);
		bpl = 1 / (beat_t)lpb;
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
				//unsigned char* pdata = CreateNewPattern() + (y * MULTIPLY);
				for(y = 0 ; y < patternlines[index] ; ++y)
				{					
					unsigned char* ptrack = psource;
					int32_t track;			
					beat_t offset;

					offset = bpl * y;
					for (track = 0; track < self->song->patterns.songtracks; ++track) {
						PatternEvent* event = (PatternEvent*)(ptrack);
						if (event->note != 255) {
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
		if ((version == 0x0000) && psyfile_getpos(self->file) == begins + size + 4) {
			size += 4;
		}
		if(version > 0) {
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
	psyfile_seek(self->file, begins + size);
}

void readinsd(SongFile* self, uint32_t size, uint32_t version)
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

	uint32_t begins = psyfile_getpos(self->file);
	if((version&0xffff0000) == VERSION_MAJOR_ZERO)
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

			if ((version & 0xFF) >= 1) 
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
	psyfile_seek(self->file, begins + size);
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

void readmacd(SongFile* self, uint32_t size, uint32_t version,
			  Properties* machinesproperties)
{
	uint32_t begins = psyfile_getpos(self->file);
	if((version&0xFFFF0000) == VERSION_MAJOR_ZERO)
	{
		int32_t index;
		psyfile_read(self->file, &index, sizeof index);
		if(index < MAX_MACHINES)
		{			
			Machine* machine;
			Properties* machineproperties;
			
			machineproperties = properties_createsection(machinesproperties, "machine");
			properties_append_int(machineproperties, "index", index, 0, MAX_MACHINES);
			machine = machineloadfilechunk(self, index, version, machineproperties);
			if (machine) {
				machines_insert(&self->song->machines, index, machine);
			}
		}
	}
	psyfile_seek(self->file, begins + size);
}

Machine* machineloadfilechunk(SongFile* self, int32_t index, uint32_t version, Properties* properties)
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
			machine_bypass(machine);
		}
		if (mute) {
			machine_mute(machine);
		}
		machine->setpanning(machine, panning / 128.f);
	}

	if (index == 64) {
		index = index;
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
	machine->seteditname(machine, editname);
	machine->loadspecific(machine, self, index);
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
	char ext[_MAX_PATH];

	extract_path(modulename, catchername, ext);
	_strlwr(catchername);
	replace_char(catchername, ' ', '-');
	if (strstr(catchername, "blitz")) {
		strcpy(catchername, "blitzn");
	}
}

void psy3_save(SongFile* self)
{
	uint32_t chunkcount;

	chunkcount = psy3_chunkcount(self->song);
	psy3_writefileheader(self);
	// the rest of the modules can be arranged in any order
	psy3_writesonginfo(self);
	psy3_writesngi(self);
	psy3_writeseqd(self);
	psy3_writepatd(self);
	psy3_writemacd(self);
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
	// rv += instruments_size(&song->machines);
	return rv;
}

void psy3_writefileheader(SongFile* self)
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
	psyfile_writestring(self->file,
		properties_readstring(self->song->properties, "title", "a Title"));
	psyfile_writestring(self->file,
		properties_readstring(self->song->properties, "credits", "unknown"));
	psyfile_writestring(self->file,
		properties_readstring(self->song->properties, "comments", "no comments"));
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
	temp = properties_int(self->song->properties,  "bpm", 125);
	psyfile_write(self->file, &temp, sizeof(temp));
	temp = properties_int(self->song->properties,  "lpb", 125);
	psyfile_write(self->file, &temp, sizeof(temp));
	temp = properties_int(self->song->properties,  "octave", 125);
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
	temp = properties_int(self->song->properties,  "tpb", 24);
	psyfile_write(self->file, &temp, sizeof(temp));
	temp = properties_int(self->song->properties,  "etpb", 0);
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
			lpb = properties_int(self->song->properties, "lpb", 4);
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

	info = machine->info(machine);
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
		bTemp = machine->bypass;
		psyfile_write(self->file, &bTemp, sizeof(bTemp));
		bTemp = machine->mute;
		psyfile_write(self->file, &bTemp, sizeof(bTemp));
		temp = (int32_t)(machine->panning(machine) * 256);
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
		tmpName = machine->editname(machine) ? machine->editname(machine) : "";
		psyfile_writestring(self->file, tmpName);
		{
			machine->savespecific(machine, self, slot);
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
		_snprintf(str, 256, "%s", name);			
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
