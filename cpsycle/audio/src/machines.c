// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machines.h"
#include "exclusivelock.h"
#include <operations.h>
#include <alignedalloc.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../../detail/trace.h"

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
static int isleaf(psy_audio_Machines*, uintptr_t slot, psy_Table* worked);

void machines_init(psy_audio_Machines* self)
{
	psy_table_init(&self->slots);	
	connections_init(&self->connections);	
	psy_table_init(&self->inputbuffers);
	psy_table_init(&self->outputbuffers);
	psy_table_init(&self->nopath);
	self->path = 0;
	self->numsamplebuffers = 100;
	self->samplebuffers = dsp.memory_alloc(MAX_STREAM_SIZE *
		self->numsamplebuffers, sizeof(float));
	assert(self->samplebuffers);
	self->currsamplebuffer = 0;
	self->slot = 0;
	self->tweakparam = 0;
	self->soloed = NOMACHINE_INDEX;
	self->buffers = 0;
	self->filemode = 0;
	self->master = 0;
	machines_initsignals(self);
}

void machines_initsignals(psy_audio_Machines* self)
{
	psy_signal_init(&self->signal_insert);
	psy_signal_init(&self->signal_removed);
	psy_signal_init(&self->signal_slotchange);	
}

void machines_dispose(psy_audio_Machines* self)
{	
	machines_disposesignals(self);
	machines_free(self);
	psy_table_dispose(&self->inputbuffers);
	psy_table_dispose(&self->outputbuffers);
	machines_freebuffers(self);
	psy_list_free(self->path);
	self->path = 0;
	psy_table_dispose(&self->slots);	
	connections_dispose(&self->connections);
	psy_table_dispose(&self->nopath);
	dsp.memory_dealloc(self->samplebuffers);
}

void machines_disposesignals(psy_audio_Machines* self)
{
	psy_signal_dispose(&self->signal_insert);
	psy_signal_dispose(&self->signal_removed);
	psy_signal_dispose(&self->signal_slotchange);	
}

