// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machine.h"
#include "machines.h"
#include "pattern.h"
#include "songio.h"
#include <string.h>
#include <operations.h>
#include <valuemapper.h>
#include "plugin_interface.h"
#include "constants.h"
#include "wire.h"
#include "song.h"
#include "player.h"

#include <stdlib.h>

#include "../../detail/trace.h"
#include "../../detail/portable.h"

/// Machinecallback
static uintptr_t machinecallback_samplerate(psy_audio_MachineCallback* self)
{
	return (self->player && self->player->driver)
		? psy_audiodriver_samplerate(self->player->driver)
		: 44100;
}

static psy_dsp_beat_t machinecallback_bpm(psy_audio_MachineCallback* self)
{ 
	return (self->player)
		? (psy_dsp_beat_t)psy_audio_player_bpm(self->player)
		: 125.f;
}

static psy_dsp_beat_t machinecallback_beatspertick(psy_audio_MachineCallback* self)
{
	assert(self);

	return (self->song)
		? 1 / (psy_dsp_beat_t)self->song->properties.tpb
		: 1 / (psy_dsp_beat_t)24.f;
}

static psy_dsp_beat_t machinecallback_beatspersample(psy_audio_MachineCallback* self)
{
	return (self->player)
		? (psy_dsp_beat_t)psy_audio_sequencer_beatspersample(&self->player->sequencer)
		: 512.f;
}

static psy_dsp_beat_t machinecallback_currbeatsperline(psy_audio_MachineCallback* self)
{
	return (self->player && self->player->driver)
		? (psy_dsp_beat_t)psy_audio_sequencer_currbeatsperline(&self->player->sequencer)
		: 4096;
}

static struct psy_audio_Samples* machinecallback_samples(psy_audio_MachineCallback* self)
{
	if (self->song) {
		return &self->song->samples;
	}
	return NULL;
}

static psy_audio_Machines* machinecallback_machines(psy_audio_MachineCallback* self)
{
	if (self->song) {
		return &self->song->machines;
	}
	return NULL;
}

static struct psy_audio_Instruments* machinecallback_instruments(psy_audio_MachineCallback* self)
{
	if (self->song) {
		return &self->song->instruments;
	}
	return NULL;
}

static struct psy_audio_MachineFactory* machinecallback_machinefactory(psy_audio_MachineCallback*
	self) {
	return 0;
}
static bool machinecallback_fileselect_load(psy_audio_MachineCallback* self) { return FALSE; }
static bool machinecallback_fileselect_save(psy_audio_MachineCallback* self) { return FALSE; }
static void machinecallback_fileselect_directory(psy_audio_MachineCallback* self) { }
static void machinecallback_output(psy_audio_MachineCallback* self, const char* text) { }

static bool machinecallback_addcapture(psy_audio_MachineCallback* self, int index)
{
	return (self->player && self->player->driver)
		? psy_audiodriver_addcapture(self->player->driver, index)
		: FALSE;
}

static bool machinecallback_removecapture(psy_audio_MachineCallback* self, int index)
{
	return (self->player && self->player->driver)
		? psy_audiodriver_removecapture(self->player->driver, index)
		: FALSE;
}

static void machinecallback_readbuffers(psy_audio_MachineCallback* self, int index, float** pleft,
	float** pright, int numsamples)
{
	if (self->player && self->player->driver) {
		psy_audiodriver_readbuffers(self->player->driver, index, pleft, pright, numsamples);
	}
}
static const char* machinecallback_capturename(psy_audio_MachineCallback* self, int index) {
	return (self->player && self->player->driver)
		? psy_audiodriver_capturename(self->player->driver, index)
		: "";
}

static int machinecallback_numcaptures(psy_audio_MachineCallback* self)
{
	return (self->player && self->player->driver)
		? psy_audiodriver_numcaptures(self->player->driver)
		: 0;	
}

static const char* machinecallback_playbackname(psy_audio_MachineCallback* self, int index)
{
	return (self->player && self->player->driver)
		? psy_audiodriver_playbackname(self->player->driver, index)
		: "";
}
static int machinecallback_numplaybacks(psy_audio_MachineCallback* self) {
	return (self->player && self->player->driver)
		? psy_audiodriver_numplaybacks(self->player->driver)
		: 0;	
}

// MachineCallback VTable
static psy_audio_MachineCallbackVtable psy_audio_machinecallbackvtable_vtable;
static int psy_audio_machinecallbackvtable_initialized = 0;

