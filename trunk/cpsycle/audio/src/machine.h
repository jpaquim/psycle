/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_audio_MACHINE_H
#define psy_audio_MACHINE_H

/* local */
#include "buffercontext.h"
#include "connections.h"
#include "cputimeclock.h"
#include "machinedefs.h"
#include "machineinfo.h"
#include "machineparam.h"
#include "patternevent.h"
#include "paramtranslator.h"
/* dsp */
#include <dsptypes.h>
/* container */
#include <signal.h>
/* platform */
#include "../../detail/psydef.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** psy_audio_Machine, psy_audio_MachineCallback
**
** Abstract base class for "Machines", the audio producing elements,
** but most classes inherit from CustomMachine offering more defaults.
**
** psy_audio_machine_work is called by the player or the mixer to fill the
** buffer provided by BufferContext. Machines themself have no buffers,
** instead BufferContext provides them during a work call. A BufferMemory
** can be set up as a persistent Buffer to record samples to assist Scopes.
** BufferContext contains an event list, too. Most plugins don't overwrite
** work but let Machine handle the events. Work splits the events according to
** their time stamp and calls for the intervals generateaudio. A plugin
** needs to override this method to to fill the buffer part. Tweaks and slides
** are handled here aswell. Slides are generated by the player and occure as
** tweaks, but with a step counter to tweak the current value. VstPlugins
** overwrite work and handle the events themself. For now, Psycle has two
** different mix systems, the Buzz wire and the newer send return mixer. To
** handle both, Machine implements the method mix for the wires. Mixer
** overwrites mix to implement the chain mixing. Mainly the player is doing the
** machine sheduling, but the mixer can exclude them by marking them in
** machines as "sent" and doing its own work calls. Machine Parameters have
** their own struct, that work independent from the machine, but machine has
** machineparam vtable calls to let psy_audio_MachineProxy handle exceptions.
** psy_audio_MachineCallback enables plugins to ask the host for user
** configurations, file dialogs, samplerate settings, tempo settings, sample,
** instrument banks and existing machines. To accompany song file loading
** and saving, plugins can save additional machine information with
** loadspecific, savespecific, loadwiremapping/savewiremapping in a song.
*/
struct psy_audio_SequencerTime;

typedef psy_dsp_big_hz_t (*fp_mcb_samplerate)(void*);
typedef	struct psy_audio_SequencerTime* (*fp_mcb_sequencertime)(void*);
typedef	psy_dsp_beat_t (*fp_mcb_bpm)(void*);
typedef	psy_dsp_beat_t (*fp_mcb_beatspertick)(void*);
typedef	psy_dsp_beat_t (*fp_mcb_beatspersample)(void*);
typedef	psy_dsp_beat_t (*fp_mcb_currbeatsperline)(void*);
typedef	struct psy_audio_Samples* (*fp_mcb_samples)(void*);
typedef	struct psy_audio_Machines* (*fp_mcb_machines)(void*);
typedef	struct psy_audio_Instruments* (*fp_mcb_instruments)(void*);
typedef	struct psy_audio_MachineFactory* (*fp_mcb_machinefactory)(void*);
typedef	bool (*fp_mcb_fileselect_load)(void*, char filter[], char inoutName[]);
typedef	bool (*fp_mcb_fileselect_save)(void*, char filter[], char inoutName[]);
typedef	void (*fp_mcb_fileselect_directory)(void*);
typedef	bool (*fp_mcb_editresize)(void*, void* sender, intptr_t w, intptr_t h);
typedef	void (*fp_mcb_output)(void*, const char* text);
typedef	bool (*fp_mcb_addcapture)(void*, int index);
typedef	bool (*fp_mcb_removecapture)(void*, int index);
typedef void (*fp_mcb_readbuffers)(void*, int index, float** pleft,
	float** pright, uintptr_t numsamples);
typedef const char* (*fp_mcb_capturename)(void*, int index);
typedef int (*fp_mcb_numcaptures)(void*);
typedef const char* (*fp_mcb_playbackname)(void*, int index);
typedef int (*fp_mcb_numplaybacks)(void*);
typedef int (*fp_mcb_onerror)(void*);
typedef int (*fp_mcb_onwarning)(void*);
typedef int (*fp_mcb_onmessage)(void*);
typedef void (*fp_mcb_buschanged)(void*);
typedef const char* (*fp_mcb_language)(void*);

typedef struct {
	fp_mcb_samplerate samplerate;
	fp_mcb_bpm bpm;
	fp_mcb_sequencertime sequencertime;
	fp_mcb_beatspertick beatspertick;
	fp_mcb_beatspersample beatspersample;
	fp_mcb_currbeatsperline currbeatsperline;
	fp_mcb_samples samples;
	fp_mcb_machines machines;
	fp_mcb_instruments instruments;
	fp_mcb_machinefactory machinefactory;
	fp_mcb_fileselect_load fileselect_load;
	fp_mcb_fileselect_save fileselect_save;
	fp_mcb_fileselect_directory fileselect_directory;	
	fp_mcb_editresize editresize;
	fp_mcb_output output;
	fp_mcb_addcapture addcapture;
	fp_mcb_removecapture removecapture;
	fp_mcb_readbuffers readbuffers;
	fp_mcb_capturename capturename;
	fp_mcb_numcaptures numcaptures;
	fp_mcb_playbackname playbackname;
	fp_mcb_numplaybacks numplaybacks;	
	fp_mcb_language language;
	fp_mcb_buschanged buschanged;
} psy_audio_MachineCallbackVtable;

