// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "signal.h"
// std
#include <stdlib.h>
#include <stdarg.h>          

// psy_Slot
// implementation
void psy_slot_init_all(psy_Slot* self, void* context, void* fp)
{
	assert(self);
	self->context = context;
	self->fp = fp;
	self->prevented = FALSE;
	self->abort = FALSE;
}

psy_Slot* psy_slot_alloc(void)
{
	return (psy_Slot*)malloc(sizeof(psy_Slot));
}

psy_Slot* psy_slot_allocinit_all(void* context, void* fp)
{
	psy_Slot* rv;

	rv = psy_slot_alloc();
	if (rv) {
		psy_slot_init_all(rv, context, fp);
	}
	return rv;
}

// psy_Signal
// prototypes
static void psy_signal_notify(psy_Signal*, void* sender);
static void psy_signal_notify1(psy_Signal*, void* sender, void* param1);
static void psy_signal_notify2(psy_Signal*, void* sender, void* param1,
	void* param2);
static void psy_signal_notify3(psy_Signal*, void* sender, void* param1,
	void* param2, void* param3);
static void psy_signal_notify4(psy_Signal*, void* sender, void* param1,
	void* param2, void* param3, void* param4);
static void psy_signal_notify_int(psy_Signal*, void* sender, intptr_t param);

static psy_List* psy_signal_findslotnode(psy_Signal*, void* context, void* fp);

typedef void (*signalcallback0)(void*, void*);
typedef void (*signalcallback_int)(void*, void*, intptr_t);
typedef void (*signalcallback_float)(void*, void*, float);
typedef void (*signalcallback1)(void*, void*, void*);
typedef void (*signalcallback2)(void*, void*, void*, void*);
typedef void (*signalcallback3)(void*, void*, void*, void*, void*);
typedef void (*signalcallback4)(void*, void*, void*, void*, void*, void*);
// implementation
void psy_signal_init(psy_Signal* self)
{
	assert(self);

	self->slots = NULL;
}

void psy_signal_dispose(psy_Signal* self)
{
	assert(self);

	psy_list_deallocate(&self->slots, (psy_fp_disposefunc)NULL);	
}

void psy_signal_connect(psy_Signal* self, void* context, void* fp)
{	
	assert(self);

	if (!psy_signal_connected(self, context, fp)) {
		psy_Slot* slot;
		
		slot = psy_slot_allocinit_all(context, fp);
		if (slot) {
			psy_list_append(&self->slots, slot);
		}
	}
}

bool psy_signal_connected(psy_Signal* self, void* context, void* fp)
{
	assert(self);

	return psy_signal_findslotnode(self, context, fp) != NULL;
}

void psy_signal_disconnect(psy_Signal* self, void* context, void* fp)
{
	psy_List* p;	

	assert(self);

	p = psy_signal_findslotnode(self, context, self);
	if (p != NULL) {				
		free(psy_list_entry(p));
		psy_list_remove(&self->slots, p);
	}
}

void psy_signal_disconnectall(psy_Signal* self)
{
	assert(self);

	psy_signal_dispose(self);
}

void psy_signal_prevent(psy_Signal* self, void* context, void* fp)
{
	psy_Slot* slot;
	
	assert(self);

	slot = psy_signal_findslot(self, context, fp);
	if (slot) {
		slot->prevented = TRUE;
	}
}

void psy_signal_enable(psy_Signal* self, void* context, void* fp)
{
	psy_Slot* slot;
	
	assert(self);

	slot = psy_signal_findslot(self, context, fp);
	if (slot) {
		slot->prevented = FALSE;
	}
}


psy_Slot* psy_signal_findslot(psy_Signal* self, void* context, void* fp)
{
	psy_List* p;

	assert(self);

	p = psy_signal_findslotnode(self, context, fp);
	return (p)
		? (psy_Slot*)psy_list_entry(p)
		: NULL;
}

psy_List* psy_signal_findslotnode(psy_Signal* self, void* context, void* fp)
{
	psy_List* rv;

	assert(self);

	rv = NULL;
	if (self->slots) {
		psy_List* p;

		for (p = self->slots; p != NULL; psy_list_next(&p)) {
			psy_Slot* slot;

			slot = (psy_Slot*)psy_list_entry(p);
			if (slot->context == context && slot->fp == fp) {
				rv = p;
				break;
			}
		}
	}
	return rv;
}