static void psy_audio_machinecallbackvtable_init(void)
{
	if (!psy_audio_machinecallbackvtable_initialized) {
		psy_audio_machinecallbackvtable_vtable.samplerate = (fp_mcb_samplerate)
			machinecallback_samplerate;
		psy_audio_machinecallbackvtable_vtable.bpm = (fp_mcb_bpm)
			machinecallback_bpm;
		psy_audio_machinecallbackvtable_vtable.beatspertick = (fp_mcb_beatspertick)
			machinecallback_beatspertick;
		psy_audio_machinecallbackvtable_vtable.beatspersample = (fp_mcb_beatspersample)
			machinecallback_beatspersample;
		psy_audio_machinecallbackvtable_vtable.currbeatsperline = (fp_mcb_currbeatsperline)
			machinecallback_currbeatsperline;
		psy_audio_machinecallbackvtable_vtable.samples = (fp_mcb_samples)
			machinecallback_samples;
		psy_audio_machinecallbackvtable_vtable.machines = (fp_mcb_machines)
			machinecallback_machines;
		psy_audio_machinecallbackvtable_vtable.instruments = (fp_mcb_instruments)
			machinecallback_instruments;
		psy_audio_machinecallbackvtable_vtable.machinefactory = (fp_mcb_machinefactory)
			machinecallback_machinefactory;
		psy_audio_machinecallbackvtable_vtable.fileselect_load = (fp_mcb_fileselect_load)
			machinecallback_fileselect_load;
		psy_audio_machinecallbackvtable_vtable.fileselect_save = (fp_mcb_fileselect_save)
			machinecallback_fileselect_save;
		psy_audio_machinecallbackvtable_vtable.fileselect_directory =
			(fp_mcb_fileselect_directory)machinecallback_fileselect_directory;
		psy_audio_machinecallbackvtable_vtable.output = (fp_mcb_output)
			machinecallback_output;
		psy_audio_machinecallbackvtable_vtable.addcapture = (fp_mcb_addcapture)
			machinecallback_addcapture;
		psy_audio_machinecallbackvtable_vtable.removecapture = (fp_mcb_removecapture)
			machinecallback_removecapture;
		psy_audio_machinecallbackvtable_vtable.readbuffers = (fp_mcb_readbuffers)
			machinecallback_readbuffers;
		psy_audio_machinecallbackvtable_vtable.capturename = (fp_mcb_capturename)
			machinecallback_capturename;
		psy_audio_machinecallbackvtable_vtable.numcaptures = (fp_mcb_numcaptures)
			machinecallback_numcaptures;
		psy_audio_machinecallbackvtable_vtable.playbackname = (fp_mcb_playbackname)
			machinecallback_playbackname;
		psy_audio_machinecallbackvtable_vtable.numplaybacks = (fp_mcb_numplaybacks)
			machinecallback_numplaybacks;
		psy_audio_machinecallbackvtable_initialized = 1;
	}
}
void psy_audio_machinecallback_init(psy_audio_MachineCallback* self,
	psy_audio_Player* player,
	psy_audio_Song* song)
{
	psy_audio_machinecallbackvtable_init();
	self->vtable = &psy_audio_machinecallbackvtable_vtable;
	self->player = player;
	self->song = song;
}

void psy_audio_machinecallback_setsong(psy_audio_MachineCallback* self,
	psy_audio_Song* song)
{
	self->song = song;
}

static psy_audio_MachineInfo const macinfo = {	
	MI_VERSION,
	0x0250,
	EFFECT | 32 | 64,
	MACHMODE_FX,
	"Machine"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,
	"Machine",
	"Psycledelics",
	"help",
	MACH_UNDEFINED,
	0,
	0,	
	""
};

struct psy_audio_Player;

static void reload(psy_audio_Machine* self) {}
static psy_audio_Machine* clone(psy_audio_Machine* self) { return 0; }
static psy_audio_Buffer* mix(psy_audio_Machine*, size_t slot, uintptr_t amount,
	psy_audio_MachineSockets*, psy_audio_Machines*, struct psy_audio_Player*);
static void work(psy_audio_Machine*, psy_audio_BufferContext*);
static uintptr_t work_dogenerateaudio(psy_audio_Machine*, psy_audio_BufferContext*,
	uintptr_t position, uintptr_t amount);
static void work_entry(psy_audio_Machine*, psy_audio_PatternEntry*);
static void work_memory(psy_audio_Machine*, psy_audio_BufferContext*);
static void generateaudio(psy_audio_Machine* self, psy_audio_BufferContext* bc) { }
static int hostevent(psy_audio_Machine* self, int const eventNr, int const val1, float const val2) { return 0; }
static void seqtick(psy_audio_Machine* self, uintptr_t channel,
	const psy_audio_PatternEvent* event) { }
