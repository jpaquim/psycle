// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machines.h"
#include "machinescmds.h"
#include "machinefactory.h"
#include "preset.h"
#include "exclusivelock.h"
#include <operations.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../../detail/trace.h"

// prototypes
static void machines_initsignals(psy_audio_Machines*);
static void machines_disposesignals(psy_audio_Machines*);
static void machines_free(psy_audio_Machines*);
static uintptr_t machines_freeslot(psy_audio_Machines*, uintptr_t start);
static void machines_setpath(psy_audio_Machines*, MachineList* path);
static void machines_sortpath(psy_audio_Machines*);
static void compute_slotpath(psy_audio_Machines*, uintptr_t slot, psy_List**);
static void machines_preparebuffers(psy_audio_Machines*, MachineList* path,
	uintptr_t amount);
static void machines_releasebuffers(psy_audio_Machines*);
static psy_audio_Buffer* machines_nextbuffer(psy_audio_Machines*,
	uintptr_t channels);
static void machines_freebuffers(psy_audio_Machines*);
static bool isleaf(psy_audio_Machines*, uintptr_t slot, psy_Table* worked);
static uintptr_t machines_findmaxindex(psy_audio_Machines*);
static void compute_leafs(psy_audio_Machines*, uintptr_t slot,
	psy_List** leafs);
// implementation
void psy_audio_machines_init(psy_audio_Machines* self)
{
	assert(self);

	psy_table_init(&self->slots);	
	psy_audio_connections_init(&self->connections);	
	psy_table_init(&self->inputbuffers);
	psy_table_init(&self->outputbuffers);
	psy_table_init(&self->nopath);
	self->path = 0;
	self->numsamplebuffers = psy_audio_MAX_MACHINE_BUFFERS;
	self->samplebuffers = dsp.memory_alloc(psy_audio_MAX_STREAM_SIZE *
		self->numsamplebuffers, sizeof(psy_dsp_amp_t));
	assert(self->samplebuffers);
	self->currsamplebuffer = 0;
	self->selected = 0;
	self->paramselected = 0;	
	self->soloed = psy_INDEX_INVALID;
	self->buffers = 0;
	self->filemode = 0;
	self->master = 0;
	self->maxindex = 0;
	self->mixercount = 0;
	self->opcount = 0;
	self->preventundoredo = FALSE;
	psy_audio_wire_init(&self->selectedwire);
	psy_undoredo_init(&self->undoredo);
	machines_initsignals(self);
}

void machines_initsignals(psy_audio_Machines* self)
{
	assert(self);

	psy_signal_init(&self->signal_insert);
	psy_signal_init(&self->signal_removed);
	psy_signal_init(&self->signal_slotchange);
	psy_signal_init(&self->signal_paramselected);
	psy_signal_init(&self->signal_wireselected);	
}

void psy_audio_machines_dispose(psy_audio_Machines* self)
{	
	assert(self);

	machines_disposesignals(self);
	machines_free(self);
	psy_table_dispose(&self->inputbuffers);
	psy_table_dispose(&self->outputbuffers);
	machines_freebuffers(self);
	psy_list_free(self->path);
	self->path = 0;
	psy_table_dispose(&self->slots);	
	psy_audio_connections_dispose(&self->connections);
	psy_table_dispose(&self->nopath);
	dsp.memory_dealloc(self->samplebuffers);
	psy_undoredo_dispose(&self->undoredo);
	self->mixercount = 0;
}

void machines_disposesignals(psy_audio_Machines* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_insert);
	psy_signal_dispose(&self->signal_removed);
	psy_signal_dispose(&self->signal_slotchange);
	psy_signal_dispose(&self->signal_paramselected);
	psy_signal_dispose(&self->signal_wireselected);
}

