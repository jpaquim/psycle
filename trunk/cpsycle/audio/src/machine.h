// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_audio_MACHINE_H
#define psy_audio_MACHINE_H

#include "machinedefs.h"
#include "machineinfo.h"
#include <signal.h>
#include "patternevent.h"
#include "buffercontext.h"
#include "connections.h"
#include "cputimeclock.h"
#include <dsptypes.h>
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NOMACHINE_INDEX UINTPTR_MAX

typedef unsigned int (*fp_mcb_samplerate)(void*);
typedef	unsigned int (*fp_mcb_bpm)(void*);
typedef	psy_dsp_beat_t (*fp_mcb_beatspersample)(void*);
typedef	psy_dsp_beat_t (*fp_mcb_currbeatsperline)(void*);
typedef	struct psy_audio_Samples* (*fp_mcb_samples)(void*);
typedef	struct psy_audio_Machines* (*fp_mcb_machines)(void*);
typedef	struct psy_audio_Instruments* (*fp_mcb_instruments)(void*);
typedef	struct psy_audio_MachineFactory* (*fp_mcb_machinefactory)(void*);
typedef	void (*fp_mcb_fileselect_load)(void*);
typedef	void (*fp_mcb_fileselect_save)(void*);
typedef	void (*fp_mcb_fileselect_directory)(void*);
typedef	void (*fp_mcb_output)(void*, const char* text);
typedef	bool (*fp_mcb_addcapture)(void*, int index);
typedef	bool (*fp_mcb_removecapture)(void*, int index);
typedef void (*fp_mcb_readbuffers)(void*, int index, float** pleft, float** pright, int numsamples);
typedef const char* (*fp_mcb_capturename)(void*, int index);
typedef int (*fp_mcb_numcaptures)(void*);
typedef const char* (*fp_mcb_playbackname)(void*, int index);
typedef int (*fp_mcb_numplaybacks)(void*);

typedef struct MachineCallback {	
	fp_mcb_samplerate samplerate;
	fp_mcb_bpm bpm;
	fp_mcb_beatspersample beatspersample;
	fp_mcb_currbeatsperline currbeatsperline;
	fp_mcb_samples samples;
	fp_mcb_machines machines;
	fp_mcb_instruments instruments;
	fp_mcb_machinefactory machinefactory;
	fp_mcb_fileselect_load fileselect_load;
	fp_mcb_fileselect_save fileselect_save;
	fp_mcb_fileselect_directory fileselect_directory;
	fp_mcb_output output;
	fp_mcb_addcapture addcapture;
	fp_mcb_removecapture removecapture;
	fp_mcb_readbuffers readbuffers;
	fp_mcb_capturename capturename;
	fp_mcb_numcaptures numcaptures;
	fp_mcb_playbackname playbackname;
	fp_mcb_numplaybacks numplaybacks;
	void* context;	
} MachineCallback;

void machinecallback_initempty(MachineCallback* self);

typedef enum {
	MACHINE_PARAMVIEW_COMPACT = 1
} MachineViewOptions;

struct psy_audio_Machine;
struct psy_audio_SongFile;

typedef	void (*fp_machine_init)(struct psy_audio_Machine*);
typedef	void (*fp_machine_reload)(struct psy_audio_Machine*);
typedef	struct psy_audio_Machine* (*fp_machine_clone)(struct psy_audio_Machine*);
typedef	psy_audio_Buffer* (*fp_machine_mix)(struct psy_audio_Machine*,
	uintptr_t slot, uintptr_t amount, psy_audio_MachineSockets*,
	struct psy_audio_Machines*);
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
typedef	int (*fp_machine_parametertype)(struct psy_audio_Machine*, uintptr_t param);
typedef	uintptr_t (*fp_machine_numparameters)(struct psy_audio_Machine*);
typedef	unsigned int (*fp_machine_numparametercols)(struct psy_audio_Machine*);
typedef	void (*fp_machine_parameterrange)(struct psy_audio_Machine*,
	uintptr_t param, int* minval, int* maxval);