static void stop(psy_audio_Machine* self) { }
static void sequencertick(psy_audio_Machine* self) { }
static psy_List* sequencerinsert(psy_audio_Machine* self, psy_List* events) { return 0; }
static void newline(psy_audio_Machine* self) { }
static const psy_audio_MachineInfo* info(psy_audio_Machine* self) { return &macinfo; }
// parameters
static psy_audio_MachineParam* parameter(psy_audio_Machine* self,
	uintptr_t param)
{
	return 0;
}
static psy_audio_MachineParam* tweakparameter(psy_audio_Machine* self,
	uintptr_t param)
{
	return psy_audio_machine_parameter(self, param);
}
static uintptr_t numparameters(psy_audio_Machine* self) { return 0; }
static uintptr_t numtweakparameters(psy_audio_Machine* self) 
{
	return psy_audio_machine_numparameters(self);
}
static void setpanning(psy_audio_Machine* self, psy_dsp_amp_t panning) { }
static psy_dsp_amp_t panning(psy_audio_Machine* self) { return (psy_dsp_amp_t) 0.5f; }
static void mute(psy_audio_Machine* self) { }
static void unmute(psy_audio_Machine* self) { }
static int muted(psy_audio_Machine* self) { return 0; }
static void bypass(psy_audio_Machine* self) { }
static void unbypass(psy_audio_Machine* self) { }
static int bypassed(psy_audio_Machine* self) { return 0; }
static void dispose(psy_audio_Machine*);
static int mode(psy_audio_Machine*);
static const char* modulepath(psy_audio_Machine* self) { return NULL; }
static uintptr_t shellidx(psy_audio_Machine* self) { return 0; }
static uintptr_t numinputs(psy_audio_Machine* self) { return 0; }
static uintptr_t numoutputs(psy_audio_Machine* self) { return 0; }	
static void setcallback(psy_audio_Machine* self, psy_audio_MachineCallback* callback) { self->callback = callback; }
static void updatesamplerate(psy_audio_Machine* self, unsigned int samplerate) { }
static void loadspecific(psy_audio_Machine*, struct psy_audio_SongFile*,
	uintptr_t slot);
static void loadwiremapping(psy_audio_Machine*, struct psy_audio_SongFile*,
	uintptr_t slot);
static uintptr_t machine_countinputlegacywires(psy_audio_Machine*, psy_Table* legacywiretable);
static void machine_readpinmapping(psy_audio_Machine*, psy_audio_SongFile*,
	psy_audio_PinMapping*);
static void savespecific(psy_audio_Machine*, struct psy_audio_SongFile*,
	uintptr_t slot);
static void savewiremapping(psy_audio_Machine*, struct psy_audio_SongFile*,
	uintptr_t slot);
static void postload(psy_audio_Machine*, struct psy_audio_SongFile*,
	uintptr_t slot);
static unsigned int numparametercols(psy_audio_Machine* self) { return 0; }
static uintptr_t slot(psy_audio_Machine* self) { return UINTPTR_MAX; }
static void setslot(psy_audio_Machine* self, uintptr_t slot) { }
static int haseditor(psy_audio_Machine* self) { return 0; }
static void seteditorhandle(psy_audio_Machine* self, void* handle) { }
static void editorsize(psy_audio_Machine* self, int* width, int* height)
{ 
	*width = 0;
	*height = 0;
}
static void editoridle(psy_audio_Machine* self) { }
static void setposition(psy_audio_Machine* self, intptr_t x, intptr_t y) { }
static void position(psy_audio_Machine* self, intptr_t* x, intptr_t* y) { *x = *y = 0; }
static const char* editname(psy_audio_Machine* self) { return ""; }
static void seteditname(psy_audio_Machine* self, const char* name) { }
static psy_audio_Buffer* buffermemory(psy_audio_Machine* self) { return 0; }
static uintptr_t buffermemorysize(psy_audio_Machine* self) { return 0; }
static void setbuffermemorysize(psy_audio_Machine* self, uintptr_t size) { }
static psy_dsp_amp_range_t amprange(psy_audio_Machine* self)
{
	return PSY_DSP_AMP_RANGE_NATIVE;
}
// MachineParameter calls
static void param_tweak(psy_audio_Machine* self, psy_audio_MachineParam* param, float val)
{
	psy_audio_machineparam_tweak(param, val);
}

static void param_reset(psy_audio_Machine* self, psy_audio_MachineParam* param)
{
	psy_audio_machineparam_reset(param);
}

static float param_normvalue(psy_audio_Machine* self, psy_audio_MachineParam* param)
{
	return psy_audio_machineparam_normvalue(param);
}

static void param_range(psy_audio_Machine* self, psy_audio_MachineParam* param,
	intptr_t* minval, intptr_t* maxval)
{
	psy_audio_machineparam_range(param, minval, maxval);
}

static int param_type(psy_audio_Machine* self, psy_audio_MachineParam* param)
{
	return psy_audio_machineparam_type(param);
}

