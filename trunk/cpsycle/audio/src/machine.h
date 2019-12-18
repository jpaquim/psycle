// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINE_H)
#define MACHINE_H

#include "machinedefs.h"
#include "machineinfo.h"
#include <signal.h>
#include "patternevent.h"
#include "buffercontext.h"
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

struct Machine;

typedef	void (*fp_machine_init)(struct Machine*);
typedef	struct Machine* (*fp_machine_clone)(struct Machine*);
typedef	Buffer* (*fp_machine_mix)(struct Machine*, size_t slot, unsigned int amount, MachineSockets*, struct Machines*);
typedef	void (*fp_machine_work)(struct Machine*, BufferContext*);
typedef	void (*fp_machine_generateaudio)(struct Machine*, BufferContext*);
typedef	int (*fp_machine_hostevent)(struct Machine*, int const eventNr, int const val1, float const val2);
typedef	void (*fp_machine_seqtick)(struct Machine*, int channel, const PatternEvent*);
typedef	void (*fp_machine_stop)(struct Machine*);
typedef	void (*fp_machine_sequencertick)(struct Machine*);
typedef	void (*fp_machine_sequencerlinetick)(struct Machine*);
typedef	List* (*fp_machine_sequencerinsert)(struct Machine*, List* events);
typedef	void (*fp_machine_setpanning)(struct Machine*, psy_dsp_amp_t);
typedef	psy_dsp_amp_t (*fp_machine_panning)(struct Machine*);
typedef	void (*fp_machine_mute)(struct Machine*);	
typedef	void (*fp_machine_unmute)(struct Machine*);
typedef	int (*fp_machine_muted)(struct Machine*);
typedef	void (*fp_machine_bypass)(struct Machine*);
typedef	void (*fp_machine_unbypass)(struct Machine*);
typedef	int (*fp_machine_bypassed)(struct Machine*);
typedef	void (*fp_machine_setvalue)(struct Machine*, int param, int value);	
typedef	const MachineInfo* (*fp_machine_info)(struct Machine*);
typedef	void (*fp_machine_dispose)(struct Machine*);
typedef	int (*fp_machine_mode)(struct Machine*);	
typedef	void (*fp_machine_updatesamplerate)(struct Machine*, unsigned int samplerate);
typedef	unsigned int (*fp_machine_numinputs)(struct Machine*);
typedef	unsigned int (*fp_machine_numoutputs)(struct Machine*);
typedef	uintptr_t (*fp_machine_slot)(struct Machine*);
typedef	void (*fp_machine_setslot)(struct Machine*, uintptr_t);
	// Parameters	
typedef	int (*fp_machine_parametertype)(struct Machine*, int param);
typedef	unsigned int (*fp_machine_numparameters)(struct Machine*);
typedef	unsigned int (*fp_machine_numparametercols)(struct Machine*);
typedef	void (*fp_machine_parameterrange)(struct Machine*, int numparam, int* minval, int* maxval);
typedef	void (*fp_machine_parametertweak)(struct Machine*, int par, int val);	
typedef	void (*fp_machine_patterntweak)(struct Machine*, int par, int val);
typedef	int (*fp_machine_parameterlabel)(struct Machine*, char* txt, int param);
typedef	int (*fp_machine_parametername)(struct Machine*, char* txt, int param);
typedef	int (*fp_machine_describevalue)(struct Machine*, char* txt, int param, int value);
typedef	int (*fp_machine_parametervalue)(struct Machine*, int param);	
typedef	int (*fp_machine_paramviewoptions)(struct Machine*);

