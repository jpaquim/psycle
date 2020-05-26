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

#include <stdlib.h>

#include "../../detail/trace.h"
#include "../../detail/portable.h"

/// Machinecallback
static uintptr_t machinecallback_samplerate(void* self) { return 44100;  }
static psy_dsp_beat_t machinecallback_bpm(void* self) { return 125.f; }
static psy_dsp_beat_t machinecallback_beatspertick(void* self) { return 1 / (psy_dsp_beat_t) 24.f; }
static psy_dsp_beat_t machinecallback_beatspersample(void* self) { return 512; }
static psy_dsp_beat_t machinecallback_currbeatsperline(void* self) { return 4096; }
static struct psy_audio_Samples* machinecallback_samples(void* self) { return 0; }
static psy_audio_Machines* machinecallback_machines(void* self) { return 0; }
static struct psy_audio_Instruments* machinecallback_instruments(void* self) { return 0; }
static struct psy_audio_MachineFactory* machinecallback_machinefactory(void* self) { return 0; }
static void machinecallback_fileselect_load(void* self) { }
static void machinecallback_fileselect_save(void* self) { }
static void machinecallback_fileselect_directory(void* self) { }
static void machinecallback_output(void* self, const char* text) { }
static bool machinecallback_addcapture(void* self, int index) { return 0; }
static bool machinecallback_removecapture(void* self, int index) { return 0; }
static void machinecallback_readbuffers(void* self, int index, float** pleft, float** pright, int numsamples) { }
static const char* machinecallback_capturename(void* self, int index) { return ""; }
static int machinecallback_numcaptures(void* self) { return 0; }
static const char* machinecallback_playbackname(void* self, int index) { return ""; }
static int machinecallback_numplaybacks(void* self) { return 0; }

