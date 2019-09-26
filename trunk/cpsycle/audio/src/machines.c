// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "machines.h"
#include "exclusivelock.h"
#include <operations.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static int OnEnumFreeMachine(Machines*, int slot, Machine*);
static int OnEnumPathMachines(Machines*, int slot, Machine*);
static int machines_freeslot(Machines*, int start);
static MachineConnection* findconnection(MachineConnection*, int slot);
static MachineConnectionEntry* allocconnectionentry(int slot, float volume);
static void appendconnectionentry(MachineConnection**, MachineConnectionEntry*);
static MachineConnections* initconnections(Machines*, int slot);
static void machines_setpath(Machines*, MachineList* path);
static MachineList* compute_path(Machines*, int slot);
static void free_machinepath(List* path);
static void machines_preparebuffers(Machines*, MachineList* path, unsigned int amount);
static void machines_releasebuffers(Machines*);
static Buffer* machines_nextbuffer(Machines*, unsigned int channels);
static void machines_freebuffers(Machines*);

void machines_init(Machines* self)
{
	InitIntHashTable(&self->slots, 256);
	InitIntHashTable(&self->connections, 256);
	InitIntHashTable(&self->inputbuffers, 256);
	InitIntHashTable(&self->outputbuffers, 256);
	self->path = 0;
	self->numsamplebuffers = 100;
	self->samplebuffers = (float*)malloc(sizeof(float) * MAX_STREAM_SIZE *
		self->numsamplebuffers);
	self->currsamplebuffer = 0;
	self->slot = 0;	
	self->buffers = 0;
	signal_init(&self->signal_insert);
	signal_init(&self->signal_removed);
	signal_init(&self->signal_slotchange);
	signal_init(&self->signal_showparameters);
	self->filemode = 0;
}

void machines_dispose(Machines* self)
{	
	signal_dispose(&self->signal_insert);
	signal_dispose(&self->signal_removed);
	signal_dispose(&self->signal_slotchange);
	signal_dispose(&self->signal_showparameters);
	machines_enumerate(self, self, OnEnumFreeMachine);
	free_machinepath(self->path);
	DisposeIntHashTable(&self->slots);
	DisposeIntHashTable(&self->connections);
	machines_releasebuffers(self);
	machines_freebuffers(self);
	free(self->samplebuffers);	
}

void machines_freebuffers(Machines* self)
{
	List* p;

	for (p = self->buffers; p != 0; p = p->next) {				
		free(p->entry);		
	}
	list_free(p);
	self->buffers = 0;
}

int OnEnumFreeMachine(Machines* self, int slot, Machine* machine)
{
	machine->dispose(machine);
	free(machine);
	return 1;
}

void machines_insert(Machines* self, int slot, Machine* machine)
{	
	if (machine) {
		InsertIntHashTable(&self->slots, slot, machine);	
		if (!machines_connections(self, slot)) {
			initconnections(self, slot);
		}	
		signal_emit(&self->signal_insert, self, 1, slot);
	}
}

MachineConnections* initconnections(Machines* self, int slot)
{
	MachineConnections* connections;

	connections = (MachineConnections*) malloc(sizeof(MachineConnections));
	memset(connections, 0, sizeof(MachineConnections));
	connections->outputs = 0;
	connections->inputs = 0;
	InsertIntHashTable(&self->connections, slot, connections);
	return connections;
}

void machines_remove(Machines* self, int slot)
{	
	suspendwork();
	machines_disconnectall(self, slot);	
	RemoveIntHashTable(&self->slots, slot);
	machines_setpath(self, compute_path(self, MASTER_INDEX));
	signal_emit(&self->signal_removed, self, 1, slot);
	resumework();
}

void machines_exchange(Machines* self, int srcslot, int dstslot)
{
	Machine* src;
	Machine* dst;

	src = machines_at(self, srcslot);
	dst = machines_at(self, dstslot);
	machines_remove(self, srcslot);
	machines_remove(self, dstslot);
	machines_insert(self, srcslot, dst);
	machines_insert(self, dstslot, src);
}

