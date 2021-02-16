// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
// derived from EXS24 For Renoise Matt Allan (MIT Licence)
// https://github.com/matt-allan/renoise-exs24/blob/master/exs24.lua

#include "../../detail/prefix.h"

#include "exs24loader.h"
// local
#include "constants.h"
#include "psyconvert.h"
#include "plugin_interface.h"
#include "song.h"
#include "songio.h"
#include "machinefactory.h"
#include "wire.h"
#include "waveio.h"
// dsp
#include <envelope.h>
#include <datacompression.h>
#include <operations.h>
// file
#include <dir.h>
// platform
#include "../../detail/portable.h"

int32_t twos_complement(value, bits)
{
	// if sign bit is set(128 - 255 for 8 bit)
	if ((value & (1 << (bits - 1))) != 0) {
		return (value - (1 << bits));
	}
	return value;
}

typedef struct EXS24Zone {
	uint32_t id;
	char name[64];
	uint8_t pitch;
	uint8_t oneshot;
	uint8_t reverse;
	uint8_t key;
	int8_t fine_tuning;
	uint8_t pan;
	int8_t volume;
	uint8_t coarse_tuning;
	uint8_t key_low;
	uint8_t key_high;
	uint8_t velocity_range_on;
	uint8_t velocity_low;
	uint8_t velocity_high;
	uint32_t sample_start;
	uint32_t sample_end;
	uint32_t loop_start;
	uint32_t loop_end;
	uint32_t loop_crossfade;
	uint8_t loop_on;
	uint8_t loop_equal_power;
	int8_t output;
	uint32_t group_index;
	uint32_t sample_index;
	uint32_t sample_fade;
	uint32_t offset;
} EXS24Zone;

typedef struct EXS24Sample {
	uint32_t id;
	char name[64];
	uint32_t length;
	uint32_t sample_rate;
	uint8_t bit_depth;
	uint32_t type;	
	char file_path[256];
	char file_name[256];
} EXS24Sample;

// prototypes
static int psy_audio_exs24loader_readmagic(psy_audio_EXS24Loader*);
static int psy_audio_exs24loader_readsizeexpand(psy_audio_EXS24Loader*);
static int psy_audio_exs24loader_readzone(psy_audio_EXS24Loader*,
	uint32_t i, uint32_t size, EXS24Zone* zone);
static int psy_audio_exs24loader_readsample(psy_audio_EXS24Loader*,
	uint32_t i, uint32_t size, EXS24Sample*);

// todo: combine duplicate code (asiodriver.cpp)
#define swaplong(v) ((((v)>>24)&0xFF)|(((v)>>8)&0xFF00)|(((v)&0xFF00)<<8)|(((v)&0xFF)<<24)) 
#define swapshort(v) ((((v)>>8)&0xFF)|(((v)&0xFF)<<8))

// implementation
void psy_audio_exs24loader_init(psy_audio_EXS24Loader* self,
	psy_audio_SongFile* songfile)
{
	assert(self && songfile && songfile->file && songfile->song);
	
	psy_audio_legacywires_init(&self->legacywires);
	self->songfile = songfile;
	self->fp = songfile->file;
	self->song = songfile->song;
	self->big_endian = FALSE;
	self->progress = 0;
	self->baseC = 0x48;
}

void psy_audio_exs24loader_dispose(psy_audio_EXS24Loader* self)
{
	psy_audio_legacywires_dispose(&self->legacywires);
}

psy_audio_EXS24Loader* psy_audio_exs24loader_allocinit(
	psy_audio_SongFile* songfile)
{
	psy_audio_EXS24Loader* rv;

	rv = (psy_audio_EXS24Loader*)malloc(sizeof(psy_audio_EXS24Loader));
	if (rv) {
		psy_audio_exs24loader_init(rv, songfile);
	}
	return rv;
}

void psy_audio_exs24loader_deallocate(psy_audio_EXS24Loader* self)
{
	psy_audio_exs24loader_dispose(self);
	free(self);
}

