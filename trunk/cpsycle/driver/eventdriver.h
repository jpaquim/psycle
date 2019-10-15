// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#ifdef __cplusplus
  #define EXPORT extern "C" __declspec (dllexport)
#else
#define EXPORT __declspec (dllexport)
#endif

#if !defined(EVENTDRIVER_H)
#define EVENTDRIVER_H

#include <properties.h>

typedef struct {
	int Version;							// VERSION
	int Flags;								// Flags	
	char const *Name;						// "Windows MME Driver"
	char const *ShortName;					// "MME"	
} EventDriverInfo;

typedef void (*EVENTDRIVERWORKFN)(void* context, int cmd, unsigned char* data, unsigned int size);

typedef struct EventDriver {
	EVENTDRIVERWORKFN _pCallback;
	void* _callbackContext;
	Properties* properties;
	int (*open)(struct EventDriver*);
	void (*free)(struct EventDriver*);
	int (*init)(struct EventDriver*);
	void (*updateconfiguration)(struct EventDriver*);
	int (*dispose)(struct EventDriver*);
	int (*close)(struct EventDriver*);
	void (*connect)(struct EventDriver*, void* context, EVENTDRIVERWORKFN callback, void* handle);
	unsigned int (*samplerate)(struct EventDriver*);
	int (*error)(int, const char*);
} EventDriver;


typedef EXPORT EventDriver* (__cdecl *pfneventdriver_create)(void);

EXPORT EventDriver* __cdecl eventdriver_create(void);

EXPORT EventDriverInfo const * __cdecl GetPsycleEventDriverInfo(void);

#endif
