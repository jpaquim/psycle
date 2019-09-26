// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#ifdef __cplusplus
  #define EXPORT extern "C" __declspec (dllexport)
#else
#define EXPORT __declspec (dllexport)
#endif

#if !defined(DRIVER_H)
#define DRIVER_H

#include <properties.h>

typedef struct {
	int Version;							// VERSION
	int Flags;								// Flags	
	char const *Name;						// "Windows MME Driver"
	char const *ShortName;					// "MME"	
} AudioDriverInfo;

typedef float* (*AUDIODRIVERWORKFN)(void* context, int* numSamples);

typedef struct Driver {
	AUDIODRIVERWORKFN _pCallback;
	Properties* properties;
	int (*open)(struct Driver*);
	void (*free)(struct Driver*);
	int (*init)(struct Driver*);
	void (*updateconfiguration)(struct Driver*);
	int (*dispose)(struct Driver*);
	int (*close)(struct Driver*);
	void (*connect)(struct Driver*, void* context, AUDIODRIVERWORKFN callback, void* handle);
	unsigned int (*samplerate)(struct Driver*);
} Driver;


typedef EXPORT Driver* (__cdecl *pfndriver_create)(void);

EXPORT Driver* __cdecl driver_create(void);

EXPORT AudioDriverInfo const * __cdecl GetPsycleDriverInfo(void);

#endif