void machines_free(psy_audio_Machines* self)
{
	psy_TableIterator it;
	
	assert(self);

	for (it = psy_audio_machines_begin(self);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {			
		psy_audio_Machine* machine;

		machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
		assert(machine);
		psy_audio_machine_deallocate(machine);		
		psy_table_insert(&self->slots, psy_tableiterator_key(&it), NULL);
	}
}

void psy_audio_machines_clear(psy_audio_Machines* self)
{
	assert(self);

	psy_audio_machines_dispose(self);
	psy_audio_machines_init(self);
}

uintptr_t psy_audio_machines_maxindex(psy_audio_Machines* self)
{
	assert(self);

	return self->maxindex;	
}

void psy_audio_machines_insert(psy_audio_Machines* self, uintptr_t slot,
	psy_audio_Machine* machine)
{	
	assert(self);

	if (machine) {
		if (self->preventundoredo || slot == psy_audio_MASTER_INDEX || self->filemode) {
			psy_table_insert(&self->slots, slot, machine);
			if (slot == psy_audio_MASTER_INDEX) {
				self->master = machine;
			}
			psy_audio_machine_setslot(machine, slot);
			if (slot > self->maxindex) {
				self->maxindex = slot;
			}
			if (psy_audio_machine_type(machine) == psy_audio_MIXER) {
				++self->mixercount;
			}
			psy_signal_emit(&self->signal_insert, self, 1, slot);
			if (!self->filemode) {
				psy_audio_exclusivelock_enter();
				machines_setpath(self, psy_audio_compute_path(self,
					psy_audio_MASTER_INDEX, TRUE));
				psy_audio_exclusivelock_leave();
			}
		} else {
			psy_undoredo_execute(&self->undoredo,
				&insertmachinecommand_allocinit(self, slot, machine)
				->command);
		}
	}
}

uintptr_t psy_audio_machines_append(psy_audio_Machines* self,
	psy_audio_Machine* machine)
{
	uintptr_t slot;

	assert(self);

	slot = machines_freeslot(self,
		(psy_audio_machine_mode(machine) == psy_audio_MACHMODE_FX)
		? 0x40
		: 0);
	psy_audio_machines_insert(self, slot, machine);
	return slot;
}

void psy_audio_machines_insertmaster(psy_audio_Machines* self,
	psy_audio_Machine* master)
{
	assert(self);

	self->master = master;
	if (master) {
		psy_audio_machines_insert(self, psy_audio_MASTER_INDEX, master);
	}
}

void psy_audio_machines_erase(psy_audio_Machines* self, uintptr_t slot)
{	
	psy_audio_Machine* machine;

	assert(self);

	psy_audio_exclusivelock_enter();
	if (slot == psy_audio_MASTER_INDEX) {
		self->master = NULL;
	}
	machine = psy_audio_machines_at(self, slot);
	if (machine && psy_audio_machine_type(machine) == psy_audio_MIXER) {
		--self->mixercount;
	}
	psy_audio_machines_disconnectall(self, slot);	
	psy_table_remove(&self->slots, slot);
	machines_setpath(self, psy_audio_compute_path(self, psy_audio_MASTER_INDEX,
		TRUE));	
	if (slot == self->maxindex) {
		self->selected = machines_findmaxindex(self);
	}	
	psy_signal_emit(&self->signal_removed, self, 1, slot);	
	psy_audio_exclusivelock_leave();	
}

uintptr_t machines_findmaxindex(psy_audio_Machines* self)
{
	uintptr_t rv = 0;
	psy_TableIterator it;

	assert(self);

	for (it = psy_audio_machines_begin(self);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		psy_audio_Machine* machine;

		machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
		if (psy_audio_machine_slot(machine) > rv) {
			rv = psy_audio_machine_slot(machine);
		}		
	}
	return rv;
}

void psy_audio_machines_remove(psy_audio_Machines* self, uintptr_t slot, bool rewire)
{	
	psy_audio_Machine* machine;

	assert(self);
	
	machine = psy_audio_machines_at(self, slot);
	if (machine) {
		if (self->preventundoredo || slot == psy_audio_MASTER_INDEX || self->filemode) {
			if (rewire) {
				psy_audio_connections_rewire(&self->connections,
					psy_audio_connections_at(&self->connections, slot));
			}		
			psy_audio_machines_erase(self, slot);		
			psy_audio_machine_dispose(machine);			
			free(machine);
		} else {
			psy_undoredo_execute(&self->undoredo,
				&deletemachinecommand_allocinit(self, slot)
				->command);
		}
		++self->opcount;
	}
}

void psy_audio_machines_exchange(psy_audio_Machines* self, uintptr_t srcslot,
	uintptr_t dstslot)
{
	psy_audio_Machine* src;
	psy_audio_Machine* dst;

	assert(self);

	src = psy_audio_machines_at(self, srcslot);
	dst = psy_audio_machines_at(self, dstslot);
	if (src && dst) {		
		psy_audio_machines_erase(self, srcslot);
		psy_audio_machines_erase(self, dstslot);
		psy_audio_machines_insert(self, srcslot, dst);
		psy_audio_machines_insert(self, dstslot, src);
	}
}

uintptr_t machines_freeslot(psy_audio_Machines* self, uintptr_t start)
{
	uintptr_t rv;
	
	assert(self);

	for (rv = start; psy_table_at(&self->slots, rv) != 0; ++rv);
	return rv;
}

psy_audio_Machine* psy_audio_machines_at(psy_audio_Machines* self, uintptr_t slot)
{	
	assert(self);

	return psy_table_at(&self->slots, slot);
}

const psy_audio_Machine* psy_audio_machines_at_const(const psy_audio_Machines* self,
	uintptr_t index)
{
	assert(self);

	return (const psy_audio_Machine*)psy_table_at_const(&self->slots, index);
}

bool psy_audio_machines_valid_connection(psy_audio_Machines* self, psy_audio_Wire wire)
{
	psy_audio_Machine* src;
	psy_audio_Machine* dst;

	assert(self);

	src = psy_audio_machines_at(self, wire.src);
	dst = psy_audio_machines_at(self, wire.dst);
	if (src && dst) {
		return (psy_audio_machine_numoutputs(src) > 0 &&
			psy_audio_machine_numinputs(dst) > 0);
	}
	return FALSE;
}

void psy_audio_machines_connect(psy_audio_Machines* self, psy_audio_Wire wire)
{
	assert(self);

	if (psy_audio_wire_valid(&wire)) {
		if (self->preventundoredo || self->filemode) {
			int rv;

			if (!self->filemode) {
				psy_audio_exclusivelock_enter();
			}
			rv = psy_audio_connections_connect(&self->connections, wire);
			if (!self->filemode) {
				machines_setpath(self, psy_audio_compute_path(self,
					psy_audio_MASTER_INDEX, TRUE));
				psy_audio_exclusivelock_leave();
			}
		} else {
			psy_undoredo_execute(&self->undoredo,
				&connectmachinecommand_allocinit(self, wire)
				->command);
		}
	}
}

void psy_audio_machines_disconnect(psy_audio_Machines* self, psy_audio_Wire wire)
{
	assert(self);

	if (self->preventundoredo || self->filemode) {
		psy_audio_exclusivelock_enter();
		psy_audio_connections_disconnect(&self->connections, wire);
		machines_setpath(self, psy_audio_compute_path(self, psy_audio_MASTER_INDEX,
			TRUE));
		psy_audio_exclusivelock_leave();
	} else {		
		psy_undoredo_execute(&self->undoredo,
			&disconnectmachinecommand_allocinit(self, wire)
			->command);
	}
}

void psy_audio_machines_disconnectall(psy_audio_Machines* self, uintptr_t slot)
{
	assert(self);

	psy_audio_exclusivelock_enter();
	psy_audio_connections_disconnectall(&self->connections, slot);
	psy_audio_exclusivelock_leave();
}

int psy_audio_machines_connected(psy_audio_Machines* self, psy_audio_Wire wire)
{	
	assert(self);

	return psy_audio_connections_connected(&self->connections, wire);
}

void psy_audio_machines_updatepath(psy_audio_Machines* self)
{
	assert(self);

	machines_setpath(self, psy_audio_compute_path(self,
		psy_audio_MASTER_INDEX, TRUE));
}

void machines_setpath(psy_audio_Machines* self, MachineList* path)
{	
	assert(self);

	machines_preparebuffers(self, path, psy_audio_MAX_STREAM_SIZE);
	if (self->path) {
		psy_list_free(self->path);
	}
	self->path = path;
	// machines_sortpath(self);
}

// this orders the machines that can be processed parallel
// and adds NOMACHINE_INDEX as delimiter slot
// 1. Find leafs and add to sorted
// 2. Mark this machines
// 3. Repeat step 1 without checking input connections from
//    marked machines
// 4. Set sorted path
void machines_sortpath(psy_audio_Machines* self)
{	
	assert(self);

	if (self->path) {
		psy_Table worked;
		psy_List* sorted = 0;
		psy_List* p;
		psy_List* q;

		psy_table_init(&worked);
		p = self->path;		
		while (self->path) {
			p = self->path;			
			while (p != NULL) {
				uintptr_t slot;
				slot = (uintptr_t)psy_list_entry(p);
				if (isleaf(self, slot, &worked)) {
					psy_list_append(&sorted, (void*)(uintptr_t)slot);
					p = psy_list_remove(&self->path, p);
				} else {
					psy_list_next(&p);
				}
			}						
			for (q = sorted; q != 0;  q = q->next) {
				uintptr_t slot;

				slot = (uintptr_t)q->entry;
				psy_table_insert(&worked, slot, 0);				
			}
			psy_list_append(&sorted, (void*)(uintptr_t)psy_INDEX_INVALID);
		}
		psy_table_dispose(&worked);
		psy_list_free(self->path);
		self->path = sorted;		
	}	
}

bool isleaf(psy_audio_Machines* self, uintptr_t slot, psy_Table* worked)
{
	psy_audio_MachineSockets* sockets;

	assert(self);

	sockets = psy_audio_connections_at(&self->connections, slot);
	if (!sockets || (wiresockets_size(&sockets->inputs) == 0)) {
		return TRUE;
	} else {	
		bool rv = TRUE;
		psy_TableIterator it;

		for (it = psy_audio_wiresockets_begin(&sockets->inputs);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {

			psy_audio_WireSocket* source;

			source = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
			if (!psy_table_exists(worked, source->slot)) {
				rv = FALSE;
				break;
			}
		}
		return rv;
	}
	return TRUE;
}

void reset_nopath(psy_audio_Machines* self)
{		
	psy_TableIterator it;

	assert(self);

	psy_table_dispose(&self->nopath);
	psy_table_init(&self->nopath);		
	for (it = psy_table_begin(&self->slots); it.curr != 0;
			psy_tableiterator_inc(&it)) {
		psy_table_insert(&self->nopath, it.curr->key, 0);		
	}
}

void remove_nopath(psy_audio_Machines* self, uintptr_t slot)
{
	assert(self);

	psy_table_remove(&self->nopath, slot);
}

MachineList* nopath(psy_audio_Machines* self)
{
	psy_List* rv = 0;

	assert(self);

	psy_TableIterator it;
	for (it = psy_table_begin(&self->nopath); it.curr != 0;
			psy_tableiterator_inc(&it)) {			
		psy_list_append(&rv, (void*)it.curr->key);
	}
	return rv;
}

MachineList* psy_audio_compute_path(psy_audio_Machines* self, uintptr_t slot, bool concat)
{
	MachineList* rv = 0;	

	assert(self);

	reset_nopath(self);	
	self->currlevel = 0;
	self->maxlevel = 0;
	psy_table_init(&self->colours);
	psy_table_init(&self->levels);
	compute_slotpath(self, slot, &rv);
	psy_table_dispose(&self->colours);
	psy_table_dispose(&self->levels);
	//	Debug Path Output
	// if (rv) {
	// 	psy_List* p;

	// 	for (p = rv; p != NULL; psy_list_next(&p)) {
	// 		TRACE_INT((int)(p->entry));
	// 	}
	// 	OutputDebugString("\n");
	// }
	if (concat) {
		psy_list_cat(&rv, nopath(self));
	}
	return rv;
}

void compute_slotpath(psy_audio_Machines* self, uintptr_t slot,
	psy_List** path)
{	
	psy_audio_MachineSockets* sockets;

	assert(self);

	sockets = psy_audio_connections_at(&self->connections, slot);	
	if (sockets) {
		psy_TableIterator it;

		for (it = psy_audio_wiresockets_begin(&sockets->inputs);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_audio_WireSocket* socket;

			socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
			if (!psy_table_exists(&self->colours, socket->slot)) {
				psy_table_insert(&self->colours, socket->slot, (void*)self->currlevel);
				psy_table_insert(&self->levels, socket->slot, (void*)self->currlevel);
				++self->currlevel;
				if (self->maxlevel < self->currlevel) {
					self->maxlevel = self->currlevel;
				}
				compute_slotpath(self, socket->slot, path);
				--self->currlevel;
			} else {
				// cycle detected				
				// skip
			}			
			psy_table_remove(&self->colours, socket->slot);
		}	
	}
	if (psy_table_exists(&self->nopath, slot)) {
		remove_nopath(self, slot);
		psy_list_append(path, (void*)slot);
	}
}

void machines_preparebuffers(psy_audio_Machines* self, MachineList* path,
	uintptr_t amount)
{
	psy_TableIterator it;

	assert(self);
	
	for (it = psy_audio_machines_begin(self);
			!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {			
		psy_audio_Machine* machine;

		machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
		if (machine) {
			psy_table_insert(&self->outputbuffers, psy_tableiterator_key(&it),
				psy_list_append(&self->buffers, machines_nextbuffer(
					self, psy_audio_machine_numoutputs(machine)))->entry);
		}
	}	
}

void machines_releasebuffers(psy_audio_Machines* self)
{
	assert(self);

	psy_table_dispose(&self->inputbuffers);
	psy_table_dispose(&self->outputbuffers);
	psy_table_init(&self->inputbuffers);
	psy_table_init(&self->outputbuffers);
	machines_freebuffers(self);
	self->currsamplebuffer = 0;	
}

psy_audio_Buffer* machines_nextbuffer(psy_audio_Machines* self,
	uintptr_t channels)
{
	uintptr_t channel;
	psy_audio_Buffer* rv;

	assert(self);

	rv = psy_audio_buffer_allocinit(channels);
	for (channel = 0; channel < channels; ++channel) {
		float* samples;
				
		samples = self->samplebuffers
			? self->samplebuffers + (self->currsamplebuffer *
				psy_audio_MAX_STREAM_SIZE)
			: 0;
		dsp.clear(samples, psy_audio_MAX_STREAM_SIZE);
		++self->currsamplebuffer;
		if (self->currsamplebuffer >= self->numsamplebuffers) {
			self->currsamplebuffer = 0;
		}
		rv->samples[channel] = samples;		
	}
	return rv;
}

void machines_freebuffers(psy_audio_Machines* self)
{
	psy_List* p;
	psy_List* q;

	assert(self);

	for (p = q = self->buffers; p != NULL; psy_list_next(&p)) {
		psy_audio_Buffer* buffer;

		buffer = (psy_audio_Buffer*)psy_list_entry(p);
		psy_audio_buffer_dispose(buffer);
		free(buffer);	
	}
	psy_list_free(q);
	self->buffers = 0;
}

psy_audio_Buffer* psy_audio_machines_inputs(psy_audio_Machines* self,
	uintptr_t slot)
{
	assert(self);

	return psy_table_at(&self->inputbuffers, slot);
}

psy_audio_Buffer* psy_audio_machines_outputs(psy_audio_Machines* self,
	uintptr_t slot)
{
	assert(self);

	return psy_table_at(&self->outputbuffers, slot);
}

void psy_audio_machines_select(psy_audio_Machines* self, uintptr_t slot)
{
	assert(self);

	self->selected = slot;
	psy_signal_emit(&self->signal_slotchange, self, 1, slot);
}

void psy_audio_machines_selectparam(psy_audio_Machines* self, uintptr_t index)
{
	assert(self);

	if (psy_audio_machines_selectedmachine(self)) {
		psy_audio_machine_selectparam(psy_audio_machines_selectedmachine(self),
			index);
	}
	self->paramselected = index;
	psy_signal_emit(&self->signal_paramselected, self, 1, index);
}

uintptr_t psy_audio_machines_selected(const psy_audio_Machines* self)
{
	assert(self);

	return self->selected;
}

psy_audio_Machine* psy_audio_machines_selectedmachine(psy_audio_Machines* self)
{
	assert(self);

	return (psy_audio_Machine*)psy_audio_machines_at(self,
		psy_audio_machines_selected(self));
}

const psy_audio_Machine* psy_audio_machines_selectedmachine_const(const
	psy_audio_Machines* self)
{
	assert(self);

	return (psy_audio_Machine*)psy_audio_machines_at_const(self,
		psy_audio_machines_selected(self));
}

uintptr_t psy_audio_machines_paramselected(const psy_audio_Machines* self)
{
	assert(self);

	return self->paramselected;
}

psy_audio_Wire psy_audio_machines_selectedwire(const psy_audio_Machines* self)
{
	return self->selectedwire;
}

uintptr_t psy_audio_machines_soloed(psy_audio_Machines* self)
{
	assert(self);

	return self->soloed;
}

void psy_audio_machines_solo(psy_audio_Machines* self, uintptr_t slot)
{
	assert(self);

	if (self->filemode != 0) {
		self->soloed = slot;
	} else
	if (self->soloed == slot) {
		psy_TableIterator it;

		self->soloed = psy_INDEX_INVALID;
		for (it = psy_audio_machines_begin(self);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_audio_Machine* machine;

			machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
			if ((psy_audio_machine_mode(machine) == psy_audio_MACHMODE_GENERATOR)) {
				psy_audio_machine_unmute(machine);
			}
		}
	} else {
		psy_TableIterator it;

		self->soloed = slot;
		for (it = psy_audio_machines_begin(self); !psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			psy_audio_Machine* machine;

			machine = (psy_audio_Machine*)psy_tableiterator_value(&it);											
			if ((psy_audio_machine_mode(machine) == psy_audio_MACHMODE_GENERATOR) &&
				psy_tableiterator_key(&it) != slot) {
				psy_audio_machine_mute(machine);
			} else
			if (psy_tableiterator_key(&it) == slot) {
				psy_audio_machine_unmute(machine);
			}
		}
	}
}

psy_audio_Machine* psy_audio_machines_master(psy_audio_Machines* self)
{
	assert(self);

	return self->master;
}

void psy_audio_machines_startfilemode(psy_audio_Machines* self)
{
	assert(self);

	self->filemode = 1;
	self->connections.filemode = 1;
}

void psy_audio_machines_endfilemode(psy_audio_Machines* self)
{
	assert(self);

	machines_setpath(self, psy_audio_compute_path(self,
		psy_audio_MASTER_INDEX, TRUE));
	self->filemode = 0;	
	self->connections.filemode = 0;
}

uintptr_t psy_audio_machines_size(psy_audio_Machines* self)
{
	assert(self);

	return psy_table_size(&self->slots);
}

psy_TableIterator psy_audio_machines_begin(psy_audio_Machines* self)
{
	assert(self);

	return psy_table_begin(&self->slots);
}

bool psy_audio_machines_isvirtualgenerator(const psy_audio_Machines* self,
	uintptr_t slot)
{
	assert(self);

	if (psy_table_exists(&self->slots, slot)) {
		return (slot > 0x80 && slot < 0xFF);
	}
	return FALSE;
}

bool psy_audio_machines_ismixersend(const psy_audio_Machines* self,
	uintptr_t slot)
{
	assert(self);

	return psy_table_exists(&self->connections.sends, slot);
}

void psy_audio_machines_addmixersend(psy_audio_Machines* self, uintptr_t slot)
{
	assert(self);

	psy_table_insert(&self->connections.sends, slot, (void*)(uintptr_t)1);
}

void psy_audio_machines_removemixersend(psy_audio_Machines* self,
	uintptr_t slot)
{
	assert(self);

	psy_table_remove(&self->connections.sends, slot);
}

void  psy_audio_machines_connectasmixersend(psy_audio_Machines* self)
{
	assert(self);

	self->mixersendconnect = TRUE;
}

void  psy_audio_machines_connectasmixerinput(psy_audio_Machines* self)
{
	assert(self);

	self->mixersendconnect = FALSE;
}

bool  psy_audio_machines_isconnectasmixersend(const psy_audio_Machines* self)
{
	assert(self);

	return self->mixersendconnect;
}

void psy_audio_machines_selectwire(psy_audio_Machines* self, psy_audio_Wire wire)
{
	assert(self);

	self->selectedwire = wire;
	psy_signal_emit(&self->signal_wireselected, self, 1, &wire);
}

MachineList* psy_audio_machines_level(psy_audio_Machines* self, uintptr_t slot,
	uintptr_t level)
{
	MachineList* rv;
	MachineList* path;	
	psy_TableIterator it;

	assert(self);

	rv = NULL;
	reset_nopath(self);
	self->currlevel = 0;
	self->maxlevel = 0;
	psy_table_init(&self->colours);
	psy_table_init(&self->levels);
	path = NULL;
	compute_slotpath(self, slot, &path);	
	for (it = psy_table_begin(&self->levels);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {		
		if ((uintptr_t)psy_tableiterator_value(&it) == level) {
			psy_list_append(&rv, (void*)psy_tableiterator_key(&it));
		}
	}
	psy_table_dispose(&self->colours);
	psy_table_dispose(&self->levels);
	psy_list_free(path);
	return rv;
}

void psy_audio_machines_levels(psy_audio_Machines* self, psy_Table* rv)
{	
	MachineList* path;
	psy_TableIterator it;

	assert(self);
	
	reset_nopath(self);
	self->currlevel = 0;
	self->maxlevel = 0;
	psy_table_init(&self->colours);
	psy_table_init(&self->levels);
	path = NULL;
	compute_slotpath(self, psy_audio_MASTER_INDEX, &path);
	psy_table_clear(rv);
	for (it = psy_table_begin(&self->levels);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_table_insert(rv, psy_tableiterator_key(&it), psy_tableiterator_value(&it));
	}
	psy_table_dispose(&self->colours);
	psy_table_dispose(&self->levels);
	psy_list_free(path);	
}


MachineList* psy_audio_machines_leafs(psy_audio_Machines* self)
{
	MachineList* rv;

	rv = NULL;
	reset_nopath(self);
	self->currlevel = 0;
	self->maxlevel = 0;
	psy_table_init(&self->colours);
	compute_leafs(self, psy_audio_MASTER_INDEX, &rv);
	psy_table_dispose(&self->colours);
	return rv;
}

void compute_leafs(psy_audio_Machines* self, uintptr_t slot,
	psy_List** leafs)
{
	psy_audio_MachineSockets* sockets;
	bool inconn;

	assert(self);

	inconn = FALSE;
	sockets = psy_audio_connections_at(&self->connections, slot);
	if (sockets) {
		psy_TableIterator it;

		for (it = psy_audio_wiresockets_begin(&sockets->inputs);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			psy_audio_WireSocket* socket;

			socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
			if (!psy_table_exists(&self->colours, socket->slot)) {
				psy_table_insert(&self->colours, socket->slot, (void*)self->currlevel);				
				++self->currlevel;
				if (self->maxlevel < self->currlevel) {
					self->maxlevel = self->currlevel;
				}
				inconn = TRUE;
				compute_leafs(self, socket->slot, leafs);
				--self->currlevel;
			} else {
				// cycle detected				
				// skip
			}
			psy_table_remove(&self->colours, socket->slot);
		}
	}	
	if (psy_table_exists(&self->nopath, slot)) {
		remove_nopath(self, slot);
		if (!inconn) {
			psy_list_append(leafs, (void*)(uintptr_t)slot);
		}
	}
}



uintptr_t psy_audio_machines_levelofmachine(psy_audio_Machines* self, uintptr_t slot)
{
	uintptr_t rv;
	MachineList* path;
	
	assert(self);

	rv = psy_INDEX_INVALID;
	reset_nopath(self);
	self->currlevel = 0;
	self->maxlevel = 0;
	psy_table_init(&self->colours);
	psy_table_init(&self->levels);
	path = NULL;	
	compute_slotpath(self, psy_audio_MASTER_INDEX, &path);	
	if (psy_table_exists(&self->levels, slot)) {
		rv = (uintptr_t)psy_table_at(&self->levels, slot);
	}	
	psy_table_dispose(&self->colours);
	psy_table_dispose(&self->levels);
	psy_list_free(path);
	return rv;
}

uintptr_t psy_audio_machines_depth(psy_audio_Machines* self)
{
	assert(self);

	return self->maxlevel;
}
