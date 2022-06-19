/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_CONNECTIONS_H
#define psy_audio_CONNECTIONS_H

/* container */
#include <hashtbl.h>
#include <list.h>
#include <signal.h>
/* dsp */
#include <dsptypes.h>
/* platform */
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** psy_audio_Connections, psy_audio_PinMapping, psy_audio_PinConnection
**
** Defines the wire mapping (psy_audio_Connections) and pinmapping
** (psy_audio_PinMapping) inside a wire. No special wire class has been
** implemented like in mfc psycle. LegacyWire and Wire are only helpers
** for song loading and the machineview ui.
*/

#define psy_audio_MASTER_INDEX 128
#define psy_audio_MAX_STREAM_SIZE 256

/* psy_audio_PinConnection */
typedef struct psy_audio_PinConnection {
	uintptr_t src;
	uintptr_t dst;
} psy_audio_PinConnection;

void psy_audio_pinconnection_init(psy_audio_PinConnection* self);

INLINE void psy_audio_pinconnection_init_all(psy_audio_PinConnection* self,
	uintptr_t src, uintptr_t dst)
{
	self->src = src;
	self->dst = dst;
}

psy_audio_PinConnection* psy_audio_pinconnection_alloc(void);
psy_audio_PinConnection* psy_audio_pinconnection_allocinit_all(uintptr_t src,
	uintptr_t dst);

/* psy_audio_PinMapping */
typedef struct psy_audio_PinMapping {
	psy_List* container;
} psy_audio_PinMapping;

void psy_audio_pinmapping_init(psy_audio_PinMapping*, uintptr_t numchannels);
void psy_audio_pinmapping_dispose(psy_audio_PinMapping*);
void psy_audio_pinmapping_copy(psy_audio_PinMapping*,
	const psy_audio_PinMapping* src);
void psy_audio_pinmapping_clear(psy_audio_PinMapping*);
void psy_audio_pinmapping_autowire(psy_audio_PinMapping*,
	uintptr_t numchannels);
void psy_audio_pinmapping_connect(psy_audio_PinMapping*, uintptr_t src,
	uintptr_t dst);
void psy_audio_pinmapping_disconnect(psy_audio_PinMapping*, uintptr_t src,
	uintptr_t dst);

typedef struct {
	uintptr_t slot;	
	psy_dsp_amp_t volume;
	psy_audio_PinMapping mapping;
	bool mute;
} psy_audio_WireSocket;

psy_audio_WireSocket* psy_audio_wiresocket_allocinit(uintptr_t slot,
	psy_dsp_amp_t volume);
void psy_audio_wiresocket_dispose(psy_audio_WireSocket*);
void psy_audio_wiresocket_copy(psy_audio_WireSocket*,
	psy_audio_WireSocket* src);

typedef struct psy_audio_WireSockets {
	psy_Table sockets;
} psy_audio_WireSockets;

INLINE void wiresockets_init(psy_audio_WireSockets* self)
{
	psy_table_init(&self->sockets);
}

INLINE void wiresockets_dispose(psy_audio_WireSockets* self)
{
	psy_table_dispose_all(&self->sockets, (psy_fp_disposefunc)
		psy_audio_wiresocket_dispose);
}

INLINE void wiresockets_clear(psy_audio_WireSockets* self)
{
	psy_table_dispose_all(&self->sockets, (psy_fp_disposefunc)
		psy_audio_wiresocket_dispose);
	psy_table_init(&self->sockets);
}

INLINE uintptr_t wiresockets_size(const psy_audio_WireSockets* self)
{
	return psy_table_size(&self->sockets);
}

INLINE psy_TableIterator psy_audio_wiresockets_begin(psy_audio_WireSockets* self)
{
	return psy_table_begin(&self->sockets);
}

INLINE psy_audio_WireSocket* psy_audio_wiresockets_at(psy_audio_WireSockets* self,
	uintptr_t connectionsid)
{
	return (psy_audio_WireSocket*)psy_table_at(&self->sockets, connectionsid);
}

INLINE bool wiresockets_empty(const psy_audio_WireSockets* self)
{
	return psy_table_empty(&self->sockets);
}

INLINE void wiresockets_append(psy_audio_WireSockets* self, psy_audio_WireSocket* socket)
{
	if (psy_table_empty(&self->sockets)) {
		psy_table_insert(&self->sockets, 0, socket);
	} else {		
		if (psy_table_exists(&self->sockets, psy_table_maxkey(&self->sockets) + 1)) {
			psy_table_remove(&self->sockets, psy_table_maxkey(&self->sockets) + 1);
		}
		psy_table_insert(&self->sockets, psy_table_maxkey(&self->sockets) + 1, socket);		
	}
}