static int param_label(psy_audio_Machine* self, psy_audio_MachineParam* param, char* text)
{
	return psy_audio_machineparam_label(param, text);
}

static int param_name(psy_audio_Machine* self, psy_audio_MachineParam* param, char* text)
{
	return psy_audio_machineparam_name(param, text);
}

static int param_describe(psy_audio_Machine* self, psy_audio_MachineParam* param, char* text)
{
	return psy_audio_machineparam_describe(param, text);
}
// data
static void putdata(psy_audio_Machine* self, uint8_t* data) { }
static uint8_t* data(psy_audio_Machine* self) { return NULL; }
static uintptr_t datasize(psy_audio_Machine* self) { return 0; }
// programs
static void programname(psy_audio_Machine* self, int bnkidx, int prgIdx, char* val)
{
	psy_snprintf(val, 256, "%s", "Program 0");
}
static int numprograms(psy_audio_Machine* self)
{
	return 0;
}
static void setcurrprogram(psy_audio_Machine* self, int prgIdx) { }
static int currprogram(psy_audio_Machine* self)
{
	return 0;
}
static void bankname(psy_audio_Machine* self, int bnkidx, char* val)
{
	psy_snprintf(val, 256, "%s", "Internal");
}
static int numbanks(psy_audio_Machine* self) { return 1; }
static void setcurrbank(psy_audio_Machine* self, int prgIdx) { }
static int currbank(psy_audio_Machine* self)
{
	return 0;
}
static void currentpreset(psy_audio_Machine* self, struct psy_audio_Preset* preset) {}
static void setpresets(psy_audio_Machine* self, struct psy_audio_Presets* presets) {}
static struct psy_audio_Presets* presets(psy_audio_Machine* self) { return NULL; }
static bool acceptpresets(psy_audio_Machine* self) { return FALSE; }
static void command(psy_audio_Machine* self) { }

/// machinecallback
static uintptr_t samplerate(psy_audio_Machine* self) {
	return (self->callback)
		? self->callback->vtable->samplerate(self->callback)
		: 44100;
}

static psy_dsp_beat_t bpm(psy_audio_Machine* self)
{
	return (self->callback)
		? self->callback->vtable->bpm(self->callback)
		: 125.f;
}

static psy_dsp_beat_t beatspertick(psy_audio_Machine* self) {
	return (self->callback)
		? self->callback->vtable->beatspertick(self->callback)
		: 1 / 256.f;
}

static psy_dsp_beat_t beatspersample(psy_audio_Machine* self)
{
	return (self->callback)
		? self->callback->vtable->beatspersample(self->callback)
		: 1 / 896.f;
}

static psy_dsp_beat_t currbeatsperline(psy_audio_Machine* self)
{
	return (self->callback)
		? self->callback->vtable->currbeatsperline(self->callback)
		: (psy_dsp_beat_t) 4.0;
}

static struct psy_audio_Samples* samples(psy_audio_Machine* self)
{
	return (self->callback)
		? self->callback->vtable->samples(self->callback)
		: NULL;
}

static struct psy_audio_Machines* machines(psy_audio_Machine* self)
{
	return (self->callback)
		? self->callback->vtable->machines(self->callback)
		: NULL;
}

static struct psy_audio_Instruments* instruments(psy_audio_Machine* self)
{
	return (self->callback)
		? self->callback->vtable->instruments(self->callback)
		: NULL;
}

static struct psy_audio_MachineFactory* machinefactory(psy_audio_Machine* self)
{
	return (self->callback)
		? self->callback->vtable->machinefactory(self->callback)
		: NULL;
}

static void output(psy_audio_Machine* self, const char* text)
{
	if (self->callback) {
		self->callback->vtable->output(self->callback, text);
	}
}

static bool addcapture(psy_audio_Machine* self, int index)
{
	return (self->callback)
		? self->callback->vtable->addcapture(self->callback, index)
		: FALSE;
}

static bool removecapture(psy_audio_Machine* self, int index)
{
	return (self->callback)
		? self->callback->vtable->removecapture(self->callback, index)
		: FALSE;
}

static void readbuffers(psy_audio_Machine* self, int index, float** pleft, float** pright, int numsamples)
{ 
	if (self->callback) {
		self->callback->vtable->readbuffers(self->callback, index, pleft, pright, numsamples);
	}
}

static const char* capturename(psy_audio_Machine* self, int index)
{
	return (self->callback)
		? self->callback->vtable->capturename(self->callback, index)
		: "";
}

static int numcaptures(psy_audio_Machine* self)
{
	return (self->callback)
		? self->callback->vtable->numcaptures(self->callback)
		: 0;
}

