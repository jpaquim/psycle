// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINE_H)
#define MACHINE_H

#include "plugin_interface.h"
#include <signal.h>
#include "event.h"
#include "buffercontext.h"

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

typedef struct MachineCallback {
	struct Samples* (*samples)(void*);
	unsigned int (*samplerate)(void*);
	unsigned int (*bpm)(void*);
	struct Machines* (*machines)(void*);
	void* context;
} MachineCallback;

typedef struct Machine {	
	void (*init)(void*);
	struct Machine* (*clone)(void*);
	void (*work)(void*, BufferContext*);
	void (*generateaudio)(void *, BufferContext*);
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
	float (*pan)(void*);
	void (*setpan)(void*, float val);
	void (*updatesamplerate)(void*, unsigned int samplerate);
	unsigned int (*numinputs)(void*);
	unsigned int (*numoutputs)(void*);
	void (*setcallback)(void*, MachineCallback);
		
	Signal signal_worked;
	MachineCallback callback;
} Machine;

typedef struct {
	Machine machine;
	int mode;
} DummyMachine;

void machine_init(Machine*, MachineCallback);
void machine_dispose(Machine*);
int machine_supports(Machine*, int option);

void dummymachine_init(DummyMachine* self, MachineCallback);

#endif
