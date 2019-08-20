// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "signal.h"
#include <stdlib.h>
#include <stdarg.h>          

static void signal_notify(Signal* self, void* sender);
static void signal_notify1(Signal* self, void* sender, int param);
static void signal_notify2(Signal* self, void* sender, int param1, int param2);
static void signal_notify3(Signal* self, void* sender, int param1, int param2, int param3);

typedef void (*signalcallback0)(void*, void*);
typedef void (*signalcallback1)(void*, void*, int);
typedef void (*signalcallback2)(void*, void*, int, int);
typedef void (*signalcallback3)(void*, void*, int, int, int);

void signal_init(Signal* self)
{
	self->slots = 0;
}

void signal_connect(Signal* self, void* context, void* fp)
{
	if (self->slots) {
		Slot* node = (Slot*)malloc(sizeof(Slot));
		node->context = context;
		node->fp = fp;
		list_append(self->slots, node);
	} else {
		Slot* node;
		self->slots = list_create();
		node = (Slot*)malloc(sizeof(Slot));
		node->context = context;
		node->fp = fp;
		self->slots->node = node;
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
		signal_notify1(self, context, va_arg(ap, int));
	} else 
	if (num == 2) {
		int arg1 = va_arg(ap, int);
		int arg2 = va_arg(ap, int);
		signal_notify2(self, context, arg1, arg2);
	} else
	if (num == 3) {
		int arg1 = va_arg(ap, int);
		int arg2 = va_arg(ap, int);
		int arg3 = va_arg(ap, int);
		signal_notify3(self, context, arg1, arg2, arg3);
	}
	va_end(ap);
}

void signal_notify(Signal* self, void* sender)
{
	if (self->slots) {
		List* ptr = self->slots;
		while (ptr) {				
			Slot* slot = (Slot*) ptr->node;			
			((signalcallback0)slot->fp)(slot->context, sender);
			ptr = ptr->next;
		}
	}
}

void signal_notify1(Signal* self, void* sender, int param)
{
	if (self->slots) {
		List* ptr = self->slots;
		while (ptr) {				
			Slot* slot = (Slot*) ptr->node;			
			((signalcallback1)slot->fp)(slot->context, sender, param);
			ptr = ptr->next;
		}
	}
}

void signal_notify2(Signal* self, void* sender, int param1, int param2)
{
	if (self->slots) {
		List* ptr = self->slots;
		while (ptr) {				
			Slot* slot = (Slot*) ptr->node;			
			((signalcallback2)slot->fp)(slot->context, sender, param1, param2);
			ptr = ptr->next;
		}
	}
}

void signal_notify3(Signal* self, void* sender, int param1, int param2, int param3)
{
	if (self->slots) {
		List* ptr = self->slots;
		while (ptr) {				
			Slot* slot = (Slot*) ptr->node;			
			((signalcallback2)slot->fp)(slot->context, sender, param1, param2, param3);
			ptr = ptr->next;
		}
	}
}

void signal_dispose(Signal* self)
{
	if (self->slots) {
		List* ptr = self->slots;
		while (ptr) {				
			Slot* slot = (Slot*) ptr->node;			
			free(slot);
			ptr = ptr->next;
		}
		list_free(self->slots);
	}
}
