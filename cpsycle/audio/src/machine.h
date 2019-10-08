// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINE_H)
#define MACHINE_H

#include "machinedefs.h"
#include "plugin_interface.h"
#include <signal.h>
#include "event.h"
#include "buffercontext.h"

typedef struct MachineCallback {	
	unsigned int (*samplerate)(void*);
	unsigned int (*bpm)(void*);
	struct Samples* (*samples)(void*);
	struct Machines* (*machines)(void*);
	struct Instruments* (*instruments)(void*);	
	void* context;
} MachineCallback;

typedef struct Machine {	
	void (*init)(void*);
	struct Machine* (*clone)(void*);
	void (*work)(void*, BufferContext*);
	void (*generateaudio)(void *, BufferContext*);
	int (*hostevent)(void*, int const eventNr, int const val1, float const val2);
	void (*seqtick)(void*, int channel, const PatternEvent*);
	void (*sequencertick)(void*); // real sequencer tick
	void (*sequencerlinetick)(void*); // old tick (line based)
	// update sequencer events
	List* (*sequencerinsert)(void*, List* events);
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
	// machine callbacks
	unsigned int (*samplerate)(void*);
	unsigned int (*bpm)(void*);
	struct Samples* (*samples)(void*);
	struct Machines* (*machines)(void*);
	struct Instruments* (*instruments)(void*);
		
	Signal signal_worked;
	MachineCallback callback;
} Machine;

void machine_init(Machine*, MachineCallback);
void machine_dispose(Machine*);
int machine_supports(Machine*, int option);

#endif
