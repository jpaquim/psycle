// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include <stdlib.h>
#include <string.h>
#include "connections.h"
#include <assert.h>

void psy_audio_wire_init(psy_audio_Wire* self)
{
	self->src = UINTPTR_MAX;
	self->dst = UINTPTR_MAX;
}

bool psy_audio_wire_valid(const psy_audio_Wire* self)
{
	return self->src != UINTPTR_MAX && self->dst != UINTPTR_MAX;
}

void psy_audio_wire_invalidate(psy_audio_Wire* self)
{
	self->src = UINTPTR_MAX;
	self->dst = UINTPTR_MAX;
}

static psy_List* psy_audio_pinmapping_findnode(psy_audio_PinMapping*, uintptr_t src,
	uintptr_t dst);

void psy_audio_pinconnection_init(psy_audio_PinConnection* self)
{
	self->src = UINTPTR_MAX;
	self->dst = UINTPTR_MAX;
}

psy_audio_PinConnection* psy_audio_pinconnection_alloc(void)
{
	return (psy_audio_PinConnection*)malloc(sizeof(psy_audio_PinConnection));
}

psy_audio_PinConnection* psy_audio_pinconnection_allocinit_all(uintptr_t src,
	uintptr_t dst)
{
	psy_audio_PinConnection* rv;

	rv = psy_audio_pinconnection_alloc();
	if (rv) {
		psy_audio_pinconnection_init_all(rv, src, dst);
	}
	return rv;
}

void psy_audio_pinmapping_init(psy_audio_PinMapping* self, uintptr_t numchannels)
{
	self->container = NULL;
	if (numchannels > 0) {
		psy_audio_pinmapping_autowire(self, numchannels);
	}
}

void psy_audio_pinmapping_dispose(psy_audio_PinMapping* self)
{
	psy_audio_pinmapping_clear(self);
}

void psy_audio_pinmapping_copy(psy_audio_PinMapping* self, const psy_audio_PinMapping* src)
{
	if (src) {
		psy_List* p;

		psy_audio_pinmapping_clear(self);
		for (p = src->container; p != NULL; psy_list_next(&p)) {
			psy_audio_PinConnection* pair;
			psy_audio_PinConnection* paircopy;

			pair = (psy_audio_PinConnection*)psy_list_entry(p);
			paircopy = psy_audio_pinconnection_allocinit_all(pair->src, pair->dst);
			if (paircopy) {
				psy_list_append(&self->container, paircopy);
			}
		}		
	}
}

void psy_audio_pinmapping_clear(psy_audio_PinMapping* self)
{
	psy_list_deallocate(&self->container, (psy_fp_disposefunc)NULL);
}

void psy_audio_pinmapping_autowire(psy_audio_PinMapping* self,
	uintptr_t numchannels)
{
	uintptr_t channel;

	psy_audio_pinmapping_clear(self);
	for (channel = 0; channel < numchannels; ++channel) {		
		psy_list_append(&self->container,
			psy_audio_pinconnection_allocinit_all(channel, channel));		
	}
}

void psy_audio_pinmapping_connect(psy_audio_PinMapping* self, uintptr_t src,
	uintptr_t dst)
{
	psy_audio_pinmapping_disconnect(self, src, dst);
	psy_list_append(&self->container,
		psy_audio_pinconnection_allocinit_all(src, dst));	
}

void psy_audio_pinmapping_disconnect(psy_audio_PinMapping* self, uintptr_t src,
	uintptr_t dst)
{
	psy_List* pair;

	pair = psy_audio_pinmapping_findnode(self, src, dst);
	if (pair) {
		free(pair->entry);
		psy_list_remove(&self->container, pair);
	}
}

psy_List* psy_audio_pinmapping_findnode(psy_audio_PinMapping* self, uintptr_t src,
	uintptr_t dst)
{
	psy_List* rv;

	for (rv = self->container; rv != NULL; rv = rv->next) {
		psy_audio_PinConnection* pinconnection;

		pinconnection = (psy_audio_PinConnection*)rv->entry;
		if (pinconnection->src == src && pinconnection->dst == dst) {
			break;
		}
	}
	return rv;	
}

void psy_audio_machinesockets_init(psy_audio_MachineSockets* self)
{
	wiresockets_init(&self->inputs);
	wiresockets_init(&self->outputs);	
}

void psy_audio_machinesockets_dispose(psy_audio_MachineSockets* self)
{
	wiresockets_dispose(&self->inputs);
	wiresockets_dispose(&self->outputs);
}

void psy_audio_machinesockets_copy(psy_audio_MachineSockets* self,
	psy_audio_MachineSockets* src)
{
	psy_TableIterator it;	

	psy_audio_machinesockets_dispose(self);
	psy_audio_machinesockets_init(self);
	for (it = psy_audio_wiresockets_begin(&src->inputs);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		psy_audio_WireSocket* socket;
		psy_audio_WireSocket* newsocket;
		

		socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
		newsocket = psy_audio_wiresocket_allocinit(socket->slot, socket->volume);
		psy_audio_wiresocket_copy(newsocket, socket);
		psy_table_insert(&self->inputs.sockets, psy_tableiterator_key(&it), newsocket);		
	}
	for (it = psy_audio_wiresockets_begin(&src->outputs);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		psy_audio_WireSocket* socket;
		psy_audio_WireSocket* newsocket;


		socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
		newsocket = psy_audio_wiresocket_allocinit(socket->slot, socket->volume);
		psy_audio_wiresocket_copy(newsocket, socket);
		psy_table_insert(&self->outputs.sockets, psy_tableiterator_key(&it), newsocket);
	}	
}

