/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_MACHINESCMDS_H
#define psy_audio_MACHINESCMDS_H

/* local */
#include "machines.h"
/* container */
#include <undoredo.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	psy_Command command;
	psy_audio_Machines* machines;
	psy_audio_Machine* machine;
	uintptr_t slot;
	bool restoreconnection;
	psy_audio_Connections connections;
} InsertMachineCommand;

void insertmachinecommand_dispose(InsertMachineCommand*);

InsertMachineCommand* insertmachinecommand_allocinit(psy_audio_Machines*,
	uintptr_t slot, psy_audio_Machine*);

void insertmachinecommand_execute(InsertMachineCommand*);
void insertmachinecommand_revert(InsertMachineCommand*);

typedef struct {
	psy_Command command;
	psy_audio_Machines* machines;
	psy_audio_Machine* machine;
	uintptr_t slot;
	psy_audio_Connections connections;
} DeleteMachineCommand;

void deletemachinecommand_dispose(DeleteMachineCommand*);

DeleteMachineCommand* deletemachinecommand_allocinit(psy_audio_Machines*,
	uintptr_t slot);

void deletemachinecommand_execute(DeleteMachineCommand*);
void deletemachinecommand_revert(DeleteMachineCommand*);

typedef struct {
	psy_Command command;
	psy_audio_Machines* machines;
	psy_audio_Wire wire;
	psy_dsp_amp_t volume;
	psy_audio_PinMapping pins;
	bool restore;
} ConnectMachineCommand;

void connectmachinecommand_dispose(ConnectMachineCommand*);

ConnectMachineCommand* connectmachinecommand_allocinit(psy_audio_Machines*,
	psy_audio_Wire);

void connectmachinecommand_execute(ConnectMachineCommand*);
void connectmachinecommand_revert(ConnectMachineCommand*);

typedef struct {
	psy_Command command;
	psy_audio_Machines* machines;
	psy_audio_Wire wire;
	psy_dsp_amp_t volume;
	psy_audio_PinMapping pins;
} DisconnectMachineCommand;

void disconnectmachinecommand_dispose(DisconnectMachineCommand*);

DisconnectMachineCommand* disconnectmachinecommand_allocinit(psy_audio_Machines*,
	psy_audio_Wire);

void disconnectmachinecommand_execute(DisconnectMachineCommand*);
void disconnectmachinecommand_revert(DisconnectMachineCommand*);


#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MACHINESCMDS_H */
