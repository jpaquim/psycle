// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machines.h"
#include "exclusivelock.h"
#include <operations.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

static void machines_initsignals(Machines*);
static void machines_disposesignals(Machines*);
static void machines_free(Machines*);
static int machines_freeslot(Machines*, int start);
static void machines_setpath(Machines*, MachineList* path);
static MachineList* compute_path(Machines*, int slot);
static void compute_slotpath(Machines*, int slot, List**);
static void machines_preparebuffers(Machines*, MachineList* path, unsigned int amount);
static void machines_releasebuffers(Machines*);
static Buffer* machines_nextbuffer(Machines*, unsigned int channels);
static void machines_freebuffers(Machines*);

void machines_init(Machines* self)
{
	table_init(&self->slots);
	connections_init(&self->connections);	
	table_init(&self->inputbuffers);
	table_init(&self->outputbuffers);
	table_init(&self->nopath);
	self->path = 0;
	self->numsamplebuffers = 100;
	self->samplebuffers = (float*)malloc(sizeof(float) * MAX_STREAM_SIZE *
		self->numsamplebuffers);
	self->currsamplebuffer = 0;
	self->slot = 0;	
	self->buffers = 0;
	self->filemode = 0;
	self->master = 0;
	self->volume = 1.0f;
	machines_initsignals(self);
}

void machines_initsignals(Machines* self)
{
	signal_init(&self->signal_insert);
	signal_init(&self->signal_removed);
	signal_init(&self->signal_slotchange);
	signal_init(&self->signal_showparameters);
}

void machines_dispose(Machines* self)
{	
	machines_disposesignals(self);
	machines_free(self);
	table_dispose(&self->inputbuffers);
	table_dispose(&self->outputbuffers);
	machines_freebuffers(self);
	list_free(self->path);
	self->path = 0;
	table_dispose(&self->slots);
	connections_dispose(&self->connections);
	table_dispose(&self->nopath);	
	free(self->samplebuffers);
}

void machines_disposesignals(Machines* self)
{
	signal_dispose(&self->signal_insert);
	signal_dispose(&self->signal_removed);
	signal_dispose(&self->signal_slotchange);
	signal_dispose(&self->signal_showparameters);
}

void machines_free(Machines* self)
{
	TableIterator it;
	
	for (it = machines_begin(self); !tableiterator_equal(&it, table_end());
			tableiterator_inc(&it)) {			
		Machine* machine;

		machine = (Machine*)tableiterator_value(&it);
		machine->dispose(machine);
		free(machine);
	}
}

void machines_insert(Machines* self, int slot, Machine* machine)
{	
	if (machine) {
		table_insert(&self->slots, slot, machine);
		machine->setslot(machine, slot);
		signal_emit(&self->signal_insert, self, 1, slot);
		if (!self->filemode) {		
			lock_enter();
			machines_setpath(self, compute_path(self, MASTER_INDEX));
			lock_leave();
		}
	}
}

void machines_insertmaster(Machines* self, Machine* master)
{
	self->master = master;
	if (master) {
		machines_insert(self, MASTER_INDEX, master);
	}
}

void machines_erase(Machines* self, int slot)
{	
	lock_enter();
	if (slot == MASTER_INDEX) {
		self->master = 0;
	}
	machines_disconnectall(self, slot);	
	table_remove(&self->slots, slot);
	machines_setpath(self, compute_path(self, MASTER_INDEX));
	signal_emit(&self->signal_removed, self, 1, slot);
	lock_leave();	
}

void machines_remove(Machines* self, int slot)
{	
	Machine* machine;

	machine = machines_at(self, slot);
	if (machine) {
		machines_erase(self, slot);		
		machine->dispose(machine);
		free(machine);		
	}
}

void machines_exchange(Machines* self, int srcslot, int dstslot)
{
	Machine* src;
	Machine* dst;

	src = machines_at(self, srcslot);
	dst = machines_at(self, dstslot);
	if (src && dst) {
		machines_erase(self, srcslot);
		machines_erase(self, dstslot);
		machines_insert(self, srcslot, dst);
		machines_insert(self, dstslot, src);
	}
}

int machines_append(Machines* self, Machine* machine)
{	
	int slot;
		
	slot = machines_freeslot(self,
		(machine->mode(machine) == MACHMODE_FX) ? 0x40 : 0);	
	table_insert(&self->slots, slot, machine);
	machine->setslot(machine, slot);
	signal_emit(&self->signal_insert, self, 1, slot);
	if (!self->filemode) {		
		lock_enter();
		machines_setpath(self, compute_path(self, MASTER_INDEX));
		lock_leave();
	}
	return slot;
}

int machines_freeslot(Machines* self, int start)
{
	int rv;
	
	for (rv = start; table_at(&self->slots, rv) != 0; ++rv);
	return rv;
}

Machine* machines_at(Machines* self, int slot)
{
	return table_at(&self->slots, slot);
}

int machines_connect(Machines* self, int outputslot, int inputslot, int send)
{
	int rv;
	if (!self->filemode) {
		lock_enter();			
	}
	rv = connections_connect(&self->connections, outputslot, inputslot, send);
	if (!self->filemode) {
		machines_setpath(self, compute_path(self, MASTER_INDEX));			
		lock_leave();
	}
	return rv;
}

void machines_disconnect(Machines* self, int outputslot, int inputslot)
{
	lock_enter();	
	connections_disconnect(&self->connections, outputslot, inputslot);
	machines_setpath(self, compute_path(self, MASTER_INDEX));		
	lock_leave();
}