struct psy_audio_Player;

typedef struct psy_audio_MachineCallback {	
	psy_audio_MachineCallbackVtable* vtable;
	struct psy_audio_Player* player;
	struct psy_audio_Song* song;
} psy_audio_MachineCallback;

void psy_audio_machinecallback_init(psy_audio_MachineCallback*);
void psy_audio_machinecallback_set_player(psy_audio_MachineCallback*,
	struct psy_audio_Player*);
void psy_audio_machinecallback_set_song(psy_audio_MachineCallback*,
	struct psy_audio_Song*);


typedef enum {
	MACHINE_PARAMVIEW_COMPACT = 1
} MachineViewOptions;

/* audio */
struct psy_audio_Machine;
struct psy_audio_Player;
struct psy_audio_SongFile;
struct psy_audio_Preset;
struct psy_audio_Presets;
/* ui */
struct psy_ui_Component;

/* machine vtable function pointers */
typedef	void (*fp_machine_init)(struct psy_audio_Machine*);
typedef	void (*fp_machine_dispose)(struct psy_audio_Machine*);
typedef	struct psy_audio_Machine* (*fp_machine_clone)(struct
	psy_audio_Machine*);
typedef	void (*fp_machine_reload)(struct psy_audio_Machine*);
typedef	void (*fp_machine_work)(struct psy_audio_Machine*,
	psy_audio_BufferContext*);
typedef	psy_audio_Buffer* (*fp_machine_mix)(struct psy_audio_Machine*,
	uintptr_t slot, uintptr_t amount, psy_audio_MachineSockets*,
	struct psy_audio_Machines*, struct psy_audio_Player*);
typedef	void (*fp_machine_generateaudio)(struct psy_audio_Machine*,
	psy_audio_BufferContext*);
typedef	int (*fp_machine_hostevent)(struct psy_audio_Machine*,
	int const eventNr, int const val1, float const val2);
typedef	void (*fp_machine_seqtick)(struct psy_audio_Machine*,
	uintptr_t channel, const psy_audio_PatternEvent*);
typedef	void (*fp_machine_sequencertick)(struct psy_audio_Machine*);
typedef	void (*fp_machine_newline)(struct psy_audio_Machine*);
typedef	psy_List* (*fp_machine_sequencerinsert)(struct psy_audio_Machine*,
	psy_List* events);
typedef	void (*fp_machine_stop)(struct psy_audio_Machine*);
typedef	void (*fp_machine_setpanning)(struct psy_audio_Machine*,
	psy_dsp_amp_t);
typedef	psy_dsp_amp_t (*fp_machine_panning)(struct psy_audio_Machine*);
typedef	void (*fp_machine_mute)(struct psy_audio_Machine*);	
typedef	void (*fp_machine_unmute)(struct psy_audio_Machine*);
typedef	int (*fp_machine_muted)(struct psy_audio_Machine*);
typedef	void (*fp_machine_bypass)(struct psy_audio_Machine*);
typedef	void (*fp_machine_unbypass)(struct psy_audio_Machine*);
typedef	int (*fp_machine_bypassed)(struct psy_audio_Machine*);
typedef	void (*fp_machine_standby)(struct psy_audio_Machine*);
typedef	void (*fp_machine_deactivatestandby)(struct psy_audio_Machine*);
typedef	int (*fp_machine_hasstandby)(struct psy_audio_Machine*);
typedef	void (*fp_machine_setbus)(struct psy_audio_Machine*);
typedef	void (*fp_machine_unsetbus)(struct psy_audio_Machine*);
typedef	int (*fp_machine_isbus)(struct psy_audio_Machine*);
typedef	const psy_audio_MachineInfo* (*fp_machine_info)(struct
	psy_audio_Machine*);
typedef	int (*fp_machine_mode)(struct psy_audio_Machine*);
typedef	void (*fp_machine_updatesamplerate)(struct psy_audio_Machine*,
	psy_dsp_big_hz_t samplerate);
typedef	uintptr_t (*fp_machine_numinputs)(struct psy_audio_Machine*);
typedef	uintptr_t (*fp_machine_numoutputs)(struct psy_audio_Machine*);
typedef	uintptr_t (*fp_machine_slot)(struct psy_audio_Machine*);
typedef	void (*fp_machine_setslot)(struct psy_audio_Machine*, uintptr_t);
typedef	psy_audio_MachineParam* (*fp_machine_parameter)(struct
	psy_audio_Machine*, uintptr_t param);
typedef	psy_audio_MachineParam* (*fp_machine_tweakparameter)(struct
	psy_audio_Machine*, uintptr_t param);
typedef	psy_audio_ParamTranslator* (*fp_machine_instparamtranslator)(struct
	psy_audio_Machine*);
typedef	uintptr_t (*fp_machine_numparameters)(struct psy_audio_Machine*);
typedef	uintptr_t(*fp_machine_numtweakparameters)(struct psy_audio_Machine*);
typedef uintptr_t(*fp_machine_paramselected)(struct psy_audio_Machine*);
typedef void (*fp_machine_selectparam)(struct psy_audio_Machine*, uintptr_t index);
typedef	uintptr_t (*fp_machine_numparametercols)(struct psy_audio_Machine*);
typedef	uintptr_t(*fp_machine_paramstrobe)(const struct psy_audio_Machine*);
typedef	void (*fp_machine_param_tweak)(struct psy_audio_Machine*, struct
	psy_audio_MachineParam*, float val);
typedef	void (*fp_machine_param_reset)(struct psy_audio_Machine*, struct
	psy_audio_MachineParam*);