static const char* playbackname(psy_audio_Machine* self, int index)
{
	return (self->callback)
		? self->callback->vtable->playbackname(self->callback, index)
		: "";
}

static int numplaybacks(psy_audio_Machine* self)
{
	return (self->callback)
		? self->callback->vtable->numplaybacks(self->callback)
		: 0;
}

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.clone = clone;
		vtable.dispose = machine_base_dispose;
		vtable.reload = reload;
		vtable.mix = mix;
		vtable.work = work;
		vtable.mode = mode;
		vtable.modulepath = modulepath;
		vtable.shellidx = shellidx;
		vtable.hostevent = hostevent;
		vtable.seqtick = seqtick;
		vtable.stop = stop;
		vtable.sequencertick = sequencertick;
		vtable.newline = newline;
		vtable.sequencerinsert = sequencerinsert;
		vtable.info = info;
		vtable.parameter = parameter;
		vtable.tweakparameter = tweakparameter;
		vtable.setpanning = setpanning;
		vtable.panning = panning;
		vtable.mute = mute;
		vtable.unmute = unmute;
		vtable.muted = muted;
		vtable.bypass = bypass;
		vtable.unbypass = unbypass;
		vtable.bypassed = bypassed;
		vtable.generateaudio = generateaudio;
		vtable.numinputs = numinputs;
		vtable.numoutputs = numoutputs;	
		vtable.numparameters = numparameters;
		vtable.numtweakparameters = numtweakparameters;
		vtable.numparametercols = numparametercols;
		vtable.setcallback = setcallback;
		vtable.updatesamplerate = updatesamplerate;
		vtable.loadspecific = loadspecific;
		vtable.loadwiremapping = loadwiremapping;
		vtable.savespecific = savespecific;
		vtable.savewiremapping = savewiremapping;
		vtable.postload = postload;
		vtable.bpm = bpm;
		vtable.beatspertick = beatspertick;
		vtable.beatspersample = beatspersample;
		vtable.currbeatsperline = currbeatsperline;
		vtable.samplerate = samplerate;
		vtable.instruments = instruments;
		vtable.machinefactory = machinefactory;
		vtable.samples = samples;
		vtable.machines = machines;
		vtable.output = output;
		vtable.addcapture = addcapture;
		vtable.removecapture = removecapture;
		vtable.readbuffers = readbuffers;
		vtable.capturename = capturename;
		vtable.numcaptures = numcaptures;
		vtable.playbackname = playbackname;
		vtable.numplaybacks = numplaybacks;
		vtable.slot = slot;
		vtable.setslot = setslot;		
		vtable.haseditor = haseditor;
		vtable.seteditorhandle = seteditorhandle;
		vtable.editorsize = editorsize;
		vtable.editoridle = editoridle;
		vtable.setposition = setposition;
		vtable.position = position;
		vtable.editname = editname;
		vtable.seteditname = seteditname;
		vtable.buffermemory = buffermemory;
		vtable.buffermemorysize = buffermemorysize;
		vtable.setbuffermemorysize = setbuffermemorysize;
		vtable.amprange = amprange;
		vtable.putdata = putdata;
		vtable.data = data;
		vtable.datasize = datasize;
		vtable.programname = programname;
		vtable.numprograms = numprograms;
		vtable.setcurrprogram = setcurrprogram;
		vtable.currprogram = currprogram;
		vtable.bankname = bankname;
		vtable.numbanks = numbanks;
		vtable.setcurrbank = setcurrbank;
		vtable.currbank = currbank;
		vtable.currentpreset = currentpreset;
		vtable.setpresets = setpresets;
		vtable.presets = presets;
		vtable.acceptpresets = acceptpresets;
		vtable.command = command;
		vtable.parameter_tweak = param_tweak;
		vtable.parameter_reset = param_reset;
		vtable.parameter_normvalue = param_normvalue;
		vtable.parameter_range = param_range;
		vtable.parameter_type = param_type;
		vtable.parameter_label = param_label;
		vtable.parameter_name = param_name;
		vtable.parameter_describe = param_describe;
		vtable_initialized = 1;
	}
}

void psy_audio_machine_init(psy_audio_Machine* self, psy_audio_MachineCallback*
	callback)
{		
	memset(self, 0, sizeof(psy_audio_Machine));
	vtable_init();
	self->vtable = &vtable;
	self->callback = callback;
	self->err = 0;
	psy_audio_cputimeclock_init(&self->cpu_time);
	psy_signal_init(&self->signal_worked);	
}

void machine_base_dispose(psy_audio_Machine* self)
{
	psy_signal_dispose(&self->signal_worked);
}

