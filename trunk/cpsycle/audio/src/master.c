// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "master.h"
#include "song.h"
#include "songio.h"
#include "machines.h"
#include "plugin_interface.h"

#include <convert.h>
#include <operations.h>

#include <string.h>
#include <math.h>

#include "../../detail/portable.h"

static void master_init_memory(psy_audio_Master*);
static void master_dispose_memory(psy_audio_Master*);
static void master_generateaudio(psy_audio_Master*, psy_audio_BufferContext*);
static void master_seqtick(psy_audio_Master*, uintptr_t channel,
	const psy_audio_PatternEvent* ev);
static int master_mode(psy_audio_Master* self) { return MACHMODE_MASTER; }
static void master_dispose(psy_audio_Master*);
static const psy_audio_MachineInfo* info(psy_audio_Master*);
static void master_loadspecific(psy_audio_Master*, struct psy_audio_SongFile*,
	uintptr_t slot);
static void master_savespecific(psy_audio_Master*, struct psy_audio_SongFile*,
	uintptr_t slot);
static const char* master_editname(psy_audio_Master* self)
{
	return "Psycle Master and Minimixer";
}
static void setposition(psy_audio_Master* self, intptr_t x, intptr_t y)
{
	self->x = x;
	self->y = y;
}
static void position(psy_audio_Master* self, intptr_t* x, intptr_t* y)
{
	*x = self->x;
	*y = self->y;
}
static uintptr_t numinputs(psy_audio_Master* self) { return 2; }
static uintptr_t numoutputs(psy_audio_Master* self) { return 2; }
static uintptr_t slot(psy_audio_Master* self) { return psy_audio_MASTER_INDEX; }
// Parameters
static psy_audio_MachineParam* parameter(psy_audio_Master* self,
	uintptr_t param);
static psy_audio_MachineParam* tweakparameter(psy_audio_Master* self,
	uintptr_t param);
static uintptr_t numparameters(psy_audio_Master*);
static uintptr_t numtweakparameters(psy_audio_Master*);
static unsigned int numparametercols(psy_audio_Master*);
static void master_describeeditname(psy_audio_Master*, char* text, uintptr_t slot);

static void master_title_name(psy_audio_Master*,
	psy_audio_CustomMachineParam* sender, char* text);
static void master_title_describe(psy_audio_Master*,
	psy_audio_CustomMachineParam* sender, int* active, char* text);
static void master_slider_tweak(psy_audio_Master*,
	psy_audio_CustomMachineParam* sender, float value);
static void master_slider_normvalue(psy_audio_Master*,
	psy_audio_CustomMachineParam* sender, float* rv);
static void master_level_normvalue(psy_audio_Master*,
	psy_audio_CustomMachineParam* sender, float* rv);
static void master_level_describe(psy_audio_Master*,
	psy_audio_CustomMachineParam* sender, int* active, char* text);

static psy_audio_Buffer* master_buffermemory(psy_audio_Master*);
static uintptr_t master_buffermemorysize(psy_audio_Master*);

static psy_dsp_amp_range_t amprange(psy_audio_Master* self)
{
	return PSY_DSP_AMP_RANGE_IGNORE;
}

static psy_audio_MachineInfo const MacInfo = {
	MI_VERSION,
	0x0250,
	EFFECT | 32 | 64,
	MACHMODE_FX,
	"Master"
		#ifndef NDEBUG
		" (debug build)"
		#endif
		,
	"Master",
	"Psycledelics",
	"help",
	MACH_MASTER,
	0,
	0,
	""
};

const psy_audio_MachineInfo* psy_audio_master_info(void) { return &MacInfo; }

#define NUMROWS 3

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_audio_Master* self)
{
	if (!vtable_initialized) {
		vtable = *self->machine.vtable;
		vtable.generateaudio = (fp_machine_generateaudio) master_generateaudio;
		vtable.seqtick = (fp_machine_seqtick) master_seqtick;
		vtable.mode = (fp_machine_mode) master_mode;
		vtable.info = (fp_machine_info) info;
		vtable.dispose = (fp_machine_dispose) master_dispose;
		vtable.info = (fp_machine_info) info;
		
		vtable.numinputs = (fp_machine_numinputs) numinputs;
		vtable.numoutputs = (fp_machine_numoutputs) numoutputs;
		vtable.slot = (fp_machine_slot) slot;
		vtable.loadspecific = (fp_machine_loadspecific) master_loadspecific;
		vtable.savespecific = (fp_machine_savespecific) master_savespecific;
		vtable.editname = (fp_machine_editname) master_editname;
		vtable.setposition = (fp_machine_setposition)setposition;
		vtable.position = (fp_machine_position)position;
		// Parameter
		vtable.parameter = (fp_machine_parameter) parameter;
		vtable.tweakparameter = (fp_machine_tweakparameter) tweakparameter;
		vtable.numparametercols = (fp_machine_numparametercols) numparametercols;
		vtable.numparameters = (fp_machine_numparameters) numparameters;
		vtable.numtweakparameters = (fp_machine_numparameters)numtweakparameters;
		vtable.amprange = (fp_machine_amprange) amprange;
		// buffermemory
		vtable.buffermemory = (fp_machine_buffermemory)
			master_buffermemory;
		vtable.buffermemorysize = (fp_machine_buffermemorysize)
			master_buffermemorysize;
		vtable_initialized = 1;
	}
}