void psy_signal_emit_int(psy_Signal* self, void* sender,
	intptr_t param)
{
	assert(self);

	if (self->slots) {
		psy_List* p;

		for (p = self->slots; p != NULL; psy_list_next(&p)) {
			psy_Slot* slot;
			
			slot = (psy_Slot*)psy_list_entry(p);
			if (!slot->prevented) {
				((signalcallback_int)slot->fp)(slot->context, sender, param);
			}
		}
	}
}

void psy_signal_emit_float(psy_Signal* self, void* sender, float param)
{
	assert(self);

	if (self->slots) {
		psy_List* p;

		for (p = self->slots; p != NULL; psy_list_next(&p)) {
			psy_Slot* slot;
			
			slot = (psy_Slot*)psy_list_entry(p);
			if (!slot->prevented) {
				((signalcallback_float)slot->fp)(slot->context, sender,
					param);
			}
		}
	}
}

void psy_signal_emit(psy_Signal* self, void* sender, int num, ...)
{
	va_list ap;

	assert(self);

	va_start(ap, num);	
	if (num == 0) {
		psy_signal_notify(self, sender);
	} else 
	if (num == 1) {
		psy_signal_notify1(self, sender, va_arg(ap, void*));
	} else 
	if (num == 2) {
		void* arg1 = va_arg(ap, void*);
		void* arg2 = va_arg(ap, void*);
		psy_signal_notify2(self, sender, arg1, arg2);
	} else
	if (num == 3) {
		void* arg1 = va_arg(ap, void*);
		void* arg2 = va_arg(ap, void*);
		void* arg3 = va_arg(ap, void*);
		psy_signal_notify3(self, sender, arg1, arg2, arg3);
	} else
	if (num == 4) {
		void* arg1 = va_arg(ap, void*);
		void* arg2 = va_arg(ap, void*);
		void* arg3 = va_arg(ap, void*);
		void* arg4 = va_arg(ap, void*);
		psy_signal_notify4(self, sender, arg1, arg2, arg3, arg4);
	}
	va_end(ap);
}

void psy_signal_notify(psy_Signal* self, void* sender)
{
	assert(self);

	if (self->slots) {
		psy_List* p;
		psy_List* q;
				
		for (p = self->slots; p != NULL; p = q) {			
			psy_Slot* slot;
			bool abort;

			slot = (psy_Slot*)psy_list_entry(p);
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
	assert(self);

	if (self->slots) {
		psy_List* p;
		
		p = self->slots;
		while (p != NULL) {
			psy_Slot* slot;

			slot = (psy_Slot*)psy_list_entry(p);
			if (!slot->prevented) {
				((signalcallback_int)slot->fp)(slot->context, sender, param);
			}
			psy_list_next(&p);
		}
	}
}

void psy_signal_notify1(psy_Signal* self, void* sender, void* param)
{
	assert(self);

	if (self->slots) {
		psy_List* p;
		
		p = self->slots;
		while (p != NULL) {
			psy_Slot* slot;
			
			slot = (psy_Slot*)psy_list_entry(p);
			if (!slot->prevented) {
				((signalcallback1)slot->fp)(slot->context, sender, param);
			}
			psy_list_next(&p);
		}
	}
}

void psy_signal_notify2(psy_Signal* self, void* sender, void* param1,
	void* param2)
{
	assert(self);

	if (self->slots) {
		psy_List* p;
		
		p = self->slots;
		while (p != NULL) {
			psy_Slot* slot;
			
			slot = (psy_Slot*)psy_list_entry(p);
			if (!slot->prevented) {
				((signalcallback2)slot->fp)(slot->context, sender, param1,
					param2);
			}
			psy_list_next(&p);
		}
	}
}

void psy_signal_notify3(psy_Signal* self, void* sender, void* param1,
	void* param2, void* param3)
{
	assert(self);

	if (self->slots) {
		psy_List* p;
		
		p = self->slots;
		while (p != NULL) {
			psy_Slot* slot;
			
			slot = (psy_Slot*)psy_list_entry(p);
			if (!slot->prevented) {
				((signalcallback3)slot->fp)(slot->context, sender, param1,
					param2, param3);
			}
			psy_list_next(&p);
		}
	}
}

void psy_signal_notify4(psy_Signal* self, void* sender, void* param1,
	void* param2, void* param3, void* param4)
{
	assert(self);

	if (self->slots) {
		psy_List* p;
		
		p = self->slots;
		while (p != NULL) {
			psy_Slot* slot;
			
			slot = (psy_Slot*)psy_list_entry(p);
			if (!slot->prevented) {
				((signalcallback4)slot->fp)(slot->context, sender, param1,
					param2, param3, param4);
			}
			psy_list_next(&p);
		}
	}
}