typedef	void (*fp_machine_parametertweak)(struct psy_audio_Machine*, uintptr_t par, float val);	
typedef	void (*fp_machine_patterntweak)(struct psy_audio_Machine*, uintptr_t par, float val);
typedef	int (*fp_machine_parameterlabel)(struct psy_audio_Machine*, char* txt, uintptr_t param);
typedef	int (*fp_machine_parametername)(struct psy_audio_Machine*, char* txt, uintptr_t param);
typedef	int (*fp_machine_describevalue)(struct psy_audio_Machine*, char* txt, uintptr_t param, int value);
typedef	float (*fp_machine_parametervalue)(struct psy_audio_Machine*, uintptr_t param);	
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
typedef	psy_dsp_amp_range_t (*fp_machine_amprange)(struct psy_audio_Machine*);
// programs
typedef void (*fp_machine_programname)(struct psy_audio_Machine*, int bnkidx, int prgIdx, char* val);
typedef int (*fp_machine_numprograms)(struct psy_audio_Machine*);
typedef void (*fp_machine_setcurrprogram)(struct psy_audio_Machine*, int prgIdx);
typedef int (*fp_machine_currprogram)(struct psy_audio_Machine*);
typedef void (*fp_machine_bankname)(struct psy_audio_Machine*, int bnkidx, char* val);
typedef int (*fp_machine_numbanks)(struct psy_audio_Machine*);
typedef void (*fp_machine_setcurrbank)(struct psy_audio_Machine*, int bnkIdx);
typedef int (*fp_machine_currbank)(struct psy_audio_Machine*);
// machine callbacks
typedef	unsigned int (*fp_machine_samplerate)(struct psy_audio_Machine*);
typedef unsigned int (*fp_machine_bpm)(struct psy_audio_Machine*);
typedef psy_dsp_beat_t (*fp_machine_beatspersample)(struct psy_audio_Machine*);
typedef psy_dsp_beat_t(*fp_machine_currbeatsperline)(struct psy_audio_Machine*);
typedef	struct psy_audio_Samples* (*fp_machine_samples)(struct psy_audio_Machine*);
typedef	struct psy_audio_Machines* (*fp_machine_machines)(struct psy_audio_Machine*);
typedef	struct psy_audio_Instruments* (*fp_machine_instruments)(struct psy_audio_Machine*);
typedef	struct psy_audio_MachineFactory* (*fp_machine_machinefactory)(struct psy_audio_Machine*);
typedef void (*fp_machine_output)(struct psy_audio_Machine*, const char* text);
typedef bool (*fp_machine_addcapture)(struct psy_audio_Machine*, int idx);
typedef bool (*fp_machine_removecapture)(struct psy_audio_Machine*, int idx);
typedef void (*fp_machine_readbuffers)(struct psy_audio_Machine*, int index, float** pleft, float** pright, int numsamples);
typedef const char* (*fp_machine_capturename)(struct psy_audio_Machine*, int index);
typedef int (*fp_machine_numcaptures)(struct psy_audio_Machine*);
typedef const char* (*fp_machine_playbackname)(struct psy_audio_Machine*, int index);
typedef int (*fp_machine_numplaybacks)(struct psy_audio_Machine*);

typedef struct MachineVtable {
	fp_machine_init init;
	fp_machine_reload reload;
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
	fp_machine_amprange amprange;
	// programs
	fp_machine_programname programname;
	fp_machine_numprograms numprograms;
	fp_machine_setcurrprogram setcurrprogram;
	fp_machine_currprogram currprogram;
	fp_machine_bankname bankname;
	fp_machine_numbanks numbanks;
	fp_machine_setcurrbank setcurrbank;
	fp_machine_currbank currbank;
	// machine callbacks
	fp_machine_samplerate samplerate;
	fp_machine_bpm bpm;
	fp_machine_beatspersample beatspersample;
	fp_machine_currbeatsperline currbeatsperline;
	fp_machine_samples samples;
	fp_machine_machines machines;
	fp_machine_instruments instruments;
	fp_machine_machinefactory machinefactory;
	fp_machine_output output;
	fp_machine_addcapture addcapture;
	fp_machine_removecapture removecapture;
	fp_machine_readbuffers readbuffers;
	fp_machine_capturename capturename;
	fp_machine_numcaptures numcaptures;
	fp_machine_playbackname playbackname;
	fp_machine_numplaybacks numplaybacks;
} MachineVtable;

typedef struct psy_audio_Machine {
	MachineVtable* vtable;
	MachineCallback callback;	
	psy_Signal signal_worked;
	int err;
	psy_audio_CpuTimeClock accumulated_processing_time_;
} psy_audio_Machine;

