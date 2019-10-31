// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINE_H)
#define MACHINE_H

#include "machinedefs.h"
#include "machineinfo.h"
#include <signal.h>
#include "patternevent.h"
#include "buffercontext.h"
#include "fileio.h"
#include "connections.h"
#include <dsptypes.h>

typedef struct MachineCallback {	
	unsigned int (*samplerate)(void*);
	unsigned int (*bpm)(void*);
	struct Samples* (*samples)(void*);
	struct Machines* (*machines)(void*);
	struct Instruments* (*instruments)(void*);	
	void* context;
} MachineCallback;

typedef enum {
	MACHINE_PARAMVIEW_COMPACT = 1
} MachineViewOptions;

typedef struct Machine {	
	void (*init)(void*);
	struct Machine* (*clone)(void*);
	Buffer* (*mix)(void*, int slot, unsigned int amount, MachineSockets*, struct Machines*);
	void (*work)(void*, BufferContext*);
	void (*generateaudio)(void *, BufferContext*);
	int (*hostevent)(void*, int const eventNr, int const val1, float const val2);
	void (*seqtick)(void*, int channel, const PatternEvent*);
	void (*sequencertick)(void*); // real sequencer tick
	void (*sequencerlinetick)(void*); // old tick (line based)
	// update sequencer events
	List* (*sequencerinsert)(void*, List* events);
	void (*setpanning)(void*, amp_t);
	amp_t (*panning)(void*);
	void (*setvalue)(void*, int param, int value);	
	const MachineInfo* (*info)(void*);
	void (*dispose)(void*);
	int (*mode)(void*);	
	void (*updatesamplerate)(void*, unsigned int samplerate);
	unsigned int (*numinputs)(void*);
	unsigned int (*numoutputs)(void*);
	unsigned int (*slot)(void*);
	void (*setslot)(void*, int);
	// Parameters	
	int (*parametertype)(void*, int param);
	unsigned int (*numparameters)(void*);
	unsigned int (*numcols)(void*);
	void (*parameterrange)(void*, int numparam, int* minval, int* maxval);
	void (*parametertweak)(void*, int par, int val);	
	void (*patterntweak)(void*, int par, int val);
	int (*parameterlabel)(void*, char* txt, int param);
	int (*parametername)(void*, char* txt, int param);
	int (*describevalue)(void*, char* txt, int param, int value);
	int (*value)(void*, int param);	
	int (*paramviewoptions)(void*);

	void (*setcallback)(void*, MachineCallback);
	void (*loadspecific)(void*, PsyFile*, unsigned int slot, struct Machines*);	
	int (*haseditor)(void*);
	void (*seteditorhandle)(void*, void* handle);
	void (*editorsize)(void*, int* width, int* height);
	void (*editoridle)(void*);
	// machine callbacks
	unsigned int (*samplerate)(void*);
	unsigned int (*bpm)(void*);	
	struct Samples* (*samples)(void*);
	struct Machines* (*machines)(void*);
	struct Instruments* (*instruments)(void*);
			
	MachineCallback callback;
	int bypass;
	int mute;
	amp_t pan;
	int ismixersend;		
	Signal signal_worked;
	size_t buildid;
} Machine;

void machine_init(Machine*, MachineCallback);
void machine_dispose(Machine*);
int machine_supports(Machine*, int option);
void machine_bypass(Machine*);
void machine_unbypass(Machine*);
int machine_bypassed(Machine*);
void machine_mute(Machine*);
void machine_unmute(Machine*);
int machine_muted(Machine*);


#endif
