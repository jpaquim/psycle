// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "signal.h"
#include <stdlib.h>
#include <stdarg.h>          

static void psy_signal_notify(psy_Signal*, void* sender);
static void psy_signal_notify1(psy_Signal*, void* sender, void* param1);
static void psy_signal_notify2(psy_Signal*, void* sender, void* param1, void* param2);
static void psy_signal_notify3(psy_Signal*, void* sender, void* param1, void* param2, void* param3);
static void psy_signal_notify4(psy_Signal*, void* sender, void* param1, void* param2, void* param3, void* param4);
static void psy_signal_notify_int(psy_Signal*, void* sender, intptr_t param);

typedef void (*signalcallback0)(void*, void*);
typedef void (*signalcallback_int)(void*, void*, intptr_t);
typedef void (*signalcallback_float)(void*, void*, float);
typedef void (*signalcallback1)(void*, void*, void*);
typedef void (*signalcallback2)(void*, void*, void*, void*);
typedef void (*signalcallback3)(void*, void*, void*, void*, void*);
typedef void (*signalcallback4)(void*, void*, void*, void*, void*, void*);

void psy_signal_init(psy_Signal* self)
{
	self->slots = 0;
}

void psy_signal_dispose(psy_Signal* self)
{
	psy_list_deallocate(&self->slots, (psy_fp_disposefunc)NULL);	
}

void psy_signal_connect(psy_Signal* self, void* context, void* fp)
{
	psy_List* p;
	int connected = 0;

	p = self->slots;
	while (p != NULL) {
		psy_Slot* slot = (psy_Slot*) p->entry;
		if (slot->context == context && slot->fp == fp) {
			connected = 1;
			break;
		}
		p = p->next;
	}
	if (!connected) {
		psy_Slot* slot;
		
		slot = (psy_Slot*) malloc(sizeof(psy_Slot));
		if (slot) {
			slot->context = context;
			slot->fp = fp;
			slot->abort = 0;
			slot->prevented = 0;
			psy_list_append(&self->slots, slot);
		}
	}
}

void psy_signal_disconnect(psy_Signal* self, void* context, void* fp)
{
	psy_List* p;	

	p = self->slots;
	while (p != NULL) {		
		psy_Slot* slot = (psy_Slot*) p->entry;
		if (slot->context == context && slot->fp == fp) {
			free(slot);
			psy_list_remove(&self->slots, p);
			break;
		}
		p = p->next;
	}
}

void psy_signal_disconnectall(psy_Signal* self)
{
	psy_signal_dispose(self);
}

void psy_signal_prevent(psy_Signal* self, void* context, void* fp)
{
	psy_Slot* slot;
	
	slot = psy_signal_findslot(self, context, fp);
	if (slot) {
		slot->prevented = 1;
	}
}

void psy_signal_enable(psy_Signal* self, void* context, void* fp)
{
	psy_Slot* slot;
	
	slot = psy_signal_findslot(self, context, fp);
	if (slot) {
		slot->prevented = 0;
	}
}

psy_Slot* psy_signal_findslot(psy_Signal* self, void* context,
	void* fp)
{
	psy_Slot* rv = 0;

	if (self->slots) {
		psy_List* p;

		for (p = self->slots; p != NULL; p = p->next) {
			psy_Slot* slot;
			
			slot = (psy_Slot*) p->entry;
			if (slot->context == context && slot->fp == fp) {				
				rv = slot;
				break;
			}				
		}
	}
	return rv;
}

void psy_signal_emit_int(psy_Signal* self, void* context,
	intptr_t param)
{
	if (self->slots) {
		psy_List* p;

		for (p = self->slots; p != NULL; p = p->next) {
			psy_Slot* slot;
			
			slot = (psy_Slot*) p->entry;
			if (!slot->prevented) {
				((signalcallback_int)slot->fp)(slot->context, context, param);
			}
		}
	}
}

