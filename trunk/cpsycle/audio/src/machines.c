// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "machines.h"
#include <assert.h>

#define BUFFER_SIZE 1024

static int OnEnumFreeMachine(Machines* self, int slot, Machine* machine);
static int OnEnumPathMachines(Machines* self, int slot, Machine* machine);
static int machines_freeslot(Machines* self);
static MachineConnection* findconnection(MachineConnection* connection, int slot);
static MachineConnectionEntry* allocconnectionentry(int slot, float volume);
static void appendconnectionentry(MachineConnection** connection,
		MachineConnectionEntry* entry);
static MachineConnections* initconnections(Machines* self, int slot);
static MachineList* compute_path(Machines* self, int slot);
static void free_machinepath(List* path);

HANDLE hGuiEvent;
HANDLE hWorkDoneEvent;

void suspendwork(void)
{
	ResetEvent(hGuiEvent);
	WaitForSingleObject(hWorkDoneEvent, 200);
}

void resumework(void)
{
	SetEvent(hGuiEvent);
}

void machines_init(Machines* self)
{
	InitIntHashTable(&self->slots, 256);
	InitIntHashTable(&self->connections, 256);
	self->path = 0;
	hGuiEvent = CreateEvent(NULL, TRUE, TRUE, NULL);
	hWorkDoneEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
	self->numbuffers = 20;
	self->buffers = (float*) malloc(sizeof(float)*BUFFER_SIZE * self->numbuffers);
	self->currbuffer = 0;
	self->slot = 0;	
	signal_init(&self->signal_insert);
	signal_init(&self->signal_removed);
	signal_init(&self->signal_slotchange);
	self->filemode = 0;
}

void machines_dispose(Machines* self)
{	
	signal_dispose(&self->signal_insert);
	signal_dispose(&self->signal_removed);
	signal_dispose(&self->signal_slotchange);
	machines_enumerate(self, self, OnEnumFreeMachine);
	free_machinepath(self->path);
	DisposeIntHashTable(&self->slots);
	DisposeIntHashTable(&self->connections);
	free(self->buffers);
}

int OnEnumFreeMachine(Machines* self, int slot, Machine* machine)
{
	machine->dispose(machine);
	free(machine);
	return 1;
}

void machines_insert(Machines* self, int slot, Machine* machine)
{	
	InsertIntHashTable(&self->slots, slot, machine);	
	if (!machines_connections(self, slot)) {
		initconnections(self, slot);
	}	
	signal_emit(&self->signal_insert, self, 1, slot);	
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
	machines_disconnectall(self, slot);
	suspendwork();
	RemoveIntHashTable(&self->slots, slot);
	self->path = compute_path(self, MASTER_INDEX);
	signal_emit(&self->signal_removed, self, 1, slot);
	resumework();
}

int machines_append(Machines* self, Machine* machine)
{	
	int slot;

	slot = machines_freeslot(self);	
	InsertIntHashTable(&self->slots, slot, machine);
	if (!machines_connections(self, slot)) {
		initconnections(self, slot);
	}
	signal_emit(&self->signal_insert, self, 1, slot);
	return slot;
}

int machines_freeslot(Machines* self)
{
	int rv;
	
	for (rv = 0; SearchIntHashTable(&self->slots, rv) != 0; ++rv);
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
	if (!machines_connected(self, outputslot, inputslot)) {		
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
			self->path = compute_path(self, MASTER_INDEX);		
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
		self->path = compute_path(self, MASTER_INDEX);	
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
			dst = machines_connections(self, ((MachineConnectionEntry*) out->entry)->slot);
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
			src = machines_connections(self, ((MachineConnectionEntry*) in->entry)->slot);
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

float* machines_nextbuffer(Machines* self)
{
	float* buffer = self->buffers + (self->currbuffer * BUFFER_SIZE);
	++self->currbuffer;
	if (self->currbuffer >= self->numbuffers) {
		self->currbuffer = 0;
	}	
	return buffer;
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
	self->path = compute_path(self, MASTER_INDEX);
	self->filemode = 0;
}
