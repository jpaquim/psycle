/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_MACHINES_H
#define psy_audio_MACHINES_H

#include "machine.h"
#include "machineselection.h"
#include "connections.h"
#include <signal.h>
#include <undoredo.h>

#ifdef __cplusplus
extern "C" {
#endif

#define psy_audio_MASTER_INDEX 128
#define psy_audio_MAX_STREAM_SIZE 256
#define psy_audio_MAX_MACHINE_BUFFERS 256

typedef psy_List MachinePath;
typedef psy_List MachineList;

typedef enum {
	WHITE,
	GRAY,	
	BLACK
} Colours;

typedef struct psy_audio_Machines {	
	psy_Table slots;	
	psy_Table inputbuffers;
	psy_Table outputbuffers;
	psy_audio_Connections connections;
	MachinePath* path;
	psy_Table nopath;
	psy_Table colours;
	psy_Table levels;
	psy_List* buffers;	
	float* samplebuffers;
	int numsamplebuffers;
	int currsamplebuffer;
	psy_audio_MachineSelection selection;	
	uintptr_t paramselected;	
	uintptr_t soloed;
	psy_audio_Wire selectedwire;
	int filemode;
	uintptr_t maxindex;
	psy_Signal signal_insert;
	psy_Signal signal_removed;
	psy_Signal signal_slotchange;
	psy_Signal signal_paramselected;
	psy_Signal signal_wireselected;
	psy_audio_Machine* master;
	bool mixersendconnect;
	psy_UndoRedo undoredo;
	bool preventundoredo;
	uintptr_t mixercount;
	uintptr_t currlevel;
	uintptr_t maxlevel;
	uintptr_t opcount;
} psy_audio_Machines;

void psy_audio_machines_init(psy_audio_Machines*);
void psy_audio_machines_dispose(psy_audio_Machines*);
void psy_audio_machines_clear(psy_audio_Machines*);
void psy_audio_machines_insert(psy_audio_Machines*, uintptr_t slot,
	psy_audio_Machine*);
void psy_audio_machines_erase(psy_audio_Machines*, uintptr_t slot);
void psy_audio_machines_remove(psy_audio_Machines*, uintptr_t slot, bool rewire);
void psy_audio_machines_exchange(psy_audio_Machines*, uintptr_t srcslot,
	uintptr_t dstslot);
uintptr_t psy_audio_machines_append(psy_audio_Machines*, psy_audio_Machine*);
psy_audio_Machine* psy_audio_machines_at(psy_audio_Machines*, uintptr_t slot);
const psy_audio_Machine* psy_audio_machines_at_const(const psy_audio_Machines*,
	uintptr_t index);
uintptr_t psy_audio_machines_size(psy_audio_Machines*);
bool psy_audio_machines_valid_connection(psy_audio_Machines*, psy_audio_Wire wire);
void psy_audio_machines_connect(psy_audio_Machines*, psy_audio_Wire);
void psy_audio_machines_disconnect(psy_audio_Machines*, psy_audio_Wire);
void psy_audio_machines_disconnectall(psy_audio_Machines*, uintptr_t slot);
int psy_audio_machines_connected(psy_audio_Machines*, psy_audio_Wire);
void psy_audio_machines_updatepath(psy_audio_Machines*);

INLINE MachineList* psy_audio_machines_path(psy_audio_Machines* self)
{
	return self->path;
}

psy_audio_Buffer* psy_audio_machines_inputs(psy_audio_Machines*, uintptr_t slot);
psy_audio_Buffer* psy_audio_machines_outputs(psy_audio_Machines*, uintptr_t slot);
void psy_audio_buffer_end(psy_audio_Machines*);
void psy_audio_machines_select(psy_audio_Machines*, uintptr_t slot);
void psy_audio_machines_selectparam(psy_audio_Machines*, uintptr_t slot);
void psy_audio_machines_selectauxcolumn(psy_audio_Machines*, uintptr_t index);
uintptr_t psy_audio_machines_selected(const psy_audio_Machines*);
psy_audio_Machine* psy_audio_machines_selectedmachine(psy_audio_Machines*);
const psy_audio_Machine* psy_audio_machines_selectedmachine_const(const
	psy_audio_Machines*);
uintptr_t psy_audio_machines_paramselected(const psy_audio_Machines*);
void psy_audio_machines_selectwire(psy_audio_Machines*, psy_audio_Wire);
psy_audio_Wire psy_audio_machines_selectedwire(const psy_audio_Machines*);
uintptr_t psy_audio_machines_tweakparam(psy_audio_Machines*);
uintptr_t psy_audio_machines_soloed(psy_audio_Machines*);
void psy_audio_machines_solo(psy_audio_Machines*, uintptr_t slot);
void psy_audio_machines_insertmaster(psy_audio_Machines*, psy_audio_Machine*);
psy_audio_Machine* psy_audio_machines_master(psy_audio_Machines*);
void psy_audio_machines_startfilemode(psy_audio_Machines*);
void psy_audio_machines_endfilemode(psy_audio_Machines*);
psy_TableIterator psy_audio_machines_begin(psy_audio_Machines*);
MachineList* psy_audio_compute_path(psy_audio_Machines*, uintptr_t slot,
	bool concat);
bool psy_audio_machines_isvirtualgenerator(const psy_audio_Machines*,
	uintptr_t slot);
bool psy_audio_machines_ismixersend(const psy_audio_Machines* self, uintptr_t slot);
void psy_audio_machines_addmixersend(psy_audio_Machines* self, uintptr_t slot);
void psy_audio_machines_removemixersend(psy_audio_Machines* self, uintptr_t slot);
void psy_audio_machines_connect_as_mixersend(psy_audio_Machines*);
void psy_audio_machines_connect_as_mixerinput(psy_audio_Machines*);
bool psy_audio_machines_is_connect_as_mixersend(const psy_audio_Machines*);
void psy_audio_machines_selectwire(psy_audio_Machines*, psy_audio_Wire);

INLINE bool psy_audio_machines_hasmixer(const psy_audio_Machines* self)
{
	return self->mixercount != 0;
}

MachineList* psy_audio_machines_level(psy_audio_Machines*, uintptr_t slot,
	uintptr_t level);
uintptr_t psy_audio_machines_depth(psy_audio_Machines*);
void psy_audio_machines_levels(psy_audio_Machines*, psy_Table* rv);
MachineList* psy_audio_machines_leafs(psy_audio_Machines*);
uintptr_t psy_audio_machines_levelofmachine(psy_audio_Machines*, uintptr_t slot);

uintptr_t psy_audio_machines_maxindex(psy_audio_Machines*);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MACHINES_H */
