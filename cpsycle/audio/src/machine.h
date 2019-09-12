// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINE_H)
#define MACHINE_H

#include "plugin_interface.h"
#include <signal.h>
#include "event.h"
#include "buffer.h"
#include <list.h>

typedef enum {
	MACHMODE_GENERATOR = 0,
	MACHMODE_FX = 1,
	MACHMODE_MASTER = 2,
} MachineMode;

typedef enum  {
	MACH_UNDEFINED = -1,
	MACH_MASTER = 0,
	MACH_SINE = 1, ///< now a plugin
	MACH_DIST = 2, ///< now a plugin
	MACH_SAMPLER = 3,
	MACH_DELAY = 4, ///< now a plugin
	MACH_2PFILTER = 5, ///< now a plugin
	MACH_GAIN = 6, ///< now a plugin
	MACH_FLANGER = 7, ///< now a plugin
	MACH_PLUGIN = 8,
	MACH_VST = 9,
	MACH_VSTFX = 10,
	MACH_SCOPE = 11, ///< Test machine. removed
	MACH_XMSAMPLER = 12,
	MACH_DUPLICATOR = 13,
	MACH_MIXER = 14,
	MACH_RECORDER = 15,
	MACH_DUPLICATOR2 = 16,
	MACH_LUA = 17,
	MACH_LADSPA = 18,
	MACH_DUMMY = 255
} MachineType;

#define MACHINE_USES_INSTRUMENTS 64

typedef struct {
	struct Samples* (*samples)(void* callbackcontext);
	void* context;
} MachineCallback;

typedef struct {	
	void (*init)(void*);	
	void (*work)(void *, List* events, int numsamples, int tracks);
	void (*generateaudio)(void *, Buffer* input, Buffer* output, int numsamples, int tracks);
	int (*hostevent)(void*, int const eventNr, int const val1, float const val2);
	void (*seqtick)(void*, int channel, const PatternEvent*);
	void (*sequencertick)(void*);
	void (*parametertweak)(void*, int par, int val);
	int (*describevalue)(void*, char* txt, int const param, int const value);
	int (*value)(void*, int const param);
	void (*setvalue)(void*, int const param, int const value);	
	const CMachineInfo* (*info)(void*);
	void (*dispose)(void*);
	int (*mode)(void*);	
	Buffer inputs;
	Buffer outputs;	
	Signal signal_worked;
	MachineCallback machinecallback;
	void* callbackcontext;
} Machine;

typedef struct {
	Machine machine;	
} Master;

typedef struct {
	Machine machine;
	int mode;
} DummyMachine;

void machine_init(Machine* self);
void machine_dispose(Machine* self);
int machine_supports(Machine* self, int option);

void master_init(Master* self);

void dummymachine_init(DummyMachine* self);

#endif
