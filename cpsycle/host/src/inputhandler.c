/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "inputhandler.h"
/* std */
#include <assert.h>
/* platform */
#include "../../detail/portable.h"

/* InputSlot */
/* implementation */
void inputslot_init(InputSlot* self, InputHandlerType type,
	psy_EventDriverCmdType cmdtype, const char* section,
	uintptr_t id, void* context, fp_inputhandler_input input)
{
	assert(self);
	
	self->type = type;
	self->cmdtype = cmdtype;
	self->section = psy_strdup(section);
	self->id = id;
	self->context = context;
	self->input = input;	
}

void inputslot_dispose(InputSlot* self)
{
	free(self->section);
	self->section = NULL;
}

InputSlot* inputslot_alloc(void)
{
	return (InputSlot*)malloc(sizeof(InputSlot));
}

InputSlot* inputslot_allocinit(InputHandlerType type,
	psy_EventDriverCmdType cmdtype, const char* section,
	uintptr_t id, void* context, fp_inputhandler_input  input)
{
	InputSlot* rv;

	rv = inputslot_alloc();
	if (rv) {
		inputslot_init(rv, type, cmdtype, section, id, context, input);
	}
	return rv;
}

/* InputHandler */
/* prototypes */
static void inputhandler_oneventdriverinput(InputHandler* self,
	psy_EventDriver* sender);
static bool inputhandler_sendmessage(InputHandler*, int msg, void* param1);
/* implementation */
void inputhandler_init(InputHandler* self, psy_audio_Player* player,
	void* hostcontext, fp_inputhandler_hostcallback callback)
{
	assert(self);

	self->slots = NULL;
	self->cmd.id = -1;
	self->hostcontext = hostcontext;
	self->hostcallback = callback;
	self->sender = NULL;
	psy_signal_connect(&player->eventdrivers.signal_input,
		self, inputhandler_oneventdriverinput);
}

void inputhandler_dispose(InputHandler* self)
{
	assert(self);
	
	psy_list_deallocate(&self->slots, (psy_fp_disposefunc)inputslot_dispose);
}

void inputhandler_connect(InputHandler* self, InputHandlerType type,
	psy_EventDriverCmdType cmdtype, const char* section, uintptr_t id,
	void* context, fp_inputhandler_input input)
{
	psy_list_append(&self->slots,
		inputslot_allocinit(type, cmdtype, section, id, context, input));
}

void inputhandler_connect_host(InputHandler* self, void* context,
	fp_inputhandler_hostcallback callback)
{
	self->hostcontext = context;
	self->hostcallback = callback;
}

void inputhandler_oneventdriverinput(InputHandler* self,
	psy_EventDriver* sender)
{
	psy_List* p;

	for (p = self->slots; p != NULL; p = p->next) {
		InputSlot* slot;
		bool emit;

		slot = (InputSlot*)p->entry;
		emit = TRUE;
		if (emit && slot->type == INPUTHANDLER_FOCUS) {
			emit = inputhandler_sendmessage(self,
				INPUTHANDLER_HASFOCUS, slot->context);
		}		
		if (emit) {
			self->cmd = psy_eventdriver_getcmd(sender, slot->section);
			self->sender = sender;
			if (self->cmd.type == slot->cmdtype && slot->input(slot->context,
					self)) {
				break;
			}
		}
	}
}

bool inputhandler_sendmessage(InputHandler* self, int msg, void* param1)
{
	if (self->hostcallback) {
		return self->hostcallback(self->hostcontext, msg, param1);
	}
	return FALSE;
}

psy_EventDriverCmd inputhandler_cmd(const InputHandler* self)
{
	return self->cmd;
}

psy_EventDriver* inputhandler_sender(const InputHandler* self)
{
	return self->sender;
}