void machine_init(psy_audio_Machine*, MachineCallback);
void machine_dispose(psy_audio_Machine*);
int machine_supports(psy_audio_Machine*, int option);

// vtable calls
INLINE void psy_audio_machine_dispose(psy_audio_Machine* self)
{
	self->vtable->dispose(self);
}

INLINE void psy_audio_machine_reload(psy_audio_Machine* self)
{
	self->vtable->reload(self);
}

INLINE const psy_audio_MachineInfo* psy_audio_machine_info(psy_audio_Machine* self)
{
	return self->vtable->info(self);
}

INLINE psy_audio_Machine* psy_audio_machine_clone(psy_audio_Machine* self)
{
	return self->vtable->clone(self);
}

INLINE psy_audio_Buffer* psy_audio_machine_mix(psy_audio_Machine* self, uintptr_t slot,
	uintptr_t amount, psy_audio_MachineSockets* sockets,
	struct psy_audio_Machines* machines)
{
	return self->vtable->mix(self, slot, amount, sockets, machines);
}

INLINE void psy_audio_machine_work(psy_audio_Machine* self, psy_audio_BufferContext* bc)
{
	self->vtable->work(self, bc);
}

INLINE void psy_audio_machine_generateaudio(psy_audio_Machine* self,
	psy_audio_BufferContext* bc)
{
	self->vtable->generateaudio(self, bc);
}

INLINE void psy_audio_machine_seqtick(psy_audio_Machine* self, uintptr_t channel,
	const psy_audio_PatternEvent* event)
{
	self->vtable->seqtick(self, channel, event);
}

INLINE void psy_audio_machine_stop(psy_audio_Machine* self)
{
	self->vtable->stop(self);
}

INLINE void psy_audio_machine_sequencertick(psy_audio_Machine* self)
{
	self->vtable->sequencertick(self);
}

INLINE void psy_audio_machine_sequencerlinetick(psy_audio_Machine* self)
{
	self->vtable->sequencerlinetick(self);
}

INLINE psy_List* psy_audio_machine_sequencerinsert(psy_audio_Machine* self, psy_List* events)
{
	return self->vtable->sequencerinsert(self, events);
}

INLINE int psy_audio_machine_mode(psy_audio_Machine* self)
{
	return self->vtable->mode(self);
}

INLINE int psy_audio_machine_describevalue(psy_audio_Machine* self, char* txt, uintptr_t param,
	int value)
{
	return self->vtable->describevalue(self, txt, param, value);
}

INLINE void psy_audio_machine_parametertweak(psy_audio_Machine* self, uintptr_t param, float value)
{
	self->vtable->parametertweak(self, param, value);
}

INLINE void psy_audio_machine_patterntweak(psy_audio_Machine* self, uintptr_t param, float value)
{
	self->vtable->patterntweak(self, param, value);
}

INLINE float psy_audio_machine_parametervalue(psy_audio_Machine* self, uintptr_t param)
{
	return self->vtable->parametervalue(self, param);
}

INLINE int psy_audio_machine_paramviewoptions(psy_audio_Machine* self)
{
	return self->vtable->paramviewoptions(self);
}

INLINE void psy_audio_machine_parameterrange(struct psy_audio_Machine* self,
	uintptr_t param, int* minval, int* maxval)
{
	self->vtable->parameterrange(self, param, minval, maxval);
}

INLINE uintptr_t psy_audio_machine_parametertype(psy_audio_Machine* self, uintptr_t param)
{
	return self->vtable->parametertype(self, param);
}

INLINE uintptr_t psy_audio_machine_numparameters(psy_audio_Machine* self)
{
	return self->vtable->numparameters(self);
}

INLINE uintptr_t psy_audio_machine_numparametercols(psy_audio_Machine* self)
{
	return self->vtable->numparametercols(self);
}

INLINE int psy_audio_machine_parameterlabel(psy_audio_Machine* self, char* txt, uintptr_t param)
{
	return self->vtable->parameterlabel(self, txt, param);
}

INLINE int psy_audio_machine_parametername(psy_audio_Machine* self, char* txt, uintptr_t param)
{
	return self->vtable->parametername(self, txt, param);
}

