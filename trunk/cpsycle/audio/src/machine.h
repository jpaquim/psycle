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

#define NOMACHINE_INDEX UINTPTR_MAX

typedef struct MachineCallback {	
	unsigned int (*samplerate)(void*);
	unsigned int (*bpm)(void*);
	psy_dsp_beat_t (*beatspersample)(void*);
	struct psy_audio_Samples* (*samples)(void*);
	struct psy_audio_Machines* (*machines)(void*);
	struct psy_audio_Instruments* (*instruments)(void*);	
	void (*fileselect_load)(void*);
	void (*fileselect_save)(void*);
	void (*fileselect_directory)(void*);
	void* context;	
} MachineCallback;

typedef enum {
	MACHINE_PARAMVIEW_COMPACT = 1
} MachineViewOptions;

struct psy_audio_Machine;

typedef	void (*fp_machine_init)(struct psy_audio_Machine*);
typedef	struct psy_audio_Machine* (*fp_machine_clone)(struct psy_audio_Machine*);
typedef	psy_audio_Buffer* (*fp_machine_mix)(struct psy_audio_Machine*, size_t slot, unsigned int amount, psy_audio_MachineSockets*, struct psy_audio_Machines*);
typedef	void (*fp_machine_work)(struct psy_audio_Machine*, psy_audio_BufferContext*);
typedef	void (*fp_machine_generateaudio)(struct psy_audio_Machine*, psy_audio_BufferContext*);
typedef	int (*fp_machine_hostevent)(struct psy_audio_Machine*, int const eventNr, int const val1, float const val2);
typedef	void (*fp_machine_seqtick)(struct psy_audio_Machine*,
	uintptr_t channel, const psy_audio_PatternEvent*);
typedef	void (*fp_machine_stop)(struct psy_audio_Machine*);
typedef	void (*fp_machine_sequencertick)(struct psy_audio_Machine*);
typedef	void (*fp_machine_sequencerlinetick)(struct psy_audio_Machine*);
typedef	psy_List* (*fp_machine_sequencerinsert)(struct psy_audio_Machine*, psy_List* events);
typedef	void (*fp_machine_setpanning)(struct psy_audio_Machine*, psy_dsp_amp_t);
typedef	psy_dsp_amp_t (*fp_machine_panning)(struct psy_audio_Machine*);
typedef	void (*fp_machine_mute)(struct psy_audio_Machine*);	
typedef	void (*fp_machine_unmute)(struct psy_audio_Machine*);
typedef	int (*fp_machine_muted)(struct psy_audio_Machine*);
typedef	void (*fp_machine_bypass)(struct psy_audio_Machine*);
typedef	void (*fp_machine_unbypass)(struct psy_audio_Machine*);
typedef	int (*fp_machine_bypassed)(struct psy_audio_Machine*);
typedef	const psy_audio_MachineInfo* (*fp_machine_info)(struct psy_audio_Machine*);
typedef	void (*fp_machine_dispose)(struct psy_audio_Machine*);
typedef	int (*fp_machine_mode)(struct psy_audio_Machine*);
typedef	void (*fp_machine_updatesamplerate)(struct psy_audio_Machine*, unsigned int samplerate);
typedef	uintptr_t (*fp_machine_numinputs)(struct psy_audio_Machine*);
typedef	uintptr_t (*fp_machine_numoutputs)(struct psy_audio_Machine*);
typedef	uintptr_t (*fp_machine_slot)(struct psy_audio_Machine*);
typedef	void (*fp_machine_setslot)(struct psy_audio_Machine*, uintptr_t);
	// Parameters	
typedef	int (*fp_machine_parametertype)(struct psy_audio_Machine*, int param);
typedef	uintptr_t (*fp_machine_numparameters)(struct psy_audio_Machine*);
typedef	unsigned int (*fp_machine_numparametercols)(struct psy_audio_Machine*);
typedef	void (*fp_machine_parameterrange)(struct psy_audio_Machine*,
	int numparam, int* minval, int* maxval);
typedef	void (*fp_machine_parametertweak)(struct psy_audio_Machine*, int par, int val);	
typedef	void (*fp_machine_patterntweak)(struct psy_audio_Machine*, int par, int val);
typedef	int (*fp_machine_parameterlabel)(struct psy_audio_Machine*, char* txt, int param);
typedef	int (*fp_machine_parametername)(struct psy_audio_Machine*, char* txt, int param);
typedef	int (*fp_machine_describevalue)(struct psy_audio_Machine*, char* txt, int param, int value);
typedef	int (*fp_machine_parametervalue)(struct psy_audio_Machine*, int param);	
typedef	int (*fp_machine_paramviewoptions)(struct psy_audio_Machine*);
typedef	void (*fp_machine_setcallback)(struct psy_audio_Machine*, MachineCallback);
typedef	void (*fp_machine_loadspecific)(struct psy_audio_Machine*,
	struct psy_audio_SongFile*, uintptr_t slot);
