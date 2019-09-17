// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINES_H)
#define MACHINES_H

#include <hashtbl.h>
#include <list.h>
#include "machine.h"
#include <signal.h>

#define MASTER_INDEX 128
#define MAX_STREAM_SIZE 256

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
	IntHashTable inputbuffers;
	IntHashTable outputbuffers;
	MachinePath* path;
	List* buffers;
	float* samplebuffers;
	int numsamplebuffers;
	int currsamplebuffer;
	int slot;
	int filemode;
	Signal signal_insert;
	Signal signal_removed;
	Signal signal_slotchange;		
} Machines;

void machines_init(Machines*);
void machines_dispose(Machines*);
void machines_insert(Machines*, int slot, Machine*);
void machines_remove(Machines*, int slot);
int machines_append(Machines*, Machine*);
Machine* machines_at(Machines*, int slot);
void machines_enumerate(Machines*, void* context,
	int (*enumproc)(void*, int, Machine*));
int machines_connect(Machines*, int outputslot, int inputslot);
void machines_disconnect(Machines*, int outputslot, int inputslot);
void machines_disconnectall(Machines*, int slot);
int machines_connected(Machines*, int outputslot, int inputslot);
MachineList* machines_path(Machines* self);
MachineConnections* machines_connections(Machines*, int slot);
Buffer* machines_inputs(Machines*, unsigned int slot);
Buffer* machines_outputs(Machines*, unsigned int slot);
void machines_buffer_end(Machines*);
void machines_changeslot(Machines*, int slot);
int machines_slot(Machines*);
Machine* machines_master(Machines*);
void machines_startfilemode(Machines*);
void machines_endfilemode(Machines*);
unsigned int machines_size(Machines*);

void suspendwork(void);
void resumework(void);

#endif
