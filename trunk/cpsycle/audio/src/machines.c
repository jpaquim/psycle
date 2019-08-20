// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "machines.h"

#define BUFFER_SIZE 1024

static int OnEnumMachine(Machines* self, int slot, Machine* machine);
static int OnEnumPathMachines(Machines* self, int slot, Machine* machine);
static MachineConnection* connection_tail(MachineConnection* first);
static List* compute_path(Machines* self, MachinePath* path);
static void free_machinepath(List* path);

HANDLE hGuiEvent;
HANDLE hWorkDoneEvent;

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
}

void machines_dispose(Machines* self)
{	
	signal_dispose(&self->signal_insert);
	machines_enumerate(self, self, OnEnumMachine);
	free_machinepath(self->path);
	DisposeIntHashTable(&self->slots);
	DisposeIntHashTable(&self->connections);
	free(self->buffers);
}

int OnEnumMachine(Machines* self, int slot, Machine* machine)
{
	machine->dispose(machine);
	free(machine);
	return 1;
}

void machines_insert(Machines* self, int slot, Machine* machine)
{
	MachineConnections* connections;
	InsertIntHashTable(&self->slots, slot, machine);
	connections = (MachineConnections*) malloc(sizeof(MachineConnections));
	memset(connections, 0, sizeof(MachineConnections));
	connections->outputs.slot = -1;
	connections->inputs.slot = -1;
	InsertIntHashTable(&self->connections, slot, connections);
	signal_emit(&self->signal_insert, self, 1, slot);	
}

int machines_append(Machines* self, Machine* machine)
{
	MachineConnections* connections;
	int slot;

	slot = 0;
	while (SearchIntHashTable(&self->slots, slot)) {
		++slot;
	}
	InsertIntHashTable(&self->slots, slot, machine);
	{
		connections = (MachineConnections*) malloc(sizeof(MachineConnections));
		memset(connections, 0, sizeof(MachineConnections));
		connections->outputs.slot = -1;		
		connections->inputs.slot = -1;
	}
	InsertIntHashTable(&self->connections, slot, connections);
	signal_emit(&self->signal_insert, self, 1, slot);
	return slot;
}

Machine* machines_at(Machines* self, int slot)
{
	return SearchIntHashTable(&self->slots, slot);
}

void machines_enumerate(Machines* self, void* context, int (*enumproc)(void*, int, Machine*))
{
	Machine* machine;
	int slot;
	for (slot = 0; slot < 256; ++slot) {
		machine = (Machine*) SearchIntHashTable(&self->slots, slot);
		if (machine) {
			if (!enumproc(context, slot, machine)) {
				break;
			}
		}
	}
}

void machines_connect(Machines* self, int outputslot, int inputslot)
{
	MachineConnections* machineconnections;
	List* list;
	
	machineconnections = SearchIntHashTable(&self->connections, outputslot);
	if (machineconnections) {			
		MachineConnection* tail = connection_tail(&machineconnections->outputs);
		tail->next = (MachineConnection*) malloc(sizeof(MachineConnection));
		tail->next->volume = 1.0;
		tail->next->slot = inputslot;
		tail->next->next = 0;		
	}
	machineconnections = SearchIntHashTable(&self->connections, inputslot);
	if (machineconnections) {	
		MachineConnection* tail = connection_tail(&machineconnections->inputs);
		tail->next = (MachineConnection*) malloc(sizeof(MachineConnection));
		tail->next->volume = 1.0;
		tail->next->slot = outputslot;
		tail->next->next = 0;		
	}
	//path = compute_path(self, 0);
	list = calc_list(self, 0);	
	if (list) {		
		ResetEvent(hGuiEvent);
		WaitForSingleObject(hWorkDoneEvent, INFINITE);
		self->path = list;
		SetEvent(hGuiEvent);
	}	
}

MachineConnection* connection_tail(MachineConnection* first)
{
	MachineConnection* ptr;

	ptr = first;
	while (ptr->next != NULL) {
		ptr = ptr->next;
	}
	return ptr;
}

MachineConnections* machines_connections(Machines* self, int slot)
{
	return SearchIntHashTable(&self->connections, slot);	
}


MachineList* calc_list(Machines* self, int slot)
{
	MachineConnection* ptr;
	MachineConnections* connections;

	MachineList* list;
	list = list_create();
	list->node = (void*)slot;

	connections = machines_connections(self, slot);
	ptr = &connections->inputs;
	if (!ptr) {		
		return list;
	}
	while (ptr) {		
		if (ptr->slot != -1) {
			List* inlist;			
			inlist = calc_list(self, ptr->slot);
			if (inlist) {								
				inlist->tail->next = list;
				inlist->tail = list->tail;
				list = inlist;				
			}
		}		
		ptr = ptr->next;
	}
	return list;
}


List* compute_path(Machines* self, MachinePath* path)
{
	int masterslot = 0;
	MachineConnection* ptr;	
	MachineConnections* connections;
		
	connections = machines_connections(self, masterslot);	
	if (connections) {
		if (path == 0) {
			path = list_create();
		}
		ptr = &connections->inputs;
		while (ptr) {
			if (ptr->slot != -1) {
				
				if (path->node) {
					list_append(path->node, (void*) ptr->slot);
				} else {					
					path->node = list_create();
					((List*)path->node)->node = (void*) ptr->slot;
				}
			}
			ptr = ptr->next;		
		}		
	}
	return path;
}

void free_machinepath(List* path)
{
	if (path) {
		List* ptr;
		List* next;
		
		ptr = path;
		while (ptr != NULL) {
			next = ptr->next;
			free(ptr);
			ptr = next;
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



