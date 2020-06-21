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

static psy_audio_WireSocketEntry* wiresocketentry_allocinit(uintptr_t slot,
	psy_dsp_amp_t volume);
static void wiresocketentry_dispose(psy_audio_WireSocketEntry*);

void machinesockets_init(psy_audio_MachineSockets* self)
{
	self->outputs = 0;
	self->inputs = 0;
}

void machinesockets_dispose(psy_audio_MachineSockets* self)
{
	psy_list_deallocate(&self->inputs, (psy_fp_disposefunc)
		wiresocketentry_dispose);
	psy_list_deallocate(&self->outputs, (psy_fp_disposefunc)
		wiresocketentry_dispose);
}

void machinesockets_copy(psy_audio_MachineSockets* self,
	psy_audio_MachineSockets* src)
{
	WireSocket* p;

	machinesockets_dispose(self);
	machinesockets_init(self);
	for (p = src->inputs; p != NULL; psy_list_next(&p)) {
		psy_audio_WireSocketEntry* entry;
		psy_audio_WireSocketEntry* newentry;

		entry = (psy_audio_WireSocketEntry*)psy_list_entry(p);
		newentry = wiresocketentry_allocinit(entry->slot, entry->volume);
		psy_audio_wiresocketentry_copy(newentry, entry);
		psy_list_append(&self->inputs, newentry);
	}	
	for (p = src->outputs; p != NULL; psy_list_next(&p)) {
		psy_audio_WireSocketEntry* entry;
		psy_audio_WireSocketEntry* newentry;

		entry = (psy_audio_WireSocketEntry*)psy_list_entry(p);
		newentry = wiresocketentry_allocinit(entry->slot, entry->volume);
		psy_audio_wiresocketentry_copy(newentry, entry);
		psy_list_append(&self->outputs, newentry);
	}
}

psy_audio_MachineSockets* machinesockets_allocinit(void)
{	
	psy_audio_MachineSockets* rv;
	
	rv = malloc(sizeof(psy_audio_MachineSockets));
	if (rv) {
		machinesockets_init(rv);
	}
	return rv;
}

psy_audio_WireSocketEntry* wiresocketentry_allocinit(uintptr_t slot,
	psy_dsp_amp_t volume)
{
	psy_audio_WireSocketEntry* rv;
		
	rv = (psy_audio_WireSocketEntry*) malloc(sizeof(psy_audio_WireSocketEntry));
	if (rv) {
		rv->slot = slot;
		rv->volume = 1.0f;
		rv->id = -1;
		psy_audio_pinmapping_init(&rv->mapping, 2);		
	}
	return rv;
}

void wiresocketentry_dispose(psy_audio_WireSocketEntry* self)
{	
	psy_audio_pinmapping_dispose(&self->mapping);
}

void psy_audio_wiresocketentry_copy(psy_audio_WireSocketEntry* self,
	psy_audio_WireSocketEntry* src)
{
	self->id = src->id;
	self->slot = src->slot;
	self->volume = src->volume;
	psy_audio_pinmapping_copy(&self->mapping, &src->mapping);
}

WireSocket* connection_at(WireSocket* socket, uintptr_t slot)
{	
	WireSocket* p;
	
	p = socket;
	while (p && ((psy_audio_WireSocketEntry*)(p->entry))->slot != slot) {
		psy_list_next(&p);
	}
	return p;
}

psy_audio_WireSocketEntry* connection_input(psy_audio_Connections* self, uintptr_t outputslot, 
	uintptr_t inputslot)
{
	psy_audio_WireSocketEntry* rv = 0;
	psy_audio_MachineSockets* sockets;

	sockets = connections_at(self, inputslot);	
	if (sockets) {
		WireSocket* input_socket;

		input_socket = connection_at(sockets->inputs, outputslot);
		if (input_socket) {			
			rv = (psy_audio_WireSocketEntry*) input_socket->entry;
		}
	}
	return rv;
}

void connections_init(psy_audio_Connections* self)
{
	psy_table_init(&self->container);
	psy_table_init(&self->sends);
	psy_signal_init(&self->signal_connected);
	psy_signal_init(&self->signal_disconnected);
	self->filemode = 0;
}

void connections_dispose(psy_audio_Connections* self)
{
	psy_table_disposeall(&self->container, (psy_fp_disposefunc)
		machinesockets_dispose);	
	psy_table_dispose(&self->sends);
	psy_signal_dispose(&self->signal_connected);
	psy_signal_dispose(&self->signal_disconnected);
}

void connections_copy(psy_audio_Connections* self, psy_audio_Connections* src)
{	
	psy_TableIterator it;

	psy_table_disposeall(&self->container, (psy_fp_disposefunc)
		machinesockets_dispose);	
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
		machinesockets_copy(newsockets, sockets);
		psy_table_insert(&self->container, psy_tableiterator_key(&it), newsockets);		
	}
	for (it = psy_table_begin(&src->sends);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {				
		psy_table_insert(&self->sends, psy_tableiterator_key(&it), (void*)(uintptr_t)1);
	}
}

psy_audio_MachineSockets* connections_at(psy_audio_Connections* self, uintptr_t slot)
{
	return psy_table_at(&self->container, slot);	
}