typedef float (*fp_machine_param_normvalue)(struct psy_audio_Machine*, struct
	psy_audio_MachineParam*);
typedef void (*fp_machine_param_range)(struct psy_audio_Machine*, struct
	psy_audio_MachineParam*, intptr_t* minval, intptr_t* maxval);
typedef	int (*fp_machine_param_type)(struct psy_audio_Machine*, struct
	psy_audio_MachineParam*);
typedef	int (*fp_machine_param_label)(struct psy_audio_Machine*, struct
	psy_audio_MachineParam*, char* text);
typedef	int (*fp_machine_param_name)(struct psy_audio_Machine*, struct
	psy_audio_MachineParam*, char* text);
typedef	int (*fp_machine_param_describe)(struct psy_audio_Machine*, struct
	psy_audio_MachineParam*, char* text);
typedef	void (*fp_machine_setcallback)(struct psy_audio_Machine*,
	psy_audio_MachineCallback*);
typedef	int (*fp_machine_loadspecific)(struct psy_audio_Machine*,
	struct psy_audio_SongFile*, uintptr_t slot);
typedef	int (*fp_machine_loadwiremapping)(struct psy_audio_Machine*,
	struct psy_audio_SongFile*, uintptr_t slot);
typedef	int (*fp_machine_savespecific)(struct psy_audio_Machine*,
	struct psy_audio_SongFile*, uintptr_t slot);
typedef	int (*fp_machine_savewiremapping)(struct psy_audio_Machine*,
	struct psy_audio_SongFile*, uintptr_t slot);
typedef	void (*fp_machine_postload)(struct psy_audio_Machine*,
	struct psy_audio_SongFile*, uintptr_t slot);
/* editor */
typedef	int (*fp_machine_haseditor)(struct psy_audio_Machine*);
typedef	void (*fp_machine_seteditorhandle)(struct psy_audio_Machine*,
	void* handle);
typedef	void (*fp_machine_sethostview)(struct psy_audio_Machine*,
	struct psy_ui_Component* view);
typedef	void (*fp_machine_editorsize)(struct psy_audio_Machine*, double* width,
	double* height);
typedef	void (*fp_machine_editoridle)(struct psy_audio_Machine*);
typedef	const char* (*fp_machine_editname)(struct psy_audio_Machine*);
typedef	void (*fp_machine_seteditname)(struct psy_audio_Machine*, const char*
	name);
typedef void (*fp_machine_setposition)(struct psy_audio_Machine*, double x,
	double y);
typedef void (*fp_machine_position)(struct psy_audio_Machine*, double* x,
	double* y);
typedef	struct psy_audio_Buffer* (*fp_machine_buffermemory)(struct
	psy_audio_Machine*);
typedef	uintptr_t (*fp_machine_buffermemorysize)(struct psy_audio_Machine*);
typedef	void (*fp_machine_setbuffermemorysize)(struct psy_audio_Machine*,
	uintptr_t);
typedef	psy_dsp_amp_range_t (*fp_machine_amprange)(struct psy_audio_Machine*);
typedef void (*fp_machine_putdata)(struct psy_audio_Machine*, uint8_t* data);
typedef uint8_t* (*fp_machine_data)(struct psy_audio_Machine*);
typedef uintptr_t (*fp_machine_datasize)(struct psy_audio_Machine*);
/* program */
typedef void (*fp_machine_programname)(struct psy_audio_Machine*, uintptr_t bnkidx,
	uintptr_t prgidx, char* val);
typedef uintptr_t (*fp_machine_numprograms)(struct psy_audio_Machine*);
typedef void (*fp_machine_setcurrprogram)(struct psy_audio_Machine*,
	uintptr_t prgidx);
typedef uintptr_t(*fp_machine_currprogram)(struct psy_audio_Machine*);
/* bank */
typedef void (*fp_machine_bankname)(struct psy_audio_Machine*,
	uintptr_t bnkidx, char* val);
typedef uintptr_t (*fp_machine_numbanks)(struct psy_audio_Machine*);
typedef void (*fp_machine_setcurrbank)(struct psy_audio_Machine*, uintptr_t bnkidx);
typedef uintptr_t (*fp_machine_currbank)(struct psy_audio_Machine*);
/* auxcolumns */
typedef const char* (*fp_machine_auxcolumnname)(struct psy_audio_Machine*, uintptr_t index);
typedef uintptr_t(*fp_machine_numauxcolumns)(struct psy_audio_Machine*);
typedef uintptr_t (*fp_machine_auxcolumnselected)(struct psy_audio_Machine*);
typedef void (*fp_machine_selectauxcolumn)(struct psy_audio_Machine*, uintptr_t index);
/* presets */
typedef void (*fp_machine_currentpreset)(struct psy_audio_Machine*,
	struct psy_audio_Preset*);
typedef void (*fp_machine_setpresets)(struct psy_audio_Machine*,
	struct psy_audio_Presets*);
typedef struct psy_audio_Presets* (*fp_machine_presets)(
	struct psy_audio_Machine*);
