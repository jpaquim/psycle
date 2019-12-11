// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINES_H)
#define MACHINES_H

#include "connections.h"
#include "master.h"
#include <signal.h>
#include "../../detail/stdint.h"

#define NOMACHINE_INDEX UINTPTR_MAX
#define MASTER_INDEX 128
#define MAX_STREAM_SIZE 256

typedef List MachinePath;
typedef List MachineList;

typedef enum {
	WHITE,
	GRAY,	
	BLACK
} Colors;

typedef struct Machines {	
	Table slots;	
	Table inputbuffers;
	Table outputbuffers;
	Connections connections;
	MachinePath* path;
	Table nopath;
	Table colors;
	List* buffers;	
	float* samplebuffers;
	int numsamplebuffers;
	int currsamplebuffer;
	uintptr_t slot;
	int filemode;
	float volume;	
	psy_Signal signal_insert;
	psy_Signal signal_removed;
	psy_Signal signal_slotchange;
	Machine* master;
} Machines;

void machines_init(Machines*);
void machines_dispose(Machines*);
void machines_clear(Machines*);
void machines_insert(Machines*, uintptr_t slot, Machine*);
void machines_erase(Machines*, uintptr_t slot);
void machines_remove(Machines*, uintptr_t slot);
void machines_exchange(Machines*, uintptr_t srcslot, uintptr_t dstslot);
uintptr_t machines_append(Machines*, Machine*);
Machine* machines_at(Machines*, uintptr_t slot);
uintptr_t machines_size(Machines*);
int machines_connect(Machines*, uintptr_t outputslot, uintptr_t inputslot);
void machines_disconnect(Machines*, uintptr_t outputslot, uintptr_t inputslot);
void machines_disconnectall(Machines*, uintptr_t slot);
int machines_connected(Machines*, uintptr_t outputslot, uintptr_t inputslot);
MachineList* machines_path(Machines* self);
Buffer* machines_inputs(Machines*, uintptr_t slot);
Buffer* machines_outputs(Machines*, uintptr_t slot);
void machines_buffer_end(Machines*);
void machines_changeslot(Machines*, uintptr_t slot);
uintptr_t machines_slot(Machines*);
void machines_insertmaster(Machines*, Machine*);
Machine* machines_master(Machines*);
void machines_startfilemode(Machines*);
void machines_endfilemode(Machines*);
void machines_setvolume(Machines*, psy_dsp_amp_t volume);
psy_dsp_amp_t machines_volume(Machines*);
TableIterator machines_begin(Machines*);

#endif
