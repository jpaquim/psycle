// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(MACHINES_H)
#define MACHINES_H

#include <hashtbl.h>
#include <list.h>
#include "machine.h"
#include <signal.h>

struct MachineConnectionStruct {
	int slot;	
	float volume;	
	struct MachineConnectionStruct* next;
};

typedef struct MachineConnectionStruct MachineConnection;

typedef struct {	
	MachineConnection inputs;
	MachineConnection outputs;
} MachineConnections;

typedef List MachinePath;
typedef List MachineList;

typedef struct {	
	IntHashTable slots;
	IntHashTable connections;
	MachinePath* path;
	float* buffers;
	int numbuffers;
	int currbuffer;
	int slot;

	Signal signal_insert;
	Signal signal_slotchange;		
} Machines;

void machines_init(Machines*);
void machines_dispose(Machines*);
void machines_insert(Machines* self, int slot, Machine* machine);
int machines_append(Machines* self, Machine* machine);
Machine* machines_at(Machines* self, int slot);
void machines_enumerate(Machines* self, void* context, int (*enumproc)(void*, int, Machine*));
void machines_connect(Machines* self, int outputslot, int inputslot);
MachineConnections* machines_connections(Machines* self, int slot);
MachineList* calc_list(Machines* self, int slot);
float* machines_nextbuffer(Machines* self);
void machines_changeslot(Machines* self, int slot);
int machines_slot(Machines* self);
Machine* machines_master(Machines* self);


#endif