void machines_free(psy_audio_Machines* self)
{
	psy_TableIterator it;
	
	for (it = machines_begin(self); !psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {			
		psy_audio_Machine* machine;

		machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
		psy_audio_machine_dispose(machine);
		free(machine);
		psy_table_insert(&self->slots, psy_tableiterator_key(&it), 0);
	}
}

void machines_clear(psy_audio_Machines* self)
{
	machines_dispose(self);
	machines_init(self);
}

void machines_insert(psy_audio_Machines* self, uintptr_t slot,
	psy_audio_Machine* machine)
{	
	if (machine) {
		psy_table_insert(&self->slots, slot, machine);
		if (slot == MASTER_INDEX) {
			self->master = machine;
		}
		machine->vtable->setslot(machine, slot);
		psy_signal_emit(&self->signal_insert, self, 1, slot);
		if (!self->filemode) {		
			psy_audio_exclusivelock_enter();
			machines_setpath(self, compute_path(self, MASTER_INDEX, TRUE));
			psy_audio_exclusivelock_leave();
		}
	}
}

void machines_insertmaster(psy_audio_Machines* self, psy_audio_Machine* master)
{
	self->master = master;
	if (master) {
		machines_insert(self, MASTER_INDEX, master);
	}
}

void machines_erase(psy_audio_Machines* self, uintptr_t slot)
{	
	psy_audio_exclusivelock_enter();
	if (slot == MASTER_INDEX) {
		self->master = 0;
	}
	machines_disconnectall(self, slot);	
	psy_table_remove(&self->slots, slot);
	machines_setpath(self, compute_path(self, MASTER_INDEX, TRUE));
	psy_signal_emit(&self->signal_removed, self, 1, slot);
	psy_audio_exclusivelock_leave();	
}

void machines_remove(psy_audio_Machines* self, uintptr_t slot)
{	
	psy_audio_Machine* machine;
	
	machine = machines_at(self, slot);
	if (machine) {
		machines_erase(self, slot);		
		psy_audio_machine_dispose(machine);
		free(machine);		
	}
}

void machines_exchange(psy_audio_Machines* self, uintptr_t srcslot,
	uintptr_t dstslot)
{
	psy_audio_Machine* src;
	psy_audio_Machine* dst;

	src = machines_at(self, srcslot);
	dst = machines_at(self, dstslot);
	if (src && dst) {		
		machines_erase(self, srcslot);
		machines_erase(self, dstslot);
		machines_insert(self, srcslot, dst);
		machines_insert(self, dstslot, src);
	}
}

uintptr_t machines_append(psy_audio_Machines* self, psy_audio_Machine* machine)
{	
	uintptr_t slot;
		
	slot = machines_freeslot(self,
		(psy_audio_machine_mode(machine) == MACHMODE_FX) ? 0x40 : 0);
	psy_table_insert(&self->slots, slot, machine);
	machine->vtable->setslot(machine, slot);
	psy_signal_emit(&self->signal_insert, self, 1, slot);
	if (!self->filemode) {		
		psy_audio_exclusivelock_enter();
		machines_setpath(self, compute_path(self, MASTER_INDEX, TRUE));
		psy_audio_exclusivelock_leave();
	}
	return slot;
}

uintptr_t machines_freeslot(psy_audio_Machines* self, uintptr_t start)
{
	uintptr_t rv;
	
	for (rv = start; psy_table_at(&self->slots, rv) != 0; ++rv);
	return rv;
}

psy_audio_Machine* machines_at(psy_audio_Machines* self, uintptr_t slot)
{		
	return psy_table_at(&self->slots, slot);
}

int machines_connect(psy_audio_Machines* self, uintptr_t outputslot,
	uintptr_t inputslot)
{
	int rv;	

	if (!self->filemode) {
		psy_audio_exclusivelock_enter();			
	}
	/*if (!self->filemode) {
		psy_audio_Machine* machine;

		machine = machines_at(self, inputslot);
		if ((machine && machine->vtable->numinputs(machine) == 0)
			|| !machine) {
			return 0;
		}
	}*/
	rv = connections_connect(&self->connections, outputslot, inputslot);
	if (!self->filemode) {
		machines_setpath(self, compute_path(self, MASTER_INDEX, TRUE));			
		psy_audio_exclusivelock_leave();
	}
	return rv;
}

void machines_disconnect(psy_audio_Machines* self, uintptr_t outputslot,
	uintptr_t inputslot)
{
	psy_audio_exclusivelock_enter();	
	connections_disconnect(&self->connections, outputslot, inputslot);
	machines_setpath(self, compute_path(self, MASTER_INDEX, TRUE));		
	psy_audio_exclusivelock_leave();
}

void machines_disconnectall(psy_audio_Machines* self, uintptr_t slot)
{
	psy_audio_exclusivelock_enter();
	connections_disconnectall(&self->connections, slot);
	psy_audio_exclusivelock_leave();
}

int machines_connected(psy_audio_Machines* self, uintptr_t outputslot,
	uintptr_t inputslot)
{	
	return connections_connected(&self->connections, outputslot, inputslot);
}

void machines_setpath(psy_audio_Machines* self, MachineList* path)
{	
	machines_preparebuffers(self, path, MAX_STREAM_SIZE);
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
	if (self->path) {
		psy_Table worked;
		psy_List* sorted = 0;
		psy_List* p;
		psy_List* q;

		psy_table_init(&worked);
		p = self->path;		
		while (self->path) {
			p = self->path;			
			while (p != 0) {
				uintptr_t slot;
				slot = (uintptr_t)p->entry;
				if (isleaf(self, slot, &worked)) {
					psy_list_append(&sorted, (void*)(uintptr_t)slot);
					p = psy_list_remove(&self->path, p);
				} else {
					p = p->next;
				}
			}						
			for (q = sorted; q != 0;  q = q->next) {
				uintptr_t slot;

				slot = (uintptr_t)q->entry;
				psy_table_insert(&worked, slot, 0);				
			}
			psy_list_append(&sorted, (void*)(uintptr_t)NOMACHINE_INDEX);
		}
		psy_table_dispose(&worked);
		psy_list_free(self->path);
		self->path = sorted;		
	}	
}

int isleaf(psy_audio_Machines* self, uintptr_t slot, psy_Table* worked)
{
	psy_audio_MachineSockets* sockets;

	sockets = connections_at(&self->connections, slot);
	if (!sockets || !sockets->inputs) {
		return 1;
	}
	if (sockets->inputs) {
		int rv = 1;

		psy_List* p;
		for (p = sockets->inputs; p != 0; p = p->next) {
			psy_audio_WireSocketEntry* source;

			source = (psy_audio_WireSocketEntry*) p->entry;
			if (!psy_table_exists(worked, source->slot)) {
				rv = 0;
				break;
			}
		}
		return rv;
	}
	return 1;
}

MachineList* machines_path(psy_audio_Machines* self)
{
	return self->path;
}

void reset_nopath(psy_audio_Machines* self)
{		
	psy_TableIterator it;

	psy_table_dispose(&self->nopath);
	psy_table_init(&self->nopath);
		
	for (it = psy_table_begin(&self->slots); it.curr != 0;
			psy_tableiterator_inc(&it)) {
		psy_table_insert(&self->nopath, it.curr->key, 0);		
	}
}

void remove_nopath(psy_audio_Machines* self, uintptr_t slot)
{
	psy_table_remove(&self->nopath, slot);
}

MachineList* nopath(psy_audio_Machines* self)
{
	psy_List* rv = 0;	

	psy_TableIterator it;
	for (it = psy_table_begin(&self->nopath); it.curr != 0;
			psy_tableiterator_inc(&it)) {			
		psy_list_append(&rv, (void*)it.curr->key);
	}
	return rv;
}

MachineList* compute_path(psy_audio_Machines* self, uintptr_t slot, bool concat)
{
	MachineList* rv = 0;	

	reset_nopath(self);	
	psy_table_init(&self->colors);
	compute_slotpath(self, slot, &rv);
	psy_table_dispose(&self->colors);
	//	Debug Path Output
	// if (rv) {
	// 	psy_List* p;

	// 	for (p = rv; p != 0; p = p->next) {
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
	psy_audio_MachineSockets* connected_sockets;	

	connected_sockets = connections_at(&self->connections, slot);	
	if (connected_sockets) {
		WireSocket* p;

		for (p = connected_sockets->inputs; p != 0; p = p->next) {
			psy_audio_WireSocketEntry* entry;

			entry = (psy_audio_WireSocketEntry*) p->entry;
			if (!psy_table_exists(&self->colors, entry->slot)) {
				psy_table_insert(&self->colors, entry->slot, (void*) 1);
				compute_slotpath(self, entry->slot, path);
			} else {
				// cycle detected				
				p = p;
			}			
			psy_table_remove(&self->colors, entry->slot);			
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
	/*MachinePath* slot;	

	machines_releasebuffers(self);
	for (slot = path; slot != 0; slot = slot->next) {		
		psy_audio_Machine* machine;
		
		machine = machines_at(self, (uintptr_t) slot->entry);
		if (machine) {						
			psy_table_insert(&self->outputbuffers, (uintptr_t) slot->entry, 
				psy_list_append(&self->buffers, machines_nextbuffer(
					self, machine->numoutputs(machine)))->entry);
		}
	}*/

	psy_TableIterator it;
	
	for (it = machines_begin(self);
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

	rv = psy_audio_buffer_allocinit(channels);
	for (channel = 0; channel < channels; ++channel) {
		float* samples;
				
		samples = self->samplebuffers
			? self->samplebuffers + (self->currsamplebuffer * MAX_STREAM_SIZE)
			: 0;
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

	for (p = q = self->buffers; p != 0; p = p->next) {
		psy_audio_Buffer* buffer;

		buffer = (psy_audio_Buffer*) p->entry;
		psy_audio_buffer_dispose(buffer);
		free(buffer);	
	}
	psy_list_free(q);
	self->buffers = 0;
}


psy_audio_Buffer* machines_inputs(psy_audio_Machines* self, uintptr_t slot)
{
	return psy_table_at(&self->inputbuffers, slot);
}

psy_audio_Buffer* machines_outputs(psy_audio_Machines* self, uintptr_t slot)
{	
	return psy_table_at(&self->outputbuffers, slot);
}

void machines_changeslot(psy_audio_Machines* self, uintptr_t slot)
{
	self->slot = slot;
	psy_signal_emit(&self->signal_slotchange, self, 1, slot);
}

void machines_changetweakparam(psy_audio_Machines* self, uintptr_t param)
{
	self->tweakparam = param;
}

uintptr_t machines_slot(psy_audio_Machines* self)
{
	return self->slot;
}

uintptr_t psy_audio_machines_soloed(psy_audio_Machines* self)
{
	return self->soloed;
}

void psy_audio_machines_solo(psy_audio_Machines* self, uintptr_t slot)
{
	if (self->filemode != 0) {
		self->soloed = slot;
	} else
	if (self->soloed == slot) {
		psy_TableIterator it;

		self->soloed = NOMACHINE_INDEX;
		for (it = machines_begin(self); !psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			psy_audio_Machine* machine;

			machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
			if ((psy_audio_machine_mode(machine) == MACHMODE_GENERATOR)) {
				psy_audio_machine_unmute(machine);
			}
		}
	} else {
		psy_TableIterator it;

		self->soloed = slot;
		for (it = machines_begin(self); !psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			psy_audio_Machine* machine;

			machine = (psy_audio_Machine*)psy_tableiterator_value(&it);											
			if ((psy_audio_machine_mode(machine) == MACHMODE_GENERATOR) &&
				psy_tableiterator_key(&it) != slot) {
				psy_audio_machine_mute(machine);
			} else
			if (psy_tableiterator_key(&it) == slot) {
				psy_audio_machine_unmute(machine);
			}
		}
	}
}

psy_audio_Machine* machines_master(psy_audio_Machines* self)
{
	return self->master;
}

void machines_startfilemode(psy_audio_Machines* self)
{
	self->filemode = 1;
	self->connections.filemode = 1;
}

void machines_endfilemode(psy_audio_Machines* self)
{
	machines_setpath(self, compute_path(self, MASTER_INDEX, TRUE));
	self->filemode = 0;	
	self->connections.filemode = 0;
}

uintptr_t machines_size(psy_audio_Machines* self)
{
	return psy_table_size(&self->slots);
}

psy_TableIterator machines_begin(psy_audio_Machines* self)
{
	return psy_table_begin(&self->slots);
}

bool machines_ismixersend(psy_audio_Machines* self, uintptr_t slot)
{
	return psy_table_exists(&self->connections.sends, slot);
}

void machines_addmixersend(psy_audio_Machines* self, uintptr_t slot)
{
	psy_table_insert(&self->connections.sends, slot, (void*)(uintptr_t)1);
}

void machines_removemixersend(psy_audio_Machines* self, uintptr_t slot)
{
	psy_table_remove(&self->connections.sends, slot);
}

void machines_connectasmixersend(psy_audio_Machines* self)
{
	self->mixersendconnect = TRUE;
}

void machines_connectasmixerinput(psy_audio_Machines* self)
{
	self->mixersendconnect = FALSE;
}

bool machines_isconnectasmixersend(psy_audio_Machines* self)
{
	return self->mixersendconnect;
}