int psy_audio_exs24loader_load(psy_audio_EXS24Loader* self)
{	
	int status;
	uint32_t datasize;
	uint32_t i;
	uintptr_t sampleidx;
	psy_audio_Instrument* instrument;	

	self->progress = 0;
	self->songfile->legacywires = &self->legacywires;
	status = PSY_OK;	
	
	if (status = psy_audio_exs24loader_readmagic(self)) {
		return status;
	}
	if (status = psy_audio_exs24loader_readsizeexpand(self)) {
		return status;
	}
	datasize = psyfile_filesize(self->fp);
	i = 0;
	sampleidx = 0;
	instrument = psy_audio_instrument_allocinit();
	psy_audio_instruments_insert(&self->song->instruments, instrument,
		psy_audio_instrumentindex_make(0, 0));
	while (i + 84 < datasize) {
		uint32_t sig;		
		int status;
		uint32_t size;
		uint32_t magic;
		uint32_t chunk_type;

		if (psyfile_seek(self->fp, i) == -1) {
			return PSY_ERRFILE;
		}		
		if (status = psyfile_read(self->fp, &sig, 4)) {
			return status;
		}
		if (self->big_endian) {
			sig = swaplong(sig);
		}
		if (psyfile_seek(self->fp, i + 4) == -1) {
			return PSY_ERRFILE;
		}		
		if (status = psyfile_read(self->fp, &size, 4)) {
			return status;
		}
		if (self->big_endian) {
			size = swaplong(size);
		}
		if (psyfile_seek(self->fp, i + 16) == -1) {
			return PSY_ERRFILE;
		}
		if (status = psyfile_read(self->fp, &magic, 4)) {
			return status;
		}
		if (self->big_endian) {
			magic = swaplong(magic);
		}		

		if (self->is_size_expanded && size > 0x8000) {
			size = size - 0x8000;
		}

		chunk_type = (sig & 0x0F000000) >> 24;			
		if (chunk_type == 0x01) {
			EXS24Zone zone;
			psy_audio_InstrumentEntry* entry;
			if (size < 104) {
				return PSY_ERRFILE;
			}
			//table.insert(exs.zones, create_zone(fh, i, size + 84, big_endian))
			psy_audio_exs24loader_readzone(self, i, size + 84, &zone);
			entry = psy_audio_instrumententry_allocinit();
			entry->keyrange.low = zone.key_low;
			entry->keyrange.high = zone.key_high;
			entry->sampleindex = sampleindex_make(0, zone.sample_index);
			if (zone.sample_start != 0) {
				entry->use_loop = TRUE;
				entry->loop.type = psy_audio_SAMPLE_LOOP_DO_NOT;
				entry->loop.start = zone.sample_start;
				entry->loop.end = zone.sample_end;
			}
			entry->zone.tune = zone.key - self->baseC;
			entry->zone.zoneset = psy_audio_ZONESET_TUNE;			
			psy_audio_instrument_addentry(instrument, entry);
		} else if (chunk_type == 0x03) {
			EXS24Sample sample;
			psy_audio_Sample* wave;
			const char* parentdir;

			if ((size != 336) && (size != 592)) {
				return FALSE;
			}
			psy_audio_exs24loader_readsample(self, i, size + 84, &sample);
			// table.insert(exs.samples, create_sample(fh, i, size + 84, big_endian))			
			wave = psy_audio_sample_allocinit(0);
			psy_audio_sample_setname(wave, sample.name);
			psy_audio_samples_insert(&self->song->samples, wave,
				sampleindex_make(0, sampleidx));
			parentdir = strrchr(sample.file_path, '/');
			if ((parentdir) && (parentdir + 1) != NULL) {
				char_dyn_t* samplepath;
				psy_Path path;
				psy_Path parentpath;

				// find sample path
				// 1. assume the parent dir of exs + last dir of sample.file_path
				// 2. assume current dir of exs
				// sample.file_path
				psy_path_init(&path, self->songfile->path);
				psy_path_init(&parentpath, psy_path_prefix(&path));
				samplepath = NULL;
				samplepath = psy_strreset(&samplepath, psy_path_prefix(&parentpath));
				samplepath = psy_strcat_realloc(samplepath, "\\");
				samplepath = psy_strcat_realloc(samplepath, (parentdir + 1));
				samplepath = psy_strcat_realloc(samplepath, "\\");
				samplepath = psy_strcat_realloc(samplepath, sample.file_name);
				if (psy_audio_wave_load(wave, samplepath) != PSY_OK) {
					free(samplepath);
					samplepath = NULL;
					samplepath = psy_strreset(&samplepath, psy_path_prefix(&path));										
					samplepath = psy_strcat_realloc(samplepath, "\\");
					samplepath = psy_strcat_realloc(samplepath, sample.file_name);
					psy_audio_wave_load(wave, samplepath);
				}
				++sampleidx;
				free(samplepath);
				psy_path_dispose(&path);
				psy_path_dispose(&parentpath);
			}
		}
		i = i + size + 84;					
	}
	return PSY_OK;
}

