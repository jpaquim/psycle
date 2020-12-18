// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "psy3saver.h"
// local
#include "constants.h"
#include "plugin_interface.h"
#include "song.h"
#include "songio.h"
#include "machinefactory.h"
#include "wire.h"
// dsp
#include <datacompression.h>
#include <operations.h>
// file
#include <dir.h>
// std
#include <stdlib.h>
#include <string.h>
// platform
#include "../../detail/portable.h"

#if !defined DIVERSALIS__OS__MICROSOFT
#define _MAX_PATH 4096
#endif

static uint32_t psy_audio_psy3saver_chunkcount(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_header(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_songinfo(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_sngi(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_seqd(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_patd(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_epat(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_macd(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_insd(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_smsb(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_virg(psy_audio_PSY3Saver*);
static int psy_audio_psy3saver_write_machine(psy_audio_PSY3Saver*, psy_audio_Machine*,
	uint32_t slot);
static void psy_audio_psy3saver_savedllnameandindex(psy_audio_PSY3Saver*, const char* name,
	int32_t shellindex);
static int psy_audio_psy3saver_write_connections(psy_audio_PSY3Saver*, uintptr_t slot);
static int psy_audio_psy3saver_save_instrument(psy_audio_PSY3Saver*, psy_audio_Instrument*);
static int psy_audio_psy3saver_save_sample(psy_audio_PSY3Saver*, psy_audio_Sample*);
static short* psy_audio_psy3saver_floatbuffertoshort(float* buffer, uintptr_t numframes);

void psy_audio_psy3saver_init(psy_audio_PSY3Saver* self, psy_audio_SongFile* songfile)
{
	assert(self && songfile && songfile->song);

	self->songfile = songfile;
}

int psy_audio_psy3saver_save(psy_audio_PSY3Saver* self)
{
	uint32_t chunkcount;
	int status = PSY_OK;

	chunkcount = psy_audio_psy3saver_chunkcount(self);
	if (status = psy_audio_psy3saver_write_header(self)) {
		return status;
	}
	// the rest of the modules can be arranged in any order
	if (status = psy_audio_psy3saver_write_songinfo(self)) {
		return status;
	}
	if (status = psy_audio_psy3saver_write_sngi(self)) {
		return status;
	}
	if (status = psy_audio_psy3saver_write_seqd(self)) {
		return status;
	} 
	if (status = psy_audio_psy3saver_write_patd(self)) {
		return status;
	}
	//	
	// if (status = psy3_write_epat(self)) {
		// return status;
	//}	
	if (status = psy_audio_psy3saver_write_macd(self)) {
		return status;
	}
	if (status = psy_audio_psy3saver_write_insd(self)) {
		return status;
	}
	if (status = psy_audio_psy3saver_write_smsb(self)) {
		return status;
	}
	if (status = psy_audio_psy3saver_write_virg(self)) {
		return status;
	}
	return status;
}

uint32_t psy_audio_psy3saver_chunkcount(psy_audio_PSY3Saver* self)
{
	// 3 chunks (INFO, SNGI, SEQD. SONG is not counted as a chunk) plus:
	uint32_t rv = 3;

	// PATD
	rv += (uint32_t)psy_audio_patterns_size(&self->songfile->song->patterns);
	// MACD
	rv += (uint32_t)psy_audio_machines_size(&self->songfile->song->machines);
	// INSD
	rv += (uint32_t)psy_audio_instruments_size(&self->songfile->song->instruments, 0);
	// SMSB
	rv += (uint32_t)psy_audio_samples_groupsize(&self->songfile->song->samples);
	return rv;
}

int psy_audio_psy3saver_write_header(psy_audio_PSY3Saver* self)
{	
	// id = "PSY3SONG"; // PSY2 was 0.96 to 1.7.2
	uint32_t pos;
	uint32_t chunkcount;
	int status = PSY_OK;

	if (status = psyfile_write(self->songfile->file, "PSY3",4)) {
		return status;
	}
	if (status = psyfile_writeheader(self->songfile->file, "SONG", CURRENT_FILE_VERSION,
			0, &pos)) {
		return status;
	}
	chunkcount = psy_audio_psy3saver_chunkcount(self);
	if (status = psyfile_write(self->songfile->file, &chunkcount, sizeof(chunkcount))) {
		return status;
	}
	if (status = psyfile_writestring(self->songfile->file, PSYCLE__TITLE)) {
		return status;
	}
	if (status = psyfile_writestring(self->songfile->file, PSYCLE__VERSION)) {
		return status;
	}
	psyfile_updatesize(self->songfile->file, pos);
	return status;
}

int psy_audio_psy3saver_write_songinfo(psy_audio_PSY3Saver* self)
{		
	int status;
	uint32_t sizepos;	
	
	if (status = psyfile_writeheader(self->songfile->file, "INFO",
			CURRENT_FILE_VERSION_INFO, 0, &sizepos)) {
		return status;
	}
	if (status = psyfile_writestring(self->songfile->file,
			self->songfile->song->properties.title)) {
		return status;
	}
	if (status = psyfile_writestring(self->songfile->file,
			self->songfile->song->properties.credits)) {
		return status;
	}
	if (status = psyfile_writestring(self->songfile->file,
			self->songfile->song->properties.comments)) {
		return status;
	}
	psyfile_updatesize(self->songfile->file, sizepos);
	return status;
}

//	===================
//	song info
//	===================
//	id = "SNGI";
int psy_audio_psy3saver_write_sngi(psy_audio_PSY3Saver* self)
{
	int status;
	uint32_t sizepos;	
	uint32_t i;

	assert(self);

	if (status = psyfile_writeheader(self->songfile->file, "SNGI",
		CURRENT_FILE_VERSION_SNGI, 0, &sizepos)) {
			return status;
	}
	if (status = psyfile_write_int32(self->songfile->file, (int32_t)
			self->songfile->song->patterns.songtracks)) {
		return status;
	}
	if (status = psyfile_write_int32(self->songfile->file, (int32_t)
		self->songfile->song->properties.bpm)) {
		return status;
	}
	if (status = psyfile_write_int32(self->songfile->file, (int32_t)
		self->songfile->song->properties.lpb)) {
		return status;
	}
	if (status = psyfile_write_int32(self->songfile->file, (int32_t)
		self->songfile->song->properties.octave)) {
		return status;
	}
	// machinesoloed
	if (psy_audio_machines_soloed(&self->songfile->song->machines) != UINTPTR_MAX) {

	}
	if (status = psyfile_write_int32(self->songfile->file,
		(psy_audio_machines_soloed(&self->songfile->song->machines) != UINTPTR_MAX)
		? (int)psy_audio_machines_soloed(&self->songfile->song->machines)
		: -1)) {
		return status;
	}
	// tracksoloed
	if (status = psyfile_write_int32(self->songfile->file, 0)) {
		return status;
	}
	// seqbus
	if (status = psyfile_write_int32(self->songfile->file, 0)) {
		return status;
	}
	// paramselected
	if (status = psyfile_write_int32(self->songfile->file, 0)) {
		return status;
	}
	// auxcolselected
	if (status = psyfile_write_int32(self->songfile->file, 0)) {
		return status;
	}
	// instselected
	if (status = psyfile_write_int32(self->songfile->file, 0)) {
		return status;
	}
	// sequence width
	if (status = psyfile_write_int32(self->songfile->file, 0)) {
		return status;
	}
	for (i = 0; i < self->songfile->song->patterns.songtracks; ++i) {
		if (status = psyfile_write_uint8(self->songfile->file, 0)) {
			return status;
		}
		 // remember to count them
		if (status = psyfile_write_uint8(self->songfile->file, 0)) {
			return status;
		}
	}
	// shareTrackNames
	if (status = psyfile_write_uint8(self->songfile->file, 0)) {
		return status;
	}
	if (0) {
		uint32_t t;

		for(t = 0; t < self->songfile->song->patterns.songtracks; ++t) {
			//_trackNames[0][t]);
			if (status = psyfile_writestring(self->songfile->file, "")) {
				return status;
			}; 
		}
	}	
	if (status = psyfile_write_int32(self->songfile->file, (int32_t)
		self->songfile->song->properties.tpb)) {
		return status;
	}
	if (status = psyfile_write_int32(self->songfile->file, (int32_t)
		self->songfile->song->properties.extraticksperbeat)) {
		return status;
	}
	psyfile_updatesize(self->songfile->file, sizepos);
	return status;
}			

//	===================
//	sequence data
//	===================
//	id = "SEQD"; 
int psy_audio_psy3saver_write_seqd(psy_audio_PSY3Saver* self)
{		
	int status;
	int32_t index;
	psy_audio_SequenceTrackNode* t;

	assert(self);

	status = PSY_OK;
	for (t = self->songfile->song->sequence.tracks, index = 0; t != NULL;
			psy_list_next(&t), ++index) {
		uint32_t sizepos;
		psy_audio_SequenceTrack* track;
		// This needs to be replaced to store the Multisequence.
		static char* sequencename = "seq0";
		psy_List* s;
		
		if (status = psyfile_writeheader(self->songfile->file, "SEQD",
				CURRENT_FILE_VERSION_SEQD, 0, &sizepos)) {
			return status;
		}
		track = (psy_audio_SequenceTrack*)psy_list_entry(t);
		// sequence track number
		if (status = psyfile_write_int32(self->songfile->file, (int32_t)index)) {
			return status;
		}
		// sequence length
		if (status = psyfile_write_int32(self->songfile->file, (int32_t)
				psy_audio_sequence_size(&self->songfile->song->sequence, t))) {
			return status;
		}
		// sequence name
		if (status = psyfile_writestring(self->songfile->file, sequencename)) {
			return status;
		}		
		for (s = track->entries; s != NULL; psy_list_next(&s)) {
			psy_audio_SequenceEntry* sequenceentry;

			sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(s);
			// sequence data
			if (status = psyfile_write_int32(self->songfile->file, (int32_t)
					psy_audio_sequenceentry_patternslot(sequenceentry))) {
				return status;
			}
		}
		for (s = track->entries; s != NULL; psy_list_next(&s)) {
			psy_audio_SequenceEntry* sequenceentry;

			sequenceentry = (psy_audio_SequenceEntry*)psy_list_entry(s);
			// sequence data
			if (status = psyfile_write_float(self->songfile->file, (float)
					sequenceentry->repositionoffset)) {
				return status;
			}
		}
		psyfile_updatesize(self->songfile->file, sizepos);
	}
	return status;
}

//	===================
//	pattern data
//	===================
//	id = "PATD"; 
int psy_audio_psy3saver_write_patd(psy_audio_PSY3Saver* self)
{	
	int32_t i;
	int32_t temp;
	unsigned char shareTrackNames;
	int status = PSY_OK;

	for (i = 0; i < MAX_PATTERNS; ++i) {
		// check every pattern for validity
		if (psy_audio_sequence_patternused(&self->songfile->song->sequence, i)) {
			// ok save it
			psy_audio_Pattern* pattern;
			int32_t patternLines;
			int32_t lpb;
			unsigned char* source;
			unsigned char* copy;
			int32_t y;
			uint32_t t;
			int32_t index;
			psy_audio_PatternNode* node;
			int32_t size77;
			size_t patsize;
			uint32_t sizepos;
			
			pattern = psy_audio_patterns_at(&self->songfile->song->patterns, i);
			lpb = self->songfile->song->properties.lpb;
			patternLines = (int32_t) (pattern->length * lpb + 0.5);
			patsize = self->songfile->song->patterns.songtracks *
				patternLines * EVENT_SIZE;			

			// clear source
			source = malloc(patsize);			
			copy = source;
			for (y = 0; y < patternLines; ++y) {
				for (t = 0; t < self->songfile->song->patterns.songtracks; ++t) {
					unsigned char* data;					

					data = copy + y * self->songfile->song->patterns.songtracks * EVENT_SIZE +
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
				data = copy + y * self->songfile->song->patterns.songtracks * EVENT_SIZE
						+ t * EVENT_SIZE;
				// Psy3 PatternEntry format
				// type				offset
				// uint8_t note;		0
				// uint8_t inst;		1
				// uint8_t mach;		2
				// uint8_t cmd;			3
				// uint8_t parameter;	4
				data[0] = psy_audio_patternentry_front(entry)->note;
				data[1] = (uint8_t)(psy_audio_patternentry_front(entry)->inst & 0xFF);
				data[2] = psy_audio_patternentry_front(entry)->mach;
				data[3] = psy_audio_patternentry_front(entry)->cmd;
				data[4] = psy_audio_patternentry_front(entry)->parameter;
			}			
			size77 = beerz77comp2(source, 
				&copy, self->songfile->song->patterns.songtracks * patternLines *
				EVENT_SIZE);
			free(source);
			source = 0;			
			if (status = psyfile_writeheader(self->songfile->file, "PATD",
				CURRENT_FILE_VERSION_PATD, 0, &sizepos)) {
				return status;
			}
			index = i; // index
			psyfile_write(self->songfile->file, &index, sizeof(index));
			temp = patternLines;
			psyfile_write(self->songfile->file, &temp, sizeof(temp));
			temp = self->songfile->song->patterns.songtracks; // eventually this may be variable per pattern
			psyfile_write(self->songfile->file, &temp, sizeof(temp));

			psyfile_writestring(self->songfile->file, psy_audio_pattern_name(pattern));

			psyfile_write(self->songfile->file, &size77, sizeof(size77));
			psyfile_write(self->songfile->file, copy, size77);
			free(copy);
			copy = 0;			
			
			shareTrackNames = 0;
			if( !shareTrackNames) {
				uint32_t t;
				for(t = 0; t < self->songfile->song->patterns.songtracks; ++t) {
					psyfile_writestring(self->songfile->file, ""); //_trackNames[i][t]);
				}
			}
			psy_audio_psy3saver_write_epat(self);
			psyfile_updatesize(self->songfile->file, sizepos);
		}
	}
	return status;
}

//	===================
//	extended pattern data
//	===================
int psy_audio_psy3saver_write_epat(psy_audio_PSY3Saver* self)
{	
	psy_TableIterator it;
	int status = PSY_OK;
	int c;

	c = 0;
	// count number of valid patterns
	for (it = psy_table_begin(&self->songfile->song->patterns.slots);
			!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
		// check every pattern for validity
		if (psy_audio_sequence_patternused(&self->songfile->song->sequence,
				psy_tableiterator_key(&it))) {
			++c;
		}
	}
	// write number of valid patterns;
	if (status = psyfile_write_int32(self->songfile->file,
		(uint32_t) c)) {
		return status;
	}	
	for (it = psy_table_begin(&self->songfile->song->patterns.slots);
			!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
		// check every pattern for validity
		if (psy_audio_sequence_patternused(&self->songfile->song->sequence,
				psy_tableiterator_key(&it))) {			
			psy_audio_Pattern* pattern;										
			int32_t index;
			psy_audio_PatternNode* node;						
		
			// ok save it
			pattern = (psy_audio_Pattern*) psy_tableiterator_value(&it);
			index = psy_tableiterator_key(&it);			
			if (status = psyfile_write_int32(self->songfile->file, index)) {
				return status;
			}
			// length
			if (status = psyfile_write_float(self->songfile->file, (float)pattern->length)) {
				return status;
			}
			// num songtracks, eventually this may be variable per pattern
			if (status = psyfile_write_int32(self->songfile->file,
					(uint32_t) self->songfile->song->patterns.songtracks)) {
				return status;
			}
			// pattern label
			psyfile_writestring(self->songfile->file, psy_audio_pattern_name(pattern));
			// num pattern entries
			if (status = psyfile_write_int32(self->songfile->file,
					psy_list_size(pattern->events))) {
				return status;
			}			
			// Write Events
			for (node = pattern->events; node != 0; node = node->next) {
				psy_audio_PatternEntry* entry;
				psy_List* p;
				
				entry = (psy_audio_PatternEntry*) node->entry;
				if (status = psyfile_write_int32(self->songfile->file, entry->track)) {
					return status;
				}
				if (status = psyfile_write_float(self->songfile->file, (float)entry->offset)) {
					return status;
				}				
				// num events
				if (status = psyfile_write_int32(self->songfile->file,
						psy_list_size(entry->events))) {
					return status;
				}				
				entry = (psy_audio_PatternEntry*) node->entry;				
				for (p = entry->events; p != NULL; psy_list_next(&p)) {
					psy_audio_PatternEvent* ev;

					ev = (psy_audio_PatternEvent*)psy_list_entry(p);
					if (status = psyfile_write_int32(self->songfile->file, ev->note)) {
						return status;
					}
					if (status = psyfile_write_int32(self->songfile->file, ev->inst)) {
						return status;
					}
					if (status = psyfile_write_int32(self->songfile->file, ev->mach)) {
						return status;
					}
					if (status = psyfile_write_int32(self->songfile->file, ev->vol)) {
						return status;
					}
					if (status = psyfile_write_int32(self->songfile->file, ev->cmd)) {
						return status;
					}
					if (status = psyfile_write_int32(self->songfile->file, ev->parameter)) {
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

int psy_audio_psy3saver_write_macd(psy_audio_PSY3Saver* self)
{
	int32_t i;
	int status = PSY_OK;

	for (i = 0; i < MAX_MACHINES; ++i) {
		psy_audio_Machine* machine;

		machine = psy_audio_machines_at(&self->songfile->song->machines, i);
		if (machine) {
			int32_t index;
			uint32_t sizepos;

			if (status = psyfile_writeheader(self->songfile->file, "MACD",
					CURRENT_FILE_VERSION_MACD, 0, &sizepos)) {
				return status;
			}
			index = i; // index
			psyfile_write(self->songfile->file, &index, sizeof(index));
			psy_audio_psy3saver_write_machine(self, machine, index);
			psyfile_updatesize(self->songfile->file, sizepos);
		}
	}
	return status;
}

int psy_audio_psy3saver_write_machine(psy_audio_PSY3Saver* self, psy_audio_Machine* machine,
	uint32_t slot)
{
	const psy_audio_MachineInfo* info;
	int status = PSY_OK;
	intptr_t x;
	intptr_t y;

	info = psy_audio_machine_info(machine);
	if (info) {			
		if (status =psyfile_write_int32(self->songfile->file, (int32_t)info->type)) {
			return status;
		}
		psy_audio_psy3saver_savedllnameandindex(self, info->modulepath, info->shellidx);
		if (status = psyfile_write_uint8(self->songfile->file, (uint8_t)
				psy_audio_machine_bypassed(machine))) {
			return status;
		}
		if (status = psyfile_write_uint8(self->songfile->file, (uint8_t)
				psy_audio_machine_muted(machine))) {
			return status;
		}
		if (status = psyfile_write_int32(self->songfile->file, (int32_t)
			(psy_audio_machine_panning(machine) * 128.f))) {
			return status;
		}
		psy_audio_machine_position(machine, &x, &y);
		if (status = psyfile_write_int32(self->songfile->file, (int32_t)x)) {
			return status;
		}
		if (status = psyfile_write_int32(self->songfile->file, (int32_t)y)) {
			return status;
		}
		if (status = psy_audio_psy3saver_write_connections(self, slot)) {
			return status;
		}
		psyfile_writestring(self->songfile->file, psy_audio_machine_editname(machine)
			? psy_audio_machine_editname(machine) : "");
		psy_audio_machine_savespecific(machine, self->songfile, slot);
		psy_audio_machine_savewiremapping(machine, self->songfile, slot);
		// SaveParamMapping(pFile);
	}
	return status;
}

int psy_audio_psy3saver_write_connections(psy_audio_PSY3Saver* self, uintptr_t slot)
{
	psy_audio_MachineSockets* sockets;		
	int status = PSY_OK;
	
	sockets = psy_audio_connections_at(&self->songfile->song->machines.connections, slot);
	if (status = psyfile_write_int32(self->songfile->file,
			(int32_t)((sockets) ? wiresockets_size(&sockets->inputs) : 0))) {
		return status;
	}
	if (status = psyfile_write_int32(self->songfile->file,
		(int32_t)((sockets) ? wiresockets_size(&sockets->outputs) : 0))) {	
		return status;
	}	
	if (sockets) {
		uintptr_t maxkey;
		//uintptr_t maxkey_in;
		//uintptr_t maxkey_out;
		uintptr_t c;
		bool incon;
		bool outcon;
		
		// maxkey_in = psy_table_maxkey(&sockets->inputs.sockets);
		// maxkey_out = psy_table_maxkey(&sockets->outputs.sockets);

		// if (maxkey_in == UINTPTR_MAX) {
		//	maxkey_in = 0;
		// }
		// if (maxkey_out == UINTPTR_MAX) {
		//	maxkey_out = 0;
		//}
		//maxkey = psy_max(MAX_CONNECTIONS - 1, psy_max(maxkey_in, maxkey_out));		
		maxkey = MAX_CONNECTIONS - 1;
		for (c = 0; c <= maxkey; ++c) {
			psy_audio_WireSocket* in;
			psy_audio_WireSocket* out;
			float invol;

			in = psy_audio_wiresockets_at(&sockets->inputs, c);
			out = psy_audio_wiresockets_at(&sockets->outputs, c);
			invol = 1.f;
		
			if (in) {								
				invol = in->volume;
				if (status = psyfile_write_int32(self->songfile->file,
						(int32_t)(in->slot))) {
					return status;
				}
				incon = TRUE;
			} else {				
				if (status = psyfile_write_int32(self->songfile->file, -1)) {
					return status;
				}
				incon = FALSE;
			}
			if (out) {					
				if (status = psyfile_write_int32(self->songfile->file,
						(int32_t)(out->slot))) {
					return status;
				}
				outcon = TRUE;
			} else {				
				if (status = psyfile_write_int32(self->songfile->file, -1)) {
					return status;
				}
				outcon = FALSE;
			}
			// Incoming connections Machine vol
			if (status = psyfile_write_float(self->songfile->file, invol)) {
				return status;
			}
			// Value to multiply _inputConVol[] to have a 0.0...1.0 range
			if (status = psyfile_write_float(self->songfile->file, 1.f)) {
				return status;
			}
			if (status = psyfile_write_uint8(self->songfile->file,
					(uint8_t)outcon)) {
				return status;
			}
			if (status = psyfile_write_uint8(self->songfile->file,
					(uint8_t)incon)) {
				return status;
			}
		}
	}
	return status;
}

void psy_audio_psy3saver_savedllnameandindex(psy_audio_PSY3Saver* self, const char* filename,
	int32_t shellindex)
{
	char str[256];	
	char idxtext[8];
	int32_t index;

	str[0] = '\0';
	if (filename) {
		psy_Path path;
		idxtext[0] = '\0';

		psy_path_init(&path, filename);		
		if (strcmp(psy_path_ext(&path), "so") == 0) {
			psy_path_setext(&path, "dll");			
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
		psy_snprintf(str, 256, "%s.%s%s", psy_path_name(&path),
			psy_path_ext(&path), idxtext);
		psy_path_dispose(&path);
	}
	psyfile_write(self->songfile->file, str, strlen(str) + 1);
}

//	===================
//	instrument data
//	===================
//	id = "INSD";	
int psy_audio_psy3saver_write_insd(psy_audio_PSY3Saver* self)
{
	psy_TableIterator it;
	uint32_t sizepos;	
	int status = PSY_OK;

	;

	for (it = psy_audio_instruments_groupbegin(&self->songfile->song->instruments, 0);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_Instrument* instrument;
				
		if (status = psyfile_writeheader(self->songfile->file, "INSD",
				CURRENT_FILE_VERSION_INSD, 0, &sizepos)) {
			return status;
		}
		if (status = psyfile_write_int32(self->songfile->file, (int32_t)
				psy_tableiterator_key(&it))) {
			return status;
		}
		instrument = (psy_audio_Instrument*) psy_tableiterator_value(&it);
		psy_audio_psy3saver_save_instrument(self, instrument);
		psyfile_updatesize(self->songfile->file, sizepos);
	}
	return status;
}

int psy_audio_psy3saver_save_instrument(psy_audio_PSY3Saver* self,
	psy_audio_Instrument* instrument)
{	
	int status = PSY_OK;	
	psy_dsp_EnvelopePoint pt_start;
	psy_dsp_EnvelopePoint pt_end;
	
	// loop	
	if (status = psyfile_write_uint8(self->songfile->file, instrument->loop != FALSE)) {
		return status;
	}
	// lines	
	if (status = psyfile_write_int32(self->songfile->file,
			(int32_t) instrument->lines)) {
		return status;
	}	
	if (status = psyfile_write_uint8(self->songfile->file, (uint8_t) instrument->nna)) {
		return status;
	}
	// env_at
	pt_start = psy_dsp_envelopesettings_at(&instrument->volumeenvelope, 1);
	if (status = psyfile_write_int32(self->songfile->file, (int32_t)
		(pt_start.time * 44100 + 0.5f))) {
		return status;
	}
	// env_dt	
	pt_start = psy_dsp_envelopesettings_at(&instrument->volumeenvelope, 1);
	pt_end = psy_dsp_envelopesettings_at(&instrument->volumeenvelope, 2);
	if (status = psyfile_write_int32(self->songfile->file, (int32_t)
		((pt_end.time - pt_start.time) * 44100 + 0.5f))) {
		return status;
	}
	// env_sl
	pt_start = psy_dsp_envelopesettings_at(&instrument->volumeenvelope, 2);
	if (status = psyfile_write_int32(self->songfile->file, (int32_t)
			(pt_start.value * 100))) {
		return status;
	}
	// env_rt
	pt_start = psy_dsp_envelopesettings_at(&instrument->volumeenvelope, 2);
	pt_end = psy_dsp_envelopesettings_at(&instrument->volumeenvelope, 3);
	if (status = psyfile_write_int32(self->songfile->file, (int32_t)
		((pt_end.time - pt_start.time) * 44100 + 0.5f))) {
		return status;
	}
	// env_f_at
	pt_start = psy_dsp_envelopesettings_at(&instrument->filterenvelope, 1);	
	if (status = psyfile_write_int32(self->songfile->file, (int32_t)
		(pt_start.time * 44100 + 0.5f))) {
		return status;
	}
	// env_f_dt
	pt_start = psy_dsp_envelopesettings_at(&instrument->filterenvelope, 1);
	pt_end = psy_dsp_envelopesettings_at(&instrument->filterenvelope, 2);
	if (status = psyfile_write_int32(self->songfile->file, (int32_t)
		((pt_end.time - pt_start.time) * 44100 + 0.5f))) {
		return status;
	}
	// env_f_sl
	pt_start = psy_dsp_envelopesettings_at(&instrument->filterenvelope, 2);	
	if (status = psyfile_write_int32(self->songfile->file,
		(int32_t)(pt_start.value * 128))) {
		return status;
	}
	// env_f_rt
	pt_start = psy_dsp_envelopesettings_at(&instrument->filterenvelope, 2);
	pt_end = psy_dsp_envelopesettings_at(&instrument->filterenvelope, 3);
	if (status = psyfile_write_int32(self->songfile->file, (int32_t)
		((pt_end.time - pt_start.time) * 44100 + 0.5f))) {
		return status;
	}
	// env_f_co	
	if (status = psyfile_write_int32(self->songfile->file,
		(int32_t) (instrument->filtercutoff * 127.f))) {
		return status;
	}
	// env_f_rq	
	if (status = psyfile_write_int32(self->songfile->file,
		(int32_t) (instrument->filterres * 127))) {
		return status;
	}
	// env_f_ea	
	if (status = psyfile_write_int32(self->songfile->file, 0)) {
		return status;
	}
	// env_f_tp	
	if (status = psyfile_write_int32(self->songfile->file, (int32_t)
		instrument->filtertype)) {
		return status;
	}
	// No longer saving pan in version 2
	// legacypan	
	if (status = psyfile_write_int32(self->songfile->file, 128)) {
		return status;
	}
	if (status = psyfile_write_uint8(self->songfile->file, (uint8_t)
			instrument->randompan)) {
		return status;
	}
	if (status = psyfile_write_uint8(self->songfile->file, (uint8_t)
			instrument->_RCUT)) {
		return status;
	}
	if (status = psyfile_write_uint8(self->songfile->file, (uint8_t)
			instrument->_RRES)) {
		return status;
	}
	// No longer saving name in version 2	
	if (status = psyfile_write_uint8(self->songfile->file, 0)) {
		return status;
	}
	// No longer saving wave subchunk in version 2
	// legacynumwaves;	
	if (status = psyfile_write_int32(self->songfile->file, 0)) {
		return status;
	}
	// sampler_to_use
	if (status = psyfile_write_int32(self->songfile->file, 0)) {
		return status;
	}
	// lockinst
	if (status = psyfile_write_int32(self->songfile->file, 0)) {
		return status;
	}
	return status;
}

//	===================
//	sampulse instrument data
//	===================
//	id = "SMSB"; 

int psy_audio_psy3saver_write_smsb(psy_audio_PSY3Saver* self)
{
	psy_TableIterator it;
	uint32_t sizepos;
	int status = PSY_OK;

	for (it = psy_audio_samples_begin(&self->songfile->song->samples);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_Sample* sample;
						
		if (status = psyfile_writeheader(self->songfile->file, "SMSB",
				CURRENT_FILE_VERSION_SMSB, 0, &sizepos)) {
			return status;
		}
		if (status = psyfile_write_int32(self->songfile->file,
				psy_tableiterator_key(&it))) {
			return status;
		}
		sample = psy_audio_samples_at(&self->songfile->song->samples,
			sampleindex_make(psy_tableiterator_key(&it), 0));
		if (sample) {
			if (status = psy_audio_psy3saver_save_sample(self, sample)) {
				return status;
			}
		} else {
			return PSY_ERRFILE;
		}
		psyfile_updatesize(self->songfile->file, sizepos);
	}
	return status;
}

int psy_audio_psy3saver_save_sample(psy_audio_PSY3Saver* self, psy_audio_Sample* sample)
{			
	unsigned char* data1;
	unsigned char* data2;
	uint32_t size1;
	uint32_t size2;
	short* wavedata_left = 0;
	short* wavedata_right = 0;
	int status = PSY_OK;
	
	if (psy_audio_buffer_numchannels(&sample->channels) > 0) {
		wavedata_left = psy_audio_psy3saver_floatbuffertoshort(
			psy_audio_buffer_at(&sample->channels, 0), sample->numframes);
		if (!wavedata_left) {
			return PSY_ERRFILE;
		}
	}
	if (psy_audio_buffer_numchannels(&sample->channels) > 1) {
		wavedata_right = psy_audio_psy3saver_floatbuffertoshort(
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
	if (status = psyfile_writestring(self->songfile->file, sample->name)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint32(self->songfile->file, (uint32_t)
			sample->numframes)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_float(self->songfile->file, (float)
			sample->globalvolume)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint16(self->songfile->file, (uint16_t)
			(sample->defaultvolume * 255))) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint32(self->songfile->file, (uint32_t)
			sample->loop.start)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint32(self->songfile->file, (uint32_t)
			sample->loop.end)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_int32(self->songfile->file,
			sample->loop.type)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint32(self->songfile->file, (uint32_t)
			sample->sustainloop.start)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint32(self->songfile->file, (uint32_t)
			sample->sustainloop.end)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_int32(self->songfile->file, sample->sustainloop.type)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint32(self->songfile->file, sample->samplerate)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_int16(self->songfile->file, sample->tune)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_int16(self->songfile->file, sample->finetune)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint8(self->songfile->file, sample->stereo)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint8(self->songfile->file, (uint8_t)
			sample->panenabled)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_float(self->songfile->file, sample->panfactor)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint8(self->songfile->file, sample->surround)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint8(self->songfile->file, sample->vibrato.attack)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint8(self->songfile->file, sample->vibrato.speed)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint8(self->songfile->file, sample->vibrato.depth)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint8(self->songfile->file, (uint8_t)
			sample->vibrato.type)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write_uint32(self->songfile->file, size1)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	if (status = psyfile_write(self->songfile->file, (void*)data1, size1)) {
		free(data1);
		free(wavedata_left);
		return status;
	}
	free(data1);
	free(wavedata_left);	
	if (sample->stereo) {
		if (status = psyfile_write_uint32(self->songfile->file, size2)) {
			free(data2);
			free(wavedata_right);
			return status;
		}
		if (status = psyfile_write(self->songfile->file, (void*)data2, size2)) {
			free(data2);
			free(wavedata_right);
			return status;
		}
		free(data2);
		free(wavedata_right);
	}
	return status;
}

short* psy_audio_psy3saver_floatbuffertoshort(float* buffer, uintptr_t numframes)
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


/*
===================
Virtual Instrument (Generator)
===================
id = "VIRG";
*/
int psy_audio_psy3saver_write_virg(psy_audio_PSY3Saver* self)
{
	int status;
	int32_t i;
	uint32_t sizepos;	

	status = PSY_OK;	
	for (i = MAX_MACHINES; i < MAX_VIRTUALINSTS; i++) {
		psy_audio_Machine* machine;
		int32_t inst_idx;
		int32_t mac_idx;
		inst_idx = -1;
		mac_idx = -1;

		if (machine = psy_audio_machines_at(&self->songfile->song->machines, i)) {
			psy_audio_MachineParam* param;			
			
			param = psy_audio_machine_parameter(machine, 0);
			if (param) {				
				inst_idx = psy_audio_machine_parameter_scaledvalue(machine, param);
			}
			param = psy_audio_machine_parameter(machine, 1);
			if (param) {
				mac_idx = psy_audio_machine_parameter_scaledvalue(machine, param);
			}						
			if (inst_idx != -1 && mac_idx != -1) {
				if (status = psyfile_writeheader(self->songfile->file, "VIRG",
					CURRENT_FILE_VERSION_VIRG, 0, &sizepos)) {
					return status;
				}
				if (status = psyfile_write_int32(self->songfile->file, (int32_t)i)) {
					return status;
				}
				if (status = psyfile_write_int32(self->songfile->file, (int32_t)mac_idx)) {
					return status;
				}
				if (status = psyfile_write_int32(self->songfile->file, (int32_t)inst_idx)) {
					return status;
				}
			}
		}
	}
	return PSY_OK;
}