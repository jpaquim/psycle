// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "song.h"
#include <fileio.h>
#include "pattern.h"
#include "constants.h"
#include <datacompression.h>
#include "machinefactory.h"

static void song_initdefaults(Song* self);
static void song_initproperties(Song* self);

static void loadpsy3(Song* self, RiffFile* file, char header[9],
	MachineFactory* machinefactory, Properties* workspaceproperties);
static void readsngi(Song*, RiffFile* file, int size, int version);
static void readseqd(Song* song, RiffFile* file, int size, int version,
			  unsigned char* playorder, int* playlength);
static void readpatd(Song* song, RiffFile* file, int size, int version);
static void readinsd(Song* song, RiffFile* file, int size, int version);
static void readmacd(Song* song, RiffFile* file, int size, int version,
	MachineFactory* machinefactory, Properties* workspaceproperties);
static void loadpsy2(Song* self, RiffFile* file);
static void loadwavesubchunk(Song* song, RiffFile* file, int instrIdx, int pan, char * instrum_name, int fullopen, int loadIdx);
static Machine* machineloadfilechunk(RiffFile* file, int index, int version,
	MachineFactory* machinefactory, Machines* machines,
	Properties* workspaceproperties);

void song_init(Song* self)
{	
	machines_init(&self->machines);
	patterns_init(&self->patterns);
	sequence_init(&self->sequence, &self->patterns);
	samples_init(&self->samples);
	instruments_init(&self->instruments);
	xminstruments_init(&self->xminstruments);	
	song_initdefaults(self);	
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

void song_initdefaults(Song* self)
{
	SequenceTrack* sequencetrack;
	SequencePosition sequenceposition;
	Pattern* pattern;
	
	pattern = (Pattern*) malloc(sizeof(Pattern));
	patterns_insert(&self->patterns, 0, pattern);
	pattern_init(pattern);
	sequencetrack = malloc(sizeof(SequenceTrack));
	sequencetrack_init(sequencetrack);
	sequenceposition.track = sequence_appendtrack(&self->sequence, sequencetrack);
	sequenceposition.trackposition =
		sequence_begin(&self->sequence, sequenceposition.track, 0);
	sequence_insert(&self->sequence, sequenceposition, 0);
	song_initproperties(self);
}

void song_initproperties(Song* self)
{
	self->properties = properties_create();
	properties_append_string(self->properties, "title", "Untitled");	
	properties_append_string(self->properties, "credits", "Unnamed");
	properties_append_string(self->properties, "comments", "No Comments");
	properties_append_double(self->properties, "bpm", 125.0, 0, 999);
	properties_append_int(self->properties, "lpb", 4, 1, 999);
	properties_append_int(self->properties, "tracks", 16, 1, 64);
}

void song_load(Song* self, const char* path, MachineFactory* machinefactory, Properties** workspaceproperties)
{	
	RiffFile file;

	if (rifffile_open(&file, (char*) path))
	{
		char header[9];				

		*workspaceproperties = 0;
		sequence_clear(&self->sequence);		
		patterns_clear(&self->patterns);
		machines_startfilemode(&self->machines);
		rifffile_read(&file, header, 8);
		header[8]=0;		
		if (strcmp(header,"PSY3SONG")==0) {			
			*workspaceproperties = properties_create();
			loadpsy3(self, &file, header, machinefactory, *workspaceproperties);
		} else
		if (strcmp(header,"PSY2SONG")==0) {
			loadpsy2(self, &file);
		} else {

		}
		machines_endfilemode(&self->machines);
		rifffile_close(&file);
	}
}

void loadpsy3(Song* self, RiffFile* file, char header[9], MachineFactory* machinefactory,
			  Properties* workspaceproperties)
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
	Properties* machinesproperties;	

	machinesproperties = properties_append_int(workspaceproperties, "machines", 0, 0, 0);
	
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
				properties_write_string(self->properties, "title", name_);
				properties_write_string(self->properties, "credits", author_);
				properties_write_string(self->properties, "comments", comments_);
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
			readmacd(self, file, size, version, machinefactory,
				machinesproperties);			
			// rifffile_skip(file, size);
		} else
		if (strcmp(header,"INSD")==0) {
			int curpos=0; curpos; // not used
			rifffile_read(file, &version,sizeof(version));
			rifffile_read(file, &size,sizeof(size));
			chunkcount--;
			readinsd(self, file, size, version);
			// rifffile_skip(file, size);
		} else {
			// we are not at a valid header for some weird reason.  
			// probably there is some extra data.
			// shift back 3 bytes and try again
			rifffile_skip(file, -3);
		}
	}
	{
		int i;
		SequenceTrack* sequencetrack;
		SequencePosition sequenceposition;

		sequencetrack = malloc(sizeof(SequenceTrack));
		sequencetrack_init(sequencetrack);				
		sequenceposition.track =
			sequence_appendtrack(&self->sequence, sequencetrack);		
		for (i = 0; i < playlength; ++i) {			
			sequenceposition.trackposition =
				sequence_last(&self->sequence, sequenceposition.track);
			sequence_insert(&self->sequence, sequenceposition, playorder[i]);
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

void readinsd(Song* song, RiffFile* file, int size, int version)
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


	int sampler_to_use; // Sampler machine index for lockinst.
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

	size_t begins = rifffile_getpos(file);
	if((version&0xffff0000) == VERSION_MAJOR_ZERO)
	{
		rifffile_read(file, &index, sizeof index);
		if(index < MAX_INSTRUMENTS)
		{	
			Instrument* instrument;

			instrument = malloc(sizeof(Instrument));
			instrument_init(instrument);
			rifffile_read(file, &_loop, sizeof(_loop));
			rifffile_read(file, &_lines, sizeof(_lines));
			rifffile_read(file, &_NNA, sizeof(_NNA));

			instrument->nna = _NNA;

			rifffile_read(file, &ENV_AT, sizeof(ENV_AT));
			rifffile_read(file, &ENV_DT, sizeof(ENV_DT));
			rifffile_read(file, &ENV_SL, sizeof(ENV_SL));
			rifffile_read(file, &ENV_RT, sizeof(ENV_RT));

			instrument->volumeenvelope.attack = ENV_AT * 1.f/44100;
			instrument->volumeenvelope.decay = ENV_DT * 1.f/44100;
			instrument->volumeenvelope.sustain = ENV_SL / 100.f;
			instrument->volumeenvelope.release = ENV_RT * 1.f/44100;
			
			rifffile_read(file, &ENV_F_AT, sizeof(ENV_F_AT));
			rifffile_read(file, &ENV_F_DT, sizeof(ENV_F_DT));
			rifffile_read(file, &ENV_F_SL, sizeof(ENV_F_SL));
			rifffile_read(file, &ENV_F_RT, sizeof(ENV_F_RT));

			instrument->filterenvelope.attack = ENV_AT * 1.f/44100;
			instrument->filterenvelope.decay = ENV_F_DT * 1.f/44100;
			instrument->filterenvelope.sustain = ENV_F_SL / 128.f;
			instrument->filterenvelope.release = ENV_F_RT * 1.f/44100;

			rifffile_read(file, &ENV_F_CO, sizeof(ENV_F_CO));
			rifffile_read(file, &ENV_F_RQ, sizeof(ENV_F_RQ));
			rifffile_read(file, &ENV_F_EA, sizeof(ENV_F_EA));

			instrument->filtercutoff = ENV_F_CO / 127.f;
			instrument->filterres = ENV_F_RQ / 127.f;
			instrument->filtermodamount = ENV_F_EA / 255.f + 0.5f;
			
			rifffile_read(file, &val, sizeof(val));
			ENV_F_TP = val;


			rifffile_read(file, &pan, sizeof(pan));
			rifffile_read(file, &_RPAN, sizeof(_RPAN));
			rifffile_read(file, &_RCUT, sizeof(_RCUT));
			rifffile_read(file, &_RRES, sizeof(_RRES));

			instrument->_RPAN = _RPAN;
			instrument->_RCUT = _RCUT;
			instrument->_RRES = _RRES;			
			
			rifffile_readstring(file, instrum_name,sizeof(instrum_name));

			// now we have to read waves

			
			rifffile_read(file, &numwaves, sizeof(numwaves));
			for (i = 0; i < numwaves; i++)
			{
				loadwavesubchunk(song, file, index, pan, instrum_name, 1, i );
			}

			if ((version & 0xFF) >= 1) 
			{ //revision 1 or greater
				rifffile_read(file, &sampler_to_use, sizeof(sampler_to_use));
				rifffile_read(file, &_LOCKINST, sizeof(_LOCKINST));
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
	rifffile_seek(file, begins + size);
}

void loadwavesubchunk(Song* song, RiffFile* file, int instrIdx, int pan, char * instrum_name, int fullopen, int loadIdx)
{
	char Header[8];
	UINT version;
	UINT size;

	rifffile_read(file, &Header,4);
	Header[4] = 0;
	rifffile_read(file, &version,sizeof(version));
	rifffile_read(file, &size,sizeof(size));

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
		
		wave = malloc(sizeof(Sample));
		sample_init(wave);

		wave->panfactor = (float) pan / 256.f ; //(value_mapper::map_256_1(pan));
		wave->samplerate = 44100;
				
		rifffile_read(file, &legacyindex,sizeof(legacyindex));
		rifffile_read(file, &wave->numframes, sizeof(wave->numframes));
		rifffile_read(file, &volume, sizeof volume);
		wave->globalvolume = volume*0.01f;
		rifffile_read(file, &wave->loopstart, sizeof wave->loopstart);
		rifffile_read(file, &wave->loopend, sizeof wave->loopend);
				
		rifffile_read(file, &tmp, sizeof(tmp));
		wave->tune = tmp;
		rifffile_read(file, &tmp, sizeof(tmp));
		//Current sampler uses 100 cents. Older used +-256
		tmp = (int)((float)tmp/2.56f);
		wave->finetune = tmp;		
		rifffile_read(file, &doloop, sizeof(doloop));
		wave->looptype = doloop ? LOOP_NORMAL : LOOP_DO_NOT;
		rifffile_read(file, &wave->stereo, sizeof(&wave->stereo));
		//Old sample name, never used.
		rifffile_readstring(file, dummy,sizeof(dummy));
		wave->name = strdup(instrum_name);
				
		rifffile_read(file, &packedsize,sizeof(packedsize));
				
		if ( fullopen )
		{
			unsigned int i;
			pData = malloc(packedsize+4);// +4 to avoid any attempt at buffer overflow by the code
			rifffile_read(file, pData, packedsize);
			sounddesquash(pData, &pDest);		
			free(pData);
			wave->channels.samples[0] = malloc(sizeof(float)*wave->numframes);
			for (i = 0; i < wave->numframes; i++) {
				short val = (short) pDest[i];
				wave->channels.samples[0][i] = (float) val;				
			 }
			free(pDest);
			pData = 0;			
		}
		else
		{
			rifffile_skip(file, packedsize);

			wave->channels.samples[0] = 0;
		}

		if (wave->stereo)
		{
			rifffile_read(file, &packedsize,sizeof(packedsize));
			if ( fullopen )
			{
				unsigned int i;
				pData = malloc(packedsize+4); // +4 to avoid any attempt at buffer overflow by the code
				rifffile_read(file, pData,packedsize);
				sounddesquash(pData, &pDest);
				free(pData);
				wave->channels.samples[1] = malloc(sizeof(float)*wave->numframes);
				for (i = 0; i < wave->numframes; i++) {
					short val = (short) pDest[i];
					wave->channels.samples[1][i] = (float) val;					
				}
				free(pDest);
				pData = 0;
			}
			else
			{
				rifffile_skip(file, packedsize);
				wave->channels.samples[1] = 0;
			}
		}
		samples_insert(&song->samples, wave, instrIdx);
	}
	else
	{
		rifffile_skip(file, size);
	}
}


void readmacd(Song* song, RiffFile* file, int size, int version, MachineFactory* machinefactory,
			  Properties* machinesproperties)
{
	size_t begins = rifffile_getpos(file);
	if((version&0xFFFF0000) == VERSION_MAJOR_ZERO)
	{
		int index;
		rifffile_read(file, &index, sizeof index);
		if(index < MAX_MACHINES)
		{			
			Machine* machine;
			Properties* machineproperties;

			if (!machinesproperties->children) {
				machinesproperties->children = properties_create();
			}
			machineproperties = properties_append_int(machinesproperties->children, "machine", index, 0, MAX_MACHINES);
			machine = machineloadfilechunk(file, index, version, machinefactory,
				&song->machines, machineproperties);
			if (machine) {
				machines_insert(&song->machines, index, machine);
			}
		}
	}
	rifffile_seek(file, begins + size);
}

Machine* machineloadfilechunk(RiffFile* file, int index, int version,
	MachineFactory* machinefactory, Machines* machines, Properties* properties)
{
	// assume version 0 for now	
	Machine* machine;
	int type;
	char modulename[256];
	char editname[32];
	int i;

	properties->children = properties_create();
	rifffile_read(file, &type,sizeof(type));
	rifffile_readstring(file, modulename, 256);
	machine = machinefactory_make(machinefactory, type, modulename);
	if (!machine) {
		machine = machinefactory_make(machinefactory, MACH_DUMMY, modulename);
		type = MACH_DUMMY;
	}
	
	{
		unsigned char bypass;
		unsigned char mute;
		int panning;
		int x;
		int y;
		// char _editName[32];


		rifffile_read(file, &bypass, sizeof(bypass));
		rifffile_read(file, &mute, sizeof(mute));
		rifffile_read(file, &panning, sizeof(panning));
		rifffile_read(file, &x, sizeof(x));
		rifffile_read(file, &y, sizeof(y));
		rifffile_skip(file, 2*sizeof(int));	// numInputs, numOutputs		
		properties_append_int(properties->children, "x", x, 0, 0);
		properties_append_int(properties->children, "y", y, 0, 0);
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
		rifffile_read(file, &input ,sizeof(input));	
		// Outgoing connections Machine number
		rifffile_read(file, &output, sizeof(output)); 
		// Incoming connections Machine vol
		rifffile_read(file, &inconvol, sizeof(inconvol));
		// Value to multiply _inputConVol[] to have a 0.0...1.0 range
		rifffile_read(file, &wiremultiplier, sizeof(wiremultiplier));	
		// Outgoing connections activated
		rifffile_read(file, &connection, sizeof(connection));
		// Incoming connections activated
		rifffile_read(file, &incon, sizeof(incon));
		if (output != -1) {					
			machines_connect(machines, index, output);
		}
		//if (input != -1) {				
		//	machines_connect(machines, input, index);
		// }
	}

	rifffile_readstring(file, editname, 32);
	if (type == MACH_DUMMY) {
		char txt[40];
		strcpy(txt, "X!");
		strcat(txt, editname);
		properties_append_string(properties->children, "editname", txt);
	} else {
		properties_append_string(properties->children, "editname", editname);
	}

	return machine;	
}

void loadpsy2(Song* self, RiffFile* file)
{

}