void psy_audio_master_init(psy_audio_Master* self, psy_audio_MachineCallback* callback)
{
	memset(self, 0, sizeof(psy_audio_Master));
	psy_audio_machine_init(&self->machine, callback);	
	vtable_init(self);
	self->machine.vtable = &vtable;
	self->volume = (psy_dsp_amp_t) 1.f;
	psy_audio_custommachineparam_init(&self->param_info,
		"", "", MPF_INFOLABEL | MPF_SMALL, 0, 0x1FFE);
	psy_signal_connect(&self->param_info.machineparam.signal_name, self,
		master_title_name);
	psy_signal_connect(&self->param_info.machineparam.signal_describe, self,
		master_title_describe);
	psy_audio_custommachineparam_init(&self->param_slider,
		"Vol", "Vol", MPF_SLIDER | MPF_SMALL, 0, 0x1FE);
	psy_signal_connect(&self->param_slider.machineparam.signal_tweak, self,
		master_slider_tweak);
	psy_signal_connect(&self->param_slider.machineparam.signal_normvalue, self,
		master_slider_normvalue);
	psy_signal_connect(&self->param_slider.machineparam.signal_describe, self,
		master_level_describe);
	psy_audio_custommachineparam_init(&self->param_level,
		"", "", MPF_SLIDERLEVEL | MPF_SMALL, 0, 0xFFFF);
	psy_signal_connect(&self->param_level.machineparam.signal_normvalue, self,
		master_level_normvalue);
	master_init_memory(self);
	self->x = 320;
	self->y = 320;
}

void master_init_memory(psy_audio_Master* self)
{
	uintptr_t channel;

	psy_audio_buffer_init(&self->memorybuffer, 2);
	self->memorybuffersize = psy_audio_MAX_STREAM_SIZE;
	for (channel = 0; channel < self->memorybuffer.numchannels; ++channel) {
		self->memorybuffer.samples[channel] = dsp.memory_alloc(
			self->memorybuffersize, sizeof(psy_dsp_amp_t));
	}
	psy_audio_buffer_clearsamples(&self->memorybuffer, self->memorybuffersize);
	psy_audio_buffer_enablerms(&self->memorybuffer);
}

void master_dispose(psy_audio_Master* self)
{		
	machine_base_dispose(&self->machine);
	psy_audio_custommachineparam_dispose(&self->param_info);
	psy_audio_custommachineparam_dispose(&self->param_slider);
	psy_audio_custommachineparam_dispose(&self->param_level);
	master_dispose_memory(self);
	machine_base_dispose(&self->machine);
}

void master_dispose_memory(psy_audio_Master* self)
{
	uintptr_t channel;

	for (channel = 0; channel < self->memorybuffer.numchannels; ++channel) {
		dsp.memory_dealloc(self->memorybuffer.samples[channel]);
	}
	psy_audio_buffer_dispose(&self->memorybuffer);
}

const psy_audio_MachineInfo* info(psy_audio_Master* self)
{
	return &MacInfo;
}

void master_generateaudio(psy_audio_Master* self, psy_audio_BufferContext* bc)
{
	psy_audio_buffer_mulsamples(bc->output, bc->numsamples, self->volume);	
}

void master_seqtick(psy_audio_Master* self, uintptr_t channel,
	const psy_audio_PatternEvent* ev)
{
	if (ev->cmd == psy_audio_PATTERNCMD_SET_VOLUME) {
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->machine);
		if (machines) {
			if (ev->inst == psy_audio_NOTECOMMANDS_INST_EMPTY &&
				ev->vol == psy_audio_MASTER_INDEX) {
				psy_dsp_amp_t nv;

				nv = ev->parameter / (psy_dsp_amp_t)0x1FE;
				self->volume = nv * nv * 4.f;
			} else
				if (ev->inst != psy_audio_NOTECOMMANDS_INST_EMPTY) {
					psy_audio_MachineSockets* sockets;

					sockets = psy_audio_connections_at(&machines->connections, ev->vol);
					if (sockets) {
						WireSocket* p;
						uintptr_t c = 0;

						for (p = sockets->outputs; p != NULL && c != ev->inst;
							psy_list_next(&p), ++c);
						if (p) {
							psy_audio_WireSocketEntry* output_entry;
							psy_dsp_amp_t nv;

							output_entry = (psy_audio_WireSocketEntry*)psy_list_entry(p);
							nv = ev->parameter / (psy_dsp_amp_t)0x1FE;
							psy_audio_connections_setwirevolume(&machines->connections,
								psy_audio_wire_make(ev->vol, output_entry->slot), nv * nv * 4.f);
						}
					}
				}
		}
	}
}

