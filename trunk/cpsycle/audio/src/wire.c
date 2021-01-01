// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "wire.h"
#include "songio.h"

#include "constants.h"

// LegacyWire
void psy_audio_legacywire_copy(psy_audio_LegacyWire* self, psy_audio_LegacyWire* source)
{
	assert(self && source);

	self->_inputMachine = source->_inputMachine;
	self->_inputCon = source->_inputCon;
	self->_inputConVol = source->_inputConVol;
	self->_wireMultiplier = source->_wireMultiplier;
	self->_outputMachine = source->_outputMachine;
	self->_connection = source->_connection;
	psy_audio_pinmapping_dispose(&self->pinmapping);
	psy_audio_pinmapping_init(&self->pinmapping, 0);
	psy_audio_pinmapping_copy(&self->pinmapping, &source->pinmapping);	
}

void psy_audio_legacywire_dispose(psy_audio_LegacyWire* self)
{
	psy_audio_pinmapping_dispose(&self->pinmapping);
}

psy_audio_LegacyWire* psy_audio_legacywire_clone(psy_audio_LegacyWire* source)
{
	psy_audio_LegacyWire* rv;
	
	assert(source);

	rv = psy_audio_legacywire_allocinit();
	if (rv) {
		psy_audio_legacywire_copy(rv, source);
	}
	return rv;
}

// MachineWires
void psy_audio_machinewires_init(psy_audio_MachineWires* self)
{
	//psy_table_init(&self->wires);
	psy_table_init(self);
}

void psy_audio_machinewires_copy(psy_audio_MachineWires* self, psy_audio_MachineWires* other)
{	
	psy_TableIterator it;

	assert(self && other);

	psy_audio_machinewires_clear(self);

	for (it = psy_table_begin(other);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_machinewires_insert(self, psy_tableiterator_key(&it),
			psy_audio_legacywire_clone((psy_audio_LegacyWire*)
				psy_tableiterator_value(&it)));
	}	
}

void psy_audio_machinewires_dispose(psy_audio_MachineWires* self)
{
	//psy_table_disposeall(&self->wires, (psy_fp_disposefunc)
		//psy_audio_legacywire_dispose);
	psy_table_disposeall(self, (psy_fp_disposefunc)
		psy_audio_legacywire_dispose);
}

psy_audio_MachineWires* psy_audio_machinewires_alloc(void)
{
	return (psy_audio_MachineWires*)malloc(sizeof(psy_audio_MachineWires));
}

psy_audio_MachineWires* psy_audio_machinewires_allocinit(void)
{
	psy_audio_MachineWires* rv;

	rv = psy_audio_machinewires_alloc();
	if (rv) {
		psy_audio_machinewires_init(rv);
	}
	return rv;
}

psy_audio_MachineWires* psy_audio_machinewires_clone(psy_audio_MachineWires* source)
{
	psy_audio_MachineWires* rv;

	assert(source);

	rv = psy_audio_machinewires_allocinit();
	if (rv) {
		psy_audio_machinewires_copy(rv, source);
	}
	return rv;
}


void psy_audio_machinewires_deallocate(psy_audio_MachineWires* self)
{
	assert(self);

	psy_audio_machinewires_dispose(self);
	free(self);
}

void psy_audio_machinewires_clear(psy_audio_MachineWires* self)
{
	psy_audio_machinewires_dispose(self);
	psy_audio_machinewires_init(self);
}

void psy_audio_machinewires_insert(psy_audio_MachineWires* self,
	uintptr_t connectionid, psy_audio_LegacyWire* wire)
{
	assert(self && wire);

	if (psy_table_exists(self, connectionid)) {
		psy_audio_LegacyWire* wire;

		wire = (psy_audio_LegacyWire*)psy_table_at(self,
			connectionid);
		psy_audio_legacywire_dispose(wire);
		free(wire);
	}
	psy_table_insert(self, connectionid, wire);
}

psy_audio_LegacyWire* psy_audio_machinewires_at(psy_audio_MachineWires* self,
	uintptr_t connectionid)
{
	assert(self);

	return (psy_audio_LegacyWire*)psy_table_at(self, connectionid);
}

// LegacyWires
void psy_audio_legacywires_init(psy_audio_LegacyWires* self)
{
	assert(self);

	psy_table_init(&self->legacywires);
}

void psy_audio_legacywires_dispose(psy_audio_LegacyWires* self)
{
	psy_table_disposeall(&self->legacywires, (psy_fp_disposefunc)
		psy_audio_machinewires_dispose);	
}

void psy_audio_legacywires_insert(psy_audio_LegacyWires* self, uintptr_t macid,
	psy_audio_MachineWires* machinewires)
{
	assert(self && machinewires);

	if (psy_table_exists(&self->legacywires, macid)) {
		psy_audio_MachineWires* wires;

		wires = (psy_audio_MachineWires*)psy_table_at(&self->legacywires,
			macid);
		psy_audio_machinewires_deallocate(wires);
	}
	psy_table_insert(&self->legacywires, macid, machinewires);
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
			return (int)(intptr_t)psy_tableiterator_key(&it);
		}
	}
	return -1;
}
