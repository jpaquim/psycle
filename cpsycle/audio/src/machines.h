// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#if !defined(MACHINES_H)
#define MACHINES_H

#include <hashtbl.h>
#include <list.h>
#include "machine.h"
#include <signal.h>

#define MASTER_INDEX 128

typedef struct {
	int slot;	
	float volume;	
} MachineConnectionEntry;

typedef List MachineConnection;

typedef struct {	
	MachineConnection* inputs;
	MachineConnection* outputs;
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
	int filemode;

	Signal signal_insert;
	Signal signal_removed;
	Signal signal_slotchange;		
} Machines;

void machines_init(Machines*);
void machines_dispose(Machines*);
void machines_insert(Machines* self, int slot, Machine* machine);
void machines_remove(Machines* self, int slot);
int machines_append(Machines* self, Machine* machine);
Machine* machines_at(Machines* self, int slot);
void machines_enumerate(Machines* self, void* context, int (*enumproc)(void*, int, Machine*));
int machines_connect(Machines* self, int outputslot, int inputslot);
void machines_disconnect(Machines* self, int outputslot, int inputslot);
void machines_disconnectall(Machines* self, int slot);
int machines_connected(Machines* self, int outputslot, int inputslot);
MachineConnections* machines_connections(Machines* self, int slot);
float* machines_nextbuffer(Machines* self);
void machines_changeslot(Machines* self, int slot);
int machines_slot(Machines* self);
Machine* machines_master(Machines* self);
void machines_startfilemode(Machines* self);
void machines_endfilemode(Machines* self);

void suspendwork(void);
void resumework(void);

#endif