int psy_audio_exs24loader_readmagic(psy_audio_EXS24Loader* self)
{
	char magic[5];
	bool hasmagic;
	int status;

	if (psyfile_seek(self->fp, 16) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &magic, 4)) {
		return status;
	}	
	magic[4] = '\0';
	hasmagic = (strcmp("SOBT", magic) == 0) ||
			   (strcmp("SOBJ", magic) == 0) ||
		       (strcmp("TBOS", magic) == 0);
	if (!hasmagic) {
		return PSY_ERRFILE;
	}
	self->big_endian = (strcmp("SOBT", magic) == 0) ||
		(strcmp("SOBJ", magic) == 0);
	return PSY_OK;
}

int psy_audio_exs24loader_readsizeexpand(psy_audio_EXS24Loader* self)
{
	int status;
	uint32_t headersize;

	self->is_size_expanded = FALSE;
	if (psyfile_seek(self->fp, 4) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &headersize, 4)) {
		return status;
	}
	if (self->big_endian) {
		headersize = swaplong(headersize);
	}
	if (headersize > 0x8000) {
		self->is_size_expanded = TRUE;
	}
	return PSY_OK;
}

int psy_audio_exs24loader_readzone(psy_audio_EXS24Loader* self,
	uint32_t i, uint32_t size, EXS24Zone* zone)
{
	int status;	
	uint8_t zone_opts;
	uint8_t loop_opts;

	if (psyfile_seek(self->fp, i + 8) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &zone->id, 4)) {
		return status;
	}
	if (self->big_endian) {
		zone->id = swaplong(zone->id);
	}
	if (psyfile_seek(self->fp, i + 20) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &zone->name, 4)) {
		return status;
	}
	// rtrim zone.name = rtrim(fh : read(64))
	if (psyfile_seek(self->fp, i + 84) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &zone_opts, 1)) {
		return status;
	}
	zone->pitch = (zone_opts & (1 << 1)) == 0;
	zone->oneshot = (zone_opts & (1 << 0)) != 0;
	zone->reverse = (zone_opts & (1 << 2)) != 0;
	if (psyfile_seek(self->fp, i + 85) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &zone->key, 1)) {
		return status;
	}	
	if (psyfile_seek(self->fp, i + 86) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &zone->fine_tuning, 1)) {
		return status;
	}
	//zone->fine_tuning = twos_complement(zone->fine_tuning, 8);
	// twos_complement(string.byte(fh:read(1)), 8)
	if (psyfile_seek(self->fp, i + 87) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &zone->pan, 1)) {
		return status;
	}
	// twos_complement(string.byte(fh:read(1)), 8)
	if (psyfile_seek(self->fp, i + 88) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &zone->volume, 1)) {
		return status;
	}
	// twos_complement(string.byte(fh:read(1)), 8)
	if (psyfile_seek(self->fp, i + 164) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &zone->coarse_tuning, 1)) {
		return status;
	}
	// twos_complement(string.byte(fh:read(1)), 8)
	if (psyfile_seek(self->fp, i + 90) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &zone->key_low, 1)) {
		return status;
	}
	if (psyfile_seek(self->fp, i + 91) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &zone->key_high, 1)) {
		return status;
	}	
	zone->velocity_range_on = (zone_opts & (1 << 3)) != 0;
	if (psyfile_seek(self->fp, i + 93) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &zone->velocity_low, 1)) {
		return status;
	}	
	if (psyfile_seek(self->fp, i + 94) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &zone->velocity_high, 1)) {
		return status;
	}
	if (psyfile_seek(self->fp, i + 96) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &zone->sample_start, 4)) {
		return status;
	}
	if (self->big_endian) {
		zone->sample_start = swaplong(zone->sample_start);
	}
	if (psyfile_seek(self->fp, i + 100) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &zone->sample_end, 4)) {
		return status;
	}
	if (self->big_endian) {
		zone->sample_end = swaplong(zone->sample_end);
	}
	if (psyfile_seek(self->fp, i + 104) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &zone->loop_start, 4)) {
		return status;
	}
	if (self->big_endian) {
		zone->loop_start = swaplong(zone->loop_start);
	}
	if (psyfile_seek(self->fp, i + 108) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &zone->loop_end, 4)) {
		return status;
	}
	if (self->big_endian) {
		zone->loop_end = swaplong(zone->loop_end);
	}
	if (psyfile_seek(self->fp, i + 112) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &zone->loop_crossfade, 4)) {
		return status;
	}
	if (self->big_endian) {
		zone->loop_crossfade = swaplong(zone->loop_crossfade);
	}
	if (psyfile_seek(self->fp, i + 117) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &loop_opts, 1)) {
		return status;
	}	
	zone->loop_on = (loop_opts & (1 << 0)) != 0;
	zone->loop_equal_power = (loop_opts & (1 << 1)) != 0;
	if ((zone_opts & (1 << 6)) == 0) {
		zone->output = -1;
	} else {
		if (psyfile_seek(self->fp, i + 166) == -1) {
			return PSY_ERRFILE;
		}
		if (status = psyfile_read(self->fp, &zone->output, 1)) {
			return status;
		}
	}
	if (psyfile_seek(self->fp, i + 172) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &zone->group_index, 4)) {
		return status;
	}
	if (self->big_endian) {
		zone->group_index = swaplong(zone->group_index);
	}
	if (psyfile_seek(self->fp, i + 176) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &zone->sample_index, 4)) {
		return status;
	}
	if (self->big_endian) {
		zone->sample_index = swaplong(zone->sample_index);
	}
	zone->sample_fade = 0;
	if (size > 188) {
		if (psyfile_seek(self->fp, i + 188) == -1) {
			return PSY_ERRFILE;
		}
		if (status = psyfile_read(self->fp, &zone->sample_fade, 4)) {
			return status;
		}
		if (self->big_endian) {
			zone->sample_fade = swaplong(zone->sample_fade);
		}
	}
	zone->offset = 0;
	if (size > 192) {
		if (psyfile_seek(self->fp, i + 192) == -1) {
			return PSY_ERRFILE;
		}
		if (status = psyfile_read(self->fp, &zone->offset, 4)) {
			return status;
		}
		if (self->big_endian) {
			zone->offset = swaplong(zone->offset);
		}
	}
	return PSY_OK;
}