void work(psy_audio_Machine* self, psy_audio_BufferContext* bc)
{
	psy_List* p;
	uintptr_t amount;
	uintptr_t pos;

	amount = psy_audio_buffercontext_numsamples(bc);
	for (pos = 0, p = bc->events; p != NULL; psy_list_next(&p)) {
		psy_audio_PatternEntry* entry;

		entry = (psy_audio_PatternEntry*)psy_list_entry(p);
		if (((uintptr_t)entry->delta) >= pos) {
			amount -= work_dogenerateaudio(self, bc, pos,
				(uintptr_t)entry->delta - pos);
			work_entry(self, entry);
			pos = (uintptr_t)entry->delta;
		}
	}
	work_dogenerateaudio(self, bc, pos, amount);
	psy_audio_buffercontext_setoffset(bc, 0);
}

uintptr_t work_dogenerateaudio(psy_audio_Machine* self, psy_audio_BufferContext* bc,
	uintptr_t position, uintptr_t amount)
{
	if (amount > 0 && !psy_audio_machine_bypassed(self) &&
			!psy_audio_machine_muted(self)) {
		uintptr_t restorenumsamples;

		restorenumsamples = psy_audio_buffercontext_numsamples(bc);		
		psy_audio_buffercontext_setnumsamples(bc, amount);
		psy_audio_buffercontext_setoffset(bc, position);
		psy_audio_machine_generateaudio(self, bc);		
		psy_audio_buffercontext_setnumsamples(bc, restorenumsamples);
		psy_audio_buffercontext_setoffset(bc, 0);
	}
	return amount;
}

void work_entry(psy_audio_Machine* self, psy_audio_PatternEntry* entry)
{
	psy_List* p;

	for (p = entry->events; p != NULL; psy_list_next(&p)) {
		psy_audio_PatternEvent* ev;

		ev = (psy_audio_PatternEvent*)psy_list_entry(p);
		if (ev->note == psy_audio_NOTECOMMANDS_TWEAK) {
			if (ev->inst < psy_audio_machine_numparameters(self)) {
				psy_audio_MachineParam* param;				
				
				param = psy_audio_machine_tweakparameter(self, ev->inst);
				if (param) {
					uint16_t v;

					v = psy_audio_patternevent_tweakvalue(ev);
					if (ev->vol > 0) {
						int32_t curr;
						int32_t step;
						int32_t nv;

						curr = psy_audio_machine_parameter_patternvalue(self, param);
						step = (v - curr) / ev->vol;
						nv = curr + step;						
						if (nv < 0) {
							nv = 0;
						}
						psy_audio_machine_parameter_tweak_pattern(self, param, (uint16_t) nv);
					} else {
						psy_audio_machine_parameter_tweak_pattern(self, param, v);
					}
				}
			}
		} else 
		if (ev->note == psy_audio_NOTECOMMANDS_EMPTY && ev->cmd == psy_audio_PATTERNCMD_EXTENDED) {
			if ((ev->parameter & 0xF0) == psy_audio_PATTERNCMD_SET_BYPASS) {
				if ((ev->parameter & 0x0F) == 0) {
					psy_audio_machine_unbypass(self);
				} else {
					psy_audio_machine_bypass(self);
				}
			} else
			if ((ev->parameter & 0xF0) == psy_audio_PATTERNCMD_SET_MUTE) {
				if ((ev->parameter & 0x0F) == 0) {
					psy_audio_machine_unmute(self);
				} else {
					psy_audio_machine_mute(self);
				}
			}			
		} else {
			psy_audio_machine_seqtick(self, entry->track, ev);
		}
	}
}

void psy_audio_machine_updatememory(psy_audio_Machine* machine,
	psy_audio_BufferContext* bc)
{
	if (bc->output) {
		psy_audio_Buffer* memory;

		memory = psy_audio_machine_buffermemory(machine);
		if (memory) {			
			psy_audio_buffer_writesamples(memory, bc->output,
				psy_audio_machine_buffermemorysize(machine),
				psy_audio_buffercontext_numsamples(bc));
		}
	}
}

int mode(psy_audio_Machine* self)
{ 
	const psy_audio_MachineInfo* info;

	info = psy_audio_machine_info(self);
	return (!info) ? MACHMODE_FX : info->mode;
}

int machine_supports(psy_audio_Machine* self, int option)
{
	if (psy_audio_machine_info(self)) {
		return (psy_audio_machine_info(self)->Flags & option) == option;
	}
	return 0;
}

psy_audio_Buffer* mix(psy_audio_Machine* self,
	size_t slot,
	uintptr_t amount,
	psy_audio_MachineSockets* sockets,
	psy_audio_Machines* machines,
	struct psy_audio_Player* player)
{			
	psy_audio_Buffer* output;

