/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "machinescmds.h"
/* local */
#include "exclusivelock.h"
/* container */
#include <properties.h>
/* std */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
/* platform */
#include "../../detail/trace.h"

/* InsertMachineCommand */

/* vtable */
static psy_CommandVtable insertmachinecommand_vtable;
static bool insertmachinecommand_vtable_initialized = FALSE;

static void insertmachinecommand_vtable_init(InsertMachineCommand* self)
{
	if (!insertmachinecommand_vtable_initialized) {
		insertmachinecommand_vtable = *(self->command.vtable);
		insertmachinecommand_vtable.dispose =
			(psy_fp_command)
			insertmachinecommand_dispose;
		insertmachinecommand_vtable.execute =
			(psy_fp_command_params)
			insertmachinecommand_execute;
		insertmachinecommand_vtable.revert =
			(psy_fp_command)
			insertmachinecommand_revert;
		insertmachinecommand_vtable_initialized = TRUE;
	}
	self->command.vtable = &insertmachinecommand_vtable;
}

/* implementation */
InsertMachineCommand* insertmachinecommand_allocinit(psy_audio_Machines*
	machines, uintptr_t slot, psy_audio_Machine* machine)
{
	InsertMachineCommand* rv;

	rv = malloc(sizeof(InsertMachineCommand));
	if (rv) {
		psy_command_init(&rv->command);
		insertmachinecommand_vtable_init(rv);		
		rv->machines = machines;
		rv->machine = machine;
		rv->slot = slot;
		rv->restoreconnection = FALSE;
		psy_audio_connections_init(&rv->connections);
	}
	return rv;
}

void insertmachinecommand_dispose(InsertMachineCommand* self)
{		
	psy_audio_connections_dispose(&self->connections);
}

void insertmachinecommand_execute(InsertMachineCommand* self,
	psy_Property* params)
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
		psy_audio_machines_remove(self->machines, self->slot, FALSE);
		self->machines->preventundoredo = FALSE;
	}	
}

/* DeleteMachineCommand */

/* vtable */

static psy_CommandVtable deletemachinecommand_vtable;
static int deletemachinecommand_vtable_initialized = FALSE;

static void deletemachinecommand_vtable_init(DeleteMachineCommand* self)
{
	if (!deletemachinecommand_vtable_initialized) {
		deletemachinecommand_vtable = *(self->command.vtable);
		deletemachinecommand_vtable.dispose =
			(psy_fp_command)
			deletemachinecommand_dispose;
		deletemachinecommand_vtable.execute =
			(psy_fp_command_params)
			deletemachinecommand_execute;
		deletemachinecommand_vtable.revert =
			(psy_fp_command)
			deletemachinecommand_revert;
		deletemachinecommand_vtable_initialized = TRUE;
	}
	self->command.vtable = &deletemachinecommand_vtable;
}

/* implementation */
DeleteMachineCommand* deletemachinecommand_allocinit(
	psy_audio_Machines* machines, uintptr_t slot)
{
	DeleteMachineCommand* rv;

	rv = malloc(sizeof(DeleteMachineCommand));
	if (rv) {
		psy_command_init(&rv->command);
		deletemachinecommand_vtable_init(rv);		
		rv->machines = machines;
		rv->machine = NULL;
		rv->slot = slot;
		psy_audio_connections_init(&rv->connections);
	}
	return rv;
}

void deletemachinecommand_dispose(DeleteMachineCommand* self)
{	
	psy_audio_connections_dispose(&self->connections);
}