typedef	void (*fp_machine_savespecific)(struct psy_audio_Machine*,
	struct psy_audio_SongFile*, uintptr_t slot);
typedef	int (*fp_machine_haseditor)(struct psy_audio_Machine*);
typedef	void (*fp_machine_seteditorhandle)(struct psy_audio_Machine*, void* handle);
typedef	void (*fp_machine_editorsize)(struct psy_audio_Machine*, int* width, int* height);
typedef	void (*fp_machine_editoridle)(struct psy_audio_Machine*);
typedef	const char* (*fp_machine_editname)(struct psy_audio_Machine*);
typedef	void (*fp_machine_seteditname)(struct psy_audio_Machine*, const char* name);
typedef	struct psy_audio_Buffer* (*fp_machine_buffermemory)(struct psy_audio_Machine*);
typedef	uintptr_t (*fp_machine_buffermemorysize)(struct psy_audio_Machine*);
typedef	void (*fp_machine_setbuffermemorysize)(struct psy_audio_Machine*, uintptr_t);
// machine callbacks
typedef	unsigned int (*fp_machine_samplerate)(struct psy_audio_Machine*);
typedef unsigned int (*fp_machine_bpm)(struct psy_audio_Machine*);
typedef psy_dsp_beat_t (*fp_machine_beatspersample)(struct psy_audio_Machine*);
typedef	struct psy_audio_Samples* (*fp_machine_samples)(struct psy_audio_Machine*);
typedef	struct psy_audio_Machines* (*fp_machine_machines)(struct psy_audio_Machine*);
typedef	struct psy_audio_Instruments* (*fp_machine_instruments)(struct psy_audio_Machine*);

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
	fp_machine_beatspersample beatspersample;
	fp_machine_samples samples;
	fp_machine_machines machines;
	fp_machine_instruments instruments;
} MachineVtable;

typedef struct psy_audio_Machine {
	MachineVtable* vtable;
	MachineCallback callback;	
	psy_Signal signal_worked;	
} psy_audio_Machine;

void machine_init(psy_audio_Machine*, MachineCallback);
void machine_dispose(psy_audio_Machine*);
int machine_supports(psy_audio_Machine*, int option);

// virtual calls
int machine_describevalue(psy_audio_Machine*, char* txt, int param, int value);
void machine_parametertweak(psy_audio_Machine*, int par, int val);
void machine_patterntweak(psy_audio_Machine*, int par, int val);
int machine_parametervalue(psy_audio_Machine*, int param);
void machine_parameterrange(struct psy_audio_Machine*,
	int numparam, int* minval, int* maxval);
uintptr_t machine_numparameters(psy_audio_Machine*);
int machine_parameterlabel(psy_audio_Machine*, char* txt, int param);
int machine_parametername(psy_audio_Machine*, char* txt, int param);
void machine_setcallback(psy_audio_Machine*, MachineCallback);
void machine_setpanning(psy_audio_Machine*, psy_dsp_amp_t);
psy_dsp_amp_t machine_panning(struct psy_audio_Machine*);
uintptr_t machine_numinputs(psy_audio_Machine*);
uintptr_t machine_numoutputs(psy_audio_Machine*);

int machine_mode(psy_audio_Machine*);
void machine_mute(psy_audio_Machine*);	
void machine_unmute(psy_audio_Machine*);
int machine_muted(psy_audio_Machine*);
void machine_bypass(psy_audio_Machine*);
void machine_unbypass(psy_audio_Machine*);
int machine_bypassed(psy_audio_Machine*);
int machine_haseditor(psy_audio_Machine*);
void machine_seteditorhandle(psy_audio_Machine*, void* handle);
void machine_editorsize(psy_audio_Machine*, int* width, int* height);
void machine_editoridle(psy_audio_Machine*);
void machine_seteditname(psy_audio_Machine*, const char* name);
const char* machine_editname(psy_audio_Machine*);

unsigned int machine_bpm(struct psy_audio_Machine*);
psy_dsp_beat_t machine_beatspersample(struct psy_audio_Machine*);
unsigned int machine_samplerate(psy_audio_Machine*);
struct psy_audio_Samples* machine_samples(psy_audio_Machine*);
struct psy_audio_Machines* machine_machines(struct psy_audio_Machine*);
struct psy_audio_Instruments* machine_instruments(psy_audio_Machine*);
psy_audio_Buffer* machine_buffermemory(psy_audio_Machine*);
uintptr_t machine_buffermemorysize(psy_audio_Machine*);
#endif
