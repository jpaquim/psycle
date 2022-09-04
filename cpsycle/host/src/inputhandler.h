/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(INPUTHANDLER_H)
#define INPUTHANDLER_H

/* audio */
#include <player.h>
/* container */
#include <list.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef bool (*fp_inputhandler_input)(void* context, void* sender);
typedef bool (*fp_inputhandler_hostcallback)(void* context, int message, void* param1);

#define INPUTHANDLER_STOP TRUE;
#define INPUTHANDLER_CONTINUE FALSE;

typedef enum {
	INPUTHANDLER_IMM = 1,
	INPUTHANDLER_FOCUS = 2	
} InputHandlerType;

typedef enum {
	INPUTHANDLER_HASFOCUS	
} InputHandlerMessage;

/* InputSlot */
typedef struct InputSlot {
	InputHandlerType type;
	psy_EventDriverCmdType cmdtype;
	uintptr_t id;
	void* context;
	char* section;
	fp_inputhandler_input input;
} InputSlot;

void inputslot_init(InputSlot*, InputHandlerType, psy_EventDriverCmdType,
	const char* section, uintptr_t id, void* context, fp_inputhandler_input);
void inputslot_dispose(InputSlot*);

InputSlot* inputslot_alloc(void);
InputSlot* inputslot_allocinit(InputHandlerType type,
	psy_EventDriverCmdType, const char* section,
	uintptr_t id, void* context, fp_inputhandler_input);

/* InputHandler */
typedef struct InputHandler {
	psy_List* slots;
	psy_EventDriverCmd cmd;
	void* hostcontext;
	fp_inputhandler_hostcallback hostcallback;
	psy_EventDriver* sender;
} InputHandler;

void inputhandler_init(InputHandler*);
void inputhandler_dispose(InputHandler*);

void inputhandler_connect(InputHandler*, InputHandlerType type,
	psy_EventDriverCmdType cmdtype, const char* section,
	uintptr_t id, void* context, fp_inputhandler_input);
void inputhandler_connect_host(InputHandler*, void* context,
	fp_inputhandler_hostcallback);
psy_EventDriverCmd inputhandler_cmd(const InputHandler*);
psy_EventDriver* inputhandler_sender(const InputHandler*);

void inputhandler_event_driver_input(InputHandler* self, psy_EventDriver*);

#ifdef __cplusplus
}
#endif

#endif /* INPUTHANDLER_H */