void machines_disconnectall(Machines* self, int slot)
{
	lock_enter();
	connections_disconnectall(&self->connections, slot);
	lock_leave();
}

int machines_connected(Machines* self, int outputslot, int inputslot)
{	
	return connections_connected(&self->connections, outputslot, inputslot);
}

void machines_setpath(Machines* self, MachineList* path)
{	
	machines_preparebuffers(self, path, MAX_STREAM_SIZE);
	if (self->path) {
		list_free(self->path);
	}
	self->path = path;
}

MachineList* machines_path(Machines* self)
{
	return self->path;
}

void reset_nopath(Machines* self)
{		
	TableIterator it;

	table_dispose(&self->nopath);
	table_init(&self->nopath);
		
	for (it = table_begin(&self->slots); it.curr != 0;
			tableiterator_inc(&it)) {
		table_insert(&self->nopath, it.curr->key, 0);		
	}
}

void remove_nopath(Machines* self, int slot)
{
	table_remove(&self->nopath, slot);
}

MachineList* nopath(Machines* self)
{
	List* rv = 0;	

	TableIterator it;
	for (it = table_begin(&self->nopath); it.curr != 0;
			tableiterator_inc(&it)) {			
		list_append(&rv, (void*)it.curr->key);
	}
	return rv;
}

MachineList* compute_path(Machines* self, int slot)
{
	MachineList* rv = 0;	

	reset_nopath(self);	
	compute_slotpath(self, slot, &rv);
	//	Debug Path Output
	//	if (rv) {
	//		List* p;

	//		for (p = rv; p != 0; p = p->next) {
	//			char text[20];

	//			_snprintf(text, 20, "%d, ", (int)(p->entry));
	//			OutputDebugString(text);
	//		}
	//		OutputDebugString("\n");
	//	}
	list_cat(&rv, nopath(self));
	return rv;
}

void compute_slotpath(Machines* self, int slot, List** path)
{	
	MachineSockets* connected_sockets;

	connected_sockets = connections_at(&self->connections, slot);	
	if (connected_sockets) {
		WireSocket* p;

		for (p = connected_sockets->inputs; p != 0; p = p->next) {
			WireSocketEntry* entry;

			entry = (WireSocketEntry*) p->entry;		
			compute_slotpath(self, entry->slot, path);		
		}	
	}
	if (table_exists(&self->nopath, slot)) {
		remove_nopath(self, slot);
		list_append(path, (void*)slot);
	}
}

void machines_preparebuffers(Machines* self, MachineList* path, unsigned int amount)
{
	MachinePath* slot;	

	machines_releasebuffers(self);
	for (slot = path; slot != 0; slot = slot->next) {		
		Machine* machine;
		
		machine = machines_at(self, (unsigned int) slot->entry);
		if (machine) {						
			table_insert(&self->outputbuffers, (unsigned int) slot->entry, 
				list_append(&self->buffers, machines_nextbuffer(
					self,machine->numoutputs(machine)))->entry);
		}
	}			
}

void machines_releasebuffers(Machines* self)
{
	table_dispose(&self->inputbuffers);
	table_dispose(&self->outputbuffers);
	table_init(&self->inputbuffers);
	table_init(&self->outputbuffers);
	machines_freebuffers(self);
	self->currsamplebuffer = 0;	
}

Buffer* machines_nextbuffer(Machines* self, unsigned int channels)
{
	unsigned int channel;
	Buffer* rv;

	rv = buffer_allocinit(channels);
	for (channel = 0; channel < channels; ++channel) {
		float* samples;
		
		samples = self->samplebuffers + (self->currsamplebuffer * MAX_STREAM_SIZE);		
		++self->currsamplebuffer;
		if (self->currsamplebuffer >= self->numsamplebuffers) {
			self->currsamplebuffer = 0;
		}
		rv->samples[channel] = samples;		
	}
	return rv;
}

void machines_freebuffers(Machines* self)
{
	List* p;

	for (p = self->buffers; p != 0; p = p->next) {
		Buffer* buffer;

		buffer = (Buffer*) p->entry;
		free(buffer);	
	}
	list_free(p);
	self->buffers = 0;
}


Buffer* machines_inputs(Machines* self, unsigned int slot)
{
	return table_at(&self->inputbuffers, slot);
}

Buffer* machines_outputs(Machines* self, unsigned int slot)
{
	return table_at(&self->outputbuffers, slot);
}

void machines_changeslot(Machines* self, int slot)
{
	self->slot = slot;	
	signal_emit(&self->signal_slotchange, self, 1, slot);
}

int machines_slot(Machines* self)
{
	return self->slot;
}

Machine* machines_master(Machines* self)
{
	return self->master;
}

void machines_startfilemode(Machines* self)
{
	self->filemode = 1;
	self->connections.filemode = 1;
}

void machines_endfilemode(Machines* self)
{
	machines_setpath(self, compute_path(self, MASTER_INDEX));
	self->filemode = 0;	
	self->connections.filemode = 0;
}

unsigned int machines_size(Machines* self)
{
	return self->slots.count;
}

void machines_showparameters(Machines* self, int slot)
{
	signal_emit(&self->signal_showparameters, self, 1, slot);
}

void machines_setvolume(Machines* self, float volume)
{
	self->volume = volume;
}

float machines_volume(Machines* self)
{
	return self->volume;
}

TableIterator machines_begin(Machines* self)
{
	return table_begin(&self->slots);
}
