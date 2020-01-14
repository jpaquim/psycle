// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../detail/os.h"

#if !defined(PSY_AUDIODRIVER_H)
#define PSY_AUDIODRIVER_H

#ifndef DLL_DRIVER_EXPORTS
#define DLL_DRIVER_EXPORTS

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

#endif

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

typedef struct psy_AudioDriver {
	AUDIODRIVERWORKFN _pCallback;	
	void* _callbackContext;
	psy_Properties* properties;
	int (*open)(struct psy_AudioDriver*);
	int (*dispose)(struct psy_AudioDriver*);
	void (*deallocate)(struct psy_AudioDriver*);	
	void (*configure)(struct psy_AudioDriver*, psy_Properties*);	
	int (*close)(struct psy_AudioDriver*);
	void (*connect)(struct psy_AudioDriver*, void* context,
		AUDIODRIVERWORKFN callback,
		void* handle);
	unsigned int (*samplerate)(struct psy_AudioDriver*);
	psy_Signal signal_stop;
} psy_AudioDriver;

typedef psy_AudioDriver* (__cdecl *pfndriver_create)(void);

EXPORT psy_AudioDriver* __cdecl driver_create(void);

EXPORT AudioDriverInfo const * __cdecl GetPsycleDriverInfo(void);


#endif