void machinecallback_initempty(MachineCallback* self)
{
	self->samplerate = (fp_mcb_samplerate) machinecallback_samplerate;
	self->bpm = (fp_mcb_bpm) machinecallback_bpm;
	self->beatspertick = (fp_mcb_beatspertick) machinecallback_beatspertick;
	self->beatspersample = (fp_mcb_beatspersample) machinecallback_beatspersample;
	self->currbeatsperline = (fp_mcb_currbeatsperline) machinecallback_currbeatsperline;
	self->samples = (fp_mcb_samples) machinecallback_samples;
	self->machines = (fp_mcb_machines) machinecallback_machines;
	self->instruments = (fp_mcb_instruments) machinecallback_instruments;
	self->machinefactory = (fp_mcb_machinefactory) machinecallback_machinefactory;
	self->fileselect_load = (fp_mcb_fileselect_load) machinecallback_fileselect_load;
	self->fileselect_save = (fp_mcb_fileselect_save) machinecallback_fileselect_save;
	self->fileselect_directory = (fp_mcb_fileselect_directory) machinecallback_fileselect_directory;
	self->output = (fp_mcb_output) machinecallback_output;
	self->addcapture = (fp_mcb_addcapture) machinecallback_addcapture;
	self->removecapture = (fp_mcb_removecapture) machinecallback_removecapture;
	self->readbuffers = (fp_mcb_readbuffers) machinecallback_readbuffers;
	self->capturename = (fp_mcb_capturename) machinecallback_capturename;
	self->numcaptures = (fp_mcb_numcaptures) machinecallback_numcaptures;
	self->playbackname = (fp_mcb_playbackname) machinecallback_playbackname;
	self->numplaybacks = (fp_mcb_numplaybacks) machinecallback_numplaybacks;
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
	MACH_UNDEFINED
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
static void sequencerlinetick(psy_audio_Machine* self) { }
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
static uintptr_t numinputs(psy_audio_Machine* self) { return 0; }
static uintptr_t numoutputs(psy_audio_Machine* self) { return 0; }	
static void setcallback(psy_audio_Machine* self, MachineCallback callback) { self->callback = callback; }
static void updatesamplerate(psy_audio_Machine* self, unsigned int samplerate) { }
static void loadspecific(psy_audio_Machine*, struct psy_audio_SongFile*,
	uintptr_t slot);
static void savespecific(psy_audio_Machine*, struct psy_audio_SongFile*,
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
static const char* editname(psy_audio_Machine* self) { return ""; }
static void seteditname(psy_audio_Machine* self, const char* name) { }
static psy_audio_Buffer* buffermemory(psy_audio_Machine* self) { return 0; }
static uintptr_t buffermemorysize(psy_audio_Machine* self) { return 0; }
static void setbuffermemorysize(psy_audio_Machine* self, uintptr_t size) { }
static psy_dsp_amp_range_t amprange(psy_audio_Machine* self)
{
	return PSY_DSP_AMP_RANGE_NATIVE;
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
/// machinecallback
static uintptr_t samplerate(psy_audio_Machine* self) { return self->callback.samplerate(self->callback.context); }
static psy_dsp_beat_t bpm(psy_audio_Machine* self) { return self->callback.bpm(self->callback.context); }
static psy_dsp_beat_t beatspertick(psy_audio_Machine* self) { return self->callback.beatspertick(self->callback.context); }
static psy_dsp_beat_t beatspersample(psy_audio_Machine* self) { return self->callback.beatspersample(self->callback.context); }
static psy_dsp_beat_t currbeatsperline(psy_audio_Machine* self) { return self->callback.currbeatsperline(self->callback.context); }
static struct psy_audio_Samples* samples(psy_audio_Machine* self) { return self->callback.samples(self->callback.context); }
static struct psy_audio_Machines* machines(psy_audio_Machine* self) { return self->callback.machines(self->callback.context); }
static struct psy_audio_Instruments* instruments(psy_audio_Machine* self) { return self->callback.instruments(self->callback.context); }
static struct psy_audio_MachineFactory* machinefactory(psy_audio_Machine* self) { return self->callback.machinefactory(self->callback.context); }
static void output(psy_audio_Machine* self, const char* text) { self->callback.output(self->callback.context, text); }
static bool addcapture(psy_audio_Machine* self, int index) { return self->callback.addcapture(self->callback.context, index); }
static bool removecapture(psy_audio_Machine* self, int index) { return self->callback.removecapture(self->callback.context, index); }
static void readbuffers(psy_audio_Machine* self, int index, float** pleft, float** pright, int numsamples)
{ 
	self->callback.readbuffers(self->callback.context, index, pleft, pright, numsamples);
}

static const char* capturename(psy_audio_Machine* self, int index)
{
	return self->callback.capturename(self->callback.context, index);
}

static int numcaptures(psy_audio_Machine* self)
{
	return self->callback.numcaptures(self->callback.context);
}

static const char* playbackname(psy_audio_Machine* self, int index)
{
	return self->callback.playbackname(self->callback.context, index);
}

static int numplaybacks(psy_audio_Machine* self)
{
	return self->callback.numplaybacks(self->callback.context);
}

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.clone = clone;
		vtable.dispose = machine_dispose;
		vtable.reload = reload;
		vtable.mix = mix;
		vtable.work = work;
		vtable.mode = mode;
		vtable.hostevent = hostevent;
		vtable.seqtick = seqtick;
		vtable.stop = stop;
		vtable.sequencertick = sequencertick;
		vtable.sequencerlinetick = sequencerlinetick;
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
		vtable.savespecific = savespecific;
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
		vtable_initialized = 1;
	}
}

void machine_init(psy_audio_Machine* self, MachineCallback callback)
{		
	memset(self, 0, sizeof(psy_audio_Machine));
	vtable_init();
	self->vtable = &vtable;
	self->callback = callback;
	self->err = 0;
	psy_audio_cputimeclock_init(&self->accumulated_processing_time_);
	psy_signal_init(&self->signal_worked);	
}

void machine_dispose(psy_audio_Machine* self)
{
	psy_signal_dispose(&self->signal_worked);
}

void work(psy_audio_Machine* self, psy_audio_BufferContext* bc)
{
	psy_List* p;
	uintptr_t amount;
	uintptr_t pos;

	amount = psy_audio_buffercontext_numsamples(bc);
	for (pos = 0, p = bc->events; p != NULL; p = p->next) {
		psy_audio_PatternEntry* entry;

		entry = (psy_audio_PatternEntry*)p->entry;		
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
	}
	return amount;
}

void work_entry(psy_audio_Machine* self, psy_audio_PatternEntry* entry)
{
	psy_List* p;

	for (p = entry->events; p != NULL; p = p->next) {
		psy_audio_PatternEvent* ev;

		ev = (psy_audio_PatternEvent*) p->entry;
		if (ev->note == NOTECOMMANDS_TWEAK) {
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

						curr = psy_audio_machineparam_patternvalue(param);
						step = (v - curr) / ev->vol;
						nv = curr + step;						
						if (nv < 0) {
							nv = 0;
						}
						psy_audio_machineparam_tweak_pattern(param, (uint16_t) nv);
					} else {
						psy_audio_machineparam_tweak_pattern(param, v);
					}
				}
			}
		} else 
		if (ev->note == NOTECOMMANDS_EMPTY && ev->cmd == EXTENDED) {
			if ((ev->parameter & 0xF0) == SET_BYPASS) {
				if ((ev->parameter & 0x0F) == 0) {
					psy_audio_machine_unbypass(self);
				} else {
					psy_audio_machine_bypass(self);
				}
			} else
			if ((ev->parameter & 0xF0) == SET_MUTE) {
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
	psy_audio_MachineSockets* connected_machine_sockets,
	psy_audio_Machines* machines,
	struct psy_audio_Player* player)
{			
	psy_audio_Buffer* output;

	output = machines_outputs(machines, slot);
	if (output) {
		if (output->preventmixclear == FALSE) {
			psy_audio_buffer_clearsamples(output, amount);
		}
		if (connected_machine_sockets) {
			WireSocket* input_socket;
			
			for (input_socket = connected_machine_sockets->inputs;
					input_socket != 0; input_socket = input_socket->next) {
				psy_audio_WireSocketEntry* source = (psy_audio_WireSocketEntry*)
					input_socket->entry;
				psy_audio_buffer_mixsamples(output,
					machines_outputs(machines, source->slot),
					amount, source->volume, source->mapping);
			}							
		}
	}
	return output;
}

void loadspecific(psy_audio_Machine* self, struct psy_audio_SongFile* songfile,
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
			int minval;
			int maxval;
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

void savespecific(psy_audio_Machine* self, struct psy_audio_SongFile* songfile,
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
			scaled = psy_audio_machineparam_scaledvalue(param);
		}		
		psyfile_write_int32(songfile->file, scaled);
	}
}