void deletemachinecommand_execute(DeleteMachineCommand* self,
	psy_Property* params)
{
	psy_audio_Machine* machine;

	machine = psy_audio_machines_at(self->machines, self->slot);
	if (machine) {
		psy_audio_connections_dispose(&self->connections);
		psy_audio_connections_init(&self->connections);
		psy_audio_connections_copy(&self->connections, &self->machines->connections);		
		self->machine = psy_audio_machine_clone(machine);
		self->machines->preventundoredo = TRUE;
		psy_audio_machines_remove(self->machines, self->slot, TRUE);
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

/* ConnectMachineCommand */

/* vtable */
static psy_CommandVtable connectmachinecommand_vtable;
static bool connectmachinecommand_vtable_initialized = FALSE;

static void connectmachinecommand_vtable_init(ConnectMachineCommand* self)
{
	if (!connectmachinecommand_vtable_initialized) {
		connectmachinecommand_vtable = *(self->command.vtable);
		connectmachinecommand_vtable.dispose =
			(psy_fp_command)
			connectmachinecommand_dispose;
		connectmachinecommand_vtable.execute =
			(psy_fp_command_params)
			connectmachinecommand_execute;
		connectmachinecommand_vtable.revert =
			(psy_fp_command)
			connectmachinecommand_revert;
		connectmachinecommand_vtable_initialized = TRUE;
	}
	self->command.vtable = &connectmachinecommand_vtable;
}

/* implementation */
ConnectMachineCommand* connectmachinecommand_allocinit(
	psy_audio_Machines* machines, psy_audio_Wire wire)
{
	ConnectMachineCommand* rv;

	rv = malloc(sizeof(ConnectMachineCommand));
	if (rv) {
		psy_command_init(&rv->command);
		connectmachinecommand_vtable_init(rv);		
		rv->machines = machines;
		rv->wire = wire;
		rv->volume = (psy_dsp_amp_t)1.f;
		psy_audio_pinmapping_init(&rv->pins, 2);
		rv->restore = FALSE;
	}
	return rv;
}

void connectmachinecommand_dispose(ConnectMachineCommand* self)
{
	psy_audio_pinmapping_dispose(&self->pins);
}

void connectmachinecommand_execute(ConnectMachineCommand* self,
	psy_Property* params)
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

/* DisconnectMachineCommand */

/* vtable */
static psy_CommandVtable disconnectmachinecommand_vtable;
static bool disconnectmachinecommand_vtable_initialized = FALSE;

static void disconnectmachinecommand_vtable_init(DisconnectMachineCommand* self)
{
	if (!disconnectmachinecommand_vtable_initialized) {
		disconnectmachinecommand_vtable = *(self->command.vtable);
		disconnectmachinecommand_vtable.dispose =
			(psy_fp_command)
			disconnectmachinecommand_dispose;
		disconnectmachinecommand_vtable.execute =
			(psy_fp_command_params)
			disconnectmachinecommand_execute;
		disconnectmachinecommand_vtable.revert =
			(psy_fp_command)
			disconnectmachinecommand_revert;
		disconnectmachinecommand_vtable_initialized = TRUE;
	}
}

/* implementation */
DisconnectMachineCommand* disconnectmachinecommand_allocinit(psy_audio_Machines* machines,
	psy_audio_Wire wire)
{
	DisconnectMachineCommand* rv;

	rv = malloc(sizeof(DisconnectMachineCommand));
	if (rv) {
		psy_command_init(&rv->command);
		disconnectmachinecommand_vtable_init(rv);
		rv->command.vtable = &disconnectmachinecommand_vtable;
		rv->machines = machines;
		rv->wire = wire;
		rv->volume = (psy_dsp_amp_t)1.f;
		psy_audio_pinmapping_init(&rv->pins, 2);
	}
	return rv;
}

void disconnectmachinecommand_dispose(DisconnectMachineCommand* self)
{
	psy_audio_pinmapping_dispose(&self->pins);
}

void disconnectmachinecommand_execute(DisconnectMachineCommand* self,
	psy_Property* params)
{
	psy_audio_WireSocket* socket;

	self->machines->preventundoredo = TRUE;
	self->volume = psy_audio_connections_wirevolume(
		&self->machines->connections, self->wire);
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
