/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_MACHINESELECTION_H
#define psy_audio_MACHINESELECTION_H

/* container */
#include <list.h>
#include <signal.h>
#include <command.h>

#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

/* psy_audio_MachineIndex
**
** Addresses a MachineIndex
*/
typedef struct psy_audio_MachineIndex {
	uintptr_t group;
	uintptr_t macid;	
} psy_audio_MachineIndex;

INLINE psy_audio_MachineIndex psy_audio_machineindex_make(
	uintptr_t macid)
{
	psy_audio_MachineIndex rv;
	
	rv.macid = macid;
	return rv;
}

psy_audio_MachineIndex* psy_audio_machineindex_alloc(void);

INLINE psy_audio_MachineIndex psy_audio_machineindex_zero(void)
{
	return psy_audio_machineindex_make(psy_INDEX_INVALID);		
}

INLINE bool psy_audio_machineindex_valid(const psy_audio_MachineIndex* self)
{
	return (self->macid != psy_INDEX_INVALID);
}

INLINE bool psy_audio_machineindex_invalid(const psy_audio_MachineIndex* self)
{
	return (!psy_audio_machineindex_valid(self));
}

INLINE bool psy_audio_machineindex_equal(const psy_audio_MachineIndex* self,
	psy_audio_MachineIndex other)
{
	return (self->macid == other.macid);
}

struct psy_audio_Machine;

/*
** psy_audio_MachineSelection
**
** Selection of MachineEntries stored as OrderIndexes
*/

typedef psy_List* psy_audio_MachineSelectionIterator;

typedef struct psy_audio_MachineSelection {
	/* signals */
	psy_Signal signal_clear;
	psy_Signal signal_select;
	psy_Signal signal_deselect;
	// psy_Signal signal_update;
	/* internal */
	psy_List* entries; /* list of machine indexes */	
} psy_audio_MachineSelection;

void psy_audio_machineselection_init(psy_audio_MachineSelection*);
void psy_audio_machineselection_dispose(psy_audio_MachineSelection*);

void psy_audio_machineselection_clear(psy_audio_MachineSelection*);
void psy_audio_machineselection_copy(psy_audio_MachineSelection*,
	psy_audio_MachineSelection* other);

INLINE psy_audio_MachineSelectionIterator psy_audio_machineselection_begin(
	psy_audio_MachineSelection* self)
{
	return self->entries;
}

psy_audio_MachineIndex psy_audio_machineselection_first(
	const psy_audio_MachineSelection*);
void psy_audio_machineselection_select(psy_audio_MachineSelection*,
	psy_audio_MachineIndex);
void psy_audio_machineselection_select_first(psy_audio_MachineSelection*,
	psy_audio_MachineIndex);
void psy_audio_machineselection_deselect(psy_audio_MachineSelection*,
	psy_audio_MachineIndex);
void psy_audio_machineselection_deselectall(psy_audio_MachineSelection*);
bool psy_audio_machineselection_isselected(const psy_audio_MachineSelection*,
	psy_audio_MachineIndex);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MACHINESELECTION_H */
