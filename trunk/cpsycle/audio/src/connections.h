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
} PinConnection;

typedef struct {
	uintptr_t slot;	
	amp_t volume;
	List* mapping;
} WireSocketEntry;

typedef List WireSocket;

typedef struct {	
	WireSocket* inputs;
	WireSocket* outputs;
} MachineSockets;

void machinesockets_init(MachineSockets*);
void machinesockets_dispose(MachineSockets*);

WireSocket* connection_at(WireSocket*, uintptr_t slot);

typedef struct {
	Table container;
	Table sends;
	Signal signal_connected;
	Signal signal_disconnected;
	int filemode;
} Connections;

void connections_init(Connections*);
void connections_dispose(Connections*);
MachineSockets* connections_initslot(Connections*, uintptr_t slot);
MachineSockets* connections_at(Connections*, uintptr_t slot);
int connections_connect(Connections*, uintptr_t outputslot, uintptr_t inputslot);
void connections_disconnect(Connections*, uintptr_t outputslot, uintptr_t inputslot);
int connections_connected(Connections*, uintptr_t outputslot, uintptr_t inputslot);
void connections_disconnectall(Connections*, uintptr_t slot);
void connections_setwirevolume(Connections*, uintptr_t outputslot, uintptr_t inputslot,
	amp_t factor);
amp_t connections_wirevolume(Connections*, uintptr_t outputslot, uintptr_t inputslot);
WireSocketEntry* connection_input(Connections* self, uintptr_t outputslot, uintptr_t inputslot);


#endif