psy_audio_MachineSockets* machinesockets_allocinit(void)
{	
	psy_audio_MachineSockets* rv;
	
	rv = malloc(sizeof(psy_audio_MachineSockets));
	if (rv) {
		psy_audio_machinesockets_init(rv);
	}
	return rv;
}

psy_audio_WireSocket* psy_audio_wiresocket_allocinit(uintptr_t slot,
	psy_dsp_amp_t volume)
{
	psy_audio_WireSocket* rv;
		
	rv = (psy_audio_WireSocket*) malloc(sizeof(psy_audio_WireSocket));
	if (rv) {
		rv->slot = slot;
		rv->volume = 1.0f;		
		psy_audio_pinmapping_init(&rv->mapping, 2);		
	}
	return rv;
}

void psy_audio_wiresocket_dispose(psy_audio_WireSocket* self)
{	
	psy_audio_pinmapping_dispose(&self->mapping);
}

void psy_audio_wiresocket_copy(psy_audio_WireSocket* self,
	psy_audio_WireSocket* src)
{	
	self->slot = src->slot;
	self->volume = src->volume;
	psy_audio_pinmapping_copy(&self->mapping, &src->mapping);
}

psy_audio_WireSocket* psy_audio_connection_at(psy_audio_WireSockets* sockets, uintptr_t slot)
{	
	psy_TableIterator it;
	
	for (it = psy_audio_wiresockets_begin(sockets);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		psy_audio_WireSocket* socket;		

		socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
		if (socket->slot == slot) {
			break;
		}
	}
	if (!psy_tableiterator_equal(&it, psy_table_end())) {
		return psy_tableiterator_value(&it);
	}
	return NULL;
}

uintptr_t psy_audio_connection_id(psy_audio_WireSockets* sockets, uintptr_t slot)
{
	psy_TableIterator it;

	for (it = psy_audio_wiresockets_begin(sockets);
		!psy_tableiterator_equal(&it, psy_table_end());
		psy_tableiterator_inc(&it)) {
		psy_audio_WireSocket* socket;

		socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
		if (socket->slot == slot) {
			return psy_tableiterator_key(&it);
		}
	}	
	return UINTPTR_MAX;
}

psy_audio_WireSocket* psy_audio_connections_input(psy_audio_Connections* self, psy_audio_Wire wire)
{	
	psy_audio_MachineSockets* sockets;

	sockets = psy_audio_connections_at(self, wire.dst);	
	if (sockets) {
		return psy_audio_connection_at(&sockets->inputs, wire.src);		
	}
	return NULL;
}

uintptr_t psy_audio_connections_wireindex(psy_audio_Connections* self, psy_audio_Wire wire)
{		
	psy_audio_MachineSockets* sockets;

	sockets = psy_audio_connections_at(self, wire.src);
	if (sockets) {
		return psy_audio_connection_id(&sockets->outputs, wire.dst);
	}
	return UINTPTR_MAX;
}

void psy_audio_connections_init(psy_audio_Connections* self)
{
	psy_table_init(&self->container);
	psy_table_init(&self->sends);
	psy_signal_init(&self->signal_connected);
	psy_signal_init(&self->signal_disconnected);
	self->filemode = 0;
}

void psy_audio_connections_dispose(psy_audio_Connections* self)
{
	psy_table_disposeall(&self->container, (psy_fp_disposefunc)
		psy_audio_machinesockets_dispose);	
	psy_table_dispose(&self->sends);
	psy_signal_dispose(&self->signal_connected);
	psy_signal_dispose(&self->signal_disconnected);
}

