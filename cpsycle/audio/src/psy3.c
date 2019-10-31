// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "psy3.h"
#include "machines.h"
#include "pattern.h"
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

static void readsngi(Song*, PsyFile*, unsigned int size, int version);
static void readseqd(Song*, PsyFile*, unsigned int size, int version,
			  unsigned char* playorder, int* playlength);
static void readpatd(Song*, PsyFile*, unsigned int size, int version);
static void readinsd(Song*, PsyFile*, unsigned int size, int version);
static void readmacd(Song*, PsyFile*, unsigned int size, int version, Properties* workspace);
static void makeplugincatchername(const char* psy3dllname, char* catchername);
static void loadwavesubchunk(Song*, PsyFile*, int instrIdx, int pan, char * instrum_name, int fullopen, int loadIdx);
static Machine* machineloadfilechunk(Song*, PsyFile*, int index, int version,
	Machines* machines,
	Properties* workspace);

static size_t psy3_chunkcount(Song*);
static void psy3_writefileheader(Song*, PsyFile*);
static void psy3_writesonginfo(Song*, PsyFile*);
static void psy3_writesngi(Song*, PsyFile*);
static void psy3_writeseqd(Song*, PsyFile*);
static void psy3_writepatd(Song*, PsyFile*);
static void psy3_writemacd(Song*, PsyFile*);
static void psy3_writemachine(Song*, PsyFile*, Machine*, int slot);
static void psy3_savedllnameandindex(PsyFile*, const char* name,
	int shellindex);

void psy3_load(Song* self, PsyFile* file, char header[9],
	Properties* workspaceproperties)
{
	unsigned int version = 0;
	unsigned int size = 0;
	unsigned int index = 0;
//	int temp;
	int solo = 0;
	int playlength;
	int chunkcount = 0;		
	int progress = 0;
	long filesize = psyfile_filesize(file);
	unsigned char playorder[MAX_SONG_POSITIONS];	
	Properties* machinesproperties;
	
	machinesproperties = 
		properties_createsection(workspaceproperties, "machines");			
	
	psyfile_read(file, &version,sizeof(version));
	psyfile_read(file, &size,sizeof(size));
	psyfile_read(file, &chunkcount,sizeof(chunkcount));
	psyfile_skip(file, size-sizeof(chunkcount));

	header[4]=0;
	while(psyfile_read(file, header, 4)) {
		if (strcmp(header,"INFO")==0) {
			int curpos=0; 
			size_t begins;
			curpos; // not used
			psyfile_read(file, &version,sizeof(version));
			psyfile_read(file, &size,sizeof(size));
			begins = psyfile_getpos(file);
		//	if((version&0xFFFF0000) == VERSION_MAJOR_ZERO)
		//	{
			{
				char name_[129]; char author_[65]; char comments_[65536];
				psyfile_readstring(file, name_, sizeof name_);
				psyfile_readstring(file, author_, sizeof author_);
				psyfile_readstring(file, comments_,sizeof comments_);
				properties_write_string(self->properties, "title", name_);
				properties_write_string(self->properties, "credits", author_);
				properties_write_string(self->properties, "comments", comments_);
				//bugfix. There were songs with incorrect size.
				if(version == 0) {
				//	size= (unsigned int)(psyfile_getpos(file) - begins);
				}
			}
		//	}
			progress += 1;
			chunkcount--;
		//	psyfile_skip(file, begins + size);
		} else 
		if (strcmp(header,"SNGI")==0) {
			int curpos=0; curpos; // not used
			psyfile_read(file, &version,sizeof(version));
			psyfile_read(file, &size,sizeof(size));
			chunkcount--;
			readsngi(self, file, size, version);
			progress += 1;
		//	psyfile_skip(file, size);
		} else 
		if (strcmp(header,"SEQD")==0) {
			int curpos=0; curpos; // not used
			psyfile_read(file, &version,sizeof(version));
			psyfile_read(file, &size,sizeof(size));
			chunkcount--;			
			readseqd(self, file, size, version, playorder, &playlength);
			progress += 1;
		//	psyfile_skip(file, size);
		} else 
		if (strcmp(header,"PATD")==0) {
			int curpos=0; curpos; // not used
			psyfile_read(file, &version,sizeof(version));
			psyfile_read(file, &size,sizeof(size));
			chunkcount--;
			readpatd(self, file, size, version);
			progress += 1;
			// psyfile_skip(file, size);
		} else
		if (strcmp(header,"MACD")==0) {
			int curpos = 0; // curpos; // not used		
			psyfile_read(file, &version,sizeof(version));
			psyfile_read(file, &size,sizeof(size));
			chunkcount--;
			readmacd(self, file, size, version, machinesproperties);
			progress += 1;
			// psyfile_skip(file, size);
		} else
		if (strcmp(header,"INSD")==0) {
			int curpos=0; curpos; // not used
			psyfile_read(file, &version,sizeof(version));
			psyfile_read(file, &size,sizeof(size));
			chunkcount--;
			readinsd(self, file, size, version);
			progress += 1;			
			// psyfile_skip(file, size);
		} else {
			// we are not at a valid header for some weird reason.  
			// probably there is some extra data.
			// shift back 3 bytes and try again
			psyfile_skip(file, -3);
		}
		signal_emit(&self->signal_loadprogress, self, 1, progress);
	}
	{
		int i;		
		SequencePosition sequenceposition;
		
		sequenceposition.track =
			sequence_appendtrack(&self->sequence, sequencetrack_allocinit());		
		for (i = 0; i < playlength; ++i) {			
			sequenceposition.trackposition =
				sequence_last(&self->sequence, sequenceposition.track);
			sequence_insert(&self->sequence, sequenceposition, playorder[i]);
		}
	}	
}

