/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "psy3loader.h"
/* local */
#include "constants.h"
#include "machinefactory.h"
#include "psyconvert.h"
#include "plugin_interface.h"
#include "song.h"
#include "songio.h"
#include "wire.h"
/* dsp */
#include <datacompression.h>
#include <envelope.h>
#include <operations.h>
/* file */
#include <dir.h>
/* platform */
#include "../../detail/portable.h"

typedef struct Chunk {
	char header[5];
	int (*read)(psy_audio_PSY3Loader*);
} Chunk;

/* prototypes */
static int psy_audio_psy3loader_readversion(psy_audio_PSY3Loader*);
static int psy_audio_psy3loader_readchunks(psy_audio_PSY3Loader*);
static int psy_audio_psy3loader_readchunk(psy_audio_PSY3Loader*,
	const char* header);
static int psy_audio_psy3loader_read_info(psy_audio_PSY3Loader*);
static int psy_audio_psy3loader_read_sngi(psy_audio_PSY3Loader*);
static int psy_audio_psy3loader_read_seqd(psy_audio_PSY3Loader*);
static int psy_audio_psy3loader_read_patd(psy_audio_PSY3Loader*);
static int psy_audio_psy3loader_read_epat(psy_audio_PSY3Loader*,
	int32_t index, psy_audio_Pattern*);
static int psy_audio_psy3loader_read_insd(psy_audio_PSY3Loader*);
static int psy_audio_psy3loader_read_eins(psy_audio_PSY3Loader*);
static int psy_audio_psy3loader_read_smid(psy_audio_PSY3Loader*);
static int psy_audio_psy3loader_read_macd(psy_audio_PSY3Loader*);
static int psy_audio_psy3loader_read_smsb(psy_audio_PSY3Loader*);
static int psy_audio_psy3loader_read_virg(psy_audio_PSY3Loader*);

static int psy_audio_psy3loader_loadxminstrument(psy_audio_PSY3Loader*,
	psy_audio_Instrument*, bool islegacy, uint32_t legacyversion);
static int psy_audio_psy3loader_xminstrumentenvelopeload(psy_audio_PSY3Loader*,
	psy_dsp_Envelope* envelope,
	bool legacy, uint32_t legacyversion);
static int psy_audio_psy3loader_xmloadwav(psy_audio_PSY3Loader*, psy_audio_Sample* rv);
static int psy_audio_psy3loader_loadwavesubchunk(psy_audio_PSY3Loader*, int32_t instrIdx,
	int32_t pan, char * instrum_name, int32_t fullopen, int32_t loadIdx);
static int psy_audio_psy3loader_machineloadchunk(psy_audio_PSY3Loader*,
	int32_t index);
static int psy_audio_psy3loader_machineloadchunk_createmachine(
	psy_audio_PSY3Loader*, int32_t index, char* modulename, char* catchername,
	bool* rv_replaced, psy_audio_Machine** rv_machine);
static int psy_audio_psy3loader_loadparammapping(psy_audio_PSY3Loader*,
	psy_audio_Machine*);
static int psy_audio_psy3loader_loadismachinebus(psy_audio_PSY3Loader*,
	psy_audio_Machine*);
static void psy_audio_psy3loader_setinstrumentnames(psy_audio_PSY3Loader*);
static void psy_audio_psy3loader_postload(psy_audio_PSY3Loader*);

/* implementation */
void psy_audio_psy3loader_init(psy_audio_PSY3Loader* self,
	psy_audio_SongFile* songfile)
{
	assert(self && songfile && songfile->file && songfile->song);
	
	psy_audio_legacywires_init(&self->legacywires);
	self->songfile = songfile;
	self->fp = songfile->file;
	self->song = songfile->song;
	self->progress = 0;
}

void psy_audio_psy3loader_dispose(psy_audio_PSY3Loader* self)
{
	psy_audio_legacywires_dispose(&self->legacywires);
}

psy_audio_PSY3Loader* psy_audio_psy3loader_allocinit(
	psy_audio_SongFile* songfile)
{
	psy_audio_PSY3Loader* rv;

	rv = (psy_audio_PSY3Loader*)malloc(sizeof(psy_audio_PSY3Loader));
	if (rv) {
		psy_audio_psy3loader_init(rv, songfile);
	}
	return rv;
}

void psy_audio_psy3loader_deallocate(psy_audio_PSY3Loader* self)
{
	psy_audio_psy3loader_dispose(self);
	free(self);
}

/*
** ===================
** 	PSY3 SONG LOAD
** ===================
*/
int psy_audio_psy3loader_load(psy_audio_PSY3Loader* self)
{	
	int status;

	self->progress = 0;
	self->songfile->legacywires = &self->legacywires;
	if (status = psy_audio_psy3loader_readversion(self)) {
		return status;
	}
	status = psy_audio_psy3loader_readchunks(self);
	psy_audio_reposition(&self->song->sequence);
	psy_audio_psy3loader_setinstrumentnames(self);
	psy_audio_psy3loader_postload(self);
	return status;
}

int psy_audio_psy3loader_readversion(psy_audio_PSY3Loader* self)
{	
	uint32_t temp32;
	int status;
	
	if (status = psyfile_read(self->fp, &temp32, sizeof(temp32))) {
		return status;
	}
	self->fp->fileversion = temp32;
	if (status = psyfile_read(self->fp, &temp32, sizeof(temp32))) {
		return status;
	}
	self->fp->filesize = temp32;
	if (self->fp->fileversion > CURRENT_FILE_VERSION) {
		psy_audio_songfile_warn(self->songfile,
			"This file is from a newer version of Psycle! "
			"This process will try to load it anyway.\n");
	}
	if (status = psyfile_read(self->fp, &self->fp->chunkcount, sizeof(uint32_t))) {
		return status;
	}
	if (self->fp->fileversion >= 8) {
		char trackername[256];
		char trackervers[256];
		uintptr_t bytesread;

		if (status = psyfile_readstring(self->fp, trackername, 256)) {
			return status;
		}
		if (status = psyfile_readstring(self->fp, trackervers, 256)) {
			return status;
		}
		bytesread = 4 + psy_strlen(trackername) + psy_strlen(trackervers) + 2;
		/*
		** Size of the current Header DATA
		** This ensures that any extra data is skipped.
		*/
		if (self->fp->filesize > bytesread) {
			if (psyfile_skip(self->fp, self->fp->filesize - (uint32_t)bytesread) == -1) {
				return PSY_ERRFILE;
			}
		}
	}
	return PSY_OK;
}

int psy_audio_psy3loader_readchunks(psy_audio_PSY3Loader* self)
{
	char header[9];
	int status;

	header[4] = 0;		
	while ((psyfile_read(self->fp, header, 4) == PSY_OK) && self->fp->chunkcount) {
		if (status = psy_audio_psy3loader_readchunk(self, header)) {
			/*
			** we are not at a valid header for some weird reason.  
			** probably there is some extra data.
			**  shift back 3 bytes and try again
			*/
			if (psyfile_skip(self->fp, -3) == -1) {
				return status;
			}
			/*
			** this makes it impossible to add new chunks
			** todo: read header and skip the chunk
			** psyfile_readchunkbegin(self->file);
			** psyfile_seekchunkend(self->file);
			*/
		}
		psy_signal_emit(&self->song->signal_loadprogress, self, 1,
			self->progress);
	}
	return PSY_OK;
}

int psy_audio_psy3loader_readchunk(psy_audio_PSY3Loader* self,
	const char* header)
{		
	static Chunk chunks[] = {
		{"INFO", psy_audio_psy3loader_read_info},
		{"SNGI", psy_audio_psy3loader_read_sngi},
		{"SEQD", psy_audio_psy3loader_read_seqd},
		{"PATD", psy_audio_psy3loader_read_patd},
		{"MACD", psy_audio_psy3loader_read_macd},
		{"INSD", psy_audio_psy3loader_read_insd},
		{"EINS", psy_audio_psy3loader_read_eins},
		{"SMID", psy_audio_psy3loader_read_smid},
		{"SMSB", psy_audio_psy3loader_read_smsb},
		{"VIRG", psy_audio_psy3loader_read_virg},
		{"    ", NULL}
	};
	uintptr_t c;

	/* search chunk header and read chunk */
	for (c = 0; chunks[c].read != NULL; ++c) {
		if (strcmp(header, chunks[c].header) == 0) {
			int status;

			/* read version and size and check major zero */
			if (psyfile_readchunkbegin(self->fp) || (strcmp(header, "PATD") == 0)) {
				if (status = chunks[c].read(self)) {
					return status;
				}
			}
			psyfile_seekchunkend(self->fp);
			++self->progress;	
			break;
		}
	}
	return (chunks[c].read == NULL) /* no chunk found */
		? PSY_ERRFILE
		: PSY_OK;
}