void master_describeeditname(psy_audio_Master* self, char* text, uintptr_t slot)
{
	psy_audio_Machine* machine;

	machine = psy_audio_machines_at(psy_audio_machine_machines(&self->machine), slot);
	psy_snprintf(text, 128, "%s", (machine != NULL) ?
		psy_audio_machine_editname(machine) : "");
}

uintptr_t numparameters(psy_audio_Master* self)
{	
	return numparametercols(self) * NUMROWS;
}

unsigned int numparametercols(psy_audio_Master* self)
{
	psy_audio_MachineSockets* sockets;
	WireSocket* input_socket;
	psy_audio_Machines* machines;

	machines = psy_audio_machine_machines(&self->machine);
	sockets = psy_audio_connections_at(&machines->connections, psy_audio_MASTER_INDEX);
	if (sockets) {
		input_socket = sockets->inputs;
		if (input_socket) {
			return psy_list_size(input_socket) + 1;
		}
	}
	return 1;
}

void master_title_name(psy_audio_Master* self,
	psy_audio_CustomMachineParam* sender, char* text)
{
	if (sender->index == 0) {
		psy_snprintf(text, 128, "%s", "Master");
	} else {
		psy_snprintf(text, 10, "m%d", (int)sender->index);
	}
}

void master_title_describe(psy_audio_Master* self,
	psy_audio_CustomMachineParam* sender, int* active, char* text)
{
	*active = 0;
	if (sender->index == 0) {
		*active = 0;
	} else {
		psy_audio_MachineSockets* sockets;
		WireSocket* p;
		uintptr_t c = 1;
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->machine);
		sockets = psy_audio_connections_at(&machines->connections, psy_audio_MASTER_INDEX);
		if (sockets) {
			for (p = sockets->inputs; p != NULL && c != sender->index;
				psy_list_next(&p), ++c);
			if (p) {
				psy_audio_WireSocketEntry* input_entry;

				input_entry = (psy_audio_WireSocketEntry*)psy_list_entry(p);
				master_describeeditname(self, text, input_entry->slot);
				*active = 1;
			}
		}
	}
}

void master_slider_tweak(psy_audio_Master* self,
	psy_audio_CustomMachineParam* sender, float value)
{
	if (sender->index == 0) {
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->machine);
		if (machines) {
			self->volume = value * value * 4.f;
		}
	} else {
		psy_audio_MachineSockets* sockets;
		WireSocket* p;
		uintptr_t c = 1;
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->machine);
		sockets = psy_audio_connections_at(&machines->connections, psy_audio_MASTER_INDEX);
		if (sockets) {
			for (p = sockets->inputs; p != NULL && c != sender->index; psy_list_next(&p), ++c);
			if (p) {
				psy_audio_WireSocketEntry* input_entry;

				input_entry = (psy_audio_WireSocketEntry*)psy_list_entry(p);
				input_entry->volume = value * value * 4.f;
			}
		}
	}
}

void master_slider_normvalue(psy_audio_Master* self,
	psy_audio_CustomMachineParam* sender, float* rv)
{
	*rv = 0;
	if (sender->index == 0) {		
		*rv = (float)sqrt(self->volume) * 0.5f;		
	} else {
		psy_audio_MachineSockets* sockets;
		WireSocket* input_socket;
		uintptr_t c = 1;
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->machine);
		sockets = psy_audio_connections_at(&machines->connections, psy_audio_MASTER_INDEX);
		if (sockets) {
			for (input_socket = sockets->inputs; input_socket != 0 && c != sender->index;
				input_socket = input_socket->next, ++c);
			if (input_socket) {
				psy_audio_WireSocketEntry* input_entry;

				input_entry = (psy_audio_WireSocketEntry*)input_socket->entry;
				*rv = (float)sqrt(input_entry->volume) * 0.5f;
			}
		}
	}
}