typedef	void (*fp_machine_setcallback)(struct Machine*, MachineCallback);
typedef	void (*fp_machine_loadspecific)(struct Machine*, struct SongFile*, unsigned int slot);
typedef	void (*fp_machine_savespecific)(struct Machine*, struct SongFile*, unsigned int slot);
typedef	int (*fp_machine_haseditor)(struct Machine*);
typedef	void (*fp_machine_seteditorhandle)(struct Machine*, void* handle);
typedef	void (*fp_machine_editorsize)(struct Machine*, int* width, int* height);
typedef	void (*fp_machine_editoridle)(struct Machine*);
typedef	const char* (*fp_machine_editname)(struct Machine*);
typedef	void (*fp_machine_seteditname)(struct Machine*, const char* name);
typedef	struct Buffer* (*fp_machine_buffermemory)(struct Machine*);
typedef	uintptr_t (*fp_machine_buffermemorysize)(struct Machine*);
typedef	void (*fp_machine_setbuffermemorysize)(struct Machine*, uintptr_t);
// machine callbacks
typedef	unsigned int (*fp_machine_samplerate)(struct Machine*);
typedef unsigned int (*fp_machine_bpm)(struct Machine*);	
typedef	struct Samples* (*fp_machine_samples)(struct Machine*);
typedef	struct Machines* (*fp_machine_machines)(struct Machine*);
typedef	struct Instruments* (*fp_machine_instruments)(struct Machine*);

typedef struct MachineVtable {
	fp_machine_init init;
	fp_machine_clone clone;
	fp_machine_mix mix;
	fp_machine_work work;
	fp_machine_generateaudio generateaudio;
	fp_machine_hostevent hostevent;
	fp_machine_seqtick seqtick;
	fp_machine_stop stop;
	fp_machine_sequencertick sequencertick;
	fp_machine_sequencerlinetick sequencerlinetick;
	// update sequencer events
	fp_machine_sequencerinsert sequencerinsert;
	fp_machine_setpanning setpanning;
	fp_machine_panning panning;
	fp_machine_mute mute;
	fp_machine_unmute unmute;
	fp_machine_muted muted;
	fp_machine_bypass bypass;
	fp_machine_unbypass unbypass;
	fp_machine_bypassed bypassed;
	fp_machine_setvalue setvalue;
	fp_machine_info info;
	fp_machine_dispose dispose;
	fp_machine_mode mode;	
	fp_machine_updatesamplerate updatesamplerate;
	fp_machine_numinputs numinputs;
	fp_machine_numoutputs numoutputs;
	fp_machine_slot slot;
	fp_machine_setslot setslot;
	// Parameters	
	fp_machine_parametertype parametertype;
	fp_machine_numparameters numparameters;
	fp_machine_numparametercols numparametercols;
	fp_machine_parameterrange parameterrange;
	fp_machine_parametertweak parametertweak;
	fp_machine_patterntweak patterntweak;
	fp_machine_parameterlabel parameterlabel;
	fp_machine_parametername parametername;
	fp_machine_describevalue describevalue;
	fp_machine_parametervalue parametervalue;
	fp_machine_paramviewoptions paramviewoptions;

	fp_machine_setcallback setcallback;
	fp_machine_loadspecific loadspecific;
	fp_machine_savespecific savespecific;
	fp_machine_haseditor haseditor;
	fp_machine_seteditorhandle seteditorhandle;
	fp_machine_editorsize editorsize;
	fp_machine_editoridle editoridle;
	fp_machine_editname editname;
	fp_machine_seteditname seteditname;
	fp_machine_buffermemory buffermemory;
	fp_machine_buffermemorysize buffermemorysize;
	fp_machine_setbuffermemorysize setbuffermemorysize;
	// machine callbacks
	fp_machine_samplerate samplerate;
	fp_machine_bpm bpm;
	fp_machine_samples samples;
	fp_machine_machines machines;
	fp_machine_instruments instruments;
} MachineVtable;

typedef struct Machine {
	MachineVtable* vtable;
	MachineCallback callback;	
	psy_Signal signal_worked;
} Machine;

void machine_init(Machine*, MachineCallback);
void machine_dispose(Machine*);
int machine_supports(Machine*, int option);

#endif
