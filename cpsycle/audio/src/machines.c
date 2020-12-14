// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machines.h"
#include "machinefactory.h"
#include "preset.h"
#include "exclusivelock.h"
#include <operations.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../../detail/trace.h"

// Commands

typedef struct {
	psy_Command command;
	psy_audio_Machines* machines;
	psy_audio_Machine* machine;
	uintptr_t slot;	
	bool restoreconnection;
	psy_audio_Connections connections;
} InsertMachineCommand;

static void insertmachinecommand_dispose(InsertMachineCommand*);
static void insertmachinecommand_execute(InsertMachineCommand*);
static void insertmachinecommand_revert(InsertMachineCommand*);

// vtable
static psy_CommandVtable insertmachinecommand_vtable;
static int insertmachinecommand_vtable_initialized = 0;

static void insertmachinecommand_vtable_init(InsertMachineCommand* self)
{
	if (!insertmachinecommand_vtable_initialized) {
		insertmachinecommand_vtable = *(self->command.vtable);
		insertmachinecommand_vtable.dispose = (psy_fp_command)insertmachinecommand_dispose;
		insertmachinecommand_vtable.execute = (psy_fp_command)insertmachinecommand_execute;
		insertmachinecommand_vtable.revert = (psy_fp_command)insertmachinecommand_revert;
		insertmachinecommand_vtable_initialized = 1;
	}
}

InsertMachineCommand* insertmachinecommand_allocinit(psy_audio_Machines* machines,
	uintptr_t slot, psy_audio_Machine* machine)
{
	InsertMachineCommand* rv;

	rv = malloc(sizeof(InsertMachineCommand));	
	psy_command_init(&rv->command);
	insertmachinecommand_vtable_init(rv);
	rv->command.vtable = &insertmachinecommand_vtable;
	rv->machines = machines;	
	rv->machine = machine;
	rv->slot = slot;
	rv->restoreconnection = FALSE;
	psy_audio_connections_init(&rv->connections);
	return rv;
}

void insertmachinecommand_dispose(InsertMachineCommand* self)
{		
	psy_audio_connections_dispose(&self->connections);
}

void insertmachinecommand_execute(InsertMachineCommand* self)
{
	self->machines->preventundoredo = TRUE;
	psy_audio_machines_insert(self->machines, self->slot,
		self->machine);
	if (self->restoreconnection) {
		psy_audio_exclusivelock_enter();
		psy_audio_connections_copy(&self->machines->connections,
			&self->connections);
		psy_audio_machines_updatepath(self->machines);
		psy_audio_exclusivelock_leave();
	}
	self->machines->preventundoredo = FALSE;
}

void insertmachinecommand_revert(InsertMachineCommand* self)
{
	psy_audio_Machine* machine;

	machine = psy_audio_machines_at(self->machines, self->slot);
	if (machine) {
		psy_audio_connections_dispose(&self->connections);
		psy_audio_connections_init(&self->connections);
		psy_audio_connections_copy(&self->connections, &self->machines->connections);
		self->restoreconnection = TRUE;
		self->machine = psy_audio_machine_clone(machine);
		self->machines->preventundoredo = TRUE;
		psy_audio_machines_remove(self->machines, self->slot);
		self->machines->preventundoredo = FALSE;
	}	
}

typedef struct {
	psy_Command command;
	psy_audio_Machines* machines;
	psy_audio_Machine* machine;
	uintptr_t slot;
	psy_audio_Connections connections;
} DeleteMachineCommand;

static void deletemachinecommand_dispose(DeleteMachineCommand*);
static void deletemachinecommand_execute(DeleteMachineCommand*);
static void deletemachinecommand_revert(DeleteMachineCommand*);

// vtable
static psy_CommandVtable deletemachinecommand_vtable;
static int deletemachinecommand_vtable_initialized = 0;

static void deletemachinecommand_vtable_init(DeleteMachineCommand* self)
{
	if (!deletemachinecommand_vtable_initialized) {
		deletemachinecommand_vtable = *(self->command.vtable);
		deletemachinecommand_vtable.dispose = (psy_fp_command)deletemachinecommand_dispose;
		deletemachinecommand_vtable.execute = (psy_fp_command)deletemachinecommand_execute;
		deletemachinecommand_vtable.revert = (psy_fp_command)deletemachinecommand_revert;
		deletemachinecommand_vtable_initialized = 1;
	}
}

