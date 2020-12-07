// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifdef __cplusplus
  #define EXPORT extern "C" __declspec (dllexport)
#else
#ifdef _WIN32
#define EXPORT __declspec (dllexport)
#else
#define EXPORT
#ifndef __x86_64
#define __cdecl __attribute__((__cdecl__))
#endif
#endif
#endif

#if !defined(PSY_EVENTDRIVER_H)
#define PSY_EVENTDRIVER_H

#include <properties.h>
#include <signal.h>


#define PSY_EVENTDRIVER_PATTERNEDIT 1
#define PSY_EVENTDRIVER_NOTECOLUMN 2
#define PSY_EVENTDRIVER_SETCHORDMODE 3
#define PSY_EVENTDRIVER_INSERTNOTEOFF 4
#define PSY_EVENTDRIVER_SECTION 5

typedef struct {
	int guid;
	int Version;							
	int Flags;								
	char const *Name;				
	char const *ShortName;
} psy_EventDriverInfo;

typedef enum {
	psy_EVENTDRIVER_KEYDOWN,
	psy_EVENTDRIVER_KEYUP	
} psy_EventDriverInputType;

typedef struct 
{	
	intptr_t message;
	intptr_t param1;
	intptr_t param2;
} psy_EventDriverInput;

INLINE psy_EventDriverInput psy_eventdriverinput_make(intptr_t message,
	intptr_t param1, intptr_t param2)
{
	psy_EventDriverInput rv;

	rv.message = message;
	rv.param1 = param1;
	rv.param2 = param1;
	return rv;
}

typedef struct psy_EventDriverMidiData
{
	unsigned char byte0;
	unsigned char byte1;
	unsigned char byte2;
} psy_EventDriverMidiData;

typedef struct {
	int id;
	psy_EventDriverMidiData midi;
} psy_EventDriverCmd;

INLINE psy_EventDriverCmd psy_eventdrivercmd_makeid(int id)
{
	psy_EventDriverCmd rv;

	rv.id = id;
	return rv;
}

typedef int (*EVENTDRIVERWORKFN)(void* context, int msg, int param1, int param2);

typedef int (*psy_eventdriver_fp_open)(struct psy_EventDriver*);
typedef int (*psy_eventdriver_fp_dispose)(struct psy_EventDriver*);
typedef void (*psy_eventdriver_fp_deallocate)(struct psy_EventDriver*);
typedef void (*psy_eventdriver_fp_configure)(struct psy_EventDriver*, psy_Property*);
typedef const psy_Property* (*psy_eventdriver_fp_configuration)(const struct psy_EventDriver*);
typedef const psy_EventDriverInfo* (*psy_eventdriver_fp_info)(struct psy_EventDriver*);
typedef int (*psy_eventdriver_fp_close)(struct psy_EventDriver*);
typedef void (*psy_eventdriver_fp_write)(struct psy_EventDriver*, psy_EventDriverInput);
typedef void (*psy_eventdriver_fp_cmd)(struct psy_EventDriver*, const char* section, psy_EventDriverInput,
	psy_EventDriverCmd*);
typedef int (*psy_eventdriver_fp_error)(int, const char*);
typedef psy_EventDriverCmd(*psy_eventdriver_fp_getcmd)(struct psy_EventDriver*, const char* section);
typedef const char*(*psy_eventdriver_fp_target)(struct psy_EventDriver*);
typedef void (*psy_eventdriver_fp_setcmddef)(struct psy_EventDriver*, psy_Property*);
typedef void (*psy_eventdriver_fp_idle)(struct psy_EventDriver*);

typedef struct psy_EventDriverVTable {
	psy_eventdriver_fp_open open;
	psy_eventdriver_fp_dispose dispose;
	psy_eventdriver_fp_deallocate deallocate;	
	psy_eventdriver_fp_info info;
	psy_eventdriver_fp_configure configure;
	psy_eventdriver_fp_configuration configuration;
	psy_eventdriver_fp_close close;
	psy_eventdriver_fp_write write;
	psy_eventdriver_fp_cmd cmd;
	psy_eventdriver_fp_error error;
	psy_eventdriver_fp_getcmd getcmd;
	psy_eventdriver_fp_target target;
	psy_eventdriver_fp_setcmddef setcmddef;
	psy_eventdriver_fp_idle idle;
} psy_EventDriverVTable;

