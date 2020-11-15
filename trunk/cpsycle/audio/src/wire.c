// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "wire.h"
#include "songio.h"

#include "constants.h"

void psy_audio_legacywires_init(psy_audio_LegacyWires* self)
{
	assert(self);

	psy_table_init(&self->legacywires);
}

void psy_audio_legacywires_dispose(psy_audio_LegacyWires* self)
{
	psy_TableIterator it;

	assert(self);

	for (it = psy_table_begin(&self->legacywires);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_Table* legacywiretable;
		psy_TableIterator it_wires;

		legacywiretable = (psy_Table*)psy_tableiterator_value(&it);
		for (it_wires = psy_table_begin(legacywiretable);
				!psy_tableiterator_equal(&it_wires, psy_table_end());
				psy_tableiterator_inc(&it_wires)) {
			psy_audio_LegacyWire* legacywire;

			legacywire = (psy_audio_LegacyWire*)psy_tableiterator_value(&it_wires);
			psy_audio_legacywire_dispose(legacywire);
			free(legacywire);
		}
		psy_table_dispose(legacywiretable);
	}
	psy_table_dispose(&self->legacywires);
}

psy_Table* psy_audio_legacywires_at(psy_audio_LegacyWires* self,
	uintptr_t machineslot)
{
	assert(self);

	return psy_table_at(&self->legacywires, machineslot);
}

int psy_audio_legacywires_findlegacyoutput(psy_audio_LegacyWires* self,
	int sourcemac, int macindex)
{
	psy_Table* legacywiretable;
	psy_TableIterator it;

	assert(self);

	legacywiretable = psy_audio_legacywires_at(self, sourcemac);
	if (!legacywiretable) {
		return -1;
	}
	for (it = psy_table_begin(legacywiretable);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_LegacyWire* legacywire;
		
		if (psy_tableiterator_key(&it) >= MAX_CONNECTIONS) {
			continue;
		}
		legacywire = psy_tableiterator_value(&it);
		if (legacywire && legacywire->_connection &&
			legacywire->_outputMachine == macindex)
		{			
			return psy_tableiterator_key(&it);
		}
	}
	return -1;
}

void legacywires_load_psy2(psy_audio_SongFile* songfile, uintptr_t slot)
{
	psy_Table* legacywiretable;
	uintptr_t i;

	assert(songfile);

	legacywiretable = (psy_Table*)malloc(sizeof(psy_Table));
	psy_table_init(legacywiretable);
	psy_table_insert(&songfile->legacywires.legacywires, slot, legacywiretable);
	// resize	
	for (i = 0; i < MAX_CONNECTIONS; ++i) {
		psy_table_insert(legacywiretable, (uintptr_t)i,
			(void*)psy_audio_legacywire_allocinit());
	}
	// Incoming connections Machine number
	for (i = 0; i < MAX_CONNECTIONS; ++i) {
		int32_t input;
		psy_audio_LegacyWire* legacywire;

		psyfile_read(songfile->file, &input, sizeof(input));
		legacywire = psy_table_at(legacywiretable, i);
		if (legacywire) {
			legacywire->_inputMachine = input;
		}
	}
	// Outgoing connections Machine number
	for (i = 0; i < MAX_CONNECTIONS; ++i) {
		int32_t output;
		psy_audio_LegacyWire* legacywire;

		psyfile_read(songfile->file, &output, sizeof(output));
		legacywire = psy_table_at(legacywiretable, i);
		if (legacywire) {
			legacywire->_outputMachine = output;
		}
	}
	// Incoming connections Machine vol
	for (i = 0; i < MAX_CONNECTIONS; ++i) {
		float _inputConVol;
		psy_audio_LegacyWire* legacywire;

		psyfile_read(songfile->file, &_inputConVol, sizeof(_inputConVol));
		legacywire = psy_table_at(legacywiretable, i);
		if (legacywire) {
			legacywire->_inputConVol = _inputConVol;
		}
	}
	// Outgoing connections activated
	for (i = 0; i < MAX_CONNECTIONS; ++i) {
		uint8_t connection;
		psy_audio_LegacyWire* legacywire;

		psyfile_read(songfile->file, &connection, sizeof(connection));
		legacywire = psy_table_at(legacywiretable, i);
		if (legacywire) {
			legacywire->_connection = connection;
		}
	}
	// Incoming connections activated
	for (i = 0; i < MAX_CONNECTIONS; ++i) {
		uint8_t _inputCon;
		psy_audio_LegacyWire* legacywire;

		psyfile_read(songfile->file, &_inputCon, sizeof(_inputCon));
		legacywire = psy_table_at(legacywiretable, i);
		if (legacywire) {
			legacywire->_inputCon = _inputCon;
		}
	}
}