DeleteMachineCommand* deletemachinecommand_allocinit(psy_audio_Machines* machines,
	uintptr_t slot)
{
	DeleteMachineCommand* rv;

	rv = malloc(sizeof(DeleteMachineCommand));
	psy_command_init(&rv->command);
	deletemachinecommand_vtable_init(rv);
	rv->command.vtable = &deletemachinecommand_vtable;
	rv->machines = machines;
	rv->machine = NULL;
	rv->slot = slot;
	psy_audio_connections_init(&rv->connections);
	return rv;
}

void deletemachinecommand_dispose(DeleteMachineCommand* self)
{	
	psy_audio_connections_dispose(&self->connections);
}

void deletemachinecommand_execute(DeleteMachineCommand* self)
{
	psy_audio_Machine* machine;

	machine = psy_audio_machines_at(self->machines, self->slot);
	if (machine) {
		psy_audio_connections_dispose(&self->connections);
		psy_audio_connections_init(&self->connections);
		psy_audio_connections_copy(&self->connections, &self->machines->connections);		
		self->machine = psy_audio_machine_clone(machine);
		self->machines->preventundoredo = TRUE;
		psy_audio_machines_remove(self->machines, self->slot);
		self->machines->preventundoredo = FALSE;
	}
}

void deletemachinecommand_revert(DeleteMachineCommand* self)
{
	if (self->machine) {
		self->machines->preventundoredo = TRUE;
		psy_audio_machines_insert(self->machines, self->slot,
			self->machine);
		psy_audio_exclusivelock_enter();
		psy_audio_connections_copy(&self->machines->connections,
			&self->connections);
		psy_audio_machines_updatepath(self->machines);
		psy_audio_exclusivelock_leave();
		self->machines->preventundoredo = FALSE;
	}	
}

typedef struct {
	psy_Command command;
	psy_audio_Machines* machines;
	psy_audio_Wire wire;
	psy_dsp_amp_t volume;
	psy_audio_PinMapping pins;
	bool restore;
} ConnectMachineCommand;

static void connectmachinecommand_dispose(ConnectMachineCommand*);
static void connectmachinecommand_execute(ConnectMachineCommand*);
static void connectmachinecommand_revert(ConnectMachineCommand*);

// vtable
static psy_CommandVtable connectmachinecommand_vtable;
static int connectmachinecommand_vtable_initialized = 0;

static void connectmachinecommand_vtable_init(ConnectMachineCommand* self)
{
	if (!connectmachinecommand_vtable_initialized) {
		connectmachinecommand_vtable = *(self->command.vtable);
		connectmachinecommand_vtable.dispose = (psy_fp_command)connectmachinecommand_dispose;
		connectmachinecommand_vtable.execute = (psy_fp_command)connectmachinecommand_execute;
		connectmachinecommand_vtable.revert = (psy_fp_command)connectmachinecommand_revert;
		connectmachinecommand_vtable_initialized = 1;
	}
}

ConnectMachineCommand* connectmachinecommand_allocinit(psy_audio_Machines* machines,	
	psy_audio_Wire wire)
{
	ConnectMachineCommand* rv;

	rv = malloc(sizeof(ConnectMachineCommand));
	psy_command_init(&rv->command);
	connectmachinecommand_vtable_init(rv);
	rv->command.vtable = &connectmachinecommand_vtable;
	rv->machines = machines;
	rv->wire = wire;
	rv->volume = (psy_dsp_amp_t)1.f;
	psy_audio_pinmapping_init(&rv->pins, 2);
	rv->restore = FALSE;
	return rv;
}

void connectmachinecommand_dispose(ConnectMachineCommand* self)
{
	psy_audio_pinmapping_dispose(&self->pins);
}

