// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_MACHINES_H
#define psy_audio_MACHINES_H

#include "machine.h"
#include "connections.h"
#include <signal.h>
#include "../../detail/stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

#define psy_audio_MASTER_INDEX 128
#define psy_audio_MAX_STREAM_SIZE 256

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
	uintptr_t tweakparam;
	uintptr_t soloed;
	int filemode;
	uintptr_t maxindex;
	psy_Signal signal_insert;
	psy_Signal signal_removed;
	psy_Signal signal_slotchange;
	psy_audio_Machine* master;
	bool mixersendconnect;
} psy_audio_Machines;

void psy_audio_machines_init(psy_audio_Machines*);
void psy_audio_machines_dispose(psy_audio_Machines*);
void psy_audio_machines_clear(psy_audio_Machines*);
void psy_audio_machines_insert(psy_audio_Machines*, uintptr_t slot,
	psy_audio_Machine*);
void psy_audio_machines_erase(psy_audio_Machines*, uintptr_t slot);
void psy_audio_machines_remove(psy_audio_Machines*, uintptr_t slot);
void psy_audio_machines_exchange(psy_audio_Machines*, uintptr_t srcslot, uintptr_t dstslot);
uintptr_t psy_audio_machines_append(psy_audio_Machines*, psy_audio_Machine*);
psy_audio_Machine* psy_audio_machines_at(psy_audio_Machines*, uintptr_t slot);
uintptr_t psy_audio_machines_size(psy_audio_Machines*);
int psy_audio_machines_connect(psy_audio_Machines*, uintptr_t outputslot, uintptr_t inputslot);
void psy_audio_machines_disconnect(psy_audio_Machines*, uintptr_t outputslot, uintptr_t inputslot);
void psy_audio_machines_disconnectall(psy_audio_Machines*, uintptr_t slot);
int psy_audio_machines_connected(psy_audio_Machines*, uintptr_t outputslot, uintptr_t inputslot);

INLINE MachineList* psy_audio_machines_path(psy_audio_Machines* self)
{
	return self->path;
}

psy_audio_Buffer* psy_audio_machines_inputs(psy_audio_Machines*, uintptr_t slot);
psy_audio_Buffer* psy_audio_machines_outputs(psy_audio_Machines*, uintptr_t slot);
void psy_audio_buffer_end(psy_audio_Machines*);
void psy_audio_machines_changeslot(psy_audio_Machines*, uintptr_t slot);
void psy_audio_machines_changetweakparam(psy_audio_Machines*, uintptr_t slot);
uintptr_t psy_audio_machines_slot(psy_audio_Machines*);
uintptr_t psy_audio_machines_tweakparam(psy_audio_Machines*);
uintptr_t psy_audio_machines_soloed(psy_audio_Machines*);
void psy_audio_machines_solo(psy_audio_Machines*, uintptr_t slot);
void psy_audio_machines_insertmaster(psy_audio_Machines*, psy_audio_Machine*);
psy_audio_Machine* psy_audio_machines_master(psy_audio_Machines*);
void psy_audio_machines_startfilemode(psy_audio_Machines*);
void psy_audio_machines_endfilemode(psy_audio_Machines*);
psy_TableIterator psy_audio_machines_begin(psy_audio_Machines*);
MachineList* psy_audio_compute_path(psy_audio_Machines*, uintptr_t slot, bool concat);
bool psy_audio_machines_ismixersend(psy_audio_Machines* self, uintptr_t slot);
void psy_audio_machines_addmixersend(psy_audio_Machines* self, uintptr_t slot);
void psy_audio_machines_removemixersend(psy_audio_Machines* self, uintptr_t slot);
void psy_audio_machines_connectasmixersend(psy_audio_Machines*);
void psy_audio_machines_connectasmixerinput(psy_audio_Machines*);
bool psy_audio_machines_isconnectasmixersend(psy_audio_Machines*);
uintptr_t psy_audio_machines_maxindex(psy_audio_Machines*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MACHINES_H */
