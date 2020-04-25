// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include <stdlib.h>
#include <string.h>
#include "connections.h"
#include <assert.h>

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
	WireSocket* p;

	for (p = self->inputs; p != 0; p = p->next) {
		psy_audio_WireSocketEntry* entry;

		entry = (psy_audio_WireSocketEntry*)p->entry;
		wiresocketentry_dispose(entry);
		free(entry);
	}
	psy_list_free(self->inputs);
	self->inputs = 0;
	for (p = self->outputs; p != 0; p = p->next) {
		psy_audio_WireSocketEntry* entry;

		entry = (psy_audio_WireSocketEntry*)p->entry;
		wiresocketentry_dispose(entry);
		free(entry);
	}
	psy_list_free(self->outputs);
	self->outputs = 0;
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
		psy_audio_PinConnection* pins;

		rv->slot = slot;
		rv->volume = 1.0;
		rv->id = -1;
		rv->mapping = 0;
		pins = (psy_audio_PinConnection*)malloc(sizeof(psy_audio_PinConnection));
		if (pins) {
			pins->src = 0;
			pins->dst = 0;
			rv->mapping = psy_list_create(pins);
		}
		pins = (psy_audio_PinConnection*)malloc(sizeof(psy_audio_PinConnection));
		if (pins) {
			pins->src = 1;
			pins->dst = 1;
			psy_list_append(&rv->mapping, pins);
		}
	}
	return rv;
}

void wiresocketentry_dispose(psy_audio_WireSocketEntry* self)
{	
	psy_List* p;

	for (p = self->mapping; p != 0; p = p->next) {
		free(p->entry);
	}
	psy_list_free(self->mapping);
	self->mapping = 0;
}

WireSocket* connection_at(WireSocket* socket, uintptr_t slot)
{	
	WireSocket* p;
	
	p = socket;
	while (p && ((psy_audio_WireSocketEntry*)(p->entry))->slot != slot) {
		p = p->next;
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
	psy_TableIterator it;

	for (it = psy_table_begin(&self->container);
		!psy_tableiterator_equal(&it, psy_table_end()); psy_tableiterator_inc(&it)) {		
		psy_audio_MachineSockets* sockets;

		sockets = (psy_audio_MachineSockets*)psy_tableiterator_value(&it);
		machinesockets_dispose(sockets);
		free(sockets);
	}
	psy_table_dispose(&self->container);
	psy_table_dispose(&self->sends);
	psy_signal_dispose(&self->signal_connected);
	psy_signal_dispose(&self->signal_disconnected);
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

int connections_connect(psy_audio_Connections* self, uintptr_t outputslot, uintptr_t inputslot)
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
			free(p->entry);
			psy_list_remove(&connections->outputs, p);			
		}		
		connections = connections_at(self, inputslot);
		p = connection_at(connections->inputs, outputslot);
		if (p) {		
			free(p->entry);
			psy_list_remove(&connections->inputs, p);			
		}
		if (!self->filemode) {
			psy_signal_emit(&self->signal_disconnected, self, 2,
				outputslot, inputslot);
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
			assert(sockets);
			p = connection_at(sockets->inputs, outputslot);
			assert(p);
		}
#endif
	}
	return p != 0;
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
