/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net
*/

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

/* container */
#include <properties.h>

typedef struct {
	int guid;
	int Version;							
	int Flags;								
	char const *Name;				
	char const *ShortName;
} psy_EventDriverInfo;

struct psy_EventDriver;

typedef void (*EVENTDRIVERWORKFN)(void* context, struct psy_EventDriver* sender);

typedef enum {
	psy_EVENTDRIVER_PRESS,
	psy_EVENTDRIVER_RELEASE	
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

typedef enum {
	psy_EVENTDRIVER_CMD,
	psy_EVENTDRIVER_MIDI
} psy_EventDriverCmdType;

typedef struct {
	psy_EventDriverCmdType type;
	intptr_t id;
	psy_EventDriverMidiData midi;
} psy_EventDriverCmd;

INLINE psy_EventDriverCmd psy_eventdrivercmd_makeid(int id)
{
	psy_EventDriverCmd rv;

	rv.id = id;
	return rv;
}

/* vtable */
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
typedef void (*psy_eventdriver_fp_setcmddef)(struct psy_EventDriver*, const psy_Property*);
typedef void (*psy_eventdriver_fp_idle)(struct psy_EventDriver*);
typedef psy_EventDriverInput (*psy_eventdriver_fp_input)(struct psy_EventDriver*);

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
	psy_eventdriver_fp_setcmddef setcmddef;
	psy_eventdriver_fp_idle idle;
	psy_eventdriver_fp_input input;
} psy_EventDriverVTable;

/* psy_EventDriver */
typedef struct psy_EventDriver {
	psy_EventDriverVTable* vtable;	
	EVENTDRIVERWORKFN callback;
	void* callbackcontext;
} psy_EventDriver;

#define psy_SHIFT_ON	TRUE
#define psy_SHIFT_OFF	FALSE
#define psy_CTRL_ON		TRUE
#define psy_CTRL_OFF	FALSE
#define psy_ALT_ON		TRUE
#define psy_ALT_OFF		FALSE

INLINE uint32_t psy_audio_encodeinput(uint32_t keycode, bool shift, bool
	ctrl, bool alt, bool up)
{
	return keycode | ((uintptr_t)shift << 8) | ((uintptr_t)ctrl << 9) | ((uintptr_t)alt << 10) | ((uintptr_t)up << 11);
}

INLINE void psy_audio_decodeinput(uint32_t input, uint32_t* keycode,
	bool* shift, bool* ctrl, bool* alt, bool* up)
{
	*keycode = input & 0xFF;
	*shift = ((input >> 8) & 0x01) == 0x01;
	*ctrl = ((input >> 9) & 0x01) == 0x01;
	*alt = ((input >> 10) & 0x01) == 0x01;
	*up = ((input >> 11) & 0x01) == 0x01;
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

/* opens the driver */
INLINE int psy_eventdriver_open(psy_EventDriver* self)
{
	return self->vtable->open(self);
}

/* closes the driver */
INLINE int psy_eventdriver_close(psy_EventDriver* self)
{
	return self->vtable->close(self);
}

/* disposes the driver */
INLINE int psy_eventdriver_dispose(psy_EventDriver* self)
{
	return self->vtable->dispose(self);
}

/* releases the driver */
INLINE void psy_eventdriver_release(psy_EventDriver* self)
{
	assert(self);

	self->vtable->dispose(self);
	self->vtable->deallocate(self);
}

/* deallocates the driver */
INLINE void psy_eventdriver_deallocate(psy_EventDriver* self)
{
	assert(self);

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

INLINE void psy_eventdriver_setcmddef(psy_EventDriver* self, psy_Property*
	properties)
{
	self->vtable->setcmddef(self, properties);
}

INLINE void psy_eventdriver_idle(psy_EventDriver* self)
{
	self->vtable->idle(self);
}

INLINE psy_EventDriverInput psy_eventdriver_input(psy_EventDriver* self)
{
	return self->vtable->input(self);
}

INLINE void psy_eventdriver_connect(psy_EventDriver* self, void* context,
	EVENTDRIVERWORKFN callback)
{
	self->callback = callback;
	self->callbackcontext = context;	
}

#endif /* PSY_EVENTDRIVER_H */
