// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(CONNECTIONS_H)
#define CONNECTIONS_H

#include <hashtbl.h>
#include <list.h>
#include <signal.h>
#include <dsptypes.h>

#define MASTER_INDEX 128
#define MAX_STREAM_SIZE 256

typedef struct {
	uintptr_t src;
	uintptr_t dst;
} psy_audio_PinConnection;

typedef struct {
	uintptr_t slot;	
	psy_dsp_amp_t volume;
	psy_List* mapping;
} psy_audio_WireSocketEntry;

typedef psy_List WireSocket;

typedef struct {	
	WireSocket* inputs;
	WireSocket* outputs;
} psy_audio_MachineSockets;

void machinesockets_init(psy_audio_MachineSockets*);
void machinesockets_dispose(psy_audio_MachineSockets*);

WireSocket* connection_at(WireSocket*, uintptr_t slot);

typedef struct {
	uintptr_t src;
	uintptr_t dst;
} psy_audio_Wire;

typedef struct {
	psy_Table container;
	psy_Table sends;
	psy_Signal signal_connected;
	psy_Signal signal_disconnected;
	int filemode;
} psy_audio_Connections;

void connections_init(psy_audio_Connections*);
void connections_dispose(psy_audio_Connections*);
psy_audio_MachineSockets* connections_initslot(psy_audio_Connections*, uintptr_t slot);
psy_audio_MachineSockets* connections_at(psy_audio_Connections*, uintptr_t slot);
int connections_connect(psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
void connections_disconnect(psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
int connections_connected(psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
void connections_disconnectall(psy_audio_Connections*, uintptr_t slot);
void connections_setwirevolume(psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot,
	psy_dsp_amp_t factor);
psy_dsp_amp_t connections_wirevolume(psy_audio_Connections*, uintptr_t outputslot, 
	uintptr_t inputslot);
psy_audio_WireSocketEntry* connection_input(psy_audio_Connections* self, uintptr_t outputslot,
	uintptr_t inputslot);

#endif
