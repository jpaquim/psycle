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


typedef struct {
	struct Samples* (*samples)(void* callbackcontext);
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
	CMachineInfo* (*info)(void*);
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

void machine_init(Machine* self);
void machine_dispose(Machine* self);

void master_init(Master* self);

#endif