int machines_append(Machines* self, Machine* machine)
{	
	int slot;
	
	slot = machines_freeslot(self,
		(machine->mode(machine) == MACHMODE_FX) ? 0x40 : 0);	
	InsertIntHashTable(&self->slots, slot, machine);
	if (!machines_connections(self, slot)) {
		initconnections(self, slot);
	}
	signal_emit(&self->signal_insert, self, 1, slot);
	return slot;
}

int machines_freeslot(Machines* self, int start)
{
	int rv;
	
	for (rv = start; SearchIntHashTable(&self->slots, rv) != 0; ++rv);
	return rv;
}

Machine* machines_at(Machines* self, int slot)
{
	return SearchIntHashTable(&self->slots, slot);
}

void machines_enumerate(Machines* self, void* context,
						int (*enumproc)(void*, int, Machine*))
{
	Machine* machine;	
	int slot;

	for (slot = self->slots.keymin; slot <= self->slots.keymax; ++slot) {
		machine = machines_at(self, slot);
		if (machine) {
			if (!enumproc(context, slot, machine)) {
				break;
			}
		}
	}
}

int machines_connect(Machines* self, int outputslot, int inputslot)
{
	if (outputslot != inputslot && 
			!machines_connected(self, outputslot, inputslot)) {		
		MachineConnections* connections;		

		if (!self->filemode) {
			suspendwork();
		}
		connections = machines_connections(self, outputslot);
		if (self->filemode && !connections) {
			connections = initconnections(self, outputslot);
		}
		if (connections) {		
			appendconnectionentry(&connections->outputs,
				allocconnectionentry(inputslot, 1.f));		
		}
		connections = machines_connections(self, inputslot);
		if (self->filemode && !connections) {
			connections = initconnections(self, inputslot);
		}
		if (connections) {
			appendconnectionentry(&connections->inputs,
				allocconnectionentry(outputslot, 1.f));		
		}		
		if (!self->filemode) {
			machines_setpath(self, compute_path(self, MASTER_INDEX));			
			resumework();
		}
		return 1;
	}
	return 0;
}

MachineConnectionEntry* allocconnectionentry(int slot, float volume)
{
	MachineConnectionEntry* rv;
		
	rv = malloc(sizeof(MachineConnectionEntry));
	rv->slot = slot;
	rv->volume = 1.0;	
	return rv;
}

void appendconnectionentry(MachineConnection** connection,
	MachineConnectionEntry* entry)
{
	if (*connection) {		
		list_append(*connection, entry);		
	} else {
		*connection = list_create(entry);			
	}
}

void machines_disconnect(Machines* self, int outputslot, int inputslot)
{	
	MachineConnections* connections;

	connections = machines_connections(self, outputslot);	
	if (connections) {				
		MachineConnection* p;

		p = findconnection(connections->outputs, inputslot);
		suspendwork();
		if (p) {
			free(p->entry);
			list_remove(&connections->outputs, p);			
		}		
		connections = machines_connections(self, inputslot);
		p = findconnection(connections->inputs, outputslot);
		if (p) {		
			free(p->entry);
			list_remove(&connections->inputs, p);						
		}
		machines_setpath(self, compute_path(self, MASTER_INDEX));
		resumework();
	}	
}

void machines_disconnectall(Machines* self, int slot)
{
	MachineConnections* connections;
	connections = machines_connections(self, slot);
	if (connections) {		
		MachineConnection* out;
		MachineConnection* in;
	
		suspendwork();
		out = connections->outputs;
		while (out) {			
			MachineConnections* dst;
			MachineConnection* dstinput;
			dst = machines_connections(self,
				((MachineConnectionEntry*) out->entry)->slot);
			dstinput = findconnection(dst->inputs, slot);
			free(dstinput->entry);
			list_remove(&dst->inputs, dstinput);
			out = out->next;
		}
		list_free(connections->outputs);
		connections->outputs = 0;
		
		in = connections->inputs;
		while (in) {			
			MachineConnections* src;
			MachineConnection* srcoutput;
			src = machines_connections(self,
				((MachineConnectionEntry*) in->entry)->slot);
			srcoutput = findconnection(src->outputs, slot);
			free(srcoutput->entry);
			list_remove(&src->outputs, srcoutput);
			in = in->next;
		}
		list_free(connections->inputs);
		connections->inputs = 0;
		resumework();
	}	
}