	output = psy_audio_machines_outputs(machines, slot);
	if (output) {
		if (output->preventmixclear == FALSE) {
			psy_audio_buffer_clearsamples(output, amount);
		}
		if (sockets) {
			psy_TableIterator it;

			for (it = psy_audio_wiresockets_begin(&sockets->inputs);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
				psy_audio_WireSocket* source;

				source = (psy_audio_WireSocket*)psy_tableiterator_value(&it);				
				psy_audio_buffer_mixsamples(output,
					psy_audio_machines_outputs(machines, source->slot),
					amount, source->volume, &source->mapping);
			}							
		}
	}
	return output;
}

void loadspecific(psy_audio_Machine* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	uint32_t numparams;
	uint32_t i;
	
	psyfile_read(songfile->file, &size, sizeof(size));	
	psyfile_read(songfile->file, &numparams, sizeof(numparams));
	for (i = 0; i < numparams; ++i) {
		uint32_t temp;
		psy_audio_MachineParam* param;

		psyfile_read(songfile->file, &temp, sizeof(temp));
		param = psy_audio_machine_tweakparameter(self, i);
		if (param) {
			intptr_t minval;
			intptr_t maxval;
			float value;

			psy_audio_machineparam_range(param, &minval, &maxval);
			value = ((maxval - minval) != 0)
				? (temp - minval) / (float)(maxval - minval)
				: 0.f;
			psy_audio_machineparam_tweak(param, value);
		}		
	}
	psyfile_skip(songfile->file, size - sizeof(numparams) - (numparams * sizeof(uint32_t)));
}

void loadwiremapping(psy_audio_Machine* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uintptr_t numwires = 0;
	uintptr_t countwires;
	psy_Table* legacywiretable;

	legacywiretable = psy_audio_legacywires_at(songfile->legacywires, slot);
	if (!legacywiretable) {
		return;
	}
	numwires = machine_countinputlegacywires(self, legacywiretable);	
	for (countwires = 0; countwires < numwires; ++countwires) {
		psy_audio_LegacyWire* wire;
		int32_t wireidx;
		
		psyfile_read(songfile->file, &wireidx, sizeof(wireidx));
		wire = psy_table_at(legacywiretable, wireidx);
		if (!wire) {
			// we cannot ensure correctness from now onwards.
			psy_audio_songfile_warn(songfile,
				"loadwiremapping wire not found");
			return;
		}
		if (wireidx >= MAX_CONNECTIONS) {
			psy_audio_songfile_warn(songfile,
				"loadwiremapping old psy3 max connections limit reached");
		}
		machine_readpinmapping(self, songfile, &wire->pinmapping);
	}
}

uintptr_t machine_countinputlegacywires(psy_audio_Machine* self, psy_Table* legacywiretable)
{	
	uintptr_t rv = 0;
	psy_TableIterator it;
	
	for (it = psy_table_begin(legacywiretable);
			!psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_audio_LegacyWire* legacywire;

		legacywire = (psy_audio_LegacyWire*)psy_tableiterator_value(&it);
		if (legacywire->_inputCon) {
			++rv;
		}
	}
	return rv;
}

void machine_readpinmapping(psy_audio_Machine* self, psy_audio_SongFile* songfile,
	psy_audio_PinMapping* pinmapping)
{
	int32_t numpairs;
	int32_t j;

	psyfile_read(songfile->file, &numpairs, sizeof(numpairs));
	psy_audio_pinmapping_clear(pinmapping);
	for (j = 0; j < numpairs; ++j) {
		int16_t src;
		int16_t dst;

		psyfile_read(songfile->file, &src, sizeof(src));
		psyfile_read(songfile->file, &dst, sizeof(dst));
		psy_audio_pinmapping_connect(pinmapping, src, dst);
	}
}

