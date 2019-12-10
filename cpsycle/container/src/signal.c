// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "signal.h"
#include <stdlib.h>
#include <stdarg.h>          

static void signal_notify(Signal* self, void* sender);
static void signal_notify1(Signal* self, void* sender, void* param1);
static void signal_notify2(Signal* self, void* sender, void* param1, void* param2);
static void signal_notify3(Signal* self, void* sender, void* param1, void* param2, void* param3);
static void signal_notify_int(Signal* self, void* sender, intptr_t param);
static Slot* signal_findslot(Signal* self, void* context, void* fp);

typedef void (*signalcallback0)(void*, void*);
typedef void (*signalcallback_int)(void*, void*, intptr_t);
typedef void (*signalcallback_float)(void*, void*, float);
typedef void (*signalcallback1)(void*, void*, void*);
typedef void (*signalcallback2)(void*, void*, void*, void*);
typedef void (*signalcallback3)(void*, void*, void*, void*, void*);

void signal_init(Signal* self)
{
	self->slots = 0;
}

void signal_dispose(Signal* self)
{
	if (self->slots) {
		List* ptr = self->slots;
		while (ptr) {				
			Slot* slot = (Slot*) ptr->entry;
			free(slot);
			ptr = ptr->next;
		}
		list_free(self->slots);
	}
	self->slots = 0;	
}

void signal_connect(Signal* self, void* context, void* fp)
{
	List* p;
	int connected = 0;

	p = self->slots;
	while (p != 0) {
		Slot* slot = (Slot*) p->entry;
		if (slot->context == context && slot->fp == fp) {
			connected = 1;
			break;
		}
		p = p->next;
	}
	if (!connected) {
		Slot* slot;
		
		slot = (Slot*)malloc(sizeof(Slot));
		slot->context = context;
		slot->fp = fp;
		slot->prevented = 0;
		list_append(&self->slots, slot);
	}
}

void signal_disconnect(Signal* self, void* context, void* fp)
{
	List* p;	

	p = self->slots;
	while (p != 0) {		
		Slot* slot = (Slot*) p->entry;
		if (slot->context == context && slot->fp == fp) {
			list_remove(&self->slots, p);
			break;
		}
		p = p->next;
	}
}

void signal_disconnectall(Signal* self)
{
	signal_dispose(self);
}

void signal_prevent(Signal* self, void* context, void* fp)
{
	Slot* slot;
	
	slot = signal_findslot(self, context, fp);
	if (slot) {
		slot->prevented = 1;
	}
}

void signal_enable(Signal* self, void* context, void* fp)
{
	Slot* slot;
	
	slot = signal_findslot(self, context, fp);
	if (slot) {
		slot->prevented = 0;
	}
}

Slot* signal_findslot(Signal* self, void* context, void* fp)
{
	Slot* rv = 0;

	if (self->slots) {
		List* p = self->slots;
		for (p = self->slots; p != 0; p = p->next) {
			Slot* slot = (Slot*) p->entry;
			if (slot->context == context && slot->fp == fp) {
				rv = slot;
				break;
			}				
		}
	}
	return rv;
}

void signal_emit_int(Signal* self, void* context, intptr_t param)
{
	if (self->slots) {
		List* p = self->slots;
		for (p = self->slots; p != 0; p = p->next) {
			Slot* slot = (Slot*) p->entry;
			if (!slot->prevented) {
				((signalcallback_int)slot->fp)(slot->context, context, param);
			}
		}
	}
}

void signal_emit_float(Signal* self, void* context, float param)
{
	if (self->slots) {
		List* p = self->slots;
		for (p = self->slots; p != 0; p = p->next) {
			Slot* slot = (Slot*) p->entry;
			if (!slot->prevented) {
				((signalcallback_float)slot->fp)(slot->context, context, param);
			}
		}
	}
}

void signal_emit(Signal* self, void* context, int num, ...)
{
	va_list ap;
	va_start(ap, num);	
	if (num == 0) {
		signal_notify(self, context);
	} else 
	if (num == 1) {
		signal_notify1(self, context, va_arg(ap, void*));
	} else 
	if (num == 2) {
		void* arg1 = va_arg(ap, void*);
		void* arg2 = va_arg(ap, void*);
		signal_notify2(self, context, arg1, arg2);
	} else
	if (num == 3) {
		void* arg1 = va_arg(ap, void*);
		void* arg2 = va_arg(ap, void*);
		void* arg3 = va_arg(ap, void*);
		signal_notify3(self, context, arg1, arg2, arg3);
	}
	va_end(ap);
}

void signal_notify(Signal* self, void* sender)
{
	if (self->slots) {
		List* p;
		List* q;
				
		for (p = self->slots; p != 0; p = q) {			
			Slot* slot = (Slot*) p->entry;

			q = p->next;
			if (!slot->prevented) {
				((signalcallback0)slot->fp)(slot->context, sender);
			}			
		}
	}
}

void signal_notify_int(Signal* self, void* sender, intptr_t param)
{
	if (self->slots) {
		List* ptr = self->slots;
		while (ptr) {				
			Slot* slot = (Slot*) ptr->entry;
			if (!slot->prevented) {
				((signalcallback_int)slot->fp)(slot->context, sender, param);
			}
			ptr = ptr->next;
		}
	}
}

void signal_notify1(Signal* self, void* sender, void* param)
{
	if (self->slots) {
		List* ptr = self->slots;
		while (ptr) {
			Slot* slot = (Slot*)ptr->entry;
			if (!slot->prevented) {
				((signalcallback1)slot->fp)(slot->context, sender, param);
			}
			ptr = ptr->next;
		}
	}
}

void signal_notify2(Signal* self, void* sender, void* param1, void* param2)
{
	if (self->slots) {
		List* ptr = self->slots;
		while (ptr) {				
			Slot* slot = (Slot*) ptr->entry;
			if (!slot->prevented) {
				((signalcallback2)slot->fp)(slot->context, sender, param1, param2);
			}
			ptr = ptr->next;
		}
	}
}

void signal_notify3(Signal* self, void* sender, void* param1, void* param2, void* param3)
{
	if (self->slots) {
		List* ptr = self->slots;
		while (ptr) {				
			Slot* slot = (Slot*) ptr->entry;
			if (!slot->prevented) {
				((signalcallback3)slot->fp)(slot->context, sender, param1, param2, param3);
			}
			ptr = ptr->next;
		}
	}
}