int machines_connected(Machines* self, int outputslot, int inputslot)
{	
	MachineConnection* p = 0;
	MachineConnections* connections;	

	connections = machines_connections(self, outputslot);
	if (connections) {						
		p = findconnection(connections->outputs, inputslot);
#if defined(_DEBUG)
		if (p) {			
			connections = machines_connections(self, inputslot);
			assert(connections);
			p = findconnection(connections->inputs, outputslot);
			assert(p);
		}
#endif
	}
	return p != 0;
}

MachineConnection* findconnection(MachineConnection* connection, int slot)
{	
	MachineConnection* p;
	
	p = connection;
	while (p && ((MachineConnectionEntry*)(p->entry))->slot != slot) {
		p = p->next;
	}
	return p;
}

MachineConnections* machines_connections(Machines* self, int slot)
{
	return SearchIntHashTable(&self->connections, slot);	
}

void machines_setpath(Machines* self, MachineList* path)
{	
	machines_preparebuffers(self, path, MAX_STREAM_SIZE);
	self->path = path;
}

MachineList* machines_path(Machines* self)
{
	return self->path;
}

MachineList* compute_path(Machines* self, int slot)
{
	MachineConnection* p;
	MachineConnections* connections;

	MachineList* list;
	list = list_create((void*)slot);
	connections = machines_connections(self, slot);
	p = connections->inputs;
	if (!p) {		
		return list;
	}
	while (p) {
		List* inlist;
		MachineConnectionEntry* entry;

		entry = (MachineConnectionEntry*) p->entry;		
		inlist = compute_path(self, entry->slot);
		if (inlist) {								
			inlist->tail->next = list;
			inlist->tail = list->tail;
			list = inlist;				
		}
		p = p->next;
	}
	return list;
}

void free_machinepath(List* path)
{
	if (path) {
		List* p;
		List* next;
		
		p = path;
		while (p != NULL) {
			next = p->next;
			free(p);
			p = next;
		}
	}
}

void machines_preparebuffers(Machines* self, MachineList* path, unsigned int amount)
{
	MachinePath* p;	

	machines_releasebuffers(self);
	for (p = path; p != 0; p = p->next) {		
		Machine* machine;
		unsigned int slot;		

		slot = (unsigned int) p->entry;
		machine = machines_at(self, slot);
		if (machine) {
			Buffer* buffer;
			
			buffer = machines_nextbuffer(self, machine->numoutputs(machine));
			if (self->buffers) {
				list_append(self->buffers, buffer);
			} else {
				self->buffers = list_create(buffer);
			}
			InsertIntHashTable(&self->outputbuffers, slot, buffer);				
		}
	}			
}


Buffer* machines_nextbuffer(Machines* self, unsigned int channels)
{
	unsigned int channel;
	Buffer* rv;

	rv = (Buffer*)malloc(sizeof(Buffer));
	buffer_init(rv, channels);
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

void machines_releasebuffers(Machines* self)
{
	DisposeIntHashTable(&self->inputbuffers);
	DisposeIntHashTable(&self->outputbuffers);
	InitIntHashTable(&self->inputbuffers, 256);
	InitIntHashTable(&self->outputbuffers, 256);
	machines_freebuffers(self);
	self->currsamplebuffer = 0;
}

Buffer* machines_inputs(Machines* self, unsigned int slot)
{
	return SearchIntHashTable(&self->inputbuffers, slot);
}

Buffer* machines_outputs(Machines* self, unsigned int slot)
{
	return SearchIntHashTable(&self->outputbuffers, slot);
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
	return machines_at(self, MASTER_INDEX);	
}

void machines_startfilemode(Machines* self)
{
	self->filemode = 1;
}

void machines_endfilemode(Machines* self)
{
	machines_setpath(self, compute_path(self, MASTER_INDEX));
	self->filemode = 0;
}

unsigned int machines_size(Machines* self)
{
	return self->slots.count;
}

void machines_showparameters(Machines* self, int slot)
{
	signal_emit(&self->signal_showparameters, self, 1, slot);
}