INLINE uintptr_t psy_audio_machine_numinputs(psy_audio_Machine* self)
{
	return self->vtable->numinputs(self);
}

INLINE uintptr_t psy_audio_machine_numoutputs(psy_audio_Machine* self)
{
	return self->vtable->numoutputs(self);
}

INLINE uintptr_t psy_audio_machine_slot(psy_audio_Machine* self)
{
	return self->vtable->slot(self);
}

INLINE void psy_audio_machine_setcallback(psy_audio_Machine* self, MachineCallback callback)
{
	self->vtable->setcallback(self, callback);
}

INLINE void psy_audio_machine_setpanning(psy_audio_Machine* self, psy_dsp_amp_t pan)
{
	self->vtable->setpanning(self, pan);
}

INLINE psy_dsp_amp_t psy_audio_machine_panning(psy_audio_Machine* self)
{
	return self->vtable->panning(self);
}

INLINE void psy_audio_machine_mute(psy_audio_Machine* self)
{
	self->vtable->mute(self);
}

INLINE void psy_audio_machine_unmute(psy_audio_Machine* self)
{
	self->vtable->unmute(self);
}

INLINE int psy_audio_machine_muted(psy_audio_Machine* self)
{
	return self->vtable->muted(self);
}

INLINE void psy_audio_machine_bypass(psy_audio_Machine* self)
{
	self->vtable->bypass(self);
}

INLINE void psy_audio_machine_unbypass(psy_audio_Machine* self)
{
	self->vtable->unbypass(self);
}

INLINE int psy_audio_machine_bypassed(psy_audio_Machine* self)
{
	return self->vtable->bypassed(self);
}

INLINE int psy_audio_machine_haseditor(psy_audio_Machine* self)
{
	return self->vtable->haseditor(self);
}

INLINE void psy_audio_machine_seteditorhandle(psy_audio_Machine* self, void* handle)
{
	self->vtable->seteditorhandle(self, handle);
}

INLINE void psy_audio_machine_editorsize(psy_audio_Machine* self, int* width, int* height)
{
	self->vtable->editorsize(self, width, height);
}

INLINE void psy_audio_machine_editoridle(psy_audio_Machine* self)
{
	self->vtable->editoridle(self);
}

INLINE void psy_audio_machine_seteditname(psy_audio_Machine* self, const char* name)
{
	self->vtable->seteditname(self, name);
}

INLINE const char* psy_audio_machine_editname(psy_audio_Machine* self)
{
	return self->vtable->editname(self);
}

INLINE unsigned int psy_audio_machine_bpm(psy_audio_Machine* self)
{
	return self->vtable->bpm(self);
}

INLINE psy_dsp_beat_t psy_audio_machine_beatspersample(psy_audio_Machine* self)
{
	return self->vtable->beatspersample(self);
}

INLINE psy_dsp_beat_t psy_audio_machine_currbeatsperline(psy_audio_Machine* self)
{
	return self->vtable->currbeatsperline(self);
}

INLINE unsigned int psy_audio_machine_samplerate(psy_audio_Machine* self)
{
	return self->vtable->samplerate(self);
}

INLINE struct psy_audio_Samples* psy_audio_machine_samples(psy_audio_Machine* self)
{
	return self->vtable->samples(self);
}

INLINE struct psy_audio_Machines* psy_audio_machine_machines(psy_audio_Machine* self)
{
	return self->vtable->machines(self);
}

INLINE struct psy_audio_Instruments* psy_audio_machine_instruments(psy_audio_Machine* self)
{
	return self->vtable->instruments(self);
}

INLINE struct psy_audio_MachineFactory* psy_audio_machine_machinefactory(psy_audio_Machine* self)
{
	return self->vtable->machinefactory(self);
}

INLINE bool psy_audio_machine_addcapture(psy_audio_Machine* self, int index)
{
	return self->vtable->addcapture(self, index);
}

INLINE bool psy_audio_machine_removecapture(psy_audio_Machine* self, int index)
{
	return self->vtable->removecapture(self, index);
}

INLINE void psy_audio_machine_readbuffers(psy_audio_Machine* self, int index, float** pleft, float** pright, int numsamples)
{
	self->vtable->readbuffers(self, index, pleft, pright, numsamples);
}

INLINE const char* psy_audio_machine_capturename(psy_audio_Machine* self, int index)
{
	return self->vtable->capturename(self, index);
}

