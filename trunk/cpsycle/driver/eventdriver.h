// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#ifdef __cplusplus
  #define EXPORT extern "C" __declspec (dllexport)
#else
#ifdef _WIN32
#define EXPORT __declspec (dllexport)
#else
#define EXPORT
#define __cdecl __attribute__((__cdecl__))
#endif
#endif

#if !defined(EVENTDRIVER_H)
#define EVENTDRIVER_H

#include <properties.h>
#include <signal.h>

typedef struct {
	int Version;							// VERSION
	int Flags;								// Flags	
	char const *Name;						// "Windows MME Driver"
	char const *ShortName;					// "MME"	
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

typedef struct EventDriver {	
	psy_Properties* properties;	
	int (*open)(struct EventDriver*);
	int (*dispose)(struct EventDriver*);
	void (*free)(struct EventDriver*);	
	void (*configure)(struct EventDriver*);	
	int (*close)(struct EventDriver*);	
	void (*write)(struct EventDriver*, EventDriverData);
	void (*cmd)(struct EventDriver*, EventDriverData, EventDriverCmd*);
	int (*error)(int, const char*);
	EventDriverCmd (*getcmd)(struct EventDriver*, psy_Properties* section);
	void (*setcmddef)(struct EventDriver*, psy_Properties*);
	psy_Signal signal_input;
} EventDriver;


typedef EXPORT EventDriver* (__cdecl *pfneventdriver_create)(void);

EXPORT EventDriver* __cdecl eventdriver_create(void);

EXPORT EventDriverInfo const * __cdecl GetPsycleEventDriverInfo(void);

#endif