void connectmachinecommand_execute(ConnectMachineCommand* self)
{	
	self->machines->preventundoredo = TRUE;
	psy_audio_machines_connect(self->machines, self->wire);
	if (self->restore) {
		psy_audio_exclusivelock_enter();
		psy_audio_connections_setpinmapping(&self->machines->connections,
			self->wire, &self->pins);
		psy_audio_connections_setwirevolume(&self->machines->connections,
			self->wire, self->volume);
		psy_audio_exclusivelock_leave();
	}
	self->machines->preventundoredo = FALSE;
}

void connectmachinecommand_revert(ConnectMachineCommand* self)
{
	psy_audio_WireSocket* socket;
		
	self->machines->preventundoredo = TRUE;
	self->volume = psy_audio_connections_wirevolume(&self->machines->connections,
		self->wire);
	socket = psy_audio_connections_input(&self->machines->connections, self->wire);
	if (socket) {
		psy_audio_pinmapping_copy(&self->pins, &socket->mapping);
		self->restore = TRUE;
	} else {
		self->restore = FALSE;
	}
	psy_audio_machines_disconnect(self->machines, self->wire);
	self->machines->preventundoredo = FALSE;	
}

typedef struct {
	psy_Command command;
	psy_audio_Machines* machines;
	psy_audio_Wire wire;
	psy_dsp_amp_t volume;
	psy_audio_PinMapping pins;
} DisconnectMachineCommand;

static void disconnectmachinecommand_dispose(DisconnectMachineCommand*);
static void disconnectmachinecommand_execute(DisconnectMachineCommand*);
static void disconnectmachinecommand_revert(DisconnectMachineCommand*);

// vtable
static psy_CommandVtable disconnectmachinecommand_vtable;
static int disconnectmachinecommand_vtable_initialized = 0;

static void disconnectmachinecommand_vtable_init(DisconnectMachineCommand* self)
{
	if (!disconnectmachinecommand_vtable_initialized) {
		disconnectmachinecommand_vtable = *(self->command.vtable);
		disconnectmachinecommand_vtable.dispose = (psy_fp_command)disconnectmachinecommand_dispose;
		disconnectmachinecommand_vtable.execute = (psy_fp_command)disconnectmachinecommand_execute;
		disconnectmachinecommand_vtable.revert = (psy_fp_command)disconnectmachinecommand_revert;
		disconnectmachinecommand_vtable_initialized = 1;
	}
}

DisconnectMachineCommand* disconnectmachinecommand_allocinit(psy_audio_Machines* machines,
	psy_audio_Wire wire)
{
	DisconnectMachineCommand* rv;

	rv = malloc(sizeof(DisconnectMachineCommand));
	psy_command_init(&rv->command);
	disconnectmachinecommand_vtable_init(rv);
	rv->command.vtable = &disconnectmachinecommand_vtable;
	rv->machines = machines;
	rv->wire = wire;
	rv->volume = (psy_dsp_amp_t)1.f;
	psy_audio_pinmapping_init(&rv->pins, 2);
	return rv;
}

void disconnectmachinecommand_dispose(DisconnectMachineCommand* self)
{
	psy_audio_pinmapping_dispose(&self->pins);
}

void disconnectmachinecommand_execute(DisconnectMachineCommand* self)
{
	psy_audio_WireSocket* socket;

	self->machines->preventundoredo = TRUE;
	self->volume = psy_audio_connections_wirevolume(&self->machines->connections,
		self->wire);
	socket = psy_audio_connections_input(&self->machines->connections, self->wire);
	if (socket) {
		psy_audio_pinmapping_copy(&self->pins, &socket->mapping);
	}
	psy_audio_machines_disconnect(self->machines, self->wire);
	self->machines->preventundoredo = FALSE;
}

void disconnectmachinecommand_revert(DisconnectMachineCommand* self)
{
	self->machines->preventundoredo = TRUE;
	psy_audio_machines_connect(self->machines, self->wire);
	psy_audio_exclusivelock_enter();
	psy_audio_connections_setpinmapping(&self->machines->connections,
		self->wire, &self->pins);	
	psy_audio_connections_setwirevolume(&self->machines->connections, self->wire,
		self->volume);
	psy_audio_exclusivelock_leave();
	self->machines->preventundoredo = FALSE;
}