void postload(psy_audio_Machine* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uintptr_t c;
	psy_Table* legacywiretable;

	legacywiretable = psy_audio_legacywires_at(&songfile->legacywires, slot);
	if (!legacywiretable) {
		return;
	}

	for (c = 0; c < MAX_CONNECTIONS; ++c) {
		LegacyWire* wire;
		psy_audio_Machine* inputmachine;
		uintptr_t f;
				
		wire = psy_table_at(legacywiretable, c);
		if (!wire) {
			continue;
		}
		//load bugfix: Ensure no duplicate wires could be created.
		for (f = 0; f < c; f++) {
			LegacyWire* legacywire;

			legacywire = psy_table_at(legacywiretable, f);
			if (!legacywire) {
				continue;
			}
			if (wire->_inputCon && legacywire->_inputCon &&
				wire->_inputMachine == legacywire->_inputMachine) {
				wire->_inputCon = FALSE;
			}
		}
		inputmachine = machines_at(&songfile->song->machines, wire->_inputMachine);
		if (wire->_inputCon
			&& wire->_inputMachine >= 0 && wire->_inputMachine < MAX_MACHINES
			&& slot != wire->_inputMachine && inputmachine)
		{
			// Do not create the hidden wire from mixer send to the send machine.
			int outWire = psy_audio_legacywires_findlegacyoutput(&songfile->legacywires, wire->_inputMachine, slot);
			if (outWire != -1) {
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
				machines_connect(&songfile->song->machines, wire->_inputMachine, slot);
				connections_setwirevolume(&songfile->song->machines.connections,
					wire->_inputMachine, slot, wire->_inputConVol * wire->_wireMultiplier);
			}
		}
	}
}