INLINE int psy_audio_machine_numcaptures(psy_audio_Machine* self)
{
	return self->vtable->numcaptures(self);
}

INLINE const char* psy_audio_machine_playbackname(psy_audio_Machine* self, int index)
{
	return self->vtable->playbackname(self, index);
}

INLINE int psy_audio_machine_numplaybacks(psy_audio_Machine* self)
{
	return self->vtable->numplaybacks(self);
}

INLINE void psy_audio_machine_output(psy_audio_Machine* self, const char* text)
{
	self->vtable->output(self, text);
}

INLINE psy_audio_Buffer* psy_audio_machine_buffermemory(psy_audio_Machine* self)
{
	return self->vtable->buffermemory(self);
}

INLINE uintptr_t psy_audio_machine_buffermemorysize(psy_audio_Machine* self)
{
	return self->vtable->buffermemorysize(self);
}

INLINE void psy_audio_machine_setbuffermemorysize(psy_audio_Machine* self, uintptr_t size)
{
	self->vtable->setbuffermemorysize(self, size);
}

INLINE psy_dsp_amp_range_t psy_audio_machine_amprange(psy_audio_Machine* self)
{
	return self->vtable->amprange(self);
}

INLINE void psy_audio_machine_loadspecific(psy_audio_Machine* self,
	struct psy_audio_SongFile* songfile, uintptr_t slot)
{
	self->vtable->loadspecific(self, songfile, slot);
}

INLINE void psy_audio_machine_savespecific(psy_audio_Machine* self,
	struct psy_audio_SongFile* songfile, uintptr_t slot)
{
	self->vtable->savespecific(self, songfile, slot);
}

// parameter mapping
INLINE int machine_parametervalue_scaled(psy_audio_Machine* self,
	uintptr_t param, float value)
{	
	int minval;
	int maxval;

	psy_audio_machine_parameterrange(self, param, &minval, &maxval);
	return (int)(value * (maxval - minval) + 0.5f) + minval;
}

INLINE int machine_patternvalue_scaled(psy_audio_Machine* self,
	uintptr_t param, float value)
{
	int minval;
	int maxval;

	psy_audio_machine_parameterrange(self, param, &minval, &maxval);
	return (int)(value * (maxval - minval) + 0.5f);
}

INLINE float machine_parametervalue_normed(psy_audio_Machine* self,
	uintptr_t param, int value)
{	
	int minval;
	int maxval;

	psy_audio_machine_parameterrange(self, param, &minval, &maxval);
	return ((maxval - minval) != 0)
		? (value - minval) / (float)(maxval - minval)
		: 0.f;
}

INLINE float machine_patternvalue_normed(psy_audio_Machine* self,
	uintptr_t param, int value)
{	
	int minval;
	int maxval;	
	int range;
						
	psy_audio_machine_parameterrange(self, param, &minval, &maxval);
	range = maxval - minval;
	if (range == 0) {
		return 0.f;
	}
	return (value > range) ? range : value / (float)range;
}

// programs
INLINE void psy_audio_machine_programname(psy_audio_Machine* self, int bnkidx, int prgIdx, char* val)
{
	self->vtable->programname(self, bnkidx, prgIdx, val);
}

INLINE int psy_audio_machine_numprograms(psy_audio_Machine* self)
{
	return self->vtable->numprograms(self);
}

INLINE void psy_audio_machine_setcurrprogram(psy_audio_Machine* self, int prgIdx)
{
	self->vtable->setcurrprogram(self, prgIdx);
}

INLINE int psy_audio_machine_currprogram(psy_audio_Machine* self)
{
	return self->vtable->currprogram(self);
}

INLINE void psy_audio_machine_bankname(psy_audio_Machine* self, int bnkidx, char* val)
{
	self->vtable->bankname(self, bnkidx, val);
}

INLINE int psy_audio_machine_numbanks(psy_audio_Machine* self)
{
	return self->vtable->numbanks(self);
}

INLINE void psy_audio_machine_setcurrbank(psy_audio_Machine* self, int prgIdx)
{
	self->vtable->setcurrbank(self, prgIdx);
}

INLINE int psy_audio_machine_currbank(psy_audio_Machine* self)
{
	return self->vtable->currbank(self);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MACHINE_H */