void psy_audio_connections_copy(psy_audio_Connections* self, psy_audio_Connections* src)
{	
	psy_TableIterator it;

	psy_table_disposeall(&self->container, (psy_fp_disposefunc)
		psy_audio_machinesockets_dispose);	
	psy_table_dispose(&self->sends);
	psy_table_init(&self->container); 
	psy_table_init(&self->sends);	
	for (it = psy_table_begin(&src->container);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_MachineSockets* sockets;
		psy_audio_MachineSockets* newsockets;

		sockets = (psy_audio_MachineSockets*)psy_tableiterator_value(&it);
		newsockets = machinesockets_allocinit();
		psy_audio_machinesockets_copy(newsockets, sockets);
		psy_table_insert(&self->container, psy_tableiterator_key(&it), newsockets);		
	}
	for (it = psy_table_begin(&src->sends);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {				
		psy_table_insert(&self->sends, psy_tableiterator_key(&it), (void*)(uintptr_t)1);
	}
}

psy_audio_MachineSockets* psy_audio_connections_at(psy_audio_Connections* self, uintptr_t slot)
{
	return psy_table_at(&self->container, slot);	
}

psy_audio_MachineSockets* psy_audio_connections_initslot(psy_audio_Connections* self, uintptr_t slot)
{
	psy_audio_MachineSockets* sockets;

	sockets = machinesockets_allocinit();
	psy_table_insert(&self->container, slot, sockets);
	return sockets;
}

int psy_audio_connections_connect(psy_audio_Connections* self, psy_audio_Wire wire)
{	
	if (wire.src != wire.dst && !psy_audio_connections_connected(self, wire)) {
		psy_audio_MachineSockets* connections;
		
		// source machine
		connections = psy_audio_connections_at(self, wire.src);		
		if (!connections) {
			connections = psy_audio_connections_initslot(self, wire.src);
		}
		if (connections) {
			wiresockets_insert(&connections->outputs, wire.src_id,
				psy_audio_wiresocket_allocinit(wire.dst, 1.f));
		}
		// destination machine
		connections = psy_audio_connections_at(self, wire.dst);
		if (!connections) {
			connections = psy_audio_connections_initslot(self, wire.dst);
		}
		if (connections) {
			wiresockets_insert(&connections->inputs, wire.dst_id,
				psy_audio_wiresocket_allocinit(wire.src, 1.f));			
		}
		// finished
		if (!self->filemode) {
			psy_signal_emit(&self->signal_connected, self, 2, wire.src,
				wire.dst);
		}
		return 1;
	}
	return 0;
}

void psy_audio_connections_disconnect(psy_audio_Connections* self, psy_audio_Wire wire)
{	
	psy_audio_MachineSockets* connections;

	connections = psy_audio_connections_at(self, wire.src);
	if (connections) {				
		psy_audio_WireSocket* socket;

		socket = psy_audio_connection_at(&connections->outputs, wire.dst);
		if (socket) {
			wiresockets_remove(&connections->outputs, socket);			
		}		
		connections = psy_audio_connections_at(self, wire.dst);
		if (connections) {
			socket = psy_audio_connection_at(&connections->inputs, wire.src);
			if (socket) {
				wiresockets_remove(&connections->inputs, socket);				
			}
			if (!self->filemode) {
				psy_signal_emit(&self->signal_disconnected, self, 2,
					wire.src, wire.dst);
			}
		}
	}	
}

void psy_audio_connections_disconnectall(psy_audio_Connections* self, uintptr_t slot)
{
	psy_audio_MachineSockets* connections;
	connections = psy_audio_connections_at(self, slot);	

	if (connections) {	
		psy_TableIterator it;		
						
		while (!wiresockets_empty(&connections->outputs)) {			
			psy_audio_WireSocket* dst;			
			
			it = psy_audio_wiresockets_begin(&connections->outputs);
			dst = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
			psy_audio_connections_disconnect(self,
				psy_audio_wire_make(slot, dst->slot));
		}
		while (!wiresockets_empty(&connections->inputs)) {
			psy_audio_WireSocket* src;

			it = psy_audio_wiresockets_begin(&connections->inputs);
			src = (psy_audio_WireSocket*)psy_tableiterator_value(&it);
			psy_audio_connections_disconnect(self,
				psy_audio_wire_make(src->slot, slot));
		}
	}	
}

int psy_audio_connections_connected(psy_audio_Connections* self, psy_audio_Wire wire)
{	
	psy_audio_WireSocket* p = 0;
	psy_audio_MachineSockets* sockets;	

	sockets = psy_audio_connections_at(self, wire.src);
	if (sockets) {						
		p = psy_audio_connection_at(&sockets->outputs, wire.dst);
#if defined(_DEBUG)
		if (p) {			
			sockets = psy_audio_connections_at(self, wire.dst);
			// assert(sockets && sockets->inputs.sockets);
			if (sockets) {
				p = psy_audio_connection_at(&sockets->inputs, wire.src);
			}
			assert(p);
		}
#endif
	}
	return p != NULL;
}

void psy_audio_connections_setpinmapping(psy_audio_Connections* self,
	psy_audio_Wire wire,
	const psy_audio_PinMapping* mapping)
{
	psy_audio_WireSocket* input_entry;

	input_entry = psy_audio_connections_input(self, wire);
	if (input_entry) {
		psy_audio_pinmapping_copy(&input_entry->mapping, mapping);
	}
}

void psy_audio_connections_setwirevolume(psy_audio_Connections* self,
	psy_audio_Wire wire, psy_dsp_amp_t factor)
{	
	psy_audio_WireSocket* input_entry;

	input_entry = psy_audio_connections_input(self, wire);
	if (input_entry) {
		input_entry->volume = factor;		
	}
}

psy_dsp_amp_t psy_audio_connections_wirevolume(psy_audio_Connections* self,
	psy_audio_Wire wire)
{	
	psy_audio_WireSocket* input_entry;

	input_entry = psy_audio_connections_input(self, wire);
	return (input_entry)
		? input_entry->volume
		: 1.f;	
}
