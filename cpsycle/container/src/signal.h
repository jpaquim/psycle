/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http:/*psycle.sourceforge.net
*/

#ifndef psy_SIGNAL_H
#define psy_SIGNAL_H

#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** psy_Signal
**
** Allows communication between objects not related to an inheritance hierachy.
** Stores a list of slots containing a function callback and a context
** (destination object) pointer. There is no typesafety for context and sender.
** Before the context becomes invalid (disposed) the context must disconnect
** itself from the signal.
** usage:
** psy_signal_connect:
** adds a slot (context, function callback pointer) to the slot list.
** psy_signal_emit:
** Calls each slot with the context and sender (passed to emit) pointer
** and their additional argument(s) (passed to emit).
**
** Example: psy_ui_Button <>---- psy_Signal signal_clicked
** psy_Button:
** - init/dispose signal
** - App event loop generates a click event and calls
** - psy_signal_emit(&self->signal_clicked, self, 0);
**   1. the Signal, 2. sender (self/Button instance), 3 zero arguments.
**   The slot list delegates the event to its destinations (MainFrame).
** MainFrame <>----- psy_Button
** init: psy_signal_connect(&self->button.signal_clicked, self, onclicked)
** args: 1. the button signal, 2. destination of the click (self/mainframe),
**       3. the function callback pointer called by the button signal
** void onclicked(MainFrame* self, psy_ui_Button* sender) { }
**
** Arguments:
** psy_signal_emit: variable number of generic void* pointers
** psy_signal_emit_int: int argument
** psy_signal_emit_float: float argument
*/

#define psy_SIGNAL_NOPARAMS 0

typedef struct psy_Slot {
	void* context;  /* destination object instance pointer */
	void* fp;       /* function callback pointer           */
	bool prevented; /* prevent emit in slot list traverse  */
	bool abort;     /* abort slot list traverse            */
} psy_Slot;

void psy_slot_init_all(psy_Slot*, void* context, void* fp);

psy_Slot* psy_slot_alloc(void);
psy_Slot* psy_slot_allocinit_all(void* context, void* fp);

/* psy_Signal */
typedef struct psy_Signal {
	psy_List* slots;	
} psy_Signal;

/* init/dispose */
void psy_signal_init(psy_Signal*);
void psy_signal_dispose(psy_Signal*);
/* adds a slot (context, function callback pointer) to the slot list */
void psy_signal_connect(psy_Signal*, void* context, void* fp);
/* returns, if a slot exists, that matches the context and function callback */
bool psy_signal_connected(psy_Signal*, void* context, void* fp);
/* removes a slot (context, function callback pointer) from the slot list */
void psy_signal_disconnect(psy_Signal*, void* context, void* fp);
/* removes all slots with the context */
void psy_signal_disconnect_context(psy_Signal*, void* context);
/* clears the slot list */
void psy_signal_disconnectall(psy_Signal*);
/* excludes a slot called by psy_signal_emit */
void psy_signal_prevent(psy_Signal*, void* context, void* fp);
/* excludes all slots called by psy_signal_emit */
void psy_signal_preventall(psy_Signal*);
/* activates a slot called by psy_signal_emit (default) */
void psy_signal_enable(psy_Signal*, void* context, void* fp);
/* activates all slots called by psy_signal_emit (default) */ 
void psy_signal_enableall(psy_Signal*);
/* returns the slot matching the context and function callback or NULL */
psy_Slot* psy_signal_findslot(psy_Signal*, void* context, void* fp);
/*
** emit functions {
**
** traverses the slot list and calls the callback with the slot context,
** the sender argument and additionaly with num void* arguments
*/
void psy_signal_emit(psy_Signal*, void* sender, intptr_t num, ...);
/* 
** traverses the slot list and calls te callback with the slot context,
** the sender argument and additionaly with one int argument 
*/
void psy_signal_emit_int(psy_Signal*, void* sender, intptr_t param);
/*
** traverses the slot list and calls te callback with the slot context,
** the sender argument and additionaly with one float argument
*/
void psy_signal_emit_float(psy_Signal*, void* sender, float param);
/*
** }
*/

#ifdef __cplusplus
}
#endif

#endif /* psy_SIGNAL_H */