void master_level_normvalue(psy_audio_Master* self,
	psy_audio_CustomMachineParam* sender, float* rv)
{
	*rv = 0;
	if (sender->index == 0) {		
		psy_audio_Machines* machines;
		psy_audio_Machine* machine;
		psy_audio_Buffer* memory;

		machines = psy_audio_machine_machines(&self->machine);
		machine = psy_audio_machines_at(machines, psy_audio_MASTER_INDEX);
		memory = psy_audio_machine_buffermemory(machine);
		if (memory) {
			*rv = psy_audio_buffer_rmsdisplay(memory);
		}
	} else {
		psy_audio_MachineSockets* sockets;
		WireSocket* input_socket;
		uintptr_t c = 1;
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->machine);
		sockets = psy_audio_connections_at(&machines->connections, psy_audio_MASTER_INDEX);
		if (sockets) {
			for (input_socket = sockets->inputs; input_socket != 0 &&
				c != sender->index;
				input_socket = input_socket->next, ++c);
			if (input_socket) {
				psy_audio_WireSocketEntry* input_entry;
				psy_audio_Machine* machine;
				psy_audio_Buffer* memory;

				input_entry = (psy_audio_WireSocketEntry*)input_socket->entry;				
				machine = psy_audio_machines_at(machines, input_entry->slot);
				if (machine) {
					memory = psy_audio_machine_buffermemory(machine);
					if (memory) {
						*rv = psy_audio_buffer_rmsdisplay(memory);
					}
				}
			}
		}
	}
}

void master_level_describe(psy_audio_Master* self,
	psy_audio_CustomMachineParam* sender, int* active, char* text)
{
	if (sender->index == 0) {
		psy_snprintf(text, 10, "%.2f dB",
			(float)psy_dsp_convert_amp_to_db(self->volume));
		*active = 1;
	} else {
		psy_audio_MachineSockets* sockets;
		WireSocket* p;
		uintptr_t c = 1;
		psy_audio_Machines* machines;

		machines = psy_audio_machine_machines(&self->machine);
		sockets = psy_audio_connections_at(&machines->connections, psy_audio_MASTER_INDEX);
		*active = 0;
		if (sockets) {
			for (p = sockets->inputs; p != NULL && c != sender->index;
				psy_list_next(&p), ++c);
			if (p) {
				psy_audio_WireSocketEntry* input_entry;

				input_entry = (psy_audio_WireSocketEntry*)psy_list_entry(p);
				psy_snprintf(text, 10, "%.2f dB",
					(float)psy_dsp_convert_amp_to_db(input_entry->volume));
				*active = 1;
			}
		}
	}
}

psy_audio_MachineParam* parameter(psy_audio_Master* self, uintptr_t id)
{
	int col = id / NUMROWS;
	int row = id % NUMROWS;

	if (row == 0) {
		self->param_info.index = col;
		return &self->param_info.machineparam;
	} else
	if (row == 1) {
		self->param_slider.index = col;
		return &self->param_slider.machineparam;
	} else
	if (row == 2) {
		self->param_level.index = col;
		return &self->param_level.machineparam;
	}
	return NULL;	
}

psy_audio_MachineParam* tweakparameter(psy_audio_Master* self, uintptr_t id)
{
	if (id == 0) {
		self->param_slider.index = 0;
		return &self->param_slider.machineparam;
	}
	return NULL;
}

uintptr_t numtweakparameters(psy_audio_Master* self)
{
	return numparametercols(self);
}

void master_loadspecific(psy_audio_Master* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{	
	uint32_t size;
	int32_t outdry = 256;
	unsigned char decreaseOnClip = 0;

	// size of this part params to load
	psyfile_read(songfile->file, &size, sizeof size );
	psyfile_read(songfile->file, &outdry, sizeof outdry);
	psyfile_read(songfile->file, &decreaseOnClip, sizeof decreaseOnClip);

	self->volume = outdry / (psy_dsp_amp_t) 256;
}

void master_savespecific(psy_audio_Master* self, psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t size;
	int32_t outdry;
	unsigned char decreaseOnClip = 0;
				
	size = sizeof outdry + sizeof decreaseOnClip;
	outdry = (int32_t)(self->volume * 256);
	// size of this part params to save
	psyfile_write(songfile->file, &size, sizeof(size));
	psyfile_write(songfile->file, &outdry, sizeof(outdry));
	psyfile_write(songfile->file, &decreaseOnClip, sizeof(decreaseOnClip));
}

psy_audio_Buffer* master_buffermemory(psy_audio_Master* self)
{
	return &self->memorybuffer;
}

uintptr_t master_buffermemorysize(psy_audio_Master* self)
{
	return self->memorybuffersize;
}

void master_setbuffermemorysize(psy_audio_Master* self, uintptr_t size)
{
	// self->memorybuffersize = size;
}
