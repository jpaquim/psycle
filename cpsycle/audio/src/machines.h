// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINES_H)
#define MACHINES_H

#include "connections.h"
#include "master.h"
#include <signal.h>

#define MASTER_INDEX 128
#define MAX_STREAM_SIZE 256

typedef List MachinePath;
typedef List MachineList;

typedef struct Machines {	
	Table slots;	
	Table inputbuffers;
	Table outputbuffers;
	Connections connections;
	MachinePath* path;
	Table nopath;
	List* buffers;	
	float* samplebuffers;
	int numsamplebuffers;
	int currsamplebuffer;
	int slot;
	int filemode;
	float volume;	
	Signal signal_insert;
	Signal signal_removed;
	Signal signal_slotchange;
	Signal signal_showparameters;
	Machine* master;
} Machines;

void machines_init(Machines*);
void machines_dispose(Machines*);
void machines_clear(Machines*);
void machines_insert(Machines*, int slot, Machine*);
void machines_erase(Machines*, int slot);
void machines_remove(Machines*, int slot);
void machines_exchange(Machines*, int srcslot, int dstslot);
int machines_append(Machines*, Machine*);
Machine* machines_at(Machines*, int slot);
size_t machines_size(Machines*);
int machines_connect(Machines*, int outputslot, int inputslot, int send);
void machines_disconnect(Machines*, int outputslot, int inputslot);
void machines_disconnectall(Machines*, int slot);
int machines_connected(Machines*, int outputslot, int inputslot);
MachineList* machines_path(Machines* self);
Buffer* machines_inputs(Machines*, unsigned int slot);
Buffer* machines_outputs(Machines*, unsigned int slot);
void machines_buffer_end(Machines*);
void machines_changeslot(Machines*, int slot);
int machines_slot(Machines*);
void machines_insertmaster(Machines*, Machine*);
Machine* machines_master(Machines*);
void machines_startfilemode(Machines*);
void machines_endfilemode(Machines*);
void machines_showparameters(Machines*, int slot);
void machines_setvolume(Machines*, float volume);
float machines_volume(Machines*);
TableIterator machines_begin(Machines*);

#endif