typedef bool (*fp_machine_acceptpresets)(struct psy_audio_Machine*);
typedef	void (*fp_machine_command)(struct psy_audio_Machine*);
typedef const char* (*fp_machine_modulepath)(struct psy_audio_Machine*);
typedef uintptr_t (*fp_machine_shellidx)(struct psy_audio_Machine*);
/* machine callbacks */
typedef	psy_dsp_big_hz_t (*fp_machine_samplerate)(struct psy_audio_Machine*);
typedef psy_dsp_beat_t (*fp_machine_bpm)(struct psy_audio_Machine*);
typedef struct psy_audio_SequencerTime* (*fp_machine_sequencertime)(struct psy_audio_Machine*);
typedef psy_dsp_beat_t (*fp_machine_beatspertick)(struct psy_audio_Machine*);
typedef psy_dsp_beat_t (*fp_machine_beatspersample)(struct psy_audio_Machine*);
typedef psy_dsp_beat_t(*fp_machine_currbeatsperline)(struct
	psy_audio_Machine*);
typedef	struct psy_audio_Samples* (*fp_machine_samples)(struct
	psy_audio_Machine*);
typedef	struct psy_audio_Machines* (*fp_machine_machines)(struct
	psy_audio_Machine*);
typedef	struct psy_audio_Instruments* (*fp_machine_instruments)(struct
	psy_audio_Machine*);
typedef	struct psy_audio_MachineFactory* (*fp_machine_machinefactory)(struct
	psy_audio_Machine*);
typedef void (*fp_machine_output)(struct psy_audio_Machine*, const char* text);
typedef	bool(*fp_machine_editresize)(struct psy_audio_Machine*, intptr_t w, intptr_t h);
typedef	void (*fp_machine_buschanged)(struct psy_audio_Machine*);
typedef bool (*fp_machine_addcapture)(struct psy_audio_Machine*, int idx);
typedef bool (*fp_machine_removecapture)(struct psy_audio_Machine*, int idx);
typedef void (*fp_machine_readbuffers)(struct psy_audio_Machine*, int index,
	float** pleft, float** pright, uintptr_t numsamples);
typedef const char* (*fp_machine_capturename)(struct psy_audio_Machine*,
	int index);
typedef int (*fp_machine_numcaptures)(struct psy_audio_Machine*);
typedef const char* (*fp_machine_playbackname)(struct psy_audio_Machine*,
	int index);
typedef int (*fp_machine_numplaybacks)(struct psy_audio_Machine*);
typedef const char* (*fp_machine_language)(struct psy_audio_Machine*);

typedef struct {
	/* the life cycle of a machine */
	fp_machine_init init;
	fp_machine_setcallback setcallback;
	fp_machine_dispose dispose;
	fp_machine_clone clone;
	/* processing audio */
	fp_machine_work work;
	fp_machine_mix mix;
	fp_machine_generateaudio generateaudio;
	fp_machine_sequencertick sequencertick;
	fp_machine_newline newline;
	fp_machine_seqtick seqtick;
	fp_machine_stop stop;
	fp_machine_hostevent hostevent;	
	fp_machine_sequencerinsert sequencerinsert;

	/* (de)serialization */
	fp_machine_loadspecific loadspecific;
	fp_machine_loadwiremapping loadwiremapping;	
	fp_machine_savespecific savespecific;
	fp_machine_savewiremapping savewiremapping;
	fp_machine_postload postload;
	fp_machine_reload reload;	
	/* connections */
	fp_machine_numinputs numinputs;
	fp_machine_numoutputs numoutputs;
	fp_machine_slot slot;
	fp_machine_setslot setslot;
	fp_machine_amprange amprange;
	/* panning */
	fp_machine_setpanning setpanning;
	fp_machine_panning panning;
	/* general information */
	fp_machine_info info;
	fp_machine_seteditname seteditname;
	fp_machine_editname editname;
	fp_machine_mode mode;
	fp_machine_modulepath modulepath;
	fp_machine_shellidx shellidx;
	/* parameters */
	fp_machine_numparametercols numparametercols;
	fp_machine_numparameters numparameters;
	fp_machine_parameter parameter;
	fp_machine_numtweakparameters numtweakparameters;	
	fp_machine_tweakparameter tweakparameter;
	fp_machine_paramselected paramselected;
	fp_machine_selectparam selectparam;
	fp_machine_paramstrobe paramstrobe;
	/* machineparameter calls */
	fp_machine_param_type parameter_type;
	fp_machine_param_name parameter_name;
	fp_machine_param_range parameter_range;
	fp_machine_param_describe parameter_describe;
	fp_machine_param_normvalue parameter_normvalue;
	fp_machine_param_label parameter_label;	
	fp_machine_param_tweak parameter_tweak;
	fp_machine_param_reset parameter_reset;
	fp_machine_instparamtranslator instparamtranslator;
	/* data */
	fp_machine_putdata putdata;
	fp_machine_data data;
	fp_machine_datasize datasize;
	fp_machine_updatesamplerate updatesamplerate;
	/* programs */
	fp_machine_setcurrprogram setcurrprogram;
	fp_machine_currprogram currprogram;
	fp_machine_programname programname;
	fp_machine_numprograms numprograms;
	fp_machine_setcurrbank setcurrbank;
	fp_machine_bankname bankname;		
	fp_machine_currbank currbank;
	fp_machine_numbanks numbanks;
	fp_machine_currentpreset currentpreset;
	fp_machine_setpresets setpresets;
	fp_machine_presets presets;
	fp_machine_acceptpresets acceptpresets;
	fp_machine_command command;	
	/* auxcolumn */
	fp_machine_auxcolumnname auxcolumnname;
	fp_machine_numauxcolumns numauxcolumns;
	fp_machine_auxcolumnselected auxcolumnselected;
	fp_machine_selectauxcolumn selectauxcolumn;
	/* gui stuff */
	fp_machine_haseditor haseditor;
	fp_machine_seteditorhandle seteditorhandle;
	fp_machine_editorsize editorsize;
	fp_machine_editresize editresize;
	fp_machine_editoridle editoridle;
	fp_machine_setposition setposition;
	fp_machine_position position;
	fp_machine_sethostview sethostview;
	/* states */
	fp_machine_bypassed bypassed;
	fp_machine_bypass bypass;
	fp_machine_unbypass unbypass;
	fp_machine_muted muted;
	fp_machine_mute mute;
	fp_machine_unmute unmute;
	fp_machine_standby standby;
	fp_machine_deactivatestandby deactivatestandby;
	fp_machine_hasstandby hasstandby;
	fp_machine_isbus isbus;
	fp_machine_setbus setbus;
	fp_machine_unsetbus unsetbus;
	/* buffer memory */
	fp_machine_buffermemory buffermemory;
	fp_machine_buffermemorysize buffermemorysize;
	fp_machine_setbuffermemorysize setbuffermemorysize;
	/* machine callbacks */
	fp_machine_samplerate samplerate;
	fp_machine_bpm bpm;
	fp_machine_sequencertime sequencertime;
	fp_machine_beatspertick beatspertick;
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
	fp_machine_language language;
	fp_machine_buschanged buschanged;
} MachineVtable;