void savespecific(psy_audio_Machine* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{		
	uint32_t numparams;
	uint32_t size;
	uint32_t i;
		
	numparams = (uint32_t) psy_audio_machine_numtweakparameters(self);
	size = sizeof(numparams) + (numparams * sizeof(numparams));		
	psyfile_write(songfile->file, &size, sizeof(size));
	psyfile_write(songfile->file, &numparams, sizeof(numparams));
	for (i = 0; i < numparams; ++i) {
		int32_t scaled = 0;
		psy_audio_MachineParam* param;

		param = psy_audio_machine_tweakparameter(self, i);
		if (param) {
			scaled = psy_audio_machine_parameter_scaledvalue(self, param);
		}		
		psyfile_write_int32(songfile->file, scaled);
	}
}

void savewiremapping(psy_audio_Machine* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	psy_audio_Connections* connections;
	psy_audio_MachineSockets* sockets;	
	uintptr_t i;

	connections = &songfile->song->machines.connections;
	sockets = psy_audio_connections_at(connections, slot);
	if (sockets) {
		psy_TableIterator it;

		for (it = psy_audio_wiresockets_begin(&sockets->inputs);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			psy_audio_WireSocket* input_socket;
			psy_List* node;
			uint32_t numPairs;

			input_socket = (psy_audio_WireSocket*)psy_tableiterator_value(&it);							
			numPairs = psy_list_size(input_socket->mapping.container);
			i = psy_tableiterator_key(&it);

			if (i <= INT32_MAX) {
				psyfile_write_int32(songfile->file, (int32_t)i);
			} else {
				psy_audio_songfile_warn(songfile,
					"savewiremapping connection id reached 32 bit limit");
				return;
			}
			if (i <= INT32_MAX) {
				psyfile_write_int32(songfile->file, (int32_t)numPairs);
			} else {
				psy_audio_songfile_warn(songfile,
					"savewiremapping number of pinconnections reached 32 bit limit");
				return;
			}
			for (node = input_socket->mapping.container; node != NULL; node = node->next) {
				psy_audio_PinConnection* pair;

				pair = (psy_audio_PinConnection*)node->entry;
				if (pair->src <= INT16_MAX) {
					psyfile_write_int16(songfile->file, (int16_t)pair->src);
				} else {
					psy_audio_songfile_warn(songfile,
						"savewiremapping src pin number reached 16 bit limit");
					return;
				}
				if (pair->dst <= INT16_MAX) {
					psyfile_write_int16(songfile->file, (int16_t)pair->dst);
				} else {
					psy_audio_songfile_warn(songfile,
						"savewiremapping dst pin number reached 16 bit limit");
					return;
				}
			}			
		}	
	}
}

void postload(psy_audio_Machine* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uintptr_t c;
	psy_Table* legacywiretable;

	legacywiretable = psy_audio_legacywires_at(songfile->legacywires, slot);
	if (!legacywiretable) {
		return;
	}

	for (c = 0; c < MAX_CONNECTIONS; ++c) {
		psy_audio_LegacyWire* wire;
		psy_audio_Machine* inputmachine;
		uintptr_t f;
				
		wire = psy_table_at(legacywiretable, c);
		if (!wire) {
			continue;
		}
		//load bugfix: Ensure no duplicate wires could be created.
		for (f = 0; f < c; f++) {
			psy_audio_LegacyWire* legacywire;

			legacywire = psy_table_at(legacywiretable, f);
			if (!legacywire) {
				continue;
			}
			if (wire->_inputCon && legacywire->_inputCon &&
				wire->_inputMachine == legacywire->_inputMachine) {
				wire->_inputCon = FALSE;
			}
		}
		inputmachine = psy_audio_machines_at(&songfile->song->machines, wire->_inputMachine);
		if (wire->_inputCon
			&& wire->_inputMachine >= 0 && wire->_inputMachine < MAX_MACHINES
			&& slot != wire->_inputMachine && inputmachine)
		{
			// Do not create the hidden wire from mixer send to the send machine.
			int outWire = psy_audio_legacywires_findlegacyoutput(songfile->legacywires, wire->_inputMachine, slot);
			if (outWire != -1) {
				psy_audio_Wire newwire;

				newwire = psy_audio_wire_makeall(wire->_inputMachine, outWire, slot, c);
				//if (wire.pinMapping.size() > 0) {
				//	inWires[c].ConnectSource(*_pMachine[wire._inputMachine], 0
				//		, FindLegacyOutput(_pMachine[wire._inputMachine], _macIndex)
				//		, &wire.pinMapping);
				//} else {
				//	inWires[c].ConnectSource(*_pMachine[wire._inputMachine], 0
				//		, FindLegacyOutput(_pMachine[wire._inputMachine], _macIndex));
				//}
				//while (wire->_inputConVol * wire->_wireMultiplier > 8.0f) { //psycle 1.10.1 alpha bugfix
					//wire->_inputConVol /= 32768.f;
				//}
				//while (wire->_inputConVol > 0.f && wire->_inputConVol * wire->_wireMultiplier < 0.0002f) { //psycle 1.10.1 alpha bugfix
					//wire->_inputConVol *= 32768.f;
				//}
				//inWires[c].SetVolume(wire._inputConVol * wire._wireMultiplier);
				if (psy_audio_machines_valid_connection(
						&songfile->song->machines,
						newwire)) {
					psy_audio_machines_connect(&songfile->song->machines, newwire);
					psy_audio_connections_setwirevolume(&songfile->song->machines.connections,
						newwire,
						wire->_inputConVol * wire->_wireMultiplier);
					psy_audio_connections_setpinmapping(&songfile->song->machines.connections,
						newwire,
						&wire->pinmapping);
				}
			}
		}
	}	
}