// Machines
static void machines_initsignals(psy_audio_Machines*);
static void machines_disposesignals(psy_audio_Machines*);
static void machines_free(psy_audio_Machines*);
static uintptr_t machines_freeslot(psy_audio_Machines*, uintptr_t start);
static void machines_setpath(psy_audio_Machines*, MachineList* path);
static void machines_sortpath(psy_audio_Machines*);
static void compute_slotpath(psy_audio_Machines*, uintptr_t slot, psy_List**);
static void machines_preparebuffers(psy_audio_Machines*, MachineList* path,
	uintptr_t amount);
static void machines_releasebuffers(psy_audio_Machines*);
static psy_audio_Buffer* machines_nextbuffer(psy_audio_Machines*,
	uintptr_t channels);
static void machines_freebuffers(psy_audio_Machines*);
static bool isleaf(psy_audio_Machines*, uintptr_t slot, psy_Table* worked);
static uintptr_t machines_findmaxindex(psy_audio_Machines*);



void psy_audio_machines_init(psy_audio_Machines* self)
{
	psy_table_init(&self->slots);	
	psy_audio_connections_init(&self->connections);	
	psy_table_init(&self->inputbuffers);
	psy_table_init(&self->outputbuffers);
	psy_table_init(&self->nopath);
	self->path = 0;
	self->numsamplebuffers = 100;
	self->samplebuffers = dsp.memory_alloc(psy_audio_MAX_STREAM_SIZE *
		self->numsamplebuffers, sizeof(float));
	assert(self->samplebuffers);
	self->currsamplebuffer = 0;
	self->slot = 0;
	self->tweakparam = 0;
	self->soloed = UINTPTR_MAX;
	self->buffers = 0;
	self->filemode = 0;
	self->master = 0;
	self->maxindex = 0;
	self->preventundoredo = FALSE;
	psy_undoredo_init(&self->undoredo);
	machines_initsignals(self);
}

void machines_initsignals(psy_audio_Machines* self)
{
	psy_signal_init(&self->signal_insert);
	psy_signal_init(&self->signal_removed);
	psy_signal_init(&self->signal_slotchange);	
}

void psy_audio_machines_dispose(psy_audio_Machines* self)
{	
	machines_disposesignals(self);
	machines_free(self);
	psy_table_dispose(&self->inputbuffers);
	psy_table_dispose(&self->outputbuffers);
	machines_freebuffers(self);
	psy_list_free(self->path);
	self->path = 0;
	psy_table_dispose(&self->slots);	
	psy_audio_connections_dispose(&self->connections);
	psy_table_dispose(&self->nopath);
	dsp.memory_dealloc(self->samplebuffers);
	psy_undoredo_dispose(&self->undoredo);
}

void machines_disposesignals(psy_audio_Machines* self)
{
	psy_signal_dispose(&self->signal_insert);
	psy_signal_dispose(&self->signal_removed);
	psy_signal_dispose(&self->signal_slotchange);	
}