typedef struct psy_audio_Machine {
	MachineVtable* vtable;
	psy_audio_MachineCallback* callback;	
	psy_Signal signal_worked;
	int err;
	psy_audio_CpuTimeClock cpu_time;
} psy_audio_Machine;

void psy_audio_machine_init(psy_audio_Machine*, psy_audio_MachineCallback*);
void machine_base_dispose(psy_audio_Machine*);
void psy_audio_machine_deallocate(psy_audio_Machine*);
int machine_supports(psy_audio_Machine*, int option);

INLINE void psy_audio_machine_setcallback(psy_audio_Machine* self,
	psy_audio_MachineCallback* callback)
{
	self->vtable->setcallback(self, callback);
}

INLINE void psy_audio_machine_dispose(psy_audio_Machine* self)
{
	self->vtable->dispose(self);
}

INLINE psy_audio_Machine* psy_audio_machine_clone(psy_audio_Machine* self)
{
	return self->vtable->clone(self);
}

INLINE void psy_audio_machine_work(psy_audio_Machine* self,
	psy_audio_BufferContext* bc)
{
	self->vtable->work(self, bc);
}

INLINE psy_audio_Buffer* psy_audio_machine_mix(psy_audio_Machine* self,
	uintptr_t slot, uintptr_t amount, psy_audio_MachineSockets* sockets,
	struct psy_audio_Machines* machines,
	struct psy_audio_Player* player)
{
	return self->vtable->mix(self, slot, amount, sockets, machines, player);
}

INLINE void psy_audio_machine_generateaudio(psy_audio_Machine* self,
	psy_audio_BufferContext* bc)
{
	self->vtable->generateaudio(self, bc);
}

INLINE intptr_t psy_audio_machine_type(psy_audio_Machine* self)
{	
	const psy_audio_MachineInfo* info;

	info = self->vtable->info(self);
	return (info)
		? info->type
		: psy_audio_UNDEFINED;	
}

INLINE void psy_audio_machine_sequencertick(psy_audio_Machine* self)
{
	self->vtable->sequencertick(self);
}

INLINE void psy_audio_machine_newline(psy_audio_Machine* self)
{
	self->vtable->newline(self);
}

INLINE void psy_audio_machine_seqtick(psy_audio_Machine* self, uintptr_t
	channel, const psy_audio_PatternEvent* event)
{
	self->vtable->seqtick(self, channel, event);
}

INLINE void psy_audio_machine_stop(psy_audio_Machine* self)
{
	self->vtable->stop(self);
}

INLINE psy_List* psy_audio_machine_sequencerinsert(psy_audio_Machine* self,
	psy_List* events)
{
	return self->vtable->sequencerinsert(self, events);
}

INLINE int psy_audio_machine_loadspecific(psy_audio_Machine* self,
	struct psy_audio_SongFile* songfile, uintptr_t slot)
{
	return self->vtable->loadspecific(self, songfile, slot);
}

INLINE int psy_audio_machine_loadwiremapping(psy_audio_Machine* self,
	struct psy_audio_SongFile* songfile, uintptr_t slot)
{
	return self->vtable->loadwiremapping(self, songfile, slot);
}

INLINE int psy_audio_machine_savespecific(psy_audio_Machine* self,
	struct psy_audio_SongFile* songfile, uintptr_t slot)
{
	return self->vtable->savespecific(self, songfile, slot);
}

INLINE int psy_audio_machine_savewiremapping(psy_audio_Machine* self,
	struct psy_audio_SongFile* songfile, uintptr_t slot)
{
	return self->vtable->savewiremapping(self, songfile, slot);
}

INLINE void psy_audio_machine_postload(psy_audio_Machine* self,
	struct psy_audio_SongFile* songfile, uintptr_t slot)
{
	self->vtable->postload(self, songfile, slot);
}