void psy_audio_psy3loader_postload(psy_audio_PSY3Loader* self)
{					
	psy_TableIterator it;
	
	for (it = psy_audio_machines_begin(&self->song->machines);
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

	for (it = psy_audio_instruments_groupbegin(&self->song->instruments, 0);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_Instrument* instrument;
		psy_audio_Sample* sample;

		instrument = psy_tableiterator_value(&it);
		sample = psy_audio_samples_at(&self->song->samples,
			psy_audio_sampleindex_make(psy_tableiterator_key(&it), 0));
		if (sample) {
			psy_audio_instrument_setname(instrument, psy_audio_sample_name(sample));
		}
	}
}

/*	PSY3 chunk methods */

int psy_audio_psy3loader_read_info(psy_audio_PSY3Loader* self)
{	
	int status;
	char name[129];
	char author[65];
	char comments[65536];	
	psy_audio_SongProperties songproperties;

	if (status = psyfile_readstring(self->fp, name, sizeof name)) {
		return status;
	}
	if (status = psyfile_readstring(self->fp, author, sizeof author)) {
		return status;
	}
	if (status = psyfile_readstring(self->fp, comments, sizeof comments)) {
		return status;
	}
	psy_audio_songproperties_init(&songproperties, name, author, comments);
	psy_audio_song_setproperties(self->song, &songproperties);
	/* bugfix. There were songs with incorrect size. */
	if (psyfile_currchunkversion(self->fp) == 0) {
		self->fp->currchunk.size =
			(uint32_t)(psyfile_getpos(self->fp) -
				self->fp->currchunk.begins);
	}
	return PSY_OK;
}

int psy_audio_psy3loader_read_sngi(psy_audio_PSY3Loader* self)
{		
	int32_t i;
	int32_t temp;
	uint32_t utemp;
	int32_t songtracks;
	int32_t tracksoloed;
	int32_t seqbus;
	int32_t paramselected;
	int32_t auxcolselected;
	int32_t instselected;	
	uint8_t sharetracknames;
	int status;
		
	/*
	** why all these temps?  to make sure if someone changes the defs of
	** any of these members, the rest of the file reads ok.  assume 
	** everything is an int32_t, when we write we do the same thing.
	*/

	/* # of tracks for whole song */
	if (status = psyfile_read(self->fp, &temp, sizeof temp)) {
		return status;
	}
	songtracks = temp;
	psy_audio_song_setnumsongtracks(self->song, songtracks);
	/* bpm */
	{ /* \todo: this was a hack added in 1.9alpha to allow decimal bpm values */
		int32_t bpmcoarse;
		int32_t bpmfine;		
		short temp16 = 0;

		if (status = psyfile_read(self->fp, &temp16, sizeof temp16)) {
			return status;
		}
		bpmcoarse = temp16;
		if (status = psyfile_read(self->fp, &temp16, sizeof temp16)) {
			return status;
		}
		bpmfine = temp16;
		psy_audio_song_setbpm(self->song, bpmcoarse + (bpmfine / 100.0));
	}
	/* linesperbeat */
	if (status = psyfile_read(self->fp, &temp, sizeof temp)) {
		return status;
	}
	psy_audio_song_setlpb(self->song, temp);	
	/* current octave */
	if (status = psyfile_read(self->fp, &temp, sizeof temp)) {
		return status;
	}
	psy_audio_song_setoctave(self->song, temp);	
	/*
	** machinesoloed
	** we need to buffer this because destroy machine will clear it
	*/
	if (status = psyfile_read(self->fp, &temp, sizeof temp)) {
		return status;
	}
	if (temp >= 0) {
		self->songfile->machinesoloed = (uint32_t)temp;
	} else {
		self->songfile->machinesoloed = UINTPTR_MAX;
	}
	/* tracksoloed */
	if (status = psyfile_read(self->fp, &temp, sizeof temp)) {
		return status;
	}
	tracksoloed = temp;
	if (status = psyfile_read(self->fp, &temp, sizeof temp)) {
		return status;
	}
	seqbus = temp;
	if (status = psyfile_read(self->fp, &temp, sizeof temp)) {
		return status;
	}
	paramselected = temp;
	if (status = psyfile_read(self->fp, &temp, sizeof temp)) {
		return status;
	}
	auxcolselected = temp;
	if (status = psyfile_read(self->fp, &temp, sizeof temp)) {
		return status;
	}
	instselected = temp;
	/* sequence width, for multipattern */
	if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
		return status;
	}	
	for(i = 0 ; i < songtracks; ++i)
	{
		uint8_t temp;

		/* trackmuted */
		if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
			return status;
		}
		if (temp != FALSE) {
			psy_audio_patterns_mutetrack(&self->song->patterns, i);
		}		
		/* trackarmed */
		if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
			return status;
		}
		if (temp != FALSE) {
			psy_audio_patterns_armtrack(&self->song->patterns, i);
		}		
	}
	if (tracksoloed >= 0) {
		psy_audio_patterns_setsolotrack(&self->song->patterns, tracksoloed);
	}
	if(self->fp->currchunk.version == 0) {
		/*
		** fix for a bug existing in the song saver in the 1.7.x series
		** self->fp->currchunk.size = (11 * sizeof(int32_t)) + 
		** songtracks * 2 * 1);
		** sizeof(bool));
		*/
	}
	if(self->fp->currchunk.version >= 1) {
		if (status = psyfile_read(self->fp, &sharetracknames, 1)) {
			return status;
		}
		self->song->patterns.sharetracknames = sharetracknames;
		if( sharetracknames) {
			int32_t t;

			for(t = 0; t < songtracks; ++t) {
				char txt[40];
				if (status = psyfile_readstring(self->fp, txt, 40)) {
					return status;
				}
				/* changetrackname(0,t,name); */
			}
		}
	}
	if (self->fp->currchunk.version >= 2) {
		if (status = psyfile_read(self->fp, &temp, sizeof temp)) {
			return status;
		}
		psy_audio_song_settpb(self->song, temp);		
		if (status = psyfile_read(self->fp, &temp, sizeof temp)) {
			return status;
		}
		psy_audio_song_setextraticksperbeat(self->song, temp);		
	} else {
		psy_audio_song_settpb(self->song, 24);
		psy_audio_song_setextraticksperbeat(self->song, 0);
	}
	if (self->fp->currchunk.version >= 3) {
		if (status = psyfile_read(self->fp, &utemp, sizeof utemp)) {
			return status;
		}
		psy_audio_song_setsamplerindex(self->song, utemp);
	}
/*	if (fullopen) */
	{
		/* 
		** \todo: warning! this is done here, because the plugins, when loading, need an up-to-date information.
		** it should be coded in some way to get this information from the loading song, since doing it here
		** is bad for the winamp plugin (or any other multi-document situation).
		**	global::player().setbpm(beatspermin(), linesperbeat(), extraticksperline());
		*/
	}
	return PSY_OK;
}

int psy_audio_psy3loader_read_seqd(psy_audio_PSY3Loader* self)
{	
	psy_audio_SequenceTrack* track;
	int32_t index;
	uint32_t i;
	char ptemp[256];
	int32_t playlength;
	int status;

	/* index, for multipattern - for now always 0 */
	if (status = psyfile_read(self->fp, &index, sizeof(index))) {
		return status;
	}
	if (index < 0) {
		return PSY_OK; /* skip */
	}	
	/* create new tracks (0..index), if not already existing */
	for (i = (uint32_t)psy_audio_sequence_width(&self->song->sequence); i <= (uint32_t)index; ++i) {
		psy_audio_sequence_appendtrack(&self->song->sequence,
			psy_audio_sequencetrack_allocinit());
	}
	track = psy_audio_sequence_track_at(&self->song->sequence, index);		
	assert(track);
	/* play length for this sequence */
	if (status = psyfile_read(self->fp, &playlength, sizeof(int32_t))) {
		return status;		
	}	
	if (playlength < 0) {
		/* skip */
		return PSY_OK;
	}	
	if (status = psyfile_readstring(self->fp, ptemp, sizeof ptemp)) {
		return status;
	}
	psy_audio_sequencetrack_setname(track, ptemp);
	for (i = 0; i < (uint32_t)playlength; ++i) {		
		uint32_t patternslot;		

		if (status = psyfile_read(self->fp, &patternslot, sizeof(uint32_t))) {			
			return status;
		}		
		if (patternslot == INT32_MAX - 2) {
			/*
			** this extends the seqd chunk file format to handle
			** samples
			** A slot value of INT32_MAX - s means it is a sample
			*/
			psy_audio_sequence_insert_sample(&self->songfile->song->sequence,
				psy_audio_orderindex_make(index, i), psy_audio_sampleindex_zero());
		} else if (patternslot == INT32_MAX - 1) {
			/*
			** this extends the seqd chunk file format to handle
			** markers
			** A slot value of INT32_MAX - 1 means it is a marker
			*/
			psy_audio_sequence_insert_marker(&self->songfile->song->sequence,
				psy_audio_orderindex_make(index, i), "untitled");
		} else {
			psy_audio_sequence_insert(&self->songfile->song->sequence,
				psy_audio_orderindex_make(index, i), patternslot);
		}
	}
	if (self->fp->currchunk.version > 0) {
		/*
		** this extends the seqd chunk file format to handle 
		** beat positions
		*/
		psy_audio_SequenceEntryNode* p;
				
		for (p = track->entries, i = 0; i < (uint32_t)playlength && p != NULL;
				++i, psy_list_next(&p)) {
			float repositionoffset;
			psy_audio_SequenceEntry* seqentry;
				
			seqentry = (psy_audio_SequenceEntry*)psy_list_entry(p);
			if (status = psyfile_read(self->fp, &repositionoffset, sizeof(float))) {				
				return status;
			}			
			seqentry->repositionoffset = repositionoffset;
		}			
	}
	if (self->fp->currchunk.version > 1) {
		/*
		** this extends the seqd chunk file format to handle
		** markers
		*/
		uint32_t nummarkers;
		uint32_t numsamples;
		uint32_t c;
		float tmp_float;
		psy_List* s;

		if (status = psyfile_read(self->fp, &nummarkers, sizeof(uint32_t))) {
			return status;
		}
		for (s = track->entries, c = 0; s != NULL && c < nummarkers; psy_list_next(&s)) {
			psy_audio_SequenceEntry* seqentry;

			seqentry = (psy_audio_SequenceEntry*)psy_list_entry(s);
			if (seqentry->type == psy_audio_SEQUENCEENTRY_MARKER) {
				psy_audio_SequenceMarkerEntry* marker;
				float length;
				char text[256];

				marker = (psy_audio_SequenceMarkerEntry*)seqentry;
				if (status = psyfile_read(self->fp, &length, sizeof(float))) {
					return status;
				}
				marker->length = length;
				if (status = psyfile_readstring(self->fp, text, 256)) {
					return status;
				}
				psy_strreset(&marker->text, text);
				++c;
			}
		}
		if (status = psyfile_read(self->fp, &numsamples, sizeof(uint32_t))) {
			return status;
		}
		for (s = track->entries, c = 0; s != NULL && c < numsamples; psy_list_next(&s)) {
			psy_audio_SequenceEntry* seqentry;

			seqentry = (psy_audio_SequenceEntry*)psy_list_entry(s);
			if (seqentry->type == psy_audio_SEQUENCEENTRY_SAMPLE) {
				psy_audio_SequenceSampleEntry* sample;
				int32_t slot;
				int32_t subslot;				
				uint32_t samplerindex;

				sample = (psy_audio_SequenceSampleEntry*)seqentry;
				if (status = psyfile_read(self->fp, &slot, sizeof(int32_t))) {
					return status;
				}
				sample->sampleindex.slot = slot;
				if (status = psyfile_read(self->fp, &subslot, sizeof(int32_t))) {
					return status;
				}
				sample->sampleindex.subslot = subslot;
				sample->sampleindex.slot = slot;
				if (status = psyfile_read(self->fp, &samplerindex, sizeof(uint32_t))) {
					return status;
				}
				if (samplerindex == UINT32_MAX) {
					sample->samplerindex = psy_INDEX_INVALID;
				} else {
					sample->samplerindex = samplerindex;
				}
				++c;
			}
		}
		if (status = psyfile_read(self->fp, &tmp_float, sizeof(tmp_float))) {
			return status;
		}
		track->height = tmp_float;
		if (c != numsamples) {
			return PSY_ERRFILE;
		}
	}
	return PSY_OK;
}

