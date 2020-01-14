// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_AUDIO_MACHINES_H)
#define PSY_AUDIO_MACHINES_H

#include "connections.h"
#include "master.h"
#include <signal.h>
#include "../../detail/stdint.h"

#define MASTER_INDEX 128
#define MAX_STREAM_SIZE 256

typedef psy_List MachinePath;
typedef psy_List MachineList;

typedef enum {
	WHITE,
	GRAY,	
	BLACK
} Colors;

typedef struct psy_audio_Machines {	
	psy_Table slots;
	psy_Table inputbuffers;
	psy_Table outputbuffers;
	psy_audio_Connections connections;
	MachinePath* path;
	psy_Table nopath;
	psy_Table colors;
	psy_List* buffers;	
	float* samplebuffers;
	int numsamplebuffers;
	int currsamplebuffer;
	uintptr_t slot;
	int filemode;
	float volume;	
	psy_Signal signal_insert;
	psy_Signal signal_removed;
	psy_Signal signal_slotchange;
	psy_audio_Machine* master;
} psy_audio_Machines;

void machines_init(psy_audio_Machines*);
void machines_dispose(psy_audio_Machines*);
void machines_clear(psy_audio_Machines*);
void machines_insert(psy_audio_Machines*, uintptr_t slot,
	psy_audio_Machine*);
void machines_erase(psy_audio_Machines*, uintptr_t slot);
void machines_remove(psy_audio_Machines*, uintptr_t slot);
void machines_exchange(psy_audio_Machines*, uintptr_t srcslot, uintptr_t dstslot);
uintptr_t machines_append(psy_audio_Machines*, psy_audio_Machine*);
psy_audio_Machine* machines_at(psy_audio_Machines*, uintptr_t slot);
uintptr_t machines_size(psy_audio_Machines*);
int machines_connect(psy_audio_Machines*, uintptr_t outputslot, uintptr_t inputslot);
void machines_disconnect(psy_audio_Machines*, uintptr_t outputslot, uintptr_t inputslot);
void machines_disconnectall(psy_audio_Machines*, uintptr_t slot);
int machines_connected(psy_audio_Machines*, uintptr_t outputslot, uintptr_t inputslot);
MachineList* machines_path(psy_audio_Machines* self);
psy_audio_Buffer* machines_inputs(psy_audio_Machines*, uintptr_t slot);
psy_audio_Buffer* machines_outputs(psy_audio_Machines*, uintptr_t slot);
void machines_buffer_end(psy_audio_Machines*);
void machines_changeslot(psy_audio_Machines*, uintptr_t slot);
uintptr_t machines_slot(psy_audio_Machines*);
void machines_insertmaster(psy_audio_Machines*, psy_audio_Machine*);
psy_audio_Machine* machines_master(psy_audio_Machines*);
void machines_startfilemode(psy_audio_Machines*);
void machines_endfilemode(psy_audio_Machines*);
void machines_setvolume(psy_audio_Machines*, psy_dsp_amp_t volume);
psy_dsp_amp_t machines_volume(psy_audio_Machines*);
psy_TableIterator machines_begin(psy_audio_Machines*);

#endif