INLINE void psy_audio_machine_reload(psy_audio_Machine* self)
{
	self->vtable->reload(self);
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

INLINE void psy_audio_machine_setslot(psy_audio_Machine* self, uintptr_t slot)
{
	self->vtable->setslot(self, slot);
}

INLINE void psy_audio_machine_setpanning(psy_audio_Machine* self, psy_dsp_amp_t
	pan)
{
	self->vtable->setpanning(self, pan);
}

INLINE psy_dsp_amp_t psy_audio_machine_panning(psy_audio_Machine* self)
{
	return self->vtable->panning(self);
}

INLINE const psy_audio_MachineInfo* psy_audio_machine_info(psy_audio_Machine*
	self)
{
	return self->vtable->info(self);
}

INLINE void psy_audio_machine_seteditname(psy_audio_Machine* self, const char*
	name)
{
	self->vtable->seteditname(self, name);
}

INLINE const char* psy_audio_machine_editname(psy_audio_Machine* self)
{
	return self->vtable->editname(self);
}

INLINE int psy_audio_machine_mode(psy_audio_Machine* self)
{
	return self->vtable->mode(self);
}

INLINE const char* psy_audio_machine_modulepath(psy_audio_Machine* self)
{
	return self->vtable->modulepath(self);
}

INLINE uintptr_t psy_audio_machine_shellidx(psy_audio_Machine* self)
{
	return self->vtable->shellidx(self);
}

INLINE psy_audio_ParamTranslator* psy_audio_machine_instparamtranslator(
	psy_audio_Machine* self)
{
	return self->vtable->instparamtranslator(self);
}

INLINE uintptr_t psy_audio_machine_numparametercols(psy_audio_Machine* self)
{
	return self->vtable->numparametercols(self);
}

INLINE uintptr_t psy_audio_machine_numparameters(psy_audio_Machine* self)
{
	return self->vtable->numparameters(self);
}

INLINE psy_audio_MachineParam* psy_audio_machine_parameter(psy_audio_Machine*
	self,
	uintptr_t param)
{
	return self->vtable->parameter(self, param);
}

INLINE uintptr_t psy_audio_machine_numtweakparameters(psy_audio_Machine* self)
{
	return self->vtable->numtweakparameters(self);
}

INLINE psy_audio_MachineParam* psy_audio_machine_tweakparameter(
	psy_audio_Machine* self, uintptr_t param)
{
	return self->vtable->tweakparameter(self, param);
}

INLINE uintptr_t psy_audio_machine_paramselected(
	psy_audio_Machine* self)
{
	return self->vtable->paramselected(self);
}

INLINE void psy_audio_machine_selectparam(psy_audio_Machine* self, uintptr_t index)
{
	self->vtable->selectparam(self, index);
}

INLINE uintptr_t psy_audio_machine_paramstrobe(const psy_audio_Machine* self)	
{
	return self->vtable->paramstrobe(self);
}

/* MachineParameter calls */
INLINE void psy_audio_machine_parameter_tweak(psy_audio_Machine* self,
	psy_audio_MachineParam* param, float val)
{
	self->vtable->parameter_tweak(self, param, val);
}

INLINE void psy_audio_machine_parameter_reset(psy_audio_Machine* self,
	psy_audio_MachineParam* param)
{
	self->vtable->parameter_reset(self, param);
}

INLINE float psy_audio_machine_parameter_normvalue(psy_audio_Machine* self,
	psy_audio_MachineParam* param)
{
	return self->vtable->parameter_normvalue(self, param);
}

INLINE void psy_audio_machine_parameter_range(psy_audio_Machine* self,
	psy_audio_MachineParam* param, intptr_t* minval, intptr_t* maxval)
{
	self->vtable->parameter_range(self, param, minval, maxval);
}

INLINE int psy_audio_machine_parameter_type(psy_audio_Machine* self,
	psy_audio_MachineParam* param)
{
	return self->vtable->parameter_type(self, param);
}

INLINE int psy_audio_machine_parameter_label(psy_audio_Machine* self,
	psy_audio_MachineParam* param, char* text)
{
	return self->vtable->parameter_label(self, param, text);
}

INLINE int psy_audio_machine_parameter_name(psy_audio_Machine* self,
	psy_audio_MachineParam* param, char* text)
{
	return self->vtable->parameter_name(self, param, text);
}

INLINE int psy_audio_machine_parameter_describe(psy_audio_Machine* self,
	psy_audio_MachineParam* param, char* text)
{
	return self->vtable->parameter_describe(self, param, text);
}

INLINE void psy_audio_machine_parameter_tweak_scaled(psy_audio_Machine* self,
	psy_audio_MachineParam* param, intptr_t scaledvalue)
{
	intptr_t minval;
	intptr_t maxval;
	intptr_t range;
	float value;

	psy_audio_machine_parameter_range(self, param, &minval, &maxval);
	range = maxval - minval;
	if (range == 0) {
		value = 0.f;
	} else {
		value = (scaledvalue - minval) / (float)range;
	}
	if (value > 1.f) {
		value = 1.f;
	}
	if (value < 0.f) {
		value = 0.f;
	}
	psy_audio_machine_parameter_tweak(self, param, value);
}

INLINE void psy_audio_machine_parameter_tweak_pattern(psy_audio_Machine* self,
	psy_audio_MachineParam* param, uintptr_t patternvalue)
{
	intptr_t minval;
	intptr_t maxval;
	intptr_t range;
	float value;

	psy_audio_machine_parameter_range(self, param, &minval, &maxval);
	range = maxval - minval;
	if (range == 0) {
		value = 0.f;
	} else {
		value = patternvalue / (float)range;
	}
	if (value > 1.f) {
		value = 1.f;
	}
	if (value < 0.f) {
		value = 0.f;
	}
	psy_audio_machine_parameter_tweak(self, param, value);
}

/* converts normvalue(0.f .. 1.f)->scaled integer value */
INLINE intptr_t psy_audio_machine_parameter_scaledvalue(psy_audio_Machine*
	self, psy_audio_MachineParam* param)
{
	intptr_t rv;
	intptr_t minval;
	intptr_t maxval;
	intptr_t range;

	psy_audio_machine_parameter_range(self, param, &minval, &maxval);
	range = maxval - minval;
	rv = (int)(psy_audio_machine_parameter_normvalue(self, param) * range)
		+ minval;
	return rv;
}

/*
** converts normvalue(0.f .. 1.f)->pattern integer value
** difference to scaledvalue: starting always at 0 (offseted by minvalue)
** returning the pattern tweak value
*/
INLINE intptr_t psy_audio_machine_parameter_patternvalue(psy_audio_Machine*
	self, psy_audio_MachineParam* param)
{
	intptr_t rv;
	intptr_t minval;
	intptr_t maxval;
	intptr_t range;

	psy_audio_machine_parameter_range(self, param, &minval, &maxval);
	range = maxval - minval;
	rv = (intptr_t)(psy_audio_machine_parameter_normvalue(self, param) *
		range);
	return rv;
}

INLINE void psy_audio_machine_putdata(psy_audio_Machine* self, uint8_t* data)
{
	self->vtable->putdata(self, data);
}

INLINE  uint8_t* psy_audio_machine_data(psy_audio_Machine* self)
{
	return self->vtable->data(self);
}

INLINE  uintptr_t psy_audio_machine_datasize(psy_audio_Machine* self)
{
	return self->vtable->datasize(self);
}

INLINE psy_dsp_amp_range_t psy_audio_machine_amprange(psy_audio_Machine* self)
{
	return self->vtable->amprange(self);
}

INLINE void psy_audio_machine_programname(psy_audio_Machine* self, uintptr_t bnkidx,
	uintptr_t prgidx, char* val)
{
	self->vtable->programname(self, bnkidx, prgidx, val);
}

INLINE uintptr_t psy_audio_machine_numprograms(psy_audio_Machine* self)
{
	return self->vtable->numprograms(self);
}

INLINE void psy_audio_machine_setcurrprogram(psy_audio_Machine* self,
	intptr_t prgidx)
{
	self->vtable->setcurrprogram(self, prgidx);
}

INLINE intptr_t psy_audio_machine_currprogram(psy_audio_Machine* self)
{
	return self->vtable->currprogram(self);
}

INLINE void psy_audio_machine_bankname(psy_audio_Machine* self, intptr_t bnkidx,
	char* val)
{
	self->vtable->bankname(self, bnkidx, val);
}

INLINE intptr_t psy_audio_machine_numbanks(psy_audio_Machine* self)
{
	return self->vtable->numbanks(self);
}

INLINE void psy_audio_machine_setcurrbank(psy_audio_Machine* self, intptr_t prgidx)
{
	self->vtable->setcurrbank(self, prgidx);
}

INLINE intptr_t psy_audio_machine_currbank(psy_audio_Machine* self)
{
	return self->vtable->currbank(self);
}

INLINE void psy_audio_machine_currentpreset(psy_audio_Machine* self,
	struct psy_audio_Preset* preset)
{
	self->vtable->currentpreset(self, preset);
}

INLINE void psy_audio_machine_setpresets(psy_audio_Machine* self,
	struct psy_audio_Presets* presets)
{
	self->vtable->setpresets(self, presets);
}

INLINE struct psy_audio_Presets* psy_audio_machine_presets(
	psy_audio_Machine* self)
{
	return self->vtable->presets(self);
}

INLINE bool psy_audio_machine_acceptpresets(psy_audio_Machine* self)
{
	return self->vtable->acceptpresets(self);
}

INLINE const char* psy_audio_machine_auxcolumnname(psy_audio_Machine* self, uintptr_t index)
{
	return self->vtable->auxcolumnname(self, index);
}

INLINE uintptr_t psy_audio_machine_numauxcolumns(psy_audio_Machine* self)
{
	return self->vtable->numauxcolumns(self);
}

INLINE uintptr_t psy_audio_machine_auxcolumnselected(psy_audio_Machine* self)
{
	return self->vtable->auxcolumnselected(self);
}

INLINE void psy_audio_machine_selectauxcolumn(psy_audio_Machine* self, uintptr_t index)
{
	self->vtable->selectauxcolumn(self, index);
}

INLINE void psy_audio_machine_command(psy_audio_Machine* self)
{
	self->vtable->command(self);
}

INLINE int psy_audio_machine_haseditor(psy_audio_Machine* self)
{
	return self->vtable->haseditor(self);
}

INLINE void psy_audio_machine_seteditorhandle(psy_audio_Machine* self, void*
	handle)
{
	self->vtable->seteditorhandle(self, handle);
}

INLINE void psy_audio_machine_editorsize(psy_audio_Machine* self, double* width,
	double* height)
{
	self->vtable->editorsize(self, width, height);
}

INLINE void psy_audio_machine_editoridle(psy_audio_Machine* self)
{
	self->vtable->editoridle(self);
}

INLINE void psy_audio_machine_setposition(psy_audio_Machine* self, double x,
	double y)
{
	self->vtable->setposition(self, x, y);
}

INLINE void psy_audio_machine_position(psy_audio_Machine* self, double* x,
	double* y)
{
	self->vtable->position(self, x, y);
}

INLINE void psy_audio_machine_sethostview(psy_audio_Machine* self,
	struct psy_ui_Component* view)
{
	self->vtable->sethostview(self, view);
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
/* standby */
INLINE void psy_audio_machine_standby(psy_audio_Machine* self)
{
	self->vtable->standby(self);
}

INLINE void psy_audio_machine_deactivatestandby(psy_audio_Machine* self)
{
	self->vtable->deactivatestandby(self);
}

INLINE int psy_audio_machine_hasstandby(psy_audio_Machine* self)
{
	return self->vtable->hasstandby(self);
}
/* bus */
INLINE void psy_audio_machine_setbus(psy_audio_Machine* self)
{
	self->vtable->setbus(self);
}

INLINE void psy_audio_machine_unsetbus(psy_audio_Machine* self)
{
	self->vtable->unsetbus(self);
}

INLINE int psy_audio_machine_isbus(psy_audio_Machine* self)
{
	return self->vtable->isbus(self);
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

INLINE psy_audio_Buffer* psy_audio_machine_buffermemory(psy_audio_Machine*
	self)
{
	return self->vtable->buffermemory(self);
}

INLINE uintptr_t psy_audio_machine_buffermemorysize(psy_audio_Machine* self)
{
	return self->vtable->buffermemorysize(self);
}

INLINE void psy_audio_machine_setbuffermemorysize(psy_audio_Machine* self,
	uintptr_t size)
{
	self->vtable->setbuffermemorysize(self, size);
}

INLINE void psy_audio_machine_resetbuffermemory(psy_audio_Machine* self)
{
	psy_audio_machine_setbuffermemorysize(self, psy_audio_MAX_STREAM_SIZE);
}

void psy_audio_machine_updatememory(psy_audio_Machine*,
	psy_audio_BufferContext* bc);


INLINE psy_dsp_beat_t psy_audio_machine_bpm(psy_audio_Machine* self)
{
	return self->vtable->bpm(self);
}

INLINE struct psy_audio_SequencerTime* psy_audio_machine_sequencertime(psy_audio_Machine* self)	 
{
	return self->vtable->sequencertime(self);
}

INLINE psy_dsp_beat_t psy_audio_machine_beatspertick(psy_audio_Machine* self)
{
	return self->vtable->beatspertick(self);
}

INLINE psy_dsp_beat_t psy_audio_machine_samplespertick(psy_audio_Machine* self)
{
	return 1.f / self->vtable->beatspersample(self) *
		self->vtable->beatspertick(self);
}

INLINE uintptr_t psy_audio_machine_ticksperbeat(psy_audio_Machine* self)
{
	return (uintptr_t)(1 / self->vtable->beatspertick(self));
}

INLINE psy_dsp_beat_t psy_audio_machine_beatspersample(psy_audio_Machine* self)
{
	return self->vtable->beatspersample(self);
}

INLINE psy_dsp_beat_t psy_audio_machine_currbeatsperline(psy_audio_Machine*
	self)
{
	return self->vtable->currbeatsperline(self);
}

INLINE uintptr_t psy_audio_machine_currsamplesperrow(psy_audio_Machine* self)
{
	return (uintptr_t)(self->vtable->currbeatsperline(self) * 1.f /
		self->vtable->beatspersample(self));
}

INLINE psy_dsp_beat_t psy_audio_machine_currlinesperbeat(psy_audio_Machine*
	self)
{
	return 1.f / self->vtable->currbeatsperline(self);
}

INLINE psy_dsp_big_hz_t psy_audio_machine_samplerate(psy_audio_Machine* self)
{
	return self->vtable->samplerate(self);
}

INLINE struct psy_audio_Samples* psy_audio_machine_samples(psy_audio_Machine*
	self)
{
	return self->vtable->samples(self);
}

INLINE struct psy_audio_Machines* psy_audio_machine_machines(psy_audio_Machine*
	self)
{
	return self->vtable->machines(self);
}

INLINE struct psy_audio_Instruments* psy_audio_machine_instruments(
	psy_audio_Machine* self)
{
	return self->vtable->instruments(self);
}

INLINE bool psy_audio_machine_editresize(psy_audio_Machine* self,
	uintptr_t width, uintptr_t height)
{
	return self->vtable->editresize(self, width, height);
}

INLINE void psy_audio_machine_buschanged(psy_audio_Machine* self)
{
	self->vtable->buschanged(self);
}

INLINE struct psy_audio_MachineFactory* psy_audio_machine_machinefactory(
	psy_audio_Machine* self)
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

INLINE void psy_audio_machine_readbuffers(psy_audio_Machine* self, int index,
	float** pleft, float** pright, uintptr_t numsamples)
{
	self->vtable->readbuffers(self, index, pleft, pright, numsamples);
}

INLINE const char* psy_audio_machine_capturename(psy_audio_Machine* self, int
	index)
{
	return self->vtable->capturename(self, index);
}

INLINE int psy_audio_machine_numcaptures(psy_audio_Machine* self)
{
	return self->vtable->numcaptures(self);
}

INLINE const char* psy_audio_machine_playbackname(psy_audio_Machine* self, int
	index)
{
	return self->vtable->playbackname(self, index);
}

INLINE int psy_audio_machine_numplaybacks(psy_audio_Machine* self)
{
	return self->vtable->numplaybacks(self);
}

INLINE const char* psy_audio_machine_language(psy_audio_Machine* self)
{
	return self->vtable->language(self);
}

INLINE void psy_audio_machine_output(psy_audio_Machine* self, const char* text)
{
	self->vtable->output(self, text);
}

INLINE psy_audio_CpuTimeClock psy_audio_machine_cpu_time(
	psy_audio_Machine* self)
{
	return self->cpu_time;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_audio_MACHINE_H */