psy_audio_MachineSockets* connections_initslot(psy_audio_Connections* self, uintptr_t slot)
{
	psy_audio_MachineSockets* sockets;

	sockets = machinesockets_allocinit();
	psy_table_insert(&self->container, slot, sockets);
	return sockets;
}

int connections_connect(psy_audio_Connections* self, uintptr_t outputslot,
	uintptr_t inputslot)
{
	if (outputslot != inputslot && 
			!connections_connected(self, outputslot, inputslot)) {
		psy_audio_MachineSockets* connections;

		connections = connections_at(self, outputslot);		
		if (!connections) {
			connections = connections_initslot(self, outputslot);
		}
		if (connections) {
			psy_list_append(&connections->outputs,
				wiresocketentry_allocinit(inputslot, 1.f));
		}
		connections = connections_at(self, inputslot);
		if (!connections) {
			connections = connections_initslot(self, inputslot);
		}
		if (connections) {
			psy_list_append(&connections->inputs,
				wiresocketentry_allocinit(outputslot, 1.f));
		}
		if (!self->filemode) {
			psy_signal_emit(&self->signal_connected, self, 2, outputslot,
				inputslot);
		}
		return 1;
	}
	return 0;
}

void connections_disconnect(psy_audio_Connections* self, uintptr_t outputslot, uintptr_t inputslot)
{	
	psy_audio_MachineSockets* connections;

	connections = connections_at(self, outputslot);
	if (connections) {				
		WireSocket* p;

		p = connection_at(connections->outputs, inputslot);		
		if (p) {
			free(psy_list_entry(p));
			psy_list_remove(&connections->outputs, p);			
		}		
		connections = connections_at(self, inputslot);
		if (connections) {
			p = connection_at(connections->inputs, outputslot);
			if (p) {
				free(psy_list_entry(p));
				psy_list_remove(&connections->inputs, p);
			}
			if (!self->filemode) {
				psy_signal_emit(&self->signal_disconnected, self, 2,
					outputslot, inputslot);
			}
		}
	}	
}

void connections_disconnectall(psy_audio_Connections* self, uintptr_t slot)
{
	psy_audio_MachineSockets* connections;
	connections = connections_at(self, slot);
	if (connections) {		
		WireSocket* out;
		WireSocket* in;
				
		out = connections->outputs;
		while (out) {			
			uintptr_t dstslot;
			psy_audio_MachineSockets* dst;
			WireSocket* dstinput;
			dst = connections_at(self,
				((psy_audio_WireSocketEntry*) out->entry)->slot);
			dstslot = ((psy_audio_WireSocketEntry*) out->entry)->slot;
			dstinput = connection_at(dst->inputs, slot);
			free(dstinput->entry);
			psy_list_remove(&dst->inputs, dstinput);
			out = out->next;
			if (!self->filemode) {
				psy_signal_emit(&self->signal_disconnected, self, 2, slot,
					dstslot);
			}
		}
		psy_list_free(connections->outputs);
		connections->outputs = 0;
		
		in = connections->inputs;
		while (in) {			
			psy_audio_MachineSockets* src;
			WireSocket* srcoutput;
			uintptr_t srcslot;

			src = connections_at(self,
				((psy_audio_WireSocketEntry*) in->entry)->slot);
			srcslot = ((psy_audio_WireSocketEntry*) in->entry)->slot;
			srcoutput = connection_at(src->outputs, slot);
			free(srcoutput->entry);
			psy_list_remove(&src->outputs, srcoutput);
			in = in->next;
			if (!self->filemode) {
				psy_signal_emit(&self->signal_disconnected, self, 2, srcslot,
					slot);
			}
		}
		psy_list_free(connections->inputs);
		connections->inputs = 0;		
	}	
}

int connections_connected(psy_audio_Connections* self, uintptr_t outputslot, uintptr_t inputslot)
{	
	WireSocket* p = 0;
	psy_audio_MachineSockets* sockets;	

	sockets = connections_at(self, outputslot);
	if (sockets) {						
		p = connection_at(sockets->outputs, inputslot);
#if defined(_DEBUG)
		if (p) {			
			sockets = connections_at(self, inputslot);
			assert(sockets && sockets->inputs);
			if (sockets && sockets->inputs) {
				p = connection_at(sockets->inputs, outputslot);
			}
			assert(p);
		}
#endif
	}
	return p != NULL;
}

void connections_setpinmapping(psy_audio_Connections* self,
	uintptr_t outputslot,
	uintptr_t inputslot,
	const psy_audio_PinMapping* mapping)
{
	psy_audio_WireSocketEntry* input_entry;

	input_entry = connection_input(self, outputslot, inputslot);
	if (input_entry) {
		psy_audio_pinmapping_copy(&input_entry->mapping, mapping);
	}
}

void connections_setwirevolume(psy_audio_Connections* self, uintptr_t outputslot,
	uintptr_t inputslot, psy_dsp_amp_t factor)
{	
	psy_audio_WireSocketEntry* input_entry;

	input_entry = connection_input(self, outputslot, inputslot);		
	if (input_entry) {
		input_entry->volume = factor;		
	}
}

psy_dsp_amp_t connections_wirevolume(psy_audio_Connections* self, uintptr_t outputslot,
	uintptr_t inputslot)
{	
	psy_audio_WireSocketEntry* input_entry;

	input_entry = connection_input(self, outputslot, inputslot);		
	return input_entry ? input_entry->volume : 1.f;	
}