void readsngi(Song* song, PsyFile* file, unsigned int size, int version)
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

	size_t begins = psyfile_getpos(file);
	if((version&0xFFFF0000) == VERSION_MAJOR_ZERO)
	{
		// why all these temps?  to make sure if someone changes the defs of
		// any of these members, the rest of the file reads ok.  assume 
		// everything is an int, when we write we do the same thing.

		// # of tracks for whole song
		psyfile_read(file, &temp, sizeof temp);
		songtracks = temp;
		song->patterns.songtracks = songtracks;
		// bpm
		{///\todo: this was a hack added in 1.9alpha to allow decimal bpm values
			int bpmcoarse;
			short temp16 = 0;
			psyfile_read(file, &temp16, sizeof temp16);
			bpmcoarse = temp16;
			psyfile_read(file, &temp16, sizeof temp16);
			m_beatspermin = bpmcoarse + (temp16 / 100.0f);			
			properties_write_double(song->properties, "bpm", m_beatspermin);
		}
		// linesperbeat
		psyfile_read(file, &temp, sizeof temp);
		m_linesperbeat = temp;
		properties_write_int(song->properties, "lpb", m_linesperbeat);
		// current octave
		psyfile_read(file, &temp, sizeof temp);
		currentoctave = temp;
		// machinesoloed
		// we need to buffer this because destroy machine will clear it
		psyfile_read(file, &temp, sizeof temp);
		solo = temp;
		// tracksoloed
		psyfile_read(file, &temp, sizeof temp);
		_tracksoloed = temp;
		psyfile_read(file, &temp, sizeof temp);  
		seqbus = temp;
		psyfile_read(file, &temp, sizeof temp);  
		paramselected = temp;
		psyfile_read(file, &temp, sizeof temp);  
		auxcolselected = temp;
		psyfile_read(file, &temp, sizeof temp);  
		instselected = temp;
		// sequence width, for multipattern
		psyfile_read(file, &temp,sizeof(temp));
		_trackarmedcount = 0;
		for(i = 0 ; i < songtracks; ++i)
		{
			psyfile_read(file, &_trackmuted[i],sizeof(_trackmuted[i]));
			// remember to count them
			psyfile_read(file, &_trackarmed[i],sizeof(_trackarmed[i]));
			if(_trackarmed[i]) ++_trackarmedcount;
		}
		m_ticksperbeat=24;
		m_extraticksperline = 0;
		if(version == 0) {
			// fix for a bug existing in the song saver in the 1.7.x series
			size = (11 * sizeof(int)) + (songtracks * 2 * 1); //sizeof(bool));
		}
		if(version >= 1) {
			psyfile_read(file, &sharetracknames, 1);
			song->patterns.sharetracknames = sharetracknames;
			if( sharetracknames) {
				int t;
				for(t = 0; t < songtracks; t++) {
					char txt[40];
					psyfile_readstring(file, txt, 40);
					// changetrackname(0,t,name);
				}
			}
		}
		if (version >= 2) {
			psyfile_read(file, &temp, sizeof temp);
			m_ticksperbeat = temp;
			psyfile_read(file, &temp, sizeof temp);
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
	psyfile_seek(file, begins + size);
}

void readseqd(Song* song, PsyFile* file, unsigned int size, int version,
			  unsigned char* playorder, int* playlength)
{	
	int temp;
	int index;	
	
	size_t begins = psyfile_getpos(file);
	if((version&0xffff0000) == VERSION_MAJOR_ZERO)
	{
		// index, for multipattern - for now always 0
		psyfile_read(file, &index, sizeof index);
		if (index < MAX_SEQUENCES)
		{
			char ptemp[256];
			int i;
			// play length for this sequence
			psyfile_read(file, &temp, sizeof temp);
			*playlength = temp;
			// name, for multipattern, for now unused
			psyfile_readstring(file, ptemp, sizeof ptemp);
			for (i = 0; i < *playlength; ++i)
			{
				psyfile_read(file, &temp, sizeof temp);
				playorder[i] = temp;
			}
		}
	}
	psyfile_seek(file, begins + size);
}

unsigned char * CreateNewPattern(void)
{	
	return malloc(MULTIPLY2);	
}

void readpatd(Song* song, PsyFile* file, unsigned int size, int version)
{
	size_t begins = psyfile_getpos(file);
	if((version&0xffff0000) == VERSION_MAJOR_ZERO)
	{
		int index;
		int temp;
		int lpb;
		beat_t bpl;
		char patternname[MAX_PATTERNS][32];
		/// number of lines of each pattern
		int patternlines[MAX_PATTERNS];
		// index

		lpb = properties_int(song->properties,  "lpb", 4);
		bpl = 1 / (beat_t)lpb;
		psyfile_read(file, &index, sizeof index);
		if(index < MAX_PATTERNS)
		{
			unsigned int sizez77 = 0;
			byte* psource;
			byte* pdest;
			int y;
			// num lines
			psyfile_read(file, &temp, sizeof temp );
			// clear it out if it already exists
//			removepattern(index);
			patternlines[index] = temp;
			// num tracks per pattern // eventually this may be variable per pattern, like when we get multipattern
			psyfile_read(file, &temp, sizeof temp );
			psyfile_readstring(file, patternname[index], sizeof *patternname);
			psyfile_read(file, &sizez77, sizeof sizez77);
			psource = (byte*) malloc(sizez77);
			psyfile_read(file, psource, sizez77);			
			beerz77decomp2(psource, &pdest);
			free(psource);
			{				
				Pattern* pattern;				
				PatternNode* node = 0;				

				psource = pdest;
				pattern = pattern_allocinit();
				patterns_insert(&song->patterns, index, pattern);
				//unsigned char* pdata = CreateNewPattern() + (y * MULTIPLY);
				for(y = 0 ; y < patternlines[index] ; ++y)
				{					
					unsigned char* ptrack = psource;
					int track;			
					beat_t offset;

					offset = bpl * y;
					for (track = 0; track < song->patterns.songtracks; ++track) {
						PatternEvent* event = (PatternEvent*)(ptrack);
						if (event->note != 255) {
							node = pattern_insert(pattern, node, track, offset, event);
						}
						ptrack += sizeof(PatternEvent);
					}										
					psource += song->patterns.songtracks * EVENT_SIZE;
				}
				pattern->length = patternlines[index] * bpl;
				free(pdest);
				pdest = 0;				
			}
		}
		//\ fix for a bug existing in the song saver in the 1.7.x series
		if ((version == 0x0000) && psyfile_getpos(file) == (int)(begins+size+4)) {
			size += 4;
		}
		if(version > 0) {
			if( !song->patterns.sharetracknames) {
				int t;
				for(t = 0; t < song->patterns.songtracks; t++) {
					char name[40];
					psyfile_readstring(file, name, 40);
					// changetrackname(index,t,name);
				}
			}
		}
	}
	psyfile_seek(file, begins + size);
}

void readinsd(Song* song, PsyFile* file, unsigned int size, int version)
{	
///\name Loop stuff
	///\{
	unsigned char _loop;
	int _lines;
	///\}

	///\verbatim
	/// NNA values overview:
	///
	/// 0 = Note Cut      [Fast Release 'Default']
	/// 1 = Note Release  [Release Stage]
	/// 2 = Note Continue [No NNA]
	///\endverbatim
	unsigned char _NNA;


	int sampler_to_use = -1; // Sampler machine index for lockinst.
	unsigned char _LOCKINST;	// Force this instrument number to change the selected machine to use a specific sampler when editing (i.e. when using the pc or midi keyboards, not the notes already existing in a pattern)

	///\name Amplitude Envelope overview:
	///\{
	/// Attack Time [in Samples at 44.1Khz, independently of the real samplerate]
	int ENV_AT;	
	/// Decay Time [in Samples at 44.1Khz, independently of the real samplerate]
	int ENV_DT;	
	/// Sustain Level [in %]
	int ENV_SL;	
	/// Release Time [in Samples at 44.1Khz, independently of the real samplerate]
	int ENV_RT;	
	///\}
	
	///\name Filter 
	///\{
	/// Attack Time [in Samples at 44.1Khz]
	int ENV_F_AT;	
	/// Decay Time [in Samples at 44.1Khz]
	int ENV_F_DT;	
	/// Sustain Level [0..128]
	int ENV_F_SL;	
	/// Release Time [in Samples at 44.1Khz]
	int ENV_F_RT;	

	/// Cutoff Frequency [0-127]
	int ENV_F_CO;	
	/// Resonance [0-127]
	int ENV_F_RQ;	
	/// EnvAmount [-128,128]
	int ENV_F_EA;	
	/// Filter Type. See psycle::helpers::dsp::FilterType. [0..6]
	int ENV_F_TP;	
	///\}
	unsigned char _RPAN;
	unsigned char _RCUT;
	unsigned char _RRES;
	char instrum_name[32];

	int val;
	int pan=128;
	int numwaves;
	int i;
	int index;

	size_t begins = psyfile_getpos(file);
	if((version&0xffff0000) == VERSION_MAJOR_ZERO)
	{
		psyfile_read(file, &index, sizeof index);
		if(index < MAX_INSTRUMENTS)
		{	
			Instrument* instrument;

			instrument = malloc(sizeof(Instrument));
			instrument_init(instrument);
			psyfile_read(file, &_loop, sizeof(_loop));
			psyfile_read(file, &_lines, sizeof(_lines));
			psyfile_read(file, &_NNA, sizeof(_NNA));

			instrument->nna = _NNA;

			psyfile_read(file, &ENV_AT, sizeof(ENV_AT));
			psyfile_read(file, &ENV_DT, sizeof(ENV_DT));
			psyfile_read(file, &ENV_SL, sizeof(ENV_SL));
			psyfile_read(file, &ENV_RT, sizeof(ENV_RT));

			adsr_settings_setattack(
				&instrument->volumeenvelope, ENV_AT * 1.f/44100);
			adsr_settings_setdecay(
				&instrument->volumeenvelope, ENV_DT * 1.f/44100);
			adsr_settings_setsustain(
				&instrument->volumeenvelope, ENV_SL / 100.f);
			adsr_settings_setrelease(
				&instrument->volumeenvelope, ENV_RT * 1.f/44100);
			
			psyfile_read(file, &ENV_F_AT, sizeof(ENV_F_AT));
			psyfile_read(file, &ENV_F_DT, sizeof(ENV_F_DT));
			psyfile_read(file, &ENV_F_SL, sizeof(ENV_F_SL));
			psyfile_read(file, &ENV_F_RT, sizeof(ENV_F_RT));

			adsr_settings_setattack(
				&instrument->filterenvelope, ENV_AT * 1.f/44100);
			adsr_settings_setdecay(
				&instrument->filterenvelope, ENV_F_DT * 1.f/44100);
			adsr_settings_setsustain(
				&instrument->filterenvelope, ENV_F_SL / 128.f);
			adsr_settings_setrelease(
				&instrument->filterenvelope, ENV_F_RT * 1.f/44100);

			psyfile_read(file, &ENV_F_CO, sizeof(ENV_F_CO));
			psyfile_read(file, &ENV_F_RQ, sizeof(ENV_F_RQ));
			psyfile_read(file, &ENV_F_EA, sizeof(ENV_F_EA));

			instrument->filtercutoff = ENV_F_CO / 127.f;
			instrument->filterres = ENV_F_RQ / 127.f;
			instrument->filtermodamount = ENV_F_EA / 255.f + 0.5f;
			
			psyfile_read(file, &val, sizeof(val));
			ENV_F_TP = val;
			instrument->filtertype = (FilterType) val;

			psyfile_read(file, &pan, sizeof(pan));
			psyfile_read(file, &_RPAN, sizeof(_RPAN));
			psyfile_read(file, &_RCUT, sizeof(_RCUT));
			psyfile_read(file, &_RRES, sizeof(_RRES));

			instrument->_RPAN = _RPAN;
			instrument->_RCUT = _RCUT;
			instrument->_RRES = _RRES;			
			
			psyfile_readstring(file, instrum_name,sizeof(instrum_name));

			// now we have to read waves

			
			psyfile_read(file, &numwaves, sizeof(numwaves));
			for (i = 0; i < numwaves; i++)
			{
				loadwavesubchunk(song, file, index, pan, instrum_name, 1, i );
			}

			if ((version & 0xFF) >= 1) 
			{ //revision 1 or greater
				psyfile_read(file, &sampler_to_use, sizeof(sampler_to_use));
				psyfile_read(file, &_LOCKINST, sizeof(_LOCKINST));
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
			instruments_insert(&song->instruments, instrument, index);			
		}
	}
	psyfile_seek(file, begins + size);
}

void loadwavesubchunk(Song* song, PsyFile* file, int instrIdx, int pan, char * instrum_name, int fullopen, int loadIdx)
{
	char Header[8];
	unsigned int version;
	unsigned int size;

	psyfile_read(file, &Header,4);
	Header[4] = 0;
	psyfile_read(file, &version,sizeof(version));
	psyfile_read(file, &size,sizeof(size));

	//fileformat supports several waves, but sampler only supports one.
	if (strcmp(Header,"WAVE")==0 && version <= CURRENT_FILE_VERSION_WAVE || loadIdx == 0)
	{
		Sample* wave;
		//This index represented the index position of this wave for the instrument. 0 to 15.
		unsigned int legacyindex;
		unsigned short volume = 0;
		int tmp = 0;
		unsigned char doloop = 0;
		char dummy[32];
		unsigned int packedsize;
		byte* pData;
		short* pDest;
		
		wave = sample_allocinit();
		wave->panfactor = (float) pan / 256.f ; //(value_mapper::map_256_1(pan));
		wave->samplerate = 44100;				
		psyfile_read(file, &legacyindex,sizeof(legacyindex));
		psyfile_read(file, &wave->numframes, sizeof(wave->numframes));
		psyfile_read(file, &volume, sizeof volume);
		wave->globalvolume = volume*0.01f;
		psyfile_read(file, &wave->loopstart, sizeof wave->loopstart);
		psyfile_read(file, &wave->loopend, sizeof wave->loopend);
				
		psyfile_read(file, &tmp, sizeof(tmp));
		wave->tune = tmp;
		psyfile_read(file, &tmp, sizeof(tmp));
		//Current sampler uses 100 cents. Older used +-256
		tmp = (int)((float)tmp/2.56f);
		wave->finetune = tmp;		
		psyfile_read(file, &doloop, sizeof(doloop));
		wave->looptype = doloop ? LOOP_NORMAL : LOOP_DO_NOT;
		psyfile_read(file, &wave->stereo, sizeof(&wave->stereo));
		//Old sample name, never used.
		psyfile_readstring(file, dummy,sizeof(dummy));
		wave->name = _strdup(instrum_name);
				
		psyfile_read(file, &packedsize,sizeof(packedsize));
				
		if ( fullopen )
		{
			unsigned int i;
			pData = malloc(packedsize+4);// +4 to avoid any attempt at buffer overflow by the code
			psyfile_read(file, pData, packedsize);
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
			psyfile_skip(file, packedsize);

			wave->channels.samples[0] = 0;
		}

		if (wave->stereo)
		{
			psyfile_read(file, &packedsize,sizeof(packedsize));
			if ( fullopen )
			{
				unsigned int i;
				pData = malloc(packedsize+4); // +4 to avoid any attempt at buffer overflow by the code
				psyfile_read(file, pData,packedsize);
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
				psyfile_skip(file, packedsize);
				wave->channels.samples[1] = 0;
			}
		}
		samples_insert(&song->samples, wave, instrIdx);
	}
	else
	{
		psyfile_skip(file, size);
	}
}

void readmacd(Song* self, PsyFile* file, unsigned int size, int version,
			  Properties* machinesproperties)
{
	size_t begins = psyfile_getpos(file);
	if((version&0xFFFF0000) == VERSION_MAJOR_ZERO)
	{
		int index;
		psyfile_read(file, &index, sizeof index);
		if(index < MAX_MACHINES)
		{			
			Machine* machine;
			Properties* machineproperties;
			
			machineproperties = properties_createsection(machinesproperties, "machine");
			properties_append_int(machineproperties, "index", index, 0, MAX_MACHINES);
			machine = machineloadfilechunk(self, file, index, version,
				&self->machines, machineproperties);
			if (machine) {
				machines_insert(&self->machines, index, machine);
			}
		}
	}
	psyfile_seek(file, begins + size);
}

Machine* machineloadfilechunk(Song* self, PsyFile* file, int index, int version,
	Machines* machines, Properties* properties)
{
	// assume version 0 for now	
	Machine* machine;
	int type;
	char modulename[256];
	char plugincatchername[256];
	char editname[32];
	int i;
	
	psyfile_read(file, &type,sizeof(type));
	psyfile_readstring(file, modulename, 256);
	makeplugincatchername(modulename, plugincatchername);
	machine = machinefactory_makemachine(self->machinefactory, type,
		plugincatchername);
	if (!machine) {
		machine = machinefactory_makemachine(self->machinefactory, MACH_DUMMY, 
			plugincatchername);
		type = MACH_DUMMY;
	}
	
	{
		unsigned char bypass;
		unsigned char mute;
		int panning;
		int x;
		int y;
		
		psyfile_read(file, &bypass, sizeof(bypass));
		psyfile_read(file, &mute, sizeof(mute));
		psyfile_read(file, &panning, sizeof(panning));
		psyfile_read(file, &x, sizeof(x));
		psyfile_read(file, &y, sizeof(y));
		psyfile_skip(file, 2*sizeof(int));	// numInputs, numOutputs				
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
		int input;
		int output;
		float inconvol;
		float wiremultiplier;
		unsigned char connection;
		unsigned char incon;

		// Incoming connections Machine number
		psyfile_read(file, &input ,sizeof(input));	
		// Outgoing connections Machine number
		psyfile_read(file, &output, sizeof(output)); 
		// Incoming connections Machine vol
		psyfile_read(file, &inconvol, sizeof(inconvol));
		// Value to multiply _inputConVol[] to have a 0.0...1.0 range
		psyfile_read(file, &wiremultiplier, sizeof(wiremultiplier));	
		// Outgoing connections activated
		psyfile_read(file, &connection, sizeof(connection));
		// Incoming connections activated
		psyfile_read(file, &incon, sizeof(incon));
		if (connection && output != -1) {					
			machines_connect(machines, index, output, 0);
		}
		if (incon && input != -1) {
			machines_connect(machines, input, index, 0);
		}
	}

	psyfile_readstring(file, editname, 32);
	if (type == MACH_DUMMY) {
		char txt[40];
		strcpy(txt, "X!");
		strcat(txt, editname);
		properties_append_string(properties, "editname", txt);
	} else {
		properties_append_string(properties, "editname", editname);
	}
	machine->loadspecific(machine, file, index, machines);
	return machine;	
}

static char* replace_char(char* str, char c, char r)
{
	char* p;
	
	for (p = strchr(str, c); p != 0; p = strchr(p + 1, c)) *p = r;
	return p;
}

void makeplugincatchername(const char* psy3dllname, char* catchername)
{
	char ext[_MAX_PATH];

	extract_path(psy3dllname, catchername, ext);
	_strlwr(catchername);
	replace_char(catchername, ' ', '-');
}

void psy3_save(Song* song, PsyFile* file, Properties* workspaceproperties)
{
	size_t chunkcount;

	chunkcount = psy3_chunkcount(song);
	psy3_writefileheader(song, file);
	// the rest of the modules can be arranged in any order
	psy3_writesonginfo(song, file);
	psy3_writesngi(song, file);
	psy3_writeseqd(song, file);
	psy3_writepatd(song, file);
	psy3_writemacd(song, file);
}

size_t psy3_chunkcount(Song* song)
{
	// 3 chunks (INFO, SNGI, SEQD. SONG is not counted as a chunk) plus:
	size_t rv = 3;

	// PATD
	rv += patterns_size(&song->patterns);
	// MACD
	rv += machines_size(&song->machines);
	// INSD
	// rv += instruments_size(&song->machines);
	return rv;
}

void psy3_writefileheader(Song* song, PsyFile* file)
{	
	// id = "PSY3SONG"; // PSY2 was 0.96 to 1.7.2
	size_t pos;
	int chunkcount;

	psyfile_write(file, "PSY3",4);
	pos = psyfile_writeheader(file, "SONG", CURRENT_FILE_VERSION, 0);
	chunkcount = (int) psy3_chunkcount(song);
	psyfile_write(file, &chunkcount, sizeof(chunkcount));
	psyfile_writestring(file, PSYCLE__TITLE);
	psyfile_writestring(file, PSYCLE__VERSION);
	psyfile_updatesize(file, pos);				
}

void psy3_writesonginfo(Song* song, PsyFile* file)
{		
	size_t sizepos;
	char* str;
	
	sizepos = psyfile_writeheader(file, "INFO", CURRENT_FILE_VERSION_INFO, 0);
	psyfile_writestring(file,
		properties_readstring(song->properties, "title", &str, "a Title"));	 
	psyfile_writestring(file,
		properties_readstring(song->properties, "credits", &str, "unknown"));	
	psyfile_writestring(file,
		properties_readstring(song->properties, "comments", &str,
			"no comments"));
	psyfile_updatesize(file, sizepos);	
}


void psy3_writesngi(Song* song, PsyFile* file)
{	
	// ===================
	// 	SONG INFO
	// 	===================
	// 	id = "SNGI";

	size_t sizepos;
	int temp;
	unsigned char bTemp = 0;
	int i;

	sizepos = psyfile_writeheader(file, "SNGI", CURRENT_FILE_VERSION_SNGI, 0);

	temp = song->patterns.songtracks;
	psyfile_write(file, &temp, sizeof(temp));
	temp = properties_int(song->properties,  "bpm", 125);
	psyfile_write(file, &temp, sizeof(temp));	
	temp = properties_int(song->properties,  "lpb", 125);
	psyfile_write(file, &temp, sizeof(temp));
	temp = properties_int(song->properties,  "octave", 125);
	psyfile_write(file, &temp, sizeof(temp));
	temp = 0; // machineSoloed;
	psyfile_write(file, &temp, sizeof(temp));
	temp = 0; // _trackSoloed;
	psyfile_write(file, &temp, sizeof(temp));

	temp = 0 ; // seqBus;
	psyfile_write(file, &temp, sizeof(temp));

	temp = 0; // paramSelected;
	psyfile_write(file, &temp, sizeof(temp));
	temp = 0; // auxcolSelected;
	psyfile_write(file, &temp, sizeof(temp));
	temp = 0; // instSelected;
	psyfile_write(file, &temp, sizeof(temp));

	temp = 1; // sequence width
	psyfile_write(file, &temp, sizeof(temp));

	bTemp = 0;
	for (i = 0; i < song->patterns.songtracks; ++i) {
		psyfile_write(file, &bTemp, sizeof(bTemp));
		psyfile_write(file, &bTemp, sizeof(bTemp)); // remember to count them
	}

	bTemp =  0; // shareTrackNames
	psyfile_write(file, &bTemp, sizeof(bTemp));
	if (bTemp) {
		int t;

		for(t = 0; t < song->patterns.songtracks; ++t) {
			psyfile_writestring(file, ""); //_trackNames[0][t]);
		}
	}
	temp = properties_int(song->properties,  "tpb", 24);
	psyfile_write(file, &temp, sizeof(temp));
	temp = properties_int(song->properties,  "etpb", 0);	
	psyfile_write(file, &temp, sizeof(temp));
	psyfile_updatesize(file, sizepos);
}			


void psy3_writeseqd(Song* song, PsyFile* file)
{	
	//		===================
	//		SEQUENCE DATA
	//		===================
	//		id = "SEQD"; 
	
	int index;	

	for (index=0;index < MAX_SEQUENCES; ++index)
	{
		size_t sizepos;
		static char* sequenceName = "seq0"; // This needs to be replaced when converting to Multisequence.
		int temp;		
		List* t;
		SequenceTrack* track;

		sizepos = psyfile_writeheader(file, "SEQD", CURRENT_FILE_VERSION_SEQD, 0);
		psyfile_write(file, &index, sizeof(index)); // Sequence Track number
		temp = sequence_size(&song->sequence, song->sequence.tracks);
		psyfile_write(file, &temp, sizeof(temp)); // Sequence length
		psyfile_writestring(file, sequenceName); // Sequence Name
		
		track = (SequenceTrack*)song->sequence.tracks->entry;
		for (t = track->entries ; t != 0; t = t->next) {
			SequenceEntry* entry;

			entry = (SequenceEntry*) t->entry;
			temp = entry->pattern;
			psyfile_write(file, &temp, sizeof(temp));	// Sequence data.
		}
		psyfile_updatesize(file, sizepos);
	}
}

void psy3_writepatd(Song* song, PsyFile* file)
{

	// ===================
	// PATTERN DATA
	// ===================
	// id = "PATD"; 
	
	int i;
	int temp;
	unsigned char shareTrackNames;

	for (i = 0; i < MAX_PATTERNS; ++i) {
		// check every pattern for validity
		if (sequence_patternused(&song->sequence, i)) {	
			// ok save it
			Pattern* pattern;
			int patternLines;
			int lpb;
			unsigned char* pSource;
			unsigned char* pCopy;
			int y;
			int t;
			int index;
			PatternNode* node;
			int size77;
			size_t patsize;
			size_t sizepos;
			
			pattern = patterns_at(&song->patterns, i);
			lpb = properties_int(song->properties, "lpb", 4);
			patternLines = (int) (pattern->length * lpb + 0.5);
			patsize = song->patterns.songtracks * 
				patternLines * EVENT_SIZE;
			pSource = malloc(patsize);			
			pCopy = pSource;			

			for (y = 0; y < patternLines; ++y) {
				for (t = 0; t < song->patterns.songtracks; ++t) {
					unsigned char* pData;
					PatternEvent* event;					

					pData = pCopy + y * song->patterns.songtracks * EVENT_SIZE +
						t * EVENT_SIZE;
					event = (PatternEvent*) pData;
					patternevent_clear(event);
				}
			}
			

			for (node = pattern->events; node != 0; node = node->next) {
				PatternEvent* event;
				unsigned char* pData;
				PatternEntry* entry;
				int y;
				int t;
					
				entry = (PatternEntry*) node->entry;
				y = (int) (entry->offset * lpb);
				t = entry->track;
				pData = pCopy + y * song->patterns.songtracks * EVENT_SIZE +
						t * EVENT_SIZE;
				event = (PatternEvent*) pData;
				*event = entry->event;
			}			
			size77 = beerz77comp2(pSource, 
				&pCopy, song->patterns.songtracks*patternLines*EVENT_SIZE);
			free(pSource);
			pSource = 0;			
			sizepos = psyfile_writeheader(file, "PATD",
				CURRENT_FILE_VERSION_PATD, 0);
			index = i; // index
			psyfile_write(file, &index, sizeof(index));
			temp = patternLines;
			psyfile_write(file, &temp, sizeof(temp));
			temp = song->patterns.songtracks; // eventually this may be variable per pattern
			psyfile_write(file, &temp, sizeof(temp));

			psyfile_writestring(file, pattern->label);

			psyfile_write(file, &size77, sizeof(size77));
			psyfile_write(file, pCopy, size77);
			free(pCopy);
			pCopy = 0;			
			
			shareTrackNames = 0;
			if( !shareTrackNames) {
				int t;
				for(t = 0; t < song->patterns.songtracks; ++t) {
					psyfile_writestring(file, ""); //_trackNames[i][t]);
				}
			}
			psyfile_updatesize(file, sizepos);			
		}
	}
}

void psy3_writemacd(Song* song, PsyFile* file)
{

//	===================
//	MACHINE DATA
//	===================
//	id = "MACD"; 

	int i;

	for (i = 0; i < MAX_MACHINES; ++i) {
		Machine* machine;

		machine = machines_at(&song->machines, i);
		if (machine) {
			int index;
			size_t sizepos;

			sizepos = psyfile_writeheader(file, "MACD",
				CURRENT_FILE_VERSION_MACD, 0);
			index = i; // index
			psyfile_write(file, &index, sizeof(index));
			psy3_writemachine(song, file, machine, index);
			psyfile_updatesize(file, sizepos);
		}
	}			
}

void psy3_writemachine(Song* song, PsyFile* file, Machine* machine, int slot)
{
	const MachineInfo* info;

	info = machine->info(machine);
	if (info) {
		int i;
		int unused;
		int temp;		
		unsigned char bTemp;
		MachineSockets* sockets;
		char* tmpName;

		unused = -1;
		temp = info->type;
		psyfile_write(file, &temp, sizeof(temp));
		psy3_savedllnameandindex(file, info->modulepath, info->shellidx);
		bTemp = machine->bypass;
		psyfile_write(file, &bTemp, sizeof(bTemp));
		bTemp = machine->mute;
		psyfile_write(file, &bTemp, sizeof(bTemp));
		temp = (int)(machine->panning(machine) * 256);
		psyfile_write(file, &temp, sizeof(temp));
		temp = 100; // x
		psyfile_write(file, &temp, sizeof(temp));
		temp = 100; // y
		psyfile_write(file, &temp, sizeof(temp));
		// Connections
		sockets = connections_at(&song->machines.connections, slot);
		temp = sockets && sockets->inputs ? list_size(sockets->inputs) : 0;
		psyfile_write(file, &temp, sizeof(temp));
		temp = sockets && sockets->outputs ? list_size(sockets->outputs) : 0;
		psyfile_write(file, &temp, sizeof(temp));
		for(i = 0; i < MAX_CONNECTIONS; ++i) {
			unsigned char in = 0;
			unsigned char out = 0;			

			if (sockets && sockets->inputs) {
				WireSocket* socket = connection_at(sockets->inputs, i);
				if (socket) {
					WireSocketEntry* entry = (WireSocketEntry*) socket->entry;
					temp = entry->slot;
					psyfile_write(file, &temp, sizeof(temp));
					in = 1;
				} else {
					psyfile_write(file, &unused, sizeof(unused));
				}
			} else {
				psyfile_write(file, &unused, sizeof(unused));
			}
			if (sockets && sockets->outputs) {
				WireSocket* socket = connection_at(sockets->outputs, i);
				if (socket) {
					WireSocketEntry* entry = (WireSocketEntry*) socket->entry;
					temp = entry->slot;
					psyfile_write(file, &temp, sizeof(temp));
					out = 1;
				} else {
					psyfile_write(file, &unused, sizeof(unused));
				}
			} else {
				psyfile_write(file, &unused, sizeof(unused));	
			}
			// float volume; volume = inWires[i].GetVolume();
			// float volMultiplier; volMultiplier = inWires[i].GetVolMultiplier();
			// volume /= volMultiplier;
			{
				float fTemp = 1.f;
				// volume
				psyfile_write(file, &fTemp, sizeof(fTemp));	// Incoming connections Machine vol
				// volMultiplier
				psyfile_write(file, &fTemp, sizeof(fTemp));	// Value to multiply _inputConVol[] to have a 0.0...1.0 range
				bTemp = out;
				psyfile_write(file, &bTemp, sizeof(bTemp)); // Outgoing connections activated
				bTemp = in;
				psyfile_write(file, &bTemp, sizeof(bTemp)); // Incoming connections activated
			}
		}		
		tmpName = "";
		psyfile_writestring(file, tmpName);
	}

	/*int type;	
		
	



	
	std::string tmpName = _editName;
	pFile->WriteString(tmpName);
	SaveSpecificChunk(pFile);
	SaveWireMapping(pFile);
	SaveParamMapping(pFile);*/
}

void psy3_savedllnameandindex(PsyFile* file, const char* name,
	int shellindex)
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
		int divisor=16777216;
		idxtext[4]=0;
		for (int i=0; i < 4; i++)
		{
			int residue = index%divisor;
			idxtext[3-i]=index/divisor;
			index = residue;
			divisor=divisor/256;
		}
		strcat(str2,idxtext);
	}
	pFile->Write(&str2,strlen(str2)+1);*/
}
