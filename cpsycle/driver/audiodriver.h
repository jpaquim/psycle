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
	int guid;                // Global unique identifier
	int Version;
	int Flags;
	char const *Name;        // "Windows MME Driver"
	char const *ShortName;   // "MME"	
} psy_AudioDriverInfo;

typedef psy_dsp_amp_t* (*AUDIODRIVERWORKFN)(void* context, int* numSamples,
	int* playing);

struct psy_AudioDriver;

typedef int (*psy_audiodriver_fp_open)(struct psy_AudioDriver*);
typedef bool (*psy_audiodriver_fp_opened)(const struct psy_AudioDriver*);
typedef int (*psy_audiodriver_fp_dispose)(struct psy_AudioDriver*);
typedef void (*psy_audiodriver_fp_deallocate)(struct psy_AudioDriver*);
typedef void (*psy_audiodriver_fp_refresh_ports)(struct psy_AudioDriver*);
typedef void (*psy_audiodriver_fp_configure)(struct psy_AudioDriver*, const psy_Property*);
typedef const psy_Property* (*psy_audiodriver_fp_configuration)(const struct psy_AudioDriver*);
typedef int (*psy_audiodriver_fp_close)(struct psy_AudioDriver*);
typedef void (*psy_audiodriver_fp_connect)(struct psy_AudioDriver*, void* context,
	AUDIODRIVERWORKFN callback,
	void* handle);
typedef psy_dsp_big_hz_t(*psy_audiodriver_fp_samplerate)(struct psy_AudioDriver*);
typedef const char* (*psy_audiodriver_fp_capturename)(struct psy_AudioDriver*, int index);
typedef int (*psy_audiodriver_fp_numcaptures)(struct psy_AudioDriver*);
typedef const char* (*psy_audiodriver_fp_playbackname)(struct psy_AudioDriver*, int index);
typedef int (*psy_audiodriver_fp_numplaybacks)(struct psy_AudioDriver*);
typedef int (*psy_audiodriver_fp_addcapture)(struct psy_AudioDriver*, int index);
typedef int (*psy_audiodriver_fp_removecapture)(struct psy_AudioDriver*, int index);
typedef void (*psy_audiodriver_fp_readbuffers)(struct psy_AudioDriver*, int index,
	float** pleft, float** pright, uintptr_t numsamples);
typedef const psy_AudioDriverInfo* (*psy_audiodriver_fp_info)(struct psy_AudioDriver*);
typedef uintptr_t(*psy_audiodriver_fp_playposinsamples)(struct psy_AudioDriver*);

typedef struct psy_AudioDriverVTable {
	psy_audiodriver_fp_open open;
	psy_audiodriver_fp_opened opened;
	psy_audiodriver_fp_dispose dispose;
	psy_audiodriver_fp_deallocate deallocate;
	psy_audiodriver_fp_refresh_ports refresh_ports;
	psy_audiodriver_fp_configure configure;
	psy_audiodriver_fp_configuration configuration;
	psy_audiodriver_fp_close close;
	psy_audiodriver_fp_samplerate samplerate;
	psy_audiodriver_fp_addcapture addcapture;
	psy_audiodriver_fp_removecapture removecapture;
	psy_audiodriver_fp_readbuffers readbuffers;
	psy_audiodriver_fp_capturename capturename;
	psy_audiodriver_fp_numcaptures numcaptures;
	psy_audiodriver_fp_playbackname playbackname;
	psy_audiodriver_fp_numplaybacks numplaybacks;
	psy_audiodriver_fp_playposinsamples playposinsamples;
	psy_audiodriver_fp_info info;
} psy_AudioDriverVTable;

typedef struct psy_AudioDriver {
	psy_AudioDriverVTable* vtable;
	AUDIODRIVERWORKFN callback;
	void* callbackcontext;
	void* handle;
	psy_Signal signal_stop;
} psy_AudioDriver;

#if defined DIVERSALIS__OS__MICROSOFT

typedef psy_AudioDriver* (__cdecl *pfndriver_create)(void);

EXPORT psy_AudioDriver* __cdecl driver_create(void);
EXPORT psy_AudioDriverInfo const * __cdecl GetPsycleDriverInfo(void);

#else
typedef psy_AudioDriver* (*pfndriver_create)(void);

EXPORT psy_AudioDriver* driver_create(void);
EXPORT psy_AudioDriverInfo const * GetPsycleDriverInfo(void);

#endif

INLINE int psy_audiodriver_open(psy_AudioDriver* self)
{
	return self->vtable->open(self);
}

INLINE bool psy_audiodriver_opened(const psy_AudioDriver* self)
{
	return self->vtable->opened(self);
}

INLINE int psy_audiodriver_dispose(psy_AudioDriver* self)
{
	return self->vtable->dispose(self);
}

INLINE void psy_audiodriver_deallocate(psy_AudioDriver* self)
{
	self->vtable->deallocate(self);
}

INLINE void psy_audiodriver_configure(psy_AudioDriver* self, const psy_Property* properties)
{
	self->vtable->configure(self, properties);
}

INLINE const psy_Property* psy_audiodriver_configuration(const psy_AudioDriver* self)
{
	return self->vtable->configuration(self);
}

INLINE int psy_audiodriver_close(psy_AudioDriver* self)
{
	return self->vtable->close(self);
}

INLINE void psy_audiodriver_connect(psy_AudioDriver* self, void* context,
	AUDIODRIVERWORKFN callback,
	void* handle)
{
	self->callback = callback;
	self->callbackcontext = context;
	self->handle = handle;
}

INLINE psy_dsp_big_hz_t psy_audiodriver_samplerate(psy_AudioDriver* self)
{
	return self->vtable->samplerate(self);
}

INLINE const char* psy_audiodriver_capturename(psy_AudioDriver* self, int index)
{
	return self->vtable->capturename(self, index);
}

INLINE int psy_audiodriver_numcaptures(psy_AudioDriver* self)
{
	return self->vtable->numcaptures(self);
}

INLINE const char* psy_audiodriver_playbackname(psy_AudioDriver* self, int index)
{
	return self->vtable->playbackname(self, index);
}

INLINE int psy_audiodriver_numplaybacks(psy_AudioDriver* self)
{
	return self->vtable->numplaybacks(self);
}

INLINE int psy_audiodriver_addcapture(psy_AudioDriver* self, int index)
{
	return self->vtable->addcapture(self, index);
}

INLINE int psy_audiodriver_removecapture(psy_AudioDriver* self, int index)
{
	return self->vtable->removecapture(self, index);
}

INLINE void psy_audiodriver_readbuffers(psy_AudioDriver* self, int index,
	float** pleft, float** pright, uintptr_t numsamples)
{
	self->vtable->readbuffers(self, index, pleft, pright, numsamples);
}

INLINE uintptr_t psy_audiodriver_playpos_in_samples(psy_AudioDriver* self)
{
	return self->vtable->playposinsamples(self);
}

INLINE const psy_AudioDriverInfo* psy_audiodriver_info(psy_AudioDriver* self)
{
	return self->vtable->info(self);
}

#endif