void wiresockets_insert(psy_audio_WireSockets* self,
	uintptr_t id, psy_audio_WireSocket* socket);
void wiresockets_remove(psy_audio_WireSockets* self,
	psy_audio_WireSocket* socket);

typedef struct {	
	psy_audio_WireSockets inputs;
	psy_audio_WireSockets outputs;
} psy_audio_MachineSockets;

void psy_audio_machinesockets_init(psy_audio_MachineSockets*);
void psy_audio_machinesockets_dispose(psy_audio_MachineSockets*);
void psy_audio_machinesockets_copy(psy_audio_MachineSockets*,
	psy_audio_MachineSockets* src);

psy_audio_WireSocket* psy_audio_connection_at(psy_audio_WireSockets*, uintptr_t slot);
uintptr_t psy_audio_connection_id(psy_audio_WireSockets* sockets, uintptr_t slot);

typedef struct {
	uintptr_t src;
	uintptr_t src_id;
	uintptr_t dst;	
	uintptr_t dst_id;
} psy_audio_Wire;

void psy_audio_wire_init(psy_audio_Wire* self);

INLINE void psy_audio_wire_init_all(psy_audio_Wire* self, uintptr_t src,
	uintptr_t dst)
{
	self->src = src;
	self->dst = dst;
}

psy_audio_Wire psy_audio_wire_make(uintptr_t src, uintptr_t dst);


INLINE psy_audio_Wire psy_audio_wire_makeall(uintptr_t src, uintptr_t src_id,
 uintptr_t dst, uintptr_t dst_id)
{
	psy_audio_Wire rv;

	rv.src = src;
	rv.src_id = src_id;
	rv.dst = dst;
	rv.dst_id = dst_id;
	return rv;
}

INLINE void psy_audio_wire_set(psy_audio_Wire* self, uintptr_t src,
	uintptr_t dst)
{
	self->src = src;
	self->dst = dst;
}

bool psy_audio_wire_valid(const psy_audio_Wire*);
void psy_audio_wire_invalidate(psy_audio_Wire*);


INLINE bool psy_audio_wire_equal(const psy_audio_Wire* self,
	const psy_audio_Wire* other)
{
	return (self->src == other->src) && (self->dst == other->dst);
}

psy_audio_Wire psy_audio_wire_zero(void);

/* psy_audio_Connections */
typedef struct psy_audio_Connections {
	psy_Table container;
	psy_Table sends;
	psy_Signal signal_connected;
	psy_Signal signal_disconnected;
	bool filemode;
} psy_audio_Connections;

void psy_audio_connections_init(psy_audio_Connections*);
void psy_audio_connections_dispose(psy_audio_Connections*);
void psy_audio_connections_copy(psy_audio_Connections*, psy_audio_Connections* src);
psy_audio_MachineSockets* psy_audio_connections_initslot(psy_audio_Connections*,
	uintptr_t slot);
psy_audio_MachineSockets* psy_audio_connections_at(psy_audio_Connections*,
	uintptr_t slot);
// connects the inputs with the outputs of the machine sockets
// (used by machines_remove to keep the wires)
void psy_audio_connections_rewire(psy_audio_Connections*,
	psy_audio_MachineSockets*);
int psy_audio_connections_connect(psy_audio_Connections*, psy_audio_Wire);
void psy_audio_connections_disconnect(psy_audio_Connections*, psy_audio_Wire);
int psy_audio_connections_connected(psy_audio_Connections*, psy_audio_Wire);
void psy_audio_connections_disconnectall(psy_audio_Connections*, uintptr_t slot);
void psy_audio_connections_setwirevolume(psy_audio_Connections*, psy_audio_Wire,
	psy_dsp_amp_t factor);
void psy_audio_connections_setpinmapping(psy_audio_Connections*, psy_audio_Wire,
	const psy_audio_PinMapping*);
psy_dsp_amp_t psy_audio_connections_wirevolume(psy_audio_Connections*, psy_audio_Wire);
psy_audio_WireSocket* psy_audio_connections_input(psy_audio_Connections*,
	psy_audio_Wire);
uintptr_t psy_audio_connections_wireindex(psy_audio_Connections*, psy_audio_Wire);

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_CONNECTIONS_H */