void machines_free(psy_audio_Machines* self)
{
	psy_TableIterator it;
	
	for (it = psy_audio_machines_begin(self);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {			
		psy_audio_Machine* machine;

		machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
		psy_audio_machine_dispose(machine);
		free(machine);
		psy_table_insert(&self->slots, psy_tableiterator_key(&it), 0);
	}
}

void psy_audio_machines_clear(psy_audio_Machines* self)
{
	psy_audio_machines_dispose(self);
	psy_audio_machines_init(self);
}

uintptr_t psy_audio_machines_maxindex(psy_audio_Machines* self)
{
	return self->maxindex;	
}

void psy_audio_machines_insert(psy_audio_Machines* self, uintptr_t slot,
	psy_audio_Machine* machine)
{	
	if (machine) {
		if (self->preventundoredo || slot == psy_audio_MASTER_INDEX || self->filemode) {
			psy_table_insert(&self->slots, slot, machine);
			if (slot == psy_audio_MASTER_INDEX) {
				self->master = machine;
			}
			psy_audio_machine_setslot(machine, slot);
			if (slot > self->maxindex) {
				self->maxindex = slot;
			}
			psy_signal_emit(&self->signal_insert, self, 1, slot);
			if (!self->filemode) {
				psy_audio_exclusivelock_enter();
				machines_setpath(self, psy_audio_compute_path(self,
					psy_audio_MASTER_INDEX, TRUE));
				psy_audio_exclusivelock_leave();
			}
		} else {
			psy_undoredo_execute(&self->undoredo,
				&insertmachinecommand_allocinit(self, slot, machine)
				->command);
		}
	}
}

uintptr_t psy_audio_machines_append(psy_audio_Machines* self,
	psy_audio_Machine* machine)
{
	uintptr_t slot;

	slot = machines_freeslot(self,
		(psy_audio_machine_mode(machine) == MACHMODE_FX) ? 0x40 : 0);
	psy_audio_machines_insert(self, slot, machine);
	return slot;
}

void psy_audio_machines_insertmaster(psy_audio_Machines* self,
	psy_audio_Machine* master)
{
	self->master = master;
	if (master) {
		psy_audio_machines_insert(self, psy_audio_MASTER_INDEX, master);
	}
}

void psy_audio_machines_erase(psy_audio_Machines* self, uintptr_t slot)
{	
	psy_audio_exclusivelock_enter();
	if (slot == psy_audio_MASTER_INDEX) {
		self->master = NULL;
	}
	psy_audio_machines_disconnectall(self, slot);	
	psy_table_remove(&self->slots, slot);
	machines_setpath(self, psy_audio_compute_path(self, psy_audio_MASTER_INDEX,
		TRUE));
	if (slot == self->maxindex) {
		slot = machines_findmaxindex(self);
	}
	psy_signal_emit(&self->signal_removed, self, 1, slot);
	psy_audio_exclusivelock_leave();	
}

uintptr_t machines_findmaxindex(psy_audio_Machines* self)
{
	uintptr_t rv = 0;
	psy_TableIterator it;

	for (it = psy_audio_machines_begin(self);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		psy_audio_Machine* machine;

		machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
		if (psy_audio_machine_slot(machine) > rv) {
			rv = psy_audio_machine_slot(machine);
		}		
	}
	return rv;
}

void psy_audio_machines_remove(psy_audio_Machines* self, uintptr_t slot)
{	
	psy_audio_Machine* machine;
	
	machine = psy_audio_machines_at(self, slot);
	if (machine) {
		if (self->preventundoredo || slot == psy_audio_MASTER_INDEX || self->filemode) {
			psy_audio_machines_erase(self, slot);
			psy_audio_machine_dispose(machine);
			free(machine);
		} else {
			psy_undoredo_execute(&self->undoredo,
				&deletemachinecommand_allocinit(self, slot)
				->command);
		}
	}
}

void psy_audio_machines_exchange(psy_audio_Machines* self, uintptr_t srcslot,
	uintptr_t dstslot)
{
	psy_audio_Machine* src;
	psy_audio_Machine* dst;

	src = psy_audio_machines_at(self, srcslot);
	dst = psy_audio_machines_at(self, dstslot);
	if (src && dst) {		
		psy_audio_machines_erase(self, srcslot);
		psy_audio_machines_erase(self, dstslot);
		psy_audio_machines_insert(self, srcslot, dst);
		psy_audio_machines_insert(self, dstslot, src);
	}
}

uintptr_t machines_freeslot(psy_audio_Machines* self, uintptr_t start)
{
	uintptr_t rv;
	
	for (rv = start; psy_table_at(&self->slots, rv) != 0; ++rv);
	return rv;
}

psy_audio_Machine* psy_audio_machines_at(psy_audio_Machines* self, uintptr_t slot)
{		
	return psy_table_at(&self->slots, slot);
}

bool psy_audio_machines_valid_connection(psy_audio_Machines* self, psy_audio_Wire wire)
{
	psy_audio_Machine* src;
	psy_audio_Machine* dst;

	src = psy_audio_machines_at(self, wire.src);
	dst = psy_audio_machines_at(self, wire.dst);
	if (src && dst) {
		return (psy_audio_machine_numoutputs(src) > 0 &&
			psy_audio_machine_numinputs(dst) > 0);
	}
	return FALSE;
}

void psy_audio_machines_connect(psy_audio_Machines* self, psy_audio_Wire wire)
{
	if (self->preventundoredo || self->filemode) {
		int rv;

		if (!self->filemode) {
			psy_audio_exclusivelock_enter();
		}
		rv = psy_audio_connections_connect(&self->connections, wire);
		if (!self->filemode) {
			machines_setpath(self, psy_audio_compute_path(self,
				psy_audio_MASTER_INDEX, TRUE));
			psy_audio_exclusivelock_leave();
		}		
	} else {		
		psy_undoredo_execute(&self->undoredo,
			&connectmachinecommand_allocinit(self, wire)
			->command);
	}
}

void psy_audio_machines_disconnect(psy_audio_Machines* self, psy_audio_Wire wire)
{
	if (self->preventundoredo || self->filemode) {
		psy_audio_exclusivelock_enter();
		psy_audio_connections_disconnect(&self->connections, wire);
		machines_setpath(self, psy_audio_compute_path(self, psy_audio_MASTER_INDEX,
			TRUE));
		psy_audio_exclusivelock_leave();
	} else {		
		psy_undoredo_execute(&self->undoredo,
			&disconnectmachinecommand_allocinit(self, wire)
			->command);
	}
}

void psy_audio_machines_disconnectall(psy_audio_Machines* self, uintptr_t slot)
{
	psy_audio_exclusivelock_enter();
	psy_audio_connections_disconnectall(&self->connections, slot);
	psy_audio_exclusivelock_leave();
}

int psy_audio_machines_connected(psy_audio_Machines* self, psy_audio_Wire wire)
{	
	return psy_audio_connections_connected(&self->connections, wire);
}

void psy_audio_machines_updatepath(psy_audio_Machines* self)
{
	machines_setpath(self, psy_audio_compute_path(self,
		psy_audio_MASTER_INDEX, TRUE));
}

void machines_setpath(psy_audio_Machines* self, MachineList* path)
{	
	machines_preparebuffers(self, path, psy_audio_MAX_STREAM_SIZE);
	if (self->path) {
		psy_list_free(self->path);
	}
	self->path = path;
	// machines_sortpath(self);
}

// this orders the machines that can be processed parallel
// and adds NOMACHINE_INDEX as delimiter slot
// 1. Find leafs and add to sorted
// 2. Mark this machines
// 3. Repeat step 1 without checking input connections from
//    marked machines
// 4. Set sorted path
void machines_sortpath(psy_audio_Machines* self)
{	
	if (self->path) {
		psy_Table worked;
		psy_List* sorted = 0;
		psy_List* p;
		psy_List* q;

		psy_table_init(&worked);
		p = self->path;		
		while (self->path) {
			p = self->path;			
			while (p != NULL) {
				uintptr_t slot;
				slot = (uintptr_t)psy_list_entry(p);
				if (isleaf(self, slot, &worked)) {
					psy_list_append(&sorted, (void*)(uintptr_t)slot);
					p = psy_list_remove(&self->path, p);
				} else {
					psy_list_next(&p);
				}
			}						
			for (q = sorted; q != 0;  q = q->next) {
				uintptr_t slot;

				slot = (uintptr_t)q->entry;
				psy_table_insert(&worked, slot, 0);				
			}
			psy_list_append(&sorted, (void*)(uintptr_t)UINTPTR_MAX);
		}
		psy_table_dispose(&worked);
		psy_list_free(self->path);
		self->path = sorted;		
	}	
}

bool isleaf(psy_audio_Machines* self, uintptr_t slot, psy_Table* worked)
{
	psy_audio_MachineSockets* sockets;

	sockets = psy_audio_connections_at(&self->connections, slot);
	if (!sockets || (wiresockets_size(&sockets->inputs) == 0)) {
		return TRUE;
	} else {	
		bool rv = TRUE;
		psy_TableIterator it;

		for (it = psy_audio_wiresockets_begin(&sockets->inputs);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {

			psy_audio_WireSocket* source;

			source = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
			if (!psy_table_exists(worked, source->slot)) {
				rv = FALSE;
				break;
			}
		}
		return rv;
	}
	return TRUE;
}

void reset_nopath(psy_audio_Machines* self)
{		
	psy_TableIterator it;

	psy_table_dispose(&self->nopath);
	psy_table_init(&self->nopath);
		
	for (it = psy_table_begin(&self->slots); it.curr != 0;
			psy_tableiterator_inc(&it)) {
		psy_table_insert(&self->nopath, it.curr->key, 0);		
	}
}

void remove_nopath(psy_audio_Machines* self, uintptr_t slot)
{
	psy_table_remove(&self->nopath, slot);
}

MachineList* nopath(psy_audio_Machines* self)
{
	psy_List* rv = 0;	

	psy_TableIterator it;
	for (it = psy_table_begin(&self->nopath); it.curr != 0;
			psy_tableiterator_inc(&it)) {			
		psy_list_append(&rv, (void*)it.curr->key);
	}
	return rv;
}

MachineList* psy_audio_compute_path(psy_audio_Machines* self, uintptr_t slot, bool concat)
{
	MachineList* rv = 0;	

	reset_nopath(self);	
	psy_table_init(&self->colours);
	compute_slotpath(self, slot, &rv);
	psy_table_dispose(&self->colours);
	//	Debug Path Output
	// if (rv) {
	// 	psy_List* p;

	// 	for (p = rv; p != NULL; psy_list_next(&p)) {
	// 		TRACE_INT((int)(p->entry));
	// 	}
	// 	OutputDebugString("\n");
	// }
	if (concat) {
		psy_list_cat(&rv, nopath(self));
	}
	return rv;
}

void compute_slotpath(psy_audio_Machines* self, uintptr_t slot,
	psy_List** path)
{	
	psy_audio_MachineSockets* sockets;	

	sockets = psy_audio_connections_at(&self->connections, slot);	
	if (sockets) {
		psy_TableIterator it;

		for (it = psy_audio_wiresockets_begin(&sockets->inputs);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_audio_WireSocket* socket;

			socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
			if (!psy_table_exists(&self->colours, socket->slot)) {
				psy_table_insert(&self->colours, socket->slot, (void*) 1);
				compute_slotpath(self, socket->slot, path);
			} else {
				// cycle detected				
				// skip
			}			
			psy_table_remove(&self->colours, socket->slot);
		}	
	}
	if (psy_table_exists(&self->nopath, slot)) {
		remove_nopath(self, slot);
		psy_list_append(path, (void*)slot);
	}
}

void machines_preparebuffers(psy_audio_Machines* self, MachineList* path,
	uintptr_t amount)
{
	psy_TableIterator it;
	
	for (it = psy_audio_machines_begin(self);
			!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {			
		psy_audio_Machine* machine;

		machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
		if (machine) {
			psy_table_insert(&self->outputbuffers, psy_tableiterator_key(&it),
				psy_list_append(&self->buffers, machines_nextbuffer(
					self, psy_audio_machine_numoutputs(machine)))->entry);
		}
	}	
}

void machines_releasebuffers(psy_audio_Machines* self)
{
	psy_table_dispose(&self->inputbuffers);
	psy_table_dispose(&self->outputbuffers);
	psy_table_init(&self->inputbuffers);
	psy_table_init(&self->outputbuffers);
	machines_freebuffers(self);
	self->currsamplebuffer = 0;	
}

psy_audio_Buffer* machines_nextbuffer(psy_audio_Machines* self,
	uintptr_t channels)
{
	uintptr_t channel;
	psy_audio_Buffer* rv;

	rv = psy_audio_buffer_allocinit(channels);
	for (channel = 0; channel < channels; ++channel) {
		float* samples;
				
		samples = self->samplebuffers
			? self->samplebuffers + (self->currsamplebuffer *
				psy_audio_MAX_STREAM_SIZE)
			: 0;
		++self->currsamplebuffer;
		if (self->currsamplebuffer >= self->numsamplebuffers) {
			self->currsamplebuffer = 0;
		}
		rv->samples[channel] = samples;		
	}
	return rv;
}

void machines_freebuffers(psy_audio_Machines* self)
{
	psy_List* p;
	psy_List* q;

	for (p = q = self->buffers; p != NULL; psy_list_next(&p)) {
		psy_audio_Buffer* buffer;

		buffer = (psy_audio_Buffer*)psy_list_entry(p);
		psy_audio_buffer_dispose(buffer);
		free(buffer);	
	}
	psy_list_free(q);
	self->buffers = 0;
}

psy_audio_Buffer* psy_audio_machines_inputs(psy_audio_Machines* self,
	uintptr_t slot)
{
	return psy_table_at(&self->inputbuffers, slot);
}

psy_audio_Buffer* psy_audio_machines_outputs(psy_audio_Machines* self,
	uintptr_t slot)
{	
	return psy_table_at(&self->outputbuffers, slot);
}

void psy_audio_machines_changeslot(psy_audio_Machines* self, uintptr_t slot)
{
	self->slot = slot;
	psy_signal_emit(&self->signal_slotchange, self, 1, slot);
}

void psy_audio_machines_changetweakparam(psy_audio_Machines* self,
	uintptr_t param)
{
	self->tweakparam = param;
}

uintptr_t psy_audio_machines_slot(psy_audio_Machines* self)
{
	return self->slot;
}

uintptr_t psy_audio_machines_soloed(psy_audio_Machines* self)
{
	return self->soloed;
}

void psy_audio_machines_solo(psy_audio_Machines* self, uintptr_t slot)
{
	if (self->filemode != 0) {
		self->soloed = slot;
	} else
	if (self->soloed == slot) {
		psy_TableIterator it;

		self->soloed = UINTPTR_MAX;
		for (it = psy_audio_machines_begin(self);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_audio_Machine* machine;

			machine = (psy_audio_Machine*)psy_tableiterator_value(&it);
			if ((psy_audio_machine_mode(machine) == MACHMODE_GENERATOR)) {
				psy_audio_machine_unmute(machine);
			}
		}
	} else {
		psy_TableIterator it;

		self->soloed = slot;
		for (it = psy_audio_machines_begin(self); !psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
			psy_audio_Machine* machine;

			machine = (psy_audio_Machine*)psy_tableiterator_value(&it);											
			if ((psy_audio_machine_mode(machine) == MACHMODE_GENERATOR) &&
				psy_tableiterator_key(&it) != slot) {
				psy_audio_machine_mute(machine);
			} else
			if (psy_tableiterator_key(&it) == slot) {
				psy_audio_machine_unmute(machine);
			}
		}
	}
}

psy_audio_Machine* psy_audio_machines_master(psy_audio_Machines* self)
{
	return self->master;
}

void psy_audio_machines_startfilemode(psy_audio_Machines* self)
{
	self->filemode = 1;
	self->connections.filemode = 1;
}

void psy_audio_machines_endfilemode(psy_audio_Machines* self)
{
	machines_setpath(self, psy_audio_compute_path(self,
		psy_audio_MASTER_INDEX, TRUE));
	self->filemode = 0;	
	self->connections.filemode = 0;
}

uintptr_t psy_audio_machines_size(psy_audio_Machines* self)
{
	return psy_table_size(&self->slots);
}

psy_TableIterator psy_audio_machines_begin(psy_audio_Machines* self)
{
	return psy_table_begin(&self->slots);
}

bool psy_audio_machines_ismixersend(psy_audio_Machines* self, uintptr_t slot)
{
	return psy_table_exists(&self->connections.sends, slot);
}

void psy_audio_machines_addmixersend(psy_audio_Machines* self, uintptr_t slot)
{
	psy_table_insert(&self->connections.sends, slot, (void*)(uintptr_t)1);
}

void psy_audio_machines_removemixersend(psy_audio_Machines* self,
	uintptr_t slot)
{
	psy_table_remove(&self->connections.sends, slot);
}

void  psy_audio_machines_connectasmixersend(psy_audio_Machines* self)
{
	self->mixersendconnect = TRUE;
}

void  psy_audio_machines_connectasmixerinput(psy_audio_Machines* self)
{
	self->mixersendconnect = FALSE;
}

bool  psy_audio_machines_isconnectasmixersend(psy_audio_Machines* self)
{
	return self->mixersendconnect;
}
