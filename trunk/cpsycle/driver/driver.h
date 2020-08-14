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

struct psy_AudioDriver;

typedef int (*psy_audiodriver_fp_open)(struct psy_AudioDriver*);
typedef int (*psy_audiodriver_fp_dispose)(struct psy_AudioDriver*);
typedef void (*psy_audiodriver_fp_deallocate)(struct psy_AudioDriver*);
typedef void (*psy_audiodriver_fp_configure)(struct psy_AudioDriver*, psy_Properties*);
typedef int (*psy_audiodriver_fp_close)(struct psy_AudioDriver*);
typedef void (*psy_audiodriver_fp_connect)(struct psy_AudioDriver*, void* context,
	AUDIODRIVERWORKFN callback,
	void* handle);
typedef unsigned int (*psy_audiodriver_fp_samplerate)(struct psy_AudioDriver*);
typedef const char* (*psy_audiodriver_fp_capturename)(struct psy_AudioDriver*, int index);
typedef int (*psy_audiodriver_fp_numcaptures)(struct psy_AudioDriver*);
typedef const char* (*psy_audiodriver_fp_playbackname)(struct psy_AudioDriver*, int index);
typedef int (*psy_audiodriver_fp_numplaybacks)(struct psy_AudioDriver*);
typedef int (*psy_audiodriver_fp_addcapture)(struct psy_AudioDriver*, int index);
typedef int (*psy_audiodriver_fp_removecapture)(struct psy_AudioDriver*, int index);
typedef void (*psy_audiodriver_fp_readbuffers)(struct psy_AudioDriver*, int index, float** pleft, float** pright, int numsamples);

typedef struct psy_AudioDriver {
	AUDIODRIVERWORKFN _pCallback;	
	void* _callbackContext;
	psy_Properties* properties;
	psy_audiodriver_fp_open open;		
	psy_audiodriver_fp_dispose dispose;
	psy_audiodriver_fp_deallocate deallocate;
	psy_audiodriver_fp_configure configure;
	psy_audiodriver_fp_close close;
	psy_audiodriver_fp_connect connect;
	psy_audiodriver_fp_samplerate samplerate;
	psy_audiodriver_fp_addcapture addcapture;
	psy_audiodriver_fp_removecapture removecapture;
	psy_audiodriver_fp_readbuffers readbuffers;
	psy_audiodriver_fp_capturename capturename;
	psy_audiodriver_fp_numcaptures numcaptures;
	psy_audiodriver_fp_playbackname playbackname;
	psy_audiodriver_fp_numplaybacks numplaybacks;
	psy_Signal signal_stop;
} psy_AudioDriver;

#if defined DIVERSALIS__OS__MICROSOFT

typedef psy_AudioDriver* (__cdecl *pfndriver_create)(void);

EXPORT psy_AudioDriver* __cdecl driver_create(void);
EXPORT AudioDriverInfo const * __cdecl GetPsycleDriverInfo(void);

#else
typedef psy_AudioDriver* (*pfndriver_create)(void);

EXPORT psy_AudioDriver* driver_create(void);
EXPORT AudioDriverInfo const * GetPsycleDriverInfo(void);

#endif

#endif
