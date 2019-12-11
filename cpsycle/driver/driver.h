// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../detail/os.h"

#ifdef __cplusplus
  #define EXPORT extern "C" __declspec (dllexport)
#else
#if defined DIVERSALIS__OS__MICROSOFT  
#define EXPORT __declspec (dllexport)
#else
#define EXPORT
#define __cdecl __attribute__((__cdecl__))
#endif
#endif

#if !defined(DRIVER_H)
#define DRIVER_H

#include <properties.h>
#include <signal.h>
#include "../dsp/src/dsptypes.h"

typedef struct {
	int Version;							// VERSION
	int Flags;								// Flags	
	char const *Name;						// "Windows MME Driver"
	char const *ShortName;					// "MME"	
} AudioDriverInfo;

typedef psy_dsp_amp_t* (*AUDIODRIVERWORKFN)(void* context, int* numSamples,
	int* playing);

typedef struct Driver {
	AUDIODRIVERWORKFN _pCallback;	
	void* _callbackContext;
	Properties* properties;
	int (*open)(struct Driver*);
	int (*dispose)(struct Driver*);
	void (*free)(struct Driver*);	
	void (*configure)(struct Driver*, Properties*);	
	int (*close)(struct Driver*);
	void (*connect)(struct Driver*, void* context, AUDIODRIVERWORKFN callback,
		void* handle);
	unsigned int (*samplerate)(struct Driver*);
	psy_Signal signal_stop;
} Driver;


typedef EXPORT Driver* (__cdecl *pfndriver_create)(void);

EXPORT Driver* __cdecl driver_create(void);

EXPORT AudioDriverInfo const * __cdecl GetPsycleDriverInfo(void);

#endif