int psy_audio_exs24loader_readsample(psy_audio_EXS24Loader* self,
	uint32_t i, uint32_t size, EXS24Sample* sample)
{
	int status;

	if (psyfile_seek(self->fp, i + 8) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &sample->id, 4)) {
		return status;
	}
	if (self->big_endian) {
		sample->id = swaplong(sample->id);
	}
	if (psyfile_seek(self->fp, i + 20) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &sample->name, 64)) {
		return status;
	}
	// rtrim(fh : read(64))			
	if (psyfile_seek(self->fp, i + 88) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &sample->length, 4)) {
		return status;
	}
	if (self->big_endian) {
		sample->length = swaplong(sample->length);
	}
	if (psyfile_seek(self->fp, i + 92) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &sample->sample_rate, 4)) {
		return status;
	}
	if (self->big_endian) {
		sample->sample_rate = swaplong(sample->sample_rate);
	}
	if (psyfile_seek(self->fp, i + 96) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &sample->bit_depth, 1)) {
		return status;
	}
	if (psyfile_seek(self->fp, i + 112) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &sample->type, 4)) {
		return status;
	}
	if (self->big_endian) {
		sample->type = swaplong(sample->type);
	}
	if (psyfile_seek(self->fp, i + 164) == -1) {
		return PSY_ERRFILE;
	}
	if (status = psyfile_read(self->fp, &sample->file_path, 256)) {
		return status;
	}
	// rtrim(fh : read(256))
	if (size > 420) {
		if (psyfile_seek(self->fp, i + 420) == -1) {
			return PSY_ERRFILE;
		}
		if (status = psyfile_read(self->fp, &sample->file_name, 256)) {
			return status;
		}
		//rtrim(fh : read(256))
	} else {
		if (psyfile_seek(self->fp, i + 20) == -1) {
			return PSY_ERRFILE;
		}
		if (status = psyfile_read(self->fp, &sample->file_name, 64)) {
			return status;
		}
		// rtrim(fh : read(64))
	}
	return PSY_OK;
}
			