int psy_audio_psy3loader_read_patd(psy_audio_PSY3Loader* self)
{			
	int32_t index;
	int32_t temp;
	int32_t lpb;
	psy_dsp_big_beat_t bpl;
	char patternname[32];
	/* number of lines of each pattern */
	int32_t patternlines;
	int32_t songtracks;
	int status;
	psy_audio_Pattern* pattern;
	uint32_t sizez77 = 0;
	byte* psource;
	psy_audio_LegacyPattern pdest;
	//int32_t y;
	size_t destsize;

	pattern = NULL;
	lpb = (int32_t)self->song->properties.lpb;
	bpl = 1 / (psy_dsp_big_beat_t) lpb;
	if (status = psyfile_read(self->fp, &index, sizeof index)) {
		return status;
	}	
	/* num lines */
	if (status = psyfile_read(self->fp, &temp, sizeof temp)) {
		return status;
	}
	/*
	** clear it out if it already exists
	** removepattern(index);
	*/
	patternlines = temp;
	/*
	** num tracks per pattern
	** eventually this may be variable per pattern, like when we get multipattern
	*/
	if (status = psyfile_read(self->fp, &songtracks, sizeof temp)) {
		return status;
	}
	if (status = psyfile_readstring(self->fp, patternname, 32)) {
		return status;
	}
	if (status = psyfile_read(self->fp, &sizez77, sizeof sizez77)) {
		return status;
	}
	if (self->fp->currchunk.version > 1) {
		psyfile_skip(self->fp, sizez77);
		pattern = psy_audio_pattern_allocinit();
		psy_audio_patterns_insert(&self->song->patterns, index, pattern);
		psy_audio_pattern_setname(pattern, patternname);
	} else {
		psource = (byte*)malloc(sizez77);
		if (status = psyfile_read(self->fp, psource, sizez77)) {
			free(psource);
			return status;
		}
		beerz77decomp2(psource, &pdest, &destsize);
		free(psource);
		pattern = psy_audio_pattern_allocinit();
		/* songtracks = patternlines[index] > 0 ? destsize / ((size_t)patternlines[index] * EVENT_SIZE) : 0; */
		psy_audio_convert_legacypattern(
			pattern, pdest,
			(uint32_t)psy_audio_song_numsongtracks(self->song),
			patternlines,
			lpb);
		free(pdest);
		pdest = 0;
		psy_audio_patterns_insert(&self->song->patterns, index, pattern);
		psy_audio_pattern_setname(pattern, patternname);						
		/* fix for a bug existing in the song saver in the 1.7.x series */
		if ((self->fp->currchunk.version == 0x0000) && psyfile_getpos(self->fp) ==
			self->fp->currchunk.begins + self->fp->currchunk.size + 4) {
			self->fp->currchunk.size += 4;
		}
	}	
	if (self->fp->currchunk.version > 0) {
		if (!self->song->patterns.sharetracknames) {
			uint32_t t;
			for (t = 0; t < self->song->patterns.songtracks; t++) {
				char name[40];
				if (status = psyfile_readstring(self->fp, name, 40)) {
					return status;
				}
				/* changetrackname(index,t,name); */
			}
		}
	}
	if (self->fp->currchunk.version > 1) {
		if (!pattern) {
			return PSY_ERRFILE;
		}
		psy_audio_psy3loader_read_epat(self, index, pattern);
	}			
	return PSY_OK;
}

int psy_audio_psy3loader_read_epat(psy_audio_PSY3Loader* self, int32_t index,
	psy_audio_Pattern* pattern)
{				
	int32_t temp;
	int32_t utemp;
	float ftemp;	
	uint32_t numentries;
	uint32_t c;	
	int status;

	/* pattern timesig */
	if (status = psyfile_read(self->fp, &utemp, sizeof utemp)) {
		return status;
	}
	pattern->timesig_nominator = utemp;
	if (status = psyfile_read(self->fp, &utemp, sizeof utemp)) {
		return status;
	}
	pattern->timesig_denominator = utemp;					
	/* pattern length */
	if (status = psyfile_read(self->fp, &ftemp, sizeof ftemp)) {
		return status;
	}
	psy_audio_pattern_setlength(pattern, ftemp);	
	/* num entries */
	if (status = psyfile_read(self->fp, &temp, sizeof temp)) {
		return status;
	}
	numentries = temp;
	for (c = 0; c < numentries; ++c) {
		psy_audio_PatternEntry* entry;
		uint32_t numevents;
		uint32_t j;

		entry = psy_audio_patternentry_alloc();
		entry->events = 0;
		/* read track */
		if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
			return status;
		}
		entry->track = temp;
		/* read offset */
		if (status = psyfile_read(self->fp, &ftemp, sizeof(temp))) {
			return status;
		}
		entry->offset = ftemp;				
		/* num events */
		if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
			return status;
		}
		numevents = temp;
		/* read events */
		for (j = 0; j < numevents; ++j) {
			psy_audio_PatternEvent ev;

			psy_audio_patternevent_clear(&ev);
			if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
				return status;
			}
			ev.note = temp;
			if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
				return status;
			}
			ev.inst = temp;
			if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
				return status;
			}
			ev.mach = temp;
			if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
				return status;
			}
			ev.vol = temp;
			if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
				return status;
			}
			ev.cmd = temp;
			if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
				return status;
			}
			ev.parameter = temp;
			psy_audio_patternentry_addevent(entry, &ev);
		}
		psy_list_append(&pattern->events, entry);
	}	
	return PSY_OK;
}

