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
	int Version;							
	int Flags;								
	char const *Name;				
	char const *ShortName;
} EventDriverInfo;


typedef enum {
	EVENTDRIVER_KEYDOWN,
	EVENTDRIVER_KEYUP
} EventType;

typedef struct 
{	
	intptr_t message;
	intptr_t param1;
	intptr_t param2;
} EventDriverData;

typedef struct {
	int id;
	EventDriverData data;
} EventDriverCmd;

typedef struct psy_EventDriver {
	psy_Properties* properties;	
	int (*open)(struct psy_EventDriver*);
	int (*dispose)(struct psy_EventDriver*);
	void (*free)(struct psy_EventDriver*);	
	void (*configure)(struct psy_EventDriver*, psy_Properties*);
	int (*close)(struct psy_EventDriver*);	
	void (*write)(struct psy_EventDriver*, EventDriverData);
	void (*cmd)(struct psy_EventDriver*, const char* section, EventDriverData, EventDriverCmd*);
	int (*error)(int, const char*);
	EventDriverCmd (*getcmd)(struct psy_EventDriver*, const char* section);
	void (*setcmddef)(struct psy_EventDriver*, psy_Properties*);
	int (*idle)(struct psy_EventDriver*);
	psy_Signal signal_input;
} psy_EventDriver;

INLINE uintptr_t psy_audio_encodeinput(uintptr_t keycode, bool shift, bool ctrl)
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
#else
typedef psy_EventDriver* (*pfneventdriver_create)(void);
#endif

#ifndef __x86_64
EXPORT psy_EventDriver* __cdecl eventdriver_create(void);
#else
psy_EventDriver* eventdriver_create(void);
#endif

#ifndef __x86_64
EXPORT EventDriverInfo const * __cdecl GetPsycleEventDriverInfo(void);
#else
EventDriverInfo const * GetPsycleEventDriverInfo(void);
#endif

#endif