void psy_signal_emit_float(psy_Signal* self, void* context, float param)
{
	if (self->slots) {
		psy_List* p = self->slots;
		for (p = self->slots; p != NULL; p = p->next) {
			psy_Slot* slot;
			
			slot = (psy_Slot*) p->entry;
			if (!slot->prevented) {
				((signalcallback_float)slot->fp)(slot->context, context, param);
			}
		}
	}
}

void psy_signal_emit(psy_Signal* self, void* context, int num, ...)
{
	va_list ap;
	va_start(ap, num);	
	if (num == 0) {
		psy_signal_notify(self, context);
	} else 
	if (num == 1) {
		psy_signal_notify1(self, context, va_arg(ap, void*));
	} else 
	if (num == 2) {
		void* arg1 = va_arg(ap, void*);
		void* arg2 = va_arg(ap, void*);
		psy_signal_notify2(self, context, arg1, arg2);
	} else
	if (num == 3) {
		void* arg1 = va_arg(ap, void*);
		void* arg2 = va_arg(ap, void*);
		void* arg3 = va_arg(ap, void*);
		psy_signal_notify3(self, context, arg1, arg2, arg3);
	} else
	if (num == 4) {
		void* arg1 = va_arg(ap, void*);
		void* arg2 = va_arg(ap, void*);
		void* arg3 = va_arg(ap, void*);
		void* arg4 = va_arg(ap, void*);
		psy_signal_notify4(self, context, arg1, arg2, arg3, arg4);
	}
	va_end(ap);
}

void psy_signal_notify(psy_Signal* self, void* sender)
{
	if (self->slots) {
		psy_List* p;
		psy_List* q;
				
		for (p = self->slots; p != NULL; p = q) {			
			psy_Slot* slot;
			int abort;
			
			slot = (psy_Slot*) p->entry;
			abort = slot->abort;
			q = p->next;
			if (!slot->prevented) {
				((signalcallback0)slot->fp)(slot->context, sender);
				if (abort) {
					break;
				}
			}		
		}
	}
}

void psy_signal_notify_int(psy_Signal* self, void* sender,
	intptr_t param)
{
	if (self->slots) {
		psy_List* ptr = self->slots;
		while (ptr) {				
			psy_Slot* slot = (psy_Slot*) ptr->entry;
			if (!slot->prevented) {
				((signalcallback_int)slot->fp)(slot->context, sender, param);
			}
			ptr = ptr->next;
		}
	}
}

void psy_signal_notify1(psy_Signal* self, void* sender, void* param)
{
	if (self->slots) {
		psy_List* ptr = self->slots;
		while (ptr) {
			psy_Slot* slot = (psy_Slot*) ptr->entry;
			if (!slot->prevented) {
				((signalcallback1)slot->fp)(slot->context, sender, param);
			}
			ptr = ptr->next;
		}
	}
}

void psy_signal_notify2(psy_Signal* self, void* sender, void* param1,
	void* param2)
{
	if (self->slots) {
		psy_List* ptr = self->slots;
		while (ptr) {				
			psy_Slot* slot = (psy_Slot*) ptr->entry;
			if (!slot->prevented) {
				((signalcallback2)slot->fp)(slot->context, sender, param1, param2);
			}
			ptr = ptr->next;
		}
	}
}

void psy_signal_notify3(psy_Signal* self, void* sender, void* param1,
	void* param2, void* param3)
{
	if (self->slots) {
		psy_List* ptr = self->slots;
		while (ptr) {				
			psy_Slot* slot = (psy_Slot*) ptr->entry;
			if (!slot->prevented) {
				((signalcallback3)slot->fp)(slot->context, sender, param1, param2, param3);
			}
			ptr = ptr->next;
		}
	}
}

void psy_signal_notify4(psy_Signal* self, void* sender, void* param1,
	void* param2, void* param3, void* param4)
{
	if (self->slots) {
		psy_List* ptr = self->slots;
		while (ptr) {
			psy_Slot* slot = (psy_Slot*)ptr->entry;
			if (!slot->prevented) {
				((signalcallback4)slot->fp)(slot->context, sender, param1, param2, param3, param4);
			}
			ptr = ptr->next;
		}
	}
}