int psy_audio_psy3loader_read_insd(psy_audio_PSY3Loader* self)
{	
	int status;
	int32_t index;	
		
	if (status = psyfile_read(self->fp, &index, sizeof index)) {
		return PSY_OK;
	}
	if(index < MAX_INSTRUMENTS)
	{	
		psy_audio_LegacyInstrument legacy_instr;
		psy_audio_Instrument* instrument;
		char instrum_name[32];
		psy_audio_NewNoteAction nna;
		int32_t val;
		int32_t pan;
		int32_t numwaves;
		int32_t i;
						
		psy_audio_legacyinstrument_init(&legacy_instr);
		if (status = psyfile_read(self->fp, &legacy_instr._loop,
				sizeof(legacy_instr._loop))) {
			return status;
		}		
		if (status = psyfile_read(self->fp, &legacy_instr._lines,
				sizeof(legacy_instr._lines))) {
			return status;
		}		
		if (status = psyfile_read(self->fp, &legacy_instr._NNA,
				sizeof(legacy_instr._NNA))) {
			return status;
		}
		/* 
		** \verbatim
		** NNA values overview:
		** 
		** 0 = Note Cut      [Fast Release 'Default']
		** 1 = Note Release  [Release Stage]
		** 2 = Note Continue [No NNA]
		** \endverbatim
		*/
		switch (legacy_instr._NNA) {
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
		/*
		** read envelopes
		** ENV_VOL
		*/
		if (status = psyfile_read(self->fp, &legacy_instr.ENV_AT,
				sizeof(legacy_instr.ENV_AT))) {			
			return status;
		}
		if (status = psyfile_read(self->fp, &legacy_instr.ENV_DT,
				sizeof(legacy_instr.ENV_DT))) {
			return status;
		}
		if (status = psyfile_read(self->fp, &legacy_instr.ENV_SL,
				sizeof(legacy_instr.ENV_SL))) {
			return status;
		}
		if (status = psyfile_read(self->fp, &legacy_instr.ENV_RT,
				sizeof(legacy_instr.ENV_RT))) {			
			return status;
		}		
		/* ENV_F */
		if (status = psyfile_read(self->fp, &legacy_instr.ENV_F_AT,
				sizeof(legacy_instr.ENV_F_AT))) {
			return status;
		}
		if (status = psyfile_read(self->fp, &legacy_instr.ENV_F_DT,
				sizeof(legacy_instr.ENV_F_DT))) {
			return status;
		}
		if (status = psyfile_read(self->fp, &legacy_instr.ENV_F_SL,
				sizeof(legacy_instr.ENV_F_SL))) {			
			return status;
		}
		if (status = psyfile_read(self->fp, &legacy_instr.ENV_F_RT,
				sizeof(legacy_instr.ENV_F_RT))) {			
			return status;
		}			
		if (status = psyfile_read(self->fp, &legacy_instr.ENV_F_CO,
				sizeof(legacy_instr.ENV_F_CO))) {			
			return status;
		}
		if (status = psyfile_read(self->fp, &legacy_instr.ENV_F_RQ,
				sizeof(legacy_instr.ENV_F_RQ))) {
			return status;
		}
		if (status = psyfile_read(self->fp, &legacy_instr.ENV_F_EA,
				sizeof(legacy_instr.ENV_F_EA))) {
			return status;
		}			
		if (status = psyfile_read(self->fp, &val, sizeof(val))) {			
			return status;
		}
		legacy_instr.ENV_F_TP = val;
		if (status = psyfile_read(self->fp, &pan, sizeof(pan))) {
			return status;
		}
		if (status = psyfile_read(self->fp, &legacy_instr._RPAN,
				sizeof(legacy_instr._RPAN))) {			
			return status;
		}
		if (status = psyfile_read(self->fp, &legacy_instr._RCUT,
				sizeof(legacy_instr._RCUT))) {			
			return status;
		}
		if (status = psyfile_read(self->fp, &legacy_instr._RRES,
				sizeof(legacy_instr._RRES))) {			
			return status;
		}							
		if (status = psyfile_readstring(self->fp, instrum_name, sizeof(instrum_name))) {
			return status;
		}
		/* now we have to read waves */
		if (status = psyfile_read(self->fp, &numwaves, sizeof(numwaves))) {			
			return status;
		}
		for (i = 0; i < numwaves; i++)
		{
			if (status = psy_audio_psy3loader_loadwavesubchunk(self, index,
					pan, instrum_name, 1, i)) {
				return status;
			}
		}
		if ((self->fp->currchunk.version & 0xFF) >= 1)
		{ /* revision 1 or greater */
			if (status = psyfile_read(self->fp, &legacy_instr.sampler_to_use,
					sizeof(legacy_instr.sampler_to_use))) {
				return status;
			}
			if (status = psyfile_read(self->fp, &legacy_instr._LOCKINST,
					sizeof(legacy_instr._LOCKINST))) {
				return status;
			}
		}
		/* Ensure validity of values read */
		if (legacy_instr.sampler_to_use < 0 ||
				legacy_instr.sampler_to_use >= MAX_BUSES) {
			legacy_instr._LOCKINST=FALSE;
			legacy_instr.sampler_to_use = -1;
		}
		instrument = psy_audio_instrument_allocinit();
		psy_audio_instrument_setname(instrument, instrum_name);
		psy_audio_instrument_setindex(instrument, index);
		psy_audio_instruments_insert(&self->song->instruments, instrument,
			psy_audio_instrumentindex_make(0, index));
		psy_audio_convert_legacy_to_instrument(instrument,
			legacy_instr);
	}
	return PSY_OK;
}

int psy_audio_psy3loader_read_eins(psy_audio_PSY3Loader* self)
{   
	/*
	** Legacy for sampulse previous to Psycle 1.12	
	** Version zero was the development version (1.7 alphas, psycle-core).
	** Version one is the published one (1.8.5 onwards).
	*/
	uint32_t filepos = psyfile_getpos(self->fp);
	uint32_t  numSamples;
	uint32_t i;	
	char temp[8];
	/* lowversion 0 is 1.8.5, lowversion 1 is 1.8.6 */
	uint32_t lowversion = (self->fp->currchunk.version&0xFFFF);
	int idx;		
	/* Instrument Data Load */
	uint32_t numInstruments;
	int status;

	if (status = psyfile_read(self->fp, &numInstruments,
			sizeof(numInstruments))) {
		return status;
	}
	for(i = 0; i < numInstruments && filepos < self->fp->currchunk.begins +
			self->fp->currchunk.size; i++) {
		uint32_t sizeINST=0;

		if (status = psyfile_read(self->fp, &idx, sizeof(idx))) {
			return status;
		}
		if (status = psyfile_read(self->fp, &temp, 4)) {
			return status;
		}
		temp[4]='\0';
		if (status = psyfile_read(self->fp, &sizeINST, sizeof(sizeINST))) {
			return status;
		}
		filepos = psyfile_getpos(self->fp);
		if (strcmp(temp,"INST")== 0) {
			uint32_t versionINST;

			if (status = psyfile_read(self->fp, &versionINST,
					sizeof(versionINST))) {
				return status;
			}
			if (versionINST == 1) {
				bool legacyenabled;
				if (status = psyfile_read(self->fp, &legacyenabled,
						sizeof(legacyenabled))) {
					return status;
				}
			} else {
				/* versionINST 0 was not stored, so seek back. */
				if (psyfile_seek(self->fp, filepos) == -1) {
					return PSY_ERRFILE;
				}
				versionINST = 0;
			}
			/* XMInstrument inst; */
			if (status = psy_audio_psy3loader_loadxminstrument(self, 0, TRUE,
					lowversion)) {
				return status;
			}
			/*
			** inst.Load(*pFile, versionINST, true, lowversion);
			** xminstruments.SetInst(inst,idx);
			*/
		}
		if (lowversion > 0) {
			/*
			** Version 0 doesn't write the chunk size correctly
			** so we cannot correct it in case of error
			*/
			if (psyfile_seek(self->fp, filepos + sizeINST) == -1) {
				return PSY_ERRFILE;
			}
			filepos = psyfile_getpos(self->fp);
		}
	}		
	if (status = psyfile_read(self->fp, &numSamples, sizeof(numSamples))) {
		return status;
	}
	for(i = 0;i < numSamples && filepos <
		self->fp->currchunk.begins + self->fp->currchunk.size;i++)
	{
		char temp[8];
		uint32_t versionSMPD;
		uint32_t sizeSMPD=0;

		if (status = psyfile_read(self->fp, &idx, sizeof(idx))) {
			return status;
		}
		if (status = psyfile_read(self->fp, &temp, 4)) {
			return status;
		}
		temp[4]='\0';
		if (status = psyfile_read(self->fp, &sizeSMPD, sizeof(sizeSMPD))) {
			return status;
		}
		filepos = psyfile_getpos(self->fp);
		if (strcmp(temp,"SMPD")== 0)
		{
			psy_audio_Sample* wave;
			if (status = psyfile_read(self->fp, &versionSMPD, sizeof(versionSMPD))) {
				return status;
			}
			/* versionSMPD 0 was not stored, so seek back. */
			if (versionSMPD != 1) {
				if (psyfile_seek(self->fp, filepos) == -1) {
					return PSY_ERRFILE;
				}
				versionSMPD = 0;
			}
			wave = psy_audio_sample_allocinit(1);
			if (status = psy_audio_psy3loader_xmloadwav(self, wave)) {
				return status;
			}
			if (!wave) {
				return PSY_ERRFILE;
			}
			psy_audio_samples_insert(&self->song->samples, wave,
				psy_audio_sampleindex_make(idx, 0));
			/*
			** XMInstrument::WaveData<> wave;
			** wave.Load(*pFile, versionSMPD, true);
			** samples.SetSample(wave, idx);
			*/
		}
		if (lowversion > 0) {
			/*
			** Version 0 doesn't write the chunk size correctly
			** so we cannot correct it in case of error
			*/
			if (psyfile_seek(self->fp, filepos + sizeSMPD) == -1) {
				return PSY_ERRFILE;
			}
			filepos = psyfile_getpos(self->fp);
		}
	}
	return PSY_OK;
}

int psy_audio_psy3loader_read_smid(psy_audio_PSY3Loader* self)
{		
	uint32_t groupidx;
	uint32_t instidx;
	int status;
	psy_audio_Instrument* instrument;

	assert(self);

	if (status = psyfile_read(self->fp, &instidx, sizeof(instidx))) {
		return status;
	}	
	instrument = psy_audio_instrument_allocinit();
	psy_audio_instrument_setindex(instrument, instidx);	
	if (status = psy_audio_psy3loader_loadxminstrument(self, instrument, 0,
			self->fp->currchunk.version & 0xFFFF)) {
		psy_audio_instrument_deallocate(instrument);
		return status;
	}
	/* read groupidx */
	if ((psyfile_currchunkversion(self->fp) &
			CURRENT_FILE_VERSION_SMID) > 1) {
		uint8_t revert;

		if (status = psyfile_read(self->fp, &revert, sizeof(uint8_t))) {
			psy_audio_instrument_deallocate(instrument);
			return status;
		}
		if (revert != FALSE) {
			instidx = UINT32_MAX - instidx;
		}
		if (status = psyfile_read(self->fp, &groupidx, sizeof(uint32_t))) {
			psy_audio_instrument_deallocate(instrument);
			return status;
		}
	} else {
		groupidx = 1;
	}	
	psy_audio_instruments_insert(&self->song->instruments, instrument,
		psy_audio_instrumentindex_make(groupidx, instidx));
	return PSY_OK;
}

int psy_audio_psy3loader_xmloadwav(psy_audio_PSY3Loader* self, psy_audio_Sample* wave)
{
	int status;
	uint32_t size1;
	uint32_t size2;
	char wavename[256];			
	uint32_t temp;
	uint8_t temp8;
	uint16_t temp16;			
	float ftemp;
	unsigned char btemp;	
	
	if (status = psyfile_readstring(self->fp, wavename, sizeof(wavename))) {
		return status;
	}
	psy_audio_sample_setname(wave, wavename);
	/* wavelength */
	if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->numframes = temp;
	/* global volume */
	if (status = psyfile_read(self->fp, &ftemp, sizeof(ftemp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->globalvolume = ftemp;
	/* default volume */
	if (status = psyfile_read(self->fp, &temp16, sizeof(temp16))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->defaultvolume = temp16;
	/* wave loop start */
	if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->loop.start = temp;
	/* wave loop end */
	if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->loop.end = temp;
	/* wave loop type */
	if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->loop.type = (psy_audio_SampleLoopType) temp;
	/* wave sustain loop start */
	if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->sustainloop.start = temp;
	/* wave sustain loop end */
	if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->sustainloop.end = temp;
	if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->sustainloop.type = (psy_audio_SampleLoopType) temp;			
	/*
	** "bigger than" insted of "bigger or equal", because that means
	**  interpolate between loopend and loopstart
	*/
	if (wave->loop.end > wave->numframes) {
		wave->loop.end = wave->numframes;
	}
	if (wave->sustainloop.end > wave->numframes) {
		wave->sustainloop.end = wave->numframes;
	} 
	if (self->fp->currchunk.version == 0) {
		wave->samplerate = 8363;
	} else {
		if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
			psy_audio_sample_deallocate(wave);
			return status;
		}
		wave->samplerate = temp;				
	}
	/* wave tune */
	if (status = psyfile_read(self->fp, &temp16, sizeof(temp16))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->zone.tune = temp16;
	/* wave fine tune */
	if (status = psyfile_read(self->fp, &temp16, sizeof(temp16))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->zone.finetune = temp16;
	/* wave stereo */
	if (status = psyfile_read(self->fp, &btemp, sizeof(btemp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->stereo = btemp;
	/* pan enabled */
	if (status = psyfile_read(self->fp, &btemp, sizeof(btemp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->panenabled = btemp;
	/* pan factor */
	if (status = psyfile_read(self->fp, &ftemp, sizeof(ftemp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->panfactor = ftemp;			
	if(self->fp->currchunk.version >= 1) {
		/* surround */
		if (status = psyfile_read(self->fp, &btemp, sizeof(btemp))) {
			psy_audio_sample_deallocate(wave);
			return status;
		}
		wave->surround = btemp;				
	}
	else if (wave->panfactor > 1.0f) {
		wave->surround = 1;
		wave->panfactor -= 1.0f;
	} else { 
		wave->surround = 0;
	}
	/* vibrato attack */
	if (status = psyfile_read(self->fp, &temp8, sizeof(temp8))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->vibrato.attack = temp8;			
	/* vibrato speed */
	if (status = psyfile_read(self->fp, &temp8, sizeof(temp8))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->vibrato.speed = temp8;
	/* vibrato depth */
	if (status = psyfile_read(self->fp, &temp8, sizeof(temp8))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->vibrato.depth = temp8;			
	/* vibrato type */
	if (status = psyfile_read(self->fp, &temp8, sizeof(temp8))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	if (temp8 <= psy_audio_WAVEFORMS_RANDOM) {
		wave->vibrato.type = (psy_audio_WaveForms)temp8;
	} else { 
		wave->vibrato.type = (psy_audio_WaveForms)psy_audio_WAVEFORMS_SINUS;
	}														
	{ /* wave data */
		byte* pData;
		short* pDest;
		uint32_t i;
		if (status = psyfile_read(self->fp, &size1, sizeof(size1))) {
			psy_audio_sample_deallocate(wave);			
			return status;
		}
		pData = malloc(size1);
		if (status = psyfile_read(self->fp, pData, size1)) {
			psy_audio_sample_deallocate(wave);			
			free(pData);
			return status;
		}
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

			if (status = psyfile_read(self->fp, &size2, sizeof(size1))) {
				psy_audio_sample_deallocate(wave);				
				return status;
			}
			pData = malloc(size2);
			if (status = psyfile_read(self->fp, pData, size1)) {
				psy_audio_sample_deallocate(wave);
				free(pData);
				return status;
			}
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
	return PSY_OK;
}

int psy_audio_psy3loader_loadxminstrument(psy_audio_PSY3Loader* self,
	psy_audio_Instrument* instrument, bool islegacy, uint32_t legacyversion)
{		
	/* SMID chunk */
	char name[256];
	uint16_t m_Lines;
	/* [0..1.0f] Global volume affecting all samples of the instrument. */
	float m_GlobVol;
	/* [0..1.0f] Fadeout speed. Decreasing amount for each tracker tick. */
	float m_VolumeFadeSpeed;
	/* Paninng */
	bool m_PanEnabled;
	/* Initial panFactor (if enabled) [-1..1] */
	float m_InitPan;
	bool m_Surround;
	/* Note number for center pan position */
	uint8_t m_NoteModPanCenter;
	/* -32..32. 1/256th of panFactor change per seminote. */
	int8_t m_NoteModPanSep;

	/* Cutoff Frequency [0..127] */
	uint8_t m_FilterCutoff;
	/* Resonance [0..127] */
	uint8_t m_FilterResonance;
	int32_t m_FilterType;
	int status;

	/* Randomness. Applies on new notes. */

	/* Random Volume % [ 0.0 -> No randomize. 1.0 = randomize full scale.] */
	float m_RandomVolume;
	/* Random Panning (same) */
	float m_RandomPanning;
	/* Random CutOff (same) */
	float m_RandomCutoff;
	/* Random Resonance (same) */
	float m_RandomResonance;
		
	/* Check to do when a new event comes in the channel. */
	int32_t m_DCT;
	/*
	** Action to take on the playing voice when the action defined by m_DCT comes in the same channel
	** (like the same note value).
	*/
	int32_t m_DCA;
	int legacyeins;
	static const int note_map_size = 120; /* C-0 .. B-9 */

	legacyeins = 0;
	if (status = psyfile_readstring(self->fp, name, 256)) {
		return status;
	}
	psy_audio_instrument_setname(instrument, name);
	if (status = psyfile_read(self->fp, &m_Lines, sizeof(m_Lines))) {
		return status;
	}
	if (islegacy) m_Lines = 0;

	if (status = psyfile_read(self->fp, &m_GlobVol, sizeof(m_GlobVol))) {
		return status;
	}
	if (instrument) {
		instrument->globalvolume = m_GlobVol;	
	}
	if (status = psyfile_read(self->fp, &m_VolumeFadeSpeed, sizeof(m_VolumeFadeSpeed))) {
		return status;
	}
	if (instrument) {
		instrument->volumefadespeed = m_VolumeFadeSpeed;
	}

	if (status = psyfile_read(self->fp, &m_InitPan, sizeof(m_InitPan))) {
		return status;
	}
	if (instrument) {
		instrument->initpan = m_InitPan;
	}
	if (status = psyfile_read(self->fp, &m_PanEnabled, sizeof(m_PanEnabled))) {
		return status;
	}
	if (instrument) {
		instrument->panenabled = m_PanEnabled;
	}
	if (self->fp->currchunk.version == 0) {
		m_Surround = FALSE;
	} else {
		if (status = psyfile_read(self->fp, &m_Surround, sizeof(m_Surround))) {
			return status;
		}
	}
	if (instrument) {
		instrument->surround = m_Surround;
	}
	if (status = psyfile_read(self->fp, &m_NoteModPanCenter, sizeof(m_NoteModPanCenter))) {
		return status;
	}
	if (instrument) {
		instrument->notemodpancenter = m_NoteModPanCenter;
	}
	if (status = psyfile_read(self->fp, &m_NoteModPanSep, sizeof(m_NoteModPanSep))) {
		return status;
	}
	if (instrument) {
		instrument->notemodpansep = m_NoteModPanSep;
	}	

	if (status = psyfile_read(self->fp, &m_FilterCutoff, sizeof(m_FilterCutoff))) {
		return status;
	}
	if (instrument) {
		instrument->filtercutoff = m_FilterCutoff / 127.f;
	}
	if (status = psyfile_read(self->fp, &m_FilterResonance, sizeof(m_FilterResonance))) {
		return status;
	}
	if (instrument) {
		instrument->filterres = m_FilterResonance / 127.f;
	}
	{ 
		uint16_t unused = 0;

		if (status = psyfile_read(self->fp, &unused, sizeof(unused))) {
			return status;
		}
	}
	{
		uint32_t i = 0;
		psy_dsp_FilterType ft;

		if (status = psyfile_read(self->fp, &i, sizeof(i))) {
			return status;
		}
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
			case 4:	ft = F_NONE; /* This one is kept here because it is used in load/save. Also used in Sampulse instrument filter as "use channel default" */
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
	if (status = psyfile_read(self->fp, &m_RandomVolume, sizeof(m_RandomVolume))) {
		return status;
	}
	if (instrument) {
		instrument->randomvolume = m_RandomVolume;
	}
	if (status = psyfile_read(self->fp, &m_RandomPanning, sizeof(m_RandomPanning))) {
		return status;
	}
	if (instrument) {
		instrument->randompanning = m_RandomPanning;
	}
	if (status = psyfile_read(self->fp, &m_RandomCutoff, sizeof(m_RandomCutoff))) {
		return status;
	}
	if (instrument) {
		instrument->randomcutoff = m_RandomCutoff;
	}
	if (status = psyfile_read(self->fp, &m_RandomResonance, sizeof(m_RandomResonance))) {
		return status;
	}
	if (instrument) {
		instrument->randomresonance = m_RandomResonance;
	}
	{
		uint32_t i = 0;		

		if (status = psyfile_read(self->fp, &i, sizeof(i))) {
			return status;
		}
		if (instrument) {
			/*
			** Action to take on the playing voice when any new note comes 
			** in the same channel.
			*/
			psy_audio_NewNoteAction nna;
			/*
			** NNA_STOP = 0x0,		///  [Note Cut]	
			** (This one actually does a very fast fadeout)
			** NNA_CONTINUE = 0x1,	///  [Ignore]
			** NNA_NOTEOFF = 0x2,	///  [Note off]
			** NNA_FADEOUT = 0x3	///  [Note fade]
			*/
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
		if (status = psyfile_read(self->fp, &i, sizeof(i))) {
			return status;
		}
		m_DCT = (psy_audio_DupeCheck)i;
		if (status = psyfile_read(self->fp, &i, sizeof(i))) {
			return status;
		}
		m_DCA = (psy_audio_NewNoteAction)i;
	}
	{
		int32_t i;		
		bool first;		
		uint8_t targetnote;
		uint8_t sampleslot;
		psy_audio_InstrumentEntry instentry;		

		psy_audio_instrumententry_init(&instentry);
		instentry.sampleindex = psy_audio_sampleindex_make(0, 0);
		if (instrument) {
			psy_audio_instrument_clearentries(instrument);
		}
		first = TRUE;
		for(i = 0; i < note_map_size; i++) {
			int note;

			note = i;			
			if (status = psyfile_read(self->fp, &targetnote, sizeof(targetnote))) {
				return status;
			}
			if (status = psyfile_read(self->fp, &sampleslot, sizeof(sampleslot))) {
				return status;
			}			
			if (instrument) {
				if (first) {
					instentry.sampleindex.slot = sampleslot;
					instentry.keyrange.low = note;
					if (note != targetnote) {
						instentry.fixedkey = targetnote;
					} else {
						instentry.fixedkey = psy_audio_NOTECOMMANDS_EMPTY;
					}
					first = 0;
				} else if (sampleslot != instentry.sampleindex.slot ||
						note != targetnote) {					
					instentry.keyrange.high = note - 1;				
					psy_audio_instrument_addentry(instrument, &instentry);
					if (note != targetnote) {
						instentry.fixedkey = targetnote;
					} else {
						instentry.fixedkey = psy_audio_NOTECOMMANDS_EMPTY;
					}
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
	assert(instrument);
	if (status = psy_audio_psy3loader_xminstrumentenvelopeload(self, &instrument->volumeenvelope, 0, 0)) {
		return status;
	}
	if (islegacy && legacyeins==0) {
		/* Workaround for a bug in that version */
		if (status = psy_audio_psy3loader_xminstrumentenvelopeload(self, &instrument->filterenvelope, islegacy, self->fp->currchunk.version)) {
			return status;
		}
		if (status = psy_audio_psy3loader_xminstrumentenvelopeload(self, &instrument->panenvelope, islegacy, self->fp->currchunk.version)) {
			return status;
		}
	}
	else {
		if (status = psy_audio_psy3loader_xminstrumentenvelopeload(self, &instrument->panenvelope, islegacy, self->fp->currchunk.version)) {
			return status;
		}
		if (status = psy_audio_psy3loader_xminstrumentenvelopeload(self, &instrument->filterenvelope, islegacy, self->fp->currchunk.version)) {
			return status;
		}
	}
	if (status = psy_audio_psy3loader_xminstrumentenvelopeload(self, &instrument->pitchenvelope, islegacy, self->fp->currchunk.version)) {
		return status;
	}
	/* ValidateEnabled(); */
	return PSY_OK;
}
			
/* The meaning of the first value (int), is time, and the unit depends on the context. */
typedef struct {
	int first;
	float second;
} PointValue;

int psy_audio_psy3loader_xminstrumentenvelopeload(psy_audio_PSY3Loader* self,
	psy_dsp_Envelope* envelope,
	bool legacy, uint32_t legacyversion)
{
	char temp[8];
	uint32_t version=0;
	uint32_t size=0;
	uint32_t filepos=0;
	/* Envelope is enabled or disabled */
	bool m_Enabled;
	/* if m_Carry and a new note enters, the envelope position is set to that of the previous note *on the same channel* */
	bool m_Carry;
	/*
	** Array of Points of the envelope.
	** first : time at which to set the value. Unit can be different things depending on the context.
	** second : 0 .. 1.0f . (or -1.0 1.0 or whatever else) Use it as a multiplier.
	** PointValue m_Points[1024];
	*/
	/* Loop Start Point */
	unsigned int m_LoopStart;
	/* Loop End Point */
	unsigned int m_LoopEnd; 
	/* Sustain Start Point */
	unsigned int m_SustainBegin;
	/* Sustain End Point */
	unsigned int m_SustainEnd;
	/* Envelope mode (meaning of the time value) */
	int32_t m_Mode;
	/* Indicates that this envelope is operated as an ADSR (it is an option for the visual component). */
	bool m_Adsr;
	int status;

	psy_dsp_envelope_clear(envelope);
	if (!legacy) {
		if (status = psyfile_read(self->fp, &temp, 4)) {
			return status;
		}
		temp[4]='\0';
		if (status = psyfile_read(self->fp, &version, sizeof(version))) {
			return status;
		}
		if (status = psyfile_read(self->fp, &size, sizeof(size))) {
			return status;
		}
		filepos = psyfile_getpos(self->fp);
		if (strcmp("SMIE", temp) !=0 ) {
			if (psyfile_skip(self->fp, size) == -1) {
				return PSY_ERRFILE;
			}
			return PSY_OK;
		}
	} else {
		version = legacyversion;
	}

	/* Information starts here */
	if (status = psyfile_read(self->fp, &m_Enabled, sizeof(m_Enabled))) {
		return status;
	}
	envelope->enabled = m_Enabled;
	if (status = psyfile_read(self->fp, &m_Carry, sizeof(m_Carry))) {
		return status;
	}
	envelope->carry = m_Carry;
	{
		uint32_t i32 = 0;

		if (status = psyfile_read(self->fp, &i32, sizeof(i32))) {
			return status;
		}
		m_LoopStart = i32;
		envelope->loopstart = m_LoopStart;
		if (status = psyfile_read(self->fp, &i32, sizeof(i32))) {
			return status;
		}
		m_LoopEnd = i32;
		envelope->loopend = m_LoopEnd;
		if (status = psyfile_read(self->fp, &i32, sizeof(i32))) {
			return status;
		}
		m_SustainBegin = i32;
		envelope->sustainbegin = m_SustainBegin;
		if (status = psyfile_read(self->fp, &i32, sizeof(i32))) {
			return status;
		}
		m_SustainEnd = i32;
		envelope->sustainend = m_SustainEnd;
	}
	{
		uint32_t num_of_points = 0;
		uint32_t i;

		if (status = psyfile_read(self->fp, &num_of_points, sizeof(num_of_points))) {
			return status;
		}
		for(i = 0; i < num_of_points; i++){
			PointValue value;
			/*
			** The time in which this point is placed. The unit depends on the
			** mode.
			*/
			if (status = psyfile_read(self->fp, &value.first, sizeof(value.first))) {
				return status;
			}
			/*
			** The value that this point has. Depending on the type of
			** envelope, this can range between 0.0 and 1.0 or between -1.0 and
			** 1.0
			*/
			if (status = psyfile_read(self->fp, &value.second, sizeof(value.second))) {
				return status;
			}
			/* todo range */
			psy_dsp_envelope_append(envelope,
				psy_dsp_envelopepoint_make_all(
					(psy_dsp_seconds_t)value.first,
					value.second, 0.f,
					100000.f, -1.f, 1.f));			
			/* m_Points.push_back(value); */
		}
	}
	if (version == 0) {
		m_Mode = 0; /* Mode::TICK; */
		m_Adsr = FALSE;
	}
	else {
		{
			uint32_t read;
			if (status = psyfile_read(self->fp, &read, sizeof(read))) {
				return status;
			}
			m_Mode = read;
		}
		if (status = psyfile_read(self->fp, &m_Adsr, sizeof(m_Adsr))) {
			return status;
		}
		envelope->timemode = (psy_dsp_EnvelopeTimeMode)m_Mode;		
	}
	/* (convert ms to seconds) */
	if (m_Mode == psy_dsp_ENVELOPETIME_SECONDS) {
		psy_List* node;
		int c;

		for (c = 0, node = envelope->points; node != NULL; psy_list_next(&node), ++c) {
			psy_dsp_EnvelopePoint* pt;

			pt = (psy_dsp_EnvelopePoint*)psy_list_entry(node);
			pt->time /= 1000;
			if (m_Adsr) {
				if (c == 0) {
					pt->minvalue = 0.f;
					pt->maxvalue = 0.f;
				} else if (c == 1) {
					pt->minvalue = 1.f;
					pt->maxvalue = 1.f;
				} else if (c == 2) {
					pt->minvalue = 0.f;
					pt->maxvalue = 1.f;
				} else if (c == 3) {
					pt->minvalue = 0.f;
					pt->maxvalue = 0.f;
				}			
			}
		}
	}
	/* Information ends here */
	if (!legacy) {
		if (psyfile_seek(self->fp, filepos + size) == -1) {
			return PSY_ERRFILE;
		}
	}
	return PSY_OK;
}

int psy_audio_psy3loader_read_smsb(psy_audio_PSY3Loader* self)
{	
	int status;
	uint32_t sampleidx;
	uint32_t samplesubidx;
	uint32_t size1;
	uint32_t size2;
	char wavename[256];
	uint32_t temp;
	uint8_t temp8;
	uint16_t temp16;	
	float ftemp;
	unsigned char btemp;
	psy_audio_Sample* wave;

	if (status = psyfile_read(self->fp, &sampleidx, sizeof(sampleidx))) {
		return status;
	}			
	wave = psy_audio_sample_allocinit(1);
	if (status = psyfile_readstring(self->fp, wavename, sizeof(wavename))) {
		return status;
	}
	psy_audio_sample_setname(wave, wavename);
	/* wavelength */
	if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->numframes = temp;
	/* global volume */
	if (status = psyfile_read(self->fp, &ftemp, sizeof(ftemp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->globalvolume = ftemp;
	/* default volume */
	if (status = psyfile_read(self->fp, &temp16, sizeof(temp16))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->defaultvolume = temp16;
	/* wave loop start */
	if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->loop.start = temp;
	/* wave loop end */
	if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->loop.end = temp;
	/* wave loop type */
	if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->loop.type = (psy_audio_SampleLoopType)temp;
	/* wave sustain loop start */
	if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->sustainloop.start = temp;
	/* wave sustain loop end */
	if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->sustainloop.end = temp;
	if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->sustainloop.type = (psy_audio_SampleLoopType)temp;
	/*
	** "bigger than" insted of "bigger or equal", because that means 
	**  interpolate between loopend and loopstart
	*/
	if (wave->loop.end > wave->numframes) {
		wave->loop.end = wave->numframes;
	}
	if (wave->sustainloop.end > wave->numframes) {
		wave->sustainloop.end = wave->numframes;
	}
	if (self->fp->currchunk.version == 0) {
		wave->samplerate = 8363;
	} else {
		if (status = psyfile_read(self->fp, &temp, sizeof(temp))) {
			psy_audio_sample_deallocate(wave);
			return status;
		}
		wave->samplerate = temp;
	}
	/* wave tune */
	if (status = psyfile_read(self->fp, &temp16, sizeof(temp16))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->zone.tune = temp16;
	/* wave fine tune */
	if (status = psyfile_read(self->fp, &temp16, sizeof(temp16))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->zone.finetune = temp16;
	/* wave stereo */
	if (status = psyfile_read(self->fp, &btemp, sizeof(btemp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->stereo = btemp;
	/* pan enabled */
	if (status = psyfile_read(self->fp, &btemp, sizeof(btemp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->panenabled = btemp;
	/* pan factor */
	if (status = psyfile_read(self->fp, &ftemp, sizeof(ftemp))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->panfactor = ftemp;
	if (self->fp->currchunk.version >= 1) {
		/* surround */
		if (status = psyfile_read(self->fp, &btemp, sizeof(btemp))) {
			psy_audio_sample_deallocate(wave);
			return status;
		}
		wave->surround = btemp;
	} else if (wave->panfactor > 1.0f) {
		wave->surround = 1;
		wave->panfactor -= 1.0f;
	} else {
		wave->surround = 0;
	}
	/* vibrato attack */
	if (status = psyfile_read(self->fp, &temp8, sizeof(temp8))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->vibrato.attack = temp8;
	/* vibrato speed */
	if (status = psyfile_read(self->fp, &temp8, sizeof(temp8))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->vibrato.speed = temp8;
	/* vibrato depth */
	if (status = psyfile_read(self->fp, &temp8, sizeof(temp8))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	wave->vibrato.depth = temp8;			
	/* vibrato type */
	if (status = psyfile_read(self->fp, &temp8, sizeof(temp8))) {
		psy_audio_sample_deallocate(wave);
		return status;
	}
	if (temp8 <= psy_audio_WAVEFORMS_RANDOM) {
		wave->vibrato.type = (psy_audio_WaveForms)temp8;
	} else { 
		wave->vibrato.type = (psy_audio_WaveForms)
			psy_audio_WAVEFORMS_SINUS;
	}														
	{ /* wave data */
		byte* pData;
		short* pDest;
		uint32_t i;
		if (status = psyfile_read(self->fp, &size1, sizeof(size1))) {
			psy_audio_sample_deallocate(wave);
			return status;
		}
		pData = malloc(size1);
		if (status = psyfile_read(self->fp, pData, size1)) {
			free(pData);
			psy_audio_sample_deallocate(wave);
			return status;
		}
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
			if (status = psyfile_read(self->fp, &size2, sizeof(size2))) {
				psy_audio_sample_deallocate(wave);
				return status;
			}
			pData = malloc(size2);
			if (status = psyfile_read(self->fp, pData, size2)) {
				free(pData);
				psy_audio_sample_deallocate(wave);
				return status;
			}
			sounddesquash(pData, &pDest);
			free(pData);					
			for (i = 0; i < wave->numframes; i++) {
				short val = (short) pDest[i];
				wave->channels.samples[1][i] = (float) val;					
			}
			free(pDest);
			pData = 0;					
		}
		/* read subslot */
		if ((psyfile_currchunkversion(self->fp) &
			CURRENT_FILE_VERSION_SMSB) > 1) {
			uint8_t revert;

			if (status = psyfile_read(self->fp, &revert, sizeof(uint8_t))) {
				return status;
			}
			if (revert != FALSE) {
				sampleidx = UINT32_MAX - sampleidx;
			}
			if (status = psyfile_read(self->fp, &samplesubidx, sizeof(uint32_t))) {
				return status;
			}			
		} else {
			samplesubidx = 0;
		}
		psy_audio_samples_insert(&self->song->samples, wave,
			psy_audio_sampleindex_make(sampleidx, samplesubidx));
	}
	return PSY_OK;
}

int psy_audio_psy3loader_loadwavesubchunk(psy_audio_PSY3Loader* self,
	int32_t instrIdx, int32_t pan, char * instrum_name, int32_t fullopen,
	int32_t loadIdx)
{
	char Header[8];
	uint32_t version;
	uint32_t size;
	int status;

	if (status = psyfile_read(self->fp, &Header, 4)) {
		return status;
	}
	Header[4] = 0;
	if (status = psyfile_read(self->fp, &version, sizeof(version))) {
		return status;
	}
	if (status = psyfile_read(self->fp, &size, sizeof(size))) {
		return status;
	}

	/* fileformat supports several waves, but sampler only supports one. */
	if (strcmp(Header,"WAVE") == 0 &&
		version <= CURRENT_FILE_VERSION_WAVE || loadIdx == 0) {
		psy_audio_Sample* sample;
		/* This index represented the index position of this wave for the instrument. 0 to 15. */
		uint32_t legacyindex;
		uint16_t volume = 0;		
		int32_t tmp = 0;
		uint8_t tmp_u8 = 0;
		uint16_t tmp_u16 = 0;
		uint32_t tmp_u32 = 0;		
		uint8_t doloop = 0;
		uint8_t stereo = 0;
		char dummy[32];
		uint32_t packedsize;
		byte* pData;
		int16_t* pDest;
		
		sample = psy_audio_sample_allocinit(1);
		sample->panfactor = (float) pan / 256.f ; /* (value_mapper::map_256_1(pan)); */
		sample->samplerate = 44100;		
		if (status = psyfile_read(self->fp, &tmp_u32, sizeof(uint32_t))) {
			psy_audio_sample_deallocate(sample);
			return status;
		}
		legacyindex = tmp_u32;
		if (status = psyfile_read(self->fp, &tmp_u32, sizeof(uint32_t))) {
			psy_audio_sample_deallocate(sample);
			return status;
		}
		sample->numframes = tmp_u32;
		if (status = psyfile_read(self->fp, &tmp_u16, sizeof(uint16_t))) {
			psy_audio_sample_deallocate(sample);
			return status;
		}
		volume = tmp_u16;		
		sample->globalvolume = volume * 0.01f;
		if (status = psyfile_read(self->fp, &tmp_u32, sizeof(uint32_t))) {
			psy_audio_sample_deallocate(sample);
			return status;
		}
		sample->loop.start = tmp_u32;
		if (status = psyfile_read(self->fp, &tmp_u32, sizeof(uint32_t))) {
			psy_audio_sample_deallocate(sample);
			return status;
		}
		sample->loop.end = tmp_u32;
		if (status = psyfile_read(self->fp, &tmp_u32, sizeof(uint32_t))) {
			psy_audio_sample_deallocate(sample);
			return status;
		}
		sample->zone.tune = tmp_u32;
		if (status = psyfile_read(self->fp, &tmp, sizeof(tmp))) {
			psy_audio_sample_deallocate(sample);
			return status;
		}
		/* Current sampler uses 100 cents. Older used +-256 */
		sample->zone.finetune = (int16_t)(tmp / 2.56f);
		if (status = psyfile_read(self->fp, &tmp_u8, sizeof(uint8_t))) {
			psy_audio_sample_deallocate(sample);
			return status;
		}
		doloop = tmp_u8;
		sample->loop.type = (doloop)
			? psy_audio_SAMPLE_LOOP_NORMAL
			: psy_audio_SAMPLE_LOOP_DO_NOT;
		if (status = psyfile_read(self->fp, &tmp_u8, sizeof(uint8_t))) {
			psy_audio_sample_deallocate(sample);
			return status;
		}
		stereo = tmp_u8;
		sample->stereo = stereo != 0;
		/* Old sample name, never used. */
		if (status = psyfile_readstring(self->fp, dummy, sizeof(dummy))) {
			return status;
		}
		psy_audio_sample_setname(sample, instrum_name);
		if (status = psyfile_read(self->fp, &packedsize, sizeof(packedsize))) {
			psy_audio_sample_deallocate(sample);
			return status;
		}
		if (fullopen) {
			uint32_t i;
			pData = malloc(packedsize+4); /* +4 to avoid any attempt at buffer overflow by the code */
			if (status = psyfile_read(self->fp, pData, packedsize)) {
				psy_audio_sample_deallocate(sample);
				free(pData);
				return status;
			}
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
			if (psyfile_skip(self->fp, packedsize) == -1) {
				psy_audio_sample_deallocate(sample);
				return PSY_ERRFILE;
			}
			sample->channels.samples[0] = 0;
		}
		if (sample->stereo) {
			if (status = psyfile_read(self->fp, &packedsize, sizeof(packedsize))) {
				psy_audio_sample_deallocate(sample);
				return status;
			}
			if ( fullopen ) {
				uint32_t i;

				/* +4 to avoid any attempt at buffer overflow by the code */
				pData = malloc(packedsize+4); 
				if (status = psyfile_read(self->fp, pData, packedsize)) {
					psy_audio_sample_deallocate(sample);
					free(pData);
					return status;
				}
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
				if (psyfile_skip(self->fp, packedsize) == -1) {
					psy_audio_sample_deallocate(sample);
					return PSY_ERRFILE;
				}
				sample->channels.samples[1] = 0;
			}
		}
		psy_audio_samples_insert(&self->song->samples, sample,
			psy_audio_sampleindex_make(instrIdx, 0));
	} else {
		psyfile_skip(self->fp, size);
	}
	return PSY_OK;
}

int psy_audio_psy3loader_read_macd(psy_audio_PSY3Loader* self)
{		
	int32_t index;
	int status;

	if (status = psyfile_read(self->fp, &index, sizeof index)) {
		return status;
	}
	if (index < MAX_MACHINES) {			
		if (status = psy_audio_psy3loader_machineloadchunk(self, index)) {
			return status;
		}
	}
	return PSY_OK;
}

int psy_audio_psy3loader_machineloadchunk(
	psy_audio_PSY3Loader* self, int32_t index)
{
	int status;
	psy_audio_Machine* machine;
	bool replaced;
	char catchername[512];
	char modulename[256];
	char editname[32];	
	int32_t i;
	psy_audio_MachineWires* machinewires;	
	
	if (status = psy_audio_psy3loader_machineloadchunk_createmachine(self, index,
		modulename, catchername, &replaced, &machine)) {
		return status;
	}
	if (!machine) {
		return PSY_ERRFILE;
	}
	if (machine) {
		psy_audio_machines_insert(&self->song->machines, index, machine);
	}
	{
		unsigned char bypass;
		unsigned char mute;
		int32_t panning;
		int32_t x;
		int32_t y;
		
		if (status = psyfile_read(self->fp, &bypass, sizeof(bypass))) {
			return status;
		}
		if (status = psyfile_read(self->fp, &mute, sizeof(mute))) {
			return status;
		}
		if (status = psyfile_read(self->fp, &panning, sizeof(panning))) {
			return status;
		}
		if (status = psyfile_read(self->fp, &x, sizeof(x))) {
			return status;
		}
		if (status = psyfile_read(self->fp, &y, sizeof(y))) {
			return status;
		}
		if (psyfile_skip(self->fp, 2 * sizeof(int32_t)) == -1) {	/* numInputs, numOutputs */
			return PSY_ERRFILE;
		}		
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
						
		/* Incoming connections psy_audio_Machine number */
		if (status = psyfile_read(self->fp, &input, sizeof(input))) {
			return status;
		}
		/* Outgoing connections psy_audio_Machine number */
		if (status = psyfile_read(self->fp, &output, sizeof(output))) {
			return status;
		}
		/* Incoming connections psy_audio_Machine vol */
		if (status = psyfile_read(self->fp, &inconvol, sizeof(inconvol))) {
			return status;
		}
		/* Value to multiply input_convol[] to have a 0.0...1.0 range */
		if (status = psyfile_read(self->fp, &wiremultiplier, sizeof(wiremultiplier))) {
			return status;
		}
		/* Outgoing connections activated */
		if (status = psyfile_read(self->fp, &connection, sizeof(connection))) {
			return status;
		}
		/* Incoming connections activated */
		if (status = psyfile_read(self->fp, &incon, sizeof(incon))) {
			return status;
		}
		legacywire = psy_audio_legacywire_allocinit_all(input, incon, inconvol,
			wiremultiplier, output, connection);
		if (legacywire) {
			psy_audio_machinewires_insert(machinewires, (uintptr_t)i,
				legacywire);			
		}
	}
	psy_audio_legacywires_insert(&self->legacywires, index, machinewires);	
	if (status = psyfile_readstring(self->fp, editname, 32)) {
		return status;
	}
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
	if (!replaced) {
		if (status = psy_audio_machine_loadspecific(machine, self->songfile, index)) {
			return status;
		}
	}
	if (psyfile_currchunkversion(self->fp) >= 1) {
		/* TODO: What to do on possibly wrong wire load? */
		if (status = psy_audio_machine_loadwiremapping(machine, self->songfile, index)) {
			return status;
		}
	}
	if (psyfile_currchunkversion(self->fp) >= 2) {
		psy_audio_psy3loader_loadparammapping(self, machine);
	}
	if (psyfile_currchunkversion(self->fp) >= 3) {
		psy_audio_psy3loader_loadismachinebus(self, machine);
	}
	return PSY_OK;
}

int psy_audio_psy3loader_machineloadchunk_createmachine(
	psy_audio_PSY3Loader* self, int32_t index, char* modulename,
	char* catchername, bool* rv_replaced, psy_audio_Machine** rv_machine)
{	
	int32_t type;
	int status;

	*rv_replaced = FALSE;
	modulename[0] = '\0';
	catchername[0] = '\0';
	*rv_machine = NULL;
	if (status = psyfile_read(self->fp, &type, sizeof(type))) {
		return status;
	}
	if (status = psyfile_readstring(self->fp, modulename, 256)) {
		return status;
	}
	psy_audio_plugincatcher_catchername(modulename, catchername, 0);
	/* todo shellidx; */
	*rv_machine = psy_audio_machinefactory_makemachine(self->song->machinefactory,
		type, catchername, psy_INDEX_INVALID);
	if (!*rv_machine) {
		*rv_machine = psy_audio_machinefactory_makemachine(
			self->song->machinefactory, psy_audio_DUMMY,
			catchername, psy_INDEX_INVALID);
		*rv_replaced = TRUE;
	}
	return PSY_OK;
}

int psy_audio_psy3loader_loadparammapping(psy_audio_PSY3Loader* self, 
	psy_audio_Machine* machine)
{
	uint8_t nummaps;
	uint8_t i;
	int status;
	psy_audio_ParamTranslator* translator;

	if (!psyfile_expect(self->fp, "PMAP", 4)) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &nummaps, sizeof(uint8_t))) {
		return status;
	}
	translator = psy_audio_machine_instparamtranslator(machine);
	for (i = 0; i < nummaps; ++i) {
		uint8_t idx;
		uint16_t value;

		if (status = psyfile_read(self->fp, &idx, sizeof(idx))) {
			return status;
		}
		if (status = psyfile_read(self->fp, &value, sizeof(value))) {
			return status;
		}
		if (translator) {
			psy_audio_paramtranslator_set_virtual_index(translator,
				idx, value);
		}
	}
	return PSY_OK;
}

int psy_audio_psy3loader_loadismachinebus(psy_audio_PSY3Loader* self,
	psy_audio_Machine* machine)
{
	uint8_t isbus;
	int status;

	assert(self);
	assert(machine);

	if (!psyfile_expect(self->fp, "PBUS", 4)) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &isbus, sizeof(uint8_t))) {
		return status;
	}
	if (isbus) {
		psy_audio_machine_setbus(machine);
	}
	return PSY_OK;
}

int psy_audio_psy3loader_read_virg(psy_audio_PSY3Loader* self)
{	
	int32_t virtual_inst;
	int32_t inst_idx;
	int32_t mac_idx;
	int status;
	
	if (status = psyfile_read(self->fp, &virtual_inst, sizeof(virtual_inst))) {
		return status;
	}
	if (status = psyfile_read(self->fp, &mac_idx, sizeof(mac_idx))) {
		return status;
	}
	if (status = psyfile_read(self->fp, &inst_idx, sizeof(virtual_inst))) {
		return status;
	}		
	psy_audio_song_insertvirtualgenerator(self->song, virtual_inst, mac_idx,
		inst_idx);
	return PSY_OK;
}

int psy_audio_psy3loader_loadpsins(psy_audio_PSY3Loader* self, psy_audio_InstrumentIndex index)
{
	char id[5];
	uint32_t size;
	uint32_t version;
	psy_audio_Instrument* instr;
	int status;
	/* RiffFile file; */
	/* std::map<int, int> sampMap; 
	bool preview = loadhelp.IsPreview();
	*/
	int instIdx = -1;

	/* if (!file.Open(filename) || !file.Expect("PSYI", 4)) {
		file.Close();
		return false;
	} */
	/* XMInstrument& instr = loadhelp.GetNextInstrument(instIdx); */

	/* create and insert instrument */
	instr = psy_audio_instrument_allocinit();
	psy_audio_instrument_setindex(instr, index.subslot);
	psy_audio_instruments_insert(&self->song->instruments, instr,
		psy_audio_instrumentindex_make(index.groupslot, index.subslot));

	if (status = psyfile_read(self->fp, &id, 4)) {
		return status;
	}
	id[4] = '\0';
	/* Old format, only in 1.11 betas */
	if (strncmp(id, "EINS", 4) == 0) {
		uint32_t numIns;

		psyfile_read(self->fp, &size, sizeof(size));		
		psyfile_read(self->fp, &numIns, sizeof(numIns));
		/* This loader only supports one instrument. */
		if (numIns == 1)
		{
			uint32_t sizeINST = 0;
			uint32_t numSamps;
			uint32_t i;
			int sample;

			psyfile_read(self->fp, &id, 4);
			id[4] = '\0';
			psyfile_read(self->fp, &sizeINST, sizeof(sizeINST));
			if (strncmp(id, "INST", 4) == 0) {
				uint32_t versionINST;
				psyfile_read(self->fp, &versionINST, sizeof(versionINST));

				/* instr.Load(file, versionINST, true, 0x1); */
			}
			
			psyfile_read(self->fp, &numSamps, sizeof(numSamps));
			sample = -1;
			/* if (preview) { // If preview, determine which sample to  load.
				sample = instr.NoteToSample(notecommands::middleC).second;
				if (sample == notecommands::empty) {
					const std::set<int>& samps = instr.GetWavesUsed();
					if (!samps.empty()) {
						sample = *samps.begin();
					}
				}
			  } */
			for (i = 0; i < numSamps && !psyfile_eof(self->fp); ++i) {
				uint32_t sizeSMPD = 0;

				psyfile_read(self->fp, &id, 4); id[4] = '\0';
				psyfile_read(self->fp, &sizeSMPD, sizeof(sizeSMPD));
				if (strcmp(id, "SMPD") == 0)
				{
					uint32_t versionSMPD;
					int waveidx;

					psyfile_read(self->fp, &versionSMPD, sizeof(versionSMPD));
					waveidx = -1;
					/* XMInstrument::WaveData<>& wave = loadhelp.GetNextSample(waveidx);
					wave.Load(file, versionSMPD, true);
					sampMap[i] = waveidx; */
					if (i == sample) {
						break;
					}
				}
			}
		}
	}
	/* Current format */
	else if (strncmp(id, "SMID", 4) == 0) {
		int sample;
		uint32_t begins;

		sample = -1;
		psyfile_read(self->fp, &version, sizeof(version));
		psyfile_read(self->fp, &size, sizeof(size));
		begins = psyfile_getpos(self->fp);
		if ((version & 0xFFFF0000) == VERSION_MAJOR_ZERO)
		{
			uint32_t instidx; /* Unused */

			psyfile_read(self->fp, &instidx, sizeof(instidx));
			/* instr.Load(file, version & 0xFFFF); */
		}
		
		/* if (preview) { // If preview, determine which sample to  load.
			sample = instr.NoteToSample(notecommands::middleC).second;
			if (sample == notecommands::empty) {
				const std::set<int>& samps = instr.GetWavesUsed();
				if (!samps.empty()) {
					sample = *samps.begin();
				}
			}
		 } */
		psyfile_seek(self->fp, begins + size);
		while (!psyfile_eof(self->fp)) {
			psyfile_read(self->fp, &id, 4);
			id[4] = '\0';
			if (strncmp(id, "SMSB", 4) != 0) {
				break;
			}
			psyfile_read(self->fp, &version, sizeof(version));
			psyfile_read(self->fp, &size, sizeof(size));
			begins = psyfile_getpos(self->fp);
			if ((version & 0xFFFF0000) == VERSION_MAJOR_ZERO)
			{
				uint32_t sampleidx;

				psyfile_read(self->fp, &sampleidx, sizeof(sampleidx));
				/* if (sampleidx < XMInstrument::MAX_INSTRUMENT) {
					int waveidx = -1;
					XMInstrument::WaveData<>& wave = loadhelp.GetNextSample(waveidx);
					wave.Load(file, version & 0xFFFF);
					sampMap[sampleidx] = waveidx;
					if (sampleidx == sample) {
						break;
					}
				} */
			}
			psyfile_seek(self->fp, begins + size);
		}
	} else {
		return PSY_ERRFILE;		
	}
	/*Remap 
	if (!preview) {
		for (int j = 0; j < XMInstrument::NOTE_MAP_SIZE; j++) {
			XMInstrument::NotePair pair = instr.NoteToSample(j);
			if (pair.second != 255) {
				if (sampMap.find(pair.second) != sampMap.end()) {
					pair.second = sampMap[pair.second];
				} else { pair.second = 255; }
				instr.NoteToSample(j, pair);
			}
		}
		std::set<int> list = instr.GetWavesUsed();
		for (std::set<int>::iterator ite = list.begin(); ite != list.end(); ++ite) {
			_pInstrument[*ite]->CopyXMInstrument(instr, tickstomillis(1));
		}
	} */
	
	return PSY_OK;
}
