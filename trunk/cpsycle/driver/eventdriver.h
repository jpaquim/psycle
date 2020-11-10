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

typedef struct {
	int guid;
	int Version;							
	int Flags;								
	char const *Name;				
	char const *ShortName;
} psy_EventDriverInfo;

typedef enum {
	EVENTDRIVER_KEYDOWN,
	EVENTDRIVER_KEYUP
} psy_EventType;

typedef struct 
{	
	intptr_t message;
	intptr_t param1;
	intptr_t param2;
} psy_EventDriverData;

typedef struct {
	int id;
	psy_EventDriverData data;
} psy_EventDriverCmd;

typedef struct psy_EventDriverVTable {
	int (*open)(struct psy_EventDriver*);
	int (*dispose)(struct psy_EventDriver*);
	void (*free)(struct psy_EventDriver*);
	const psy_EventDriverInfo* (*info)(struct psy_EventDriver*);
	void (*configure)(struct psy_EventDriver*, psy_Property*);
	int (*close)(struct psy_EventDriver*);
	void (*write)(struct psy_EventDriver*, psy_EventDriverData);
	void (*cmd)(struct psy_EventDriver*, const char* section, psy_EventDriverData,
		psy_EventDriverCmd*);
	int (*error)(int, const char*);
	psy_EventDriverCmd(*getcmd)(struct psy_EventDriver*, const char* section);
	void (*setcmddef)(struct psy_EventDriver*, psy_Property*);
	void (*idle)(struct psy_EventDriver*);
} psy_EventDriverVTable;

typedef struct psy_EventDriver {
	psy_EventDriverVTable* vtable;
	psy_Property* properties;	
	psy_Signal signal_input;
} psy_EventDriver;

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
	self->vtable->free(self);
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

INLINE int psy_eventdriver_close(psy_EventDriver* self)
{
	return self->vtable->close(self);
}

INLINE void psy_eventdriver_write(psy_EventDriver* self, psy_EventDriverData input)
{
	self->vtable->write(self, input);
}

INLINE void psy_eventdriver_cmd(psy_EventDriver* self, const char* section,
	psy_EventDriverData input, psy_EventDriverCmd* cmd)
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

#endif