typedef struct psy_EventDriver {
	psy_EventDriverVTable* vtable;	
	psy_Signal signal_input;
	EVENTDRIVERWORKFN callback;
	void* callbackcontext;
} psy_EventDriver;

#define psy_SHIFT_ON	TRUE
#define psy_SHIFT_OFF	FALSE
#define psy_CTRL_ON		TRUE
#define psy_CTRL_OFF	FALSE

INLINE uintptr_t psy_audio_encodeinput(uintptr_t keycode, bool shift, bool
	ctrl)
{
	return keycode | ((uintptr_t)shift << 8) | ((uintptr_t)ctrl << 9);
}

INLINE void psy_audio_decodeinput(uintptr_t input, uintptr_t* keycode,
	bool* shift, bool* ctrl)
{
	*keycode = input & 0xFF;
	*shift = ((input >> 8) & 0x01) == 0x01;
	*ctrl = ((input >> 9) & 0x01) == 0x01;
}


#ifndef __x86_64
typedef psy_EventDriver* (__cdecl *pfneventdriver_create)(void);
typedef psy_EventDriverInfo* (__cdecl* pfneventdriver_info)(void);
#else
typedef psy_EventDriver* (*pfneventdriver_create)(void);
typedef psy_EventDriverInfo* (*pfneventdriver_info)(void);
#endif

#ifndef __x86_64
EXPORT psy_EventDriver* __cdecl psy_eventdriver_create(void);
#else
psy_EventDriver* psy_eventdriver_create(void);
#endif

#ifndef __x86_64
EXPORT psy_EventDriverInfo const * __cdecl psy_eventdriver_moduleinfo(void);
#else
psy_EventDriverInfo const * psy_eventdriver_moduleinfo(void);
#endif

INLINE int psy_eventdriver_open(psy_EventDriver* self)
{
	return self->vtable->open(self);
}

INLINE int psy_eventdriver_dispose(psy_EventDriver* self)
{
	return self->vtable->dispose(self);
}

INLINE void psy_eventdriver_free(psy_EventDriver* self)
{
	self->vtable->deallocate(self);
}

INLINE const psy_EventDriverInfo* psy_eventdriver_info(psy_EventDriver* self)
{
	return self->vtable->info(self);
}

INLINE void psy_eventdriver_configure(psy_EventDriver* self, psy_Property*
	properties)
{
	self->vtable->configure(self, properties);
}

INLINE const psy_Property* psy_eventdriver_configuration(const psy_EventDriver* self)
{
	return self->vtable->configuration(self);
}

INLINE int psy_eventdriver_close(psy_EventDriver* self)
{
	return self->vtable->close(self);
}

INLINE void psy_eventdriver_write(psy_EventDriver* self, psy_EventDriverInput input)
{
	self->vtable->write(self, input);
}

INLINE void psy_eventdriver_cmd(psy_EventDriver* self, const char* section,
	psy_EventDriverInput input, psy_EventDriverCmd* cmd)
{
	self->vtable->cmd(self, section, input, cmd);
}

INLINE int psy_eventdriver_error(psy_EventDriver* self, int err,
	const char* text)
{
	return self->vtable->error(err, text);
}

INLINE psy_EventDriverCmd psy_eventdriver_getcmd(psy_EventDriver* self,
	const char* section)
{
	return self->vtable->getcmd(self, section);
}

INLINE const char* psy_eventdriver_target(psy_EventDriver* self)
{
	return self->vtable->target(self);
}

INLINE void psy_eventdriver_setcmddef(psy_EventDriver* self, psy_Property*
	properties)
{
	self->vtable->setcmddef(self, properties);
}

INLINE void psy_eventdriver_idle(psy_EventDriver* self)
{
	self->vtable->idle(self);
}

INLINE void psy_eventdriver_connect(psy_EventDriver* self, void* context,
	EVENTDRIVERWORKFN callback)
{
	self->callbackcontext = context;
	self->callback = callback;
}

INLINE int psy_eventdriver_hostevent(psy_EventDriver* self, int msg, int param1, int param2)
{
	if (self->callback) {
		return self->callback(self->callbackcontext, msg, param1, param2);
	}
	return 0;
}

#endif
