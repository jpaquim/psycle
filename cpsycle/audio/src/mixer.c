// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "mixer.h"
#include "machines.h"
#include "songio.h"
#include <operations.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <dsptypes.h>
#include <portable.h>

const psy_audio_MachineInfo* mixer_info(void)
{
	static psy_audio_MachineInfo const macinfo = {
		MI_VERSION,
		0x0250,
		GENERATOR | 32 | 64,
		MACHMODE_GENERATOR,
		"Mixer"
			#ifndef NDEBUG
			" (debug build)"
			#endif
			,
		"Mixer",
		"Psycledelics",
		"help",
		MACH_MIXER
	};
	return &macinfo;
}

static const psy_audio_MachineInfo* info(psy_audio_Mixer*);
static void mixer_dispose(psy_audio_Mixer*);
static int mixer_mode(psy_audio_Mixer* self) { return MACHMODE_FX; }
static void mixer_seqtick(psy_audio_Mixer*, uintptr_t channel,
	const psy_audio_PatternEvent*);
static uintptr_t numinputs(psy_audio_Mixer*);
static uintptr_t numoutputs(psy_audio_Mixer*);
static psy_audio_Buffer* mix(psy_audio_Mixer*, uintptr_t slot, uintptr_t amount,
	psy_audio_MachineSockets*, psy_audio_Machines*);
static void addsamples(psy_audio_Buffer* dst, psy_audio_Buffer* source,
	uintptr_t numsamples, float vol);
static void loadspecific(psy_audio_Mixer*, struct psy_audio_SongFile*,
	uintptr_t slot);
static void savespecific(psy_audio_Mixer*, struct psy_audio_SongFile*,
	uintptr_t slot);
static void onconnected(psy_audio_Mixer*, psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void ondisconnected(psy_audio_Mixer*, psy_audio_Connections*, uintptr_t outputslot, uintptr_t inputslot);
static int parametertype(psy_audio_Mixer*, uintptr_t param);
static void parameterrange(psy_audio_Mixer*, uintptr_t param, int* minval, int* maxval);
static int parameterlabel(psy_audio_Mixer*, char* txt, uintptr_t param);
static int parametername(psy_audio_Mixer*, char* txt, uintptr_t param);
static void parametertweak(psy_audio_Mixer*, uintptr_t param, int value);
static void patterntweak(psy_audio_Mixer* self, uintptr_t param, int val);
static int parametervalue(psy_audio_Mixer*, uintptr_t param);
static int describevalue(psy_audio_Mixer*, char* txt, uintptr_t param, int value);
static uintptr_t numparameters(psy_audio_Mixer*);
static uintptr_t numparametercols(psy_audio_Mixer*);
static psy_dsp_amp_range_t amprange(psy_audio_Mixer* self)
{
	return PSY_DSP_AMP_RANGE_IGNORE;
}

static int intparamvalue(float value);
static float floatparamvalue(int value);
static psy_audio_WireSocketEntry* wiresocketentry(psy_audio_Mixer*, uintptr_t input);
static void insertinputchannels(psy_audio_Mixer*, uintptr_t num, psy_audio_Machines* machines);
static int paramviewoptions(psy_audio_Machine* self) { return MACHINE_PARAMVIEW_COMPACT; }
static uintptr_t slot(psy_audio_Mixer* self) { return self->slot; }
static void setslot(psy_audio_Mixer* self, uintptr_t slot) { self->slot = slot; }
static uintptr_t mastercolumn(psy_audio_Mixer*);
static uintptr_t inputcolumn(psy_audio_Mixer*);
static uintptr_t returncolumn(psy_audio_Mixer*);
static void preparemix(psy_audio_Mixer*, psy_audio_Machines*, uintptr_t amount);
static void mixinputs(psy_audio_Mixer*, psy_audio_Machines*, uintptr_t amount);
static void workreturns(psy_audio_Mixer*, psy_audio_Machines*, uintptr_t amount);
static void mixreturns(psy_audio_Mixer*, psy_audio_Machines*, uintptr_t amount);
static void tickrms(psy_audio_Mixer*, uintptr_t amount);
static void levelmaster(psy_audio_Mixer*, uintptr_t amount);

static psy_audio_MixerChannel* mixerchannel_allocinit(uintptr_t inputslot);
static void mixerchannel_dispose(psy_audio_MixerChannel*);

static void returnchannel_init(psy_audio_ReturnChannel*, uintptr_t fxslot);
static psy_audio_ReturnChannel* returnchannel_allocinit(uintptr_t fxslot);
static void returnchannel_dispose(psy_audio_ReturnChannel*);


void mixerchannel_init(psy_audio_MixerChannel* self, uintptr_t inputslot)
{		
	self->inputslot = inputslot;
	self->drymix = 1.0f;
	self->dryonly = 0;
	self->mute = 0;
	self->panning = 0.5f;
	psy_table_init(&self->sendvols);
	self->volume = 1.0f;
	self->gain = 1.f;
	self->wetonly = 0;
}

psy_audio_MixerChannel* mixerchannel_allocinit(uintptr_t inputslot)
{
	psy_audio_MixerChannel* rv;

	rv = (psy_audio_MixerChannel*) malloc(sizeof(psy_audio_MixerChannel));
	if (rv) {
		mixerchannel_init(rv, inputslot);
	}	
	return rv;
}

void mixerchannel_dispose(psy_audio_MixerChannel* self)
{	
	psy_table_dispose(&self->sendvols);	
}

void returnchannel_init(psy_audio_ReturnChannel* self, uintptr_t fxslot)
{		
	self->fxslot = fxslot;
	self->mute = 0;
	self->panning = 0.5f;	
	self->volume = 1.0f;	
	self->mastersend = 1;
	psy_table_init(&self->sendsto);
}

psy_audio_ReturnChannel* returnchannel_allocinit(uintptr_t fxslot)
{
	psy_audio_ReturnChannel* rv;

	rv = (psy_audio_ReturnChannel*) malloc(sizeof(psy_audio_ReturnChannel));
	if (rv) {
		returnchannel_init(rv, fxslot);
	}	
	return rv;
}

void returnchannel_dispose(psy_audio_ReturnChannel* self)
{		
	psy_table_dispose(&self->sendsto);
}

static MachineVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_audio_Mixer* self)
{
	if (!vtable_initialized) {
		vtable = *self->custommachine.machine.vtable;
		vtable.info = (fp_machine_info) info;
		vtable.numinputs = (fp_machine_numinputs) numinputs;
		vtable.numoutputs = (fp_machine_numoutputs) numoutputs;
		vtable.dispose = (fp_machine_dispose) mixer_dispose;
		vtable.seqtick = (fp_machine_seqtick) mixer_seqtick;
		vtable.mode = (fp_machine_mode) mixer_mode;
		vtable.mix = (fp_machine_mix) mix;
		vtable.parametertype = (fp_machine_parametertype) parametertype;
		vtable.parameterrange = (fp_machine_parameterrange) parameterrange;
		vtable.parametername = (fp_machine_parametername) parametername;
		vtable.parameterlabel = (fp_machine_parameterlabel) parameterlabel;
		vtable.parametertweak = (fp_machine_parametertweak) parametertweak;
		vtable.patterntweak = (fp_machine_patterntweak) patterntweak;
		vtable.parametervalue = (fp_machine_parametervalue) parametervalue;
		vtable.describevalue = (fp_machine_describevalue) describevalue;
		vtable.numparameters = (fp_machine_numparameters) numparameters;
		vtable.numparametercols = (fp_machine_numparametercols) numparametercols;	
		vtable.paramviewoptions = (fp_machine_paramviewoptions) paramviewoptions;
		vtable.loadspecific = (fp_machine_loadspecific) loadspecific;
		vtable.savespecific = (fp_machine_savespecific) savespecific;
		vtable.setslot = (fp_machine_setslot) setslot;
		vtable.slot = (fp_machine_slot) slot;
		vtable.amprange = (fp_machine_amprange) amprange;
		vtable_initialized = 1;
	}
}

void mixer_init(psy_audio_Mixer* self, MachineCallback callback)
{
	psy_audio_Machine* base = (psy_audio_Machine*)self;
	psy_audio_Machines* machines;

	custommachine_init(&self->custommachine, callback);
	vtable_init(self);
	self->custommachine.machine.vtable = &vtable;
	psy_table_init(&self->inputs);
	psy_table_init(&self->sends);
	psy_table_init(&self->returns);
	machines = base->vtable->machines(base);
	psy_signal_connect(&machines->connections.signal_connected, self,
		onconnected);
	psy_signal_connect(&machines->connections.signal_disconnected, self,
		ondisconnected);
	psy_dsp_rmsvol_init(&self->masterrmsvol);
	mixerchannel_init(&self->master, 0);	
	self->slot = 65535;
	base->vtable->seteditname(base, "Mixer");
}

void mixer_dispose(psy_audio_Mixer* self)
{
	{ // dispose inputs
		psy_TableIterator it;

		for (it = psy_table_begin(&self->inputs);
			!psy_tableiterator_equal(&it, psy_table_end()); psy_tableiterator_inc(&it)) {
			psy_audio_MixerChannel* channel;

			channel = (psy_audio_MixerChannel*)psy_tableiterator_value(&it);
			mixerchannel_dispose(channel);
			free(channel);		
		}	
		psy_table_dispose(&self->inputs);
	}
	psy_table_dispose(&self->sends);
	{ // dispose returns
		psy_TableIterator it;

		for (it = psy_table_begin(&self->returns);
			!psy_tableiterator_equal(&it, psy_table_end()); psy_tableiterator_inc(&it)) {
			psy_audio_ReturnChannel* channel;

			channel = (psy_audio_ReturnChannel*)psy_tableiterator_value(&it);
			returnchannel_dispose(channel);
			free(channel);		
		}	
		psy_table_dispose(&self->returns);
	}	
	custommachine_dispose(&self->custommachine);	
}

void mixer_seqtick(psy_audio_Mixer* self, uintptr_t channel,
	const psy_audio_PatternEvent* event)
{	
	if(event->note == NOTECOMMANDS_TWEAK)
	{
		int nv = (event->cmd<<8)+event->parameter;
		uintptr_t param =  event->inst; // translate_param(event->inst);

		// if(param < GetNumParams()) {
		//	SetParameter(param,nv);
		// }
	}
	else if(event->note == NOTECOMMANDS_TWEAKSLIDE)
	{
		//\todo: Tweaks and tweak slides should not be a per-machine thing, but rather be player centric.
		// doing simply "tweak" for now..
		int nv = (event->cmd<<8)+event->parameter;
		uintptr_t param = event->inst; // translate_param(event->inst);

		//if(param < GetNumParams()) {
		//	SetParameter(param,nv);
		//}
	}
}

uintptr_t numinputs(psy_audio_Mixer* self)
{
	return 2;
}

uintptr_t numoutputs(psy_audio_Mixer* self)
{
	return 2;
}

psy_audio_Buffer* mix(psy_audio_Mixer* self, uintptr_t slot, uintptr_t amount,
	psy_audio_MachineSockets* connected_machine_sockets,
	psy_audio_Machines* machines)
{							
	preparemix(self, machines, amount);
	mixinputs(self, machines, amount);
	workreturns(self, machines, amount);	
	mixreturns(self, machines, amount);
	tickrms(self, amount);
	levelmaster(self, amount);	
	return self->master.buffer;
}

void preparemix(psy_audio_Mixer* self, psy_audio_Machines* machines,
	uintptr_t amount)
{			
	psy_TableIterator iter;

	self->master.buffer = machines_outputs(machines, self->slot);
	buffer_clearsamples(self->master.buffer, amount);	
	for (iter = psy_table_begin(&self->returns);
		!psy_tableiterator_equal(&iter, psy_table_end());
			psy_tableiterator_inc(&iter)) {	
		psy_audio_ReturnChannel* channel;

		channel = (psy_audio_ReturnChannel*) psy_tableiterator_value(&iter);
		channel->buffer = machines_outputs(machines, channel->fxslot);
		channel->fx = machines_at(machines, channel->fxslot);
		if (channel->buffer) {
			buffer_clearsamples(channel->buffer, amount);
		}
	}
}

void mixinputs(psy_audio_Mixer* self, psy_audio_Machines* machines,
	uintptr_t amount)
{		
	psy_TableIterator iter;	
	
	for (iter = psy_table_begin(&self->inputs);
			!psy_tableiterator_equal(&iter, psy_table_end());
			psy_tableiterator_inc(&iter)) {		
		psy_audio_MixerChannel* channel;		
		
		channel = (psy_audio_MixerChannel*) psy_tableiterator_value(&iter);
		if (channel) {						
			psy_audio_WireSocketEntry* input_entry;
			float wirevol = 1.f;
			psy_TableIterator fx_iter;

			channel->buffer = machines_outputs(machines, channel->inputslot);
			input_entry = wiresocketentry(self, psy_tableiterator_key(&iter));
			if (input_entry) {
				wirevol = input_entry->volume;	
			}
			addsamples(self->master.buffer, channel->buffer, amount,
				channel->volume * channel->drymix * wirevol);
			for (fx_iter = psy_table_begin(&self->returns);
					!psy_tableiterator_equal(&fx_iter, psy_table_end());
					psy_tableiterator_inc(&fx_iter)) {
				psy_audio_ReturnChannel* fxchannel;
				int sendvol;			

				fxchannel = (psy_audio_ReturnChannel*)psy_tableiterator_value(&fx_iter);
				if (fxchannel && fxchannel->buffer) {
					sendvol = (int)(uintptr_t) psy_table_at(&channel->sendvols,
						psy_tableiterator_key(&fx_iter));
					addsamples(fxchannel->buffer, channel->buffer, amount,
						(sendvol / 65535.f) * wirevol);
				}
			}
		}
	}
}

void mixreturns(psy_audio_Mixer* self, psy_audio_Machines* machines,
	uintptr_t amount)
{		
	psy_TableIterator iter;
	
	for (iter = psy_table_begin(&self->returns);
		!psy_tableiterator_equal(&iter, psy_table_end());
			psy_tableiterator_inc(&iter)) {	
		psy_audio_ReturnChannel* channel;		

		channel = psy_tableiterator_value(&iter);
		if (channel && channel->mastersend) {			
			addsamples(self->master.buffer, channel->buffer, amount,
				channel->volume);
		}
	}
}

void workreturns(psy_audio_Mixer* self, psy_audio_Machines* machines,
	uintptr_t amount)
{	
	psy_TableIterator iter;
	
	for (iter = psy_table_begin(&self->returns);
		!psy_tableiterator_equal(&iter, psy_table_end());
			psy_tableiterator_inc(&iter)) {	
		psy_audio_ReturnChannel* channel;		

		channel = psy_tableiterator_value(&iter);
		if (channel && channel->fx && channel->buffer) {
			psy_audio_BufferContext bc;
			psy_List* events = 0;			
			psy_TableIterator sendsto_iter;			
			
			buffercontext_init(&bc, events, channel->buffer, channel->buffer,
				amount, 16, 0);
			channel->fx->vtable->work(channel->fx, &bc);
		//	buffer_pan(fxbuffer, fx->panning(fx), amount);
		//	buffer_pan(fxbuffer, channel->panning, amount);
			psy_signal_emit(&channel->fx->signal_worked, channel->fx, 2, 
				channel->fxslot, &bc);
			if (channel->sendsto.count >=0 ) {
				for (sendsto_iter = psy_table_begin(&channel->sendsto);
					!psy_tableiterator_equal(&sendsto_iter, psy_table_end());
						psy_tableiterator_inc(&sendsto_iter)) {
					psy_audio_ReturnChannel* sendto;

					sendto = (psy_audio_ReturnChannel*)psy_tableiterator_value(&sendsto_iter);
					if (sendto) {		
						if (sendto->buffer) {
							addsamples(sendto->buffer, channel->buffer, amount,
								channel->volume);
						}
					}
				}
			}
		}
	}
}

void addsamples(psy_audio_Buffer* dst, psy_audio_Buffer* source,
	uintptr_t numsamples, float vol)
{
	uintptr_t channel;

	if (source) {
		for (channel = 0; channel < source->numchannels && 
			channel < dst->numchannels; ++channel) {
				dsp.add(
					source->samples[channel],
					dst->samples[channel],
					numsamples,
					vol);
			//	dsp_erase_all_nans_infinities_and_denormals(
			//		dst->samples[channel], numsamples);					
		}
	}
}

void tickrms(psy_audio_Mixer* self, uintptr_t amount)
{
	psy_dsp_rmsvol_tick(&self->masterrmsvol, 
		self->master.buffer->samples[0],
		self->master.buffer->samples[1],
		amount);
}

void levelmaster(psy_audio_Mixer* self, uintptr_t amount)
{
	dsp.mul(self->master.buffer->samples[0], amount, self->master.volume);
	dsp.mul(self->master.buffer->samples[1], amount, self->master.volume);
}

void onconnected(psy_audio_Mixer* self, psy_audio_Connections* connections,
	uintptr_t outputslot, uintptr_t inputslot)
{				
	psy_audio_Machine* base = (psy_audio_Machine*)self;
	if (inputslot == (int)self->slot) {		
		if (outputslot != (int)self->slot) {
			psy_audio_Machine* machine;
			psy_audio_Machines* machines = base->vtable->machines(base);

			machine = machines_at(machines, outputslot);
			if (machine->vtable->mode(machine) == MACHMODE_GENERATOR) {
				psy_audio_MixerChannel* channel;

				channel = mixerchannel_allocinit(outputslot);
				psy_table_insert(&self->inputs, self->inputs.count, (void*)channel);
			} else {
				psy_table_insert(&self->sends, self->sends.count, (void*)outputslot);
				psy_table_insert(&self->returns, self->returns.count, 
					returnchannel_allocinit(outputslot));				
			}
		}
	}
}

void ondisconnected(psy_audio_Mixer* self, psy_audio_Connections* connections, uintptr_t outputslot, uintptr_t inputslot)
{
	psy_audio_Machine* base = (psy_audio_Machine*)self;
	if (inputslot == (int)self->slot) {
		psy_audio_Machine* machine;
		psy_audio_Machines* machines = base->vtable->machines(base);

		machine = machines_at(machines, outputslot);
		if (machine->vtable->mode(machine) == MACHMODE_GENERATOR) {
			psy_TableIterator it;
			int c = 0;

			for (it = psy_table_begin(&self->inputs);
				!psy_tableiterator_equal(&it, psy_table_end()); psy_tableiterator_inc(&it), ++c) {
				psy_audio_MixerChannel* channel;
		
				channel = (psy_audio_MixerChannel*)psy_tableiterator_value(&it);
				if (channel->inputslot == outputslot) {
					psy_table_remove(&self->inputs, c);
					mixerchannel_dispose(channel);
					free(channel);
					break;
				}
			}			
		} else {
			psy_TableIterator it;			

			for (it = psy_table_begin(&self->returns);
					!psy_tableiterator_equal(&it, psy_table_end());
					psy_tableiterator_inc(&it)) {				
				psy_TableIterator sendsto_iter;	
				psy_audio_ReturnChannel* channel;				

				channel = psy_table_at(&self->returns, psy_tableiterator_key(&it));

				sendsto_iter = psy_table_begin(&channel->sendsto);
				while (!psy_tableiterator_equal(&sendsto_iter, psy_table_end())) {
					psy_audio_ReturnChannel* sendto;
					uintptr_t key;

					key = psy_tableiterator_key(&sendsto_iter);
					sendto = (psy_audio_ReturnChannel*)psy_tableiterator_value(&sendsto_iter);
					psy_tableiterator_inc(&sendsto_iter);
					if (sendto) {										
						if (sendto->fxslot == outputslot) {
							psy_table_remove(&channel->sendsto, key);								
						}
					}					
				}
			}

			for (it = psy_table_begin(&self->sends);
				!psy_tableiterator_equal(&it, psy_table_end()); psy_tableiterator_inc(&it)) {
				uintptr_t sendslot;
				uintptr_t c;

				c = psy_tableiterator_key(&it);
				sendslot = (uintptr_t)psy_tableiterator_value(&it);
				if (sendslot == outputslot) {
					psy_audio_ReturnChannel* returnchannel;
					psy_table_remove(&self->sends, c);					
					returnchannel = psy_table_at(&self->returns, c);
					psy_table_remove(&self->returns, c);
					if (returnchannel) {
						returnchannel_dispose(returnchannel);
						free(returnchannel);
					}					
					break;
				}
			}			
		}
	}
}

const psy_audio_MachineInfo* info(psy_audio_Mixer* self)
{	
	return mixer_info();
}

static psy_dsp_amp_t dB2Amp(psy_dsp_amp_t db)
{	
	return (psy_dsp_amp_t) pow(10.0f, db / 20.0f);
}

void patterntweak(psy_audio_Mixer* self, uintptr_t numparam, int value)
{
	psy_audio_Machine* base = (psy_audio_Machine*)self;
	uintptr_t channelindex = numparam / 16;
	uintptr_t param = numparam % 16;

	if (channelindex == 0) {		
		if (param == 0) {
			if (value >= 0x1000) {
				self->master.volume = 1.0f;
			} else
			if (value == 0) {
				self->master.volume = 0.0f;
			} else {
				psy_dsp_amp_t dbs = (value/42.67f)-96.0f;
				self->master.volume = dB2Amp(dbs);
			}			
		} else 
		if (param == 13) {
			self->master.drymix = (value >= 0x100) 
				? 1.0f
				: ((value&0xFF)/ 256.f);
		} else
		if (param == 14) { 
			self->master.gain = (value >= 1024) 
				? 4.0f
				: ((value&0x3FF)/256.0f);
		} else {			
			base->vtable->setpanning(base, (value >> 1) / 256.f);
		}
	} else
	// Inputs
	if (channelindex <= self->inputs.count) {
		psy_audio_MixerChannel* channel;

		channel = psy_table_at(&self->inputs, channelindex - 1);
		if (channel) {
			if (param == 0) { 
				channel->drymix = (value == 256) 
					? 1.0f
					: ((value&0xFF)/256.0f);
			} else
			if (param <= 12) {				 
				if (param - 1 < self->sends.count) {														
					psy_table_insert(&channel->sendvols, param - 1, 
						(void*) (ptrdiff_t)intparamvalue( 
							(value == 256) ? 1.0f : 
							((value&0xFF)/256.0f)));					
				}
			} else			
			if (param == 13) {
				channel->mute = value == 3;
				channel->wetonly = value == 2;
				channel->dryonly = value == 1;
			} else 
			if (param == 14) {
				psy_audio_WireSocketEntry* input_entry;

				input_entry = wiresocketentry(self, channelindex - 1);
				if (input_entry) {		
					float val=(value>=1024)?4.0f:((value&0x3FF)/256.0f);
					input_entry->volume = val;
				}
			} else {
				if (value >= 0x1000) {
					channel->volume = 1.0f;
				} else
				if (value == 0) {
					channel->volume = 0.0f;
				} else {
					psy_dsp_amp_t dbs = (value/42.67f)-96.0f;
					channel->volume = dB2Amp(dbs);
				}
			}
		}
	}
}


void parametertweak(psy_audio_Mixer* self, uintptr_t param, int value)
{	
	uintptr_t col;
	uintptr_t row;
	uintptr_t rows;

	rows = numparameters(self) / numparametercols(self);
	row = param % rows;
	col = param / rows;

	if (col < mastercolumn(self)) {

	} else
	if (col == mastercolumn(self)) {	// MASTER COLUMN
		if (row == self->sends.count + 1) {
			self->master.drymix = value / 65535.f;
		} else
		if (row == self->sends.count + 2) {
			self->master.gain = floatparamvalue(value) * 
				floatparamvalue(value) * 4.f;
		} else	
		if (row == self->sends.count + 3) {
			self->master.panning = floatparamvalue(value); 
		} else	
		if (row == self->sends.count + 8) {
			self->master.volume = floatparamvalue(value) * 
					floatparamvalue(value) * 4.f;			
		}
	} else 
	// Input Columns
	if (col < returncolumn(self)) {
		psy_audio_MixerChannel* channel;
			
		channel = (psy_audio_MixerChannel*) psy_table_at(&self->inputs,
			col - inputcolumn(self));
		if (channel) {
			if (row == self->sends.count + 1) {
				channel->drymix = value / 65535.f;
			} else
			if (row > 0 && row < self->sends.count + 1) {						
				psy_table_insert(&channel->sendvols, row - 1, (void*)(ptrdiff_t)value);
			} else
			if (row == self->sends.count + 2) {
				psy_audio_WireSocketEntry* input_entry;

				input_entry = wiresocketentry(self, col - inputcolumn(self));
				if (input_entry) {						
					input_entry->volume = floatparamvalue(value) * 
						floatparamvalue(value) * 4.f;
				}
			} else				
			if (row == self->sends.count + 3) {
				channel->panning = floatparamvalue(value); 
			} else				 
			if (row == self->sends.count + 4) {

			} else
			if (row == self->sends.count + 5) {
				channel->mute = value;
			} else
			if (row == self->sends.count + 6) {
				channel->dryonly = value;
			} else
			if (row == self->sends.count + 7) {
				channel->wetonly = value;
			}
			if (row == self->sends.count + 8) {
				channel->volume = floatparamvalue(value) * 
					floatparamvalue(value) * 4.f;
			}
		}
	} else
	// Return Columns
	if (col < returncolumn(self) + self->returns.count) {
		psy_audio_ReturnChannel* channel;
	
		channel = (psy_audio_ReturnChannel*) psy_table_at(&self->returns,
			col - returncolumn(self));
		if (channel) {
			if (row > 0 && row < self->sends.count + 1) {
				if (value) {
					psy_table_insert(&channel->sendsto, row - 1,
						(psy_audio_ReturnChannel*) psy_table_at(&self->returns, 
							col - returncolumn(self) + 1));
				} else {
					psy_table_remove(&channel->sendsto, row - 1);
				}
			} else
			if (row == self->sends.count + 1) {
				channel->mastersend = value; 
			} else
			if (row == self->sends.count + 3) {
				channel->panning = floatparamvalue(value); 
			} else
			if (row == self->sends.count + 4) {				
			} else
			if (row == self->sends.count + 5) {
				channel->mute = value;
			} else
			if (row == self->sends.count + 8) {
				channel->volume = floatparamvalue(value) * 
					floatparamvalue(value) * 4.f;
			}
		}
	}
}

int parametervalue(psy_audio_Mixer* self, uintptr_t param)
{	
	uintptr_t col;
	uintptr_t row;
	uintptr_t rows;

	rows = numparameters(self) / numparametercols(self);
	row = param % rows;
	col = param / rows;

	if (col < mastercolumn(self)) {

	} else
	if (col == mastercolumn(self)) {	// MASTER COLUMN
		if (row == self->sends.count + 1) {			
			return (int)(self->master.drymix * 65535);
		} else
		if (row == self->sends.count + 2) {			
			return intparamvalue(
				(float)sqrt(self->master.gain) * 0.5f);
		} else	
		if (row == self->sends.count + 3) {			
			return intparamvalue(self->master.panning);			
		} else
		if (row == self->sends.count + 8) {
			return intparamvalue(
				(float)sqrt(self->master.volume) * 0.5f);
		} else
		if (row == self->sends.count + 9) {
			return (int)(psy_dsp_rmsvol_value(&self->masterrmsvol) / 32768.f  * 65535);
		}
	} else 
	// Input Column
	if (col < returncolumn(self)) {
		psy_audio_MixerChannel* channel;
	
		channel = (psy_audio_MixerChannel*) psy_table_at(&self->inputs, 
			col - inputcolumn(self));
		if (channel) {
			if (row == self->sends.count + 1) {
				return (int)(channel->drymix * 65535);
			} else
			if (row > 0 && row < self->sends.count + 1) {			
				return (int)(ptrdiff_t)psy_table_at(&channel->sendvols, row - 1);
			} else
			if (row == self->sends.count + 2) {
				psy_audio_WireSocketEntry* input_entry;

				input_entry = wiresocketentry(self, col - inputcolumn(self));
				if (input_entry) {						
					return intparamvalue(
						(float)sqrt(input_entry->volume) * 0.5f);					
				}
			} else
			if (row == self->sends.count + 3) {
				return intparamvalue(channel->panning);
			} else
			if (row == self->sends.count + 4) {
				
			} else
			if (row == self->sends.count + 5) {
				return channel->mute;
			} else
			if (row == self->sends.count + 6) {
				return channel->dryonly;
			} else
			if (row == self->sends.count + 7) {
				return channel->wetonly;
			} else
			if (row == self->sends.count + 8) {
				return intparamvalue(
					(float)sqrt(channel->volume) * 0.5f);
			}
		}
	} else
	// Return Column
	if (col < returncolumn(self) + self->returns.count) {
		psy_audio_ReturnChannel* channel;
	
		channel = (psy_audio_ReturnChannel*) psy_table_at(&self->returns,
			col - returncolumn(self));
		if (channel) {
			if (row > 0 && row < self->sends.count + 1) {
				return psy_table_exists(&channel->sendsto, row - 1);				
			} else
			if (row == self->sends.count + 1) {
				return channel->mastersend;
			} else
			if (row == self->sends.count + 3) {
				return intparamvalue(channel->panning);
			} else
			if (row == self->sends.count + 5) {
				return channel->mute;
			} else	
			if (row == self->sends.count + 8) {
				return intparamvalue(
					(float)sqrt(channel->volume) * 0.5f);
			}
		}		
	}
	return 0;
}

int describevalue(psy_audio_Mixer* self, char* txt, uintptr_t param, int value)
{ 	
	psy_audio_Machine* base = (psy_audio_Machine*)self;
	uintptr_t col;
	uintptr_t row;
	uintptr_t rows;

	*txt = '\0';
	rows = numparameters(self) / numparametercols(self);
	row = param % rows;
	col = param / rows;
	if (col < mastercolumn(self)) {
		if (row > 0 && row < self->sends.count + 1) {
			uintptr_t index;
			uintptr_t channel;
			
			index = row - 1;
			channel = (uintptr_t) psy_table_at(&self->sends, index);
			//if (channel) {
			{
				psy_audio_Machine* machine;		
				
				machine = machines_at(base->vtable->machines(base), channel);
				if (machine && machine->vtable->info(machine)) {
					strcpy(txt, machine->vtable->info(machine)->ShortName);
					return 1;
				}				
			}
			//}
		}
	} else
	if (col == mastercolumn(self)) {	// MASTER COLUMN		
		if (row == self->sends.count + 1) {				
			psy_snprintf(txt, 20, "%d%%", (int) (self->master.drymix * 100));
			return 1;
		} else
		if (row == self->sends.count + 2) {
			float db;

			db = (psy_dsp_amp_t)(20 * log10(self->master.gain));
			psy_snprintf(txt, 10, "%.2f dB", db);
			return 1;			
		} else
		if (row == self->sends.count + 3) {
			if (self->master.panning == 0.f) {
				strcpy(txt,"left");
			} else
			if (self->master.panning == 1.f) {
				strcpy(txt,"right");
			} else
			if (self->master.panning == 0.5f) {
				strcpy(txt,"center");
			} else {
				sprintf(txt, "%.0f%%", self->master.panning * 100);
			}
			return 1;
		} else
		if (row == self->sends.count + 8) {
			float db;

			db = (psy_dsp_amp_t)(20 * log10(self->master.volume));
			psy_snprintf(txt, 10, "%.2f dB", db);
			return 1;
		}
	} else
	if (col < returncolumn(self)) {
		psy_audio_MixerChannel* channel;
	
		channel = (psy_audio_MixerChannel*) psy_table_at(&self->inputs,
			col - inputcolumn(self));
		if (channel) {
			if (row == 0) {
				psy_audio_Machine* machine;		
				
				machine = machines_at(base->vtable->machines(base),
					channel->inputslot);
				if (machine && machine->vtable->info(machine)) {
					strcpy(txt, machine->vtable->info(machine)->ShortName);					
				}
				return 1;								
			} else
			if (row > 0 && row < self->sends.count + 1) {
				psy_dsp_amp_t sendvol;
				sendvol = (int) (ptrdiff_t)psy_table_at(&channel->sendvols, row - 1) / 65535.f;
				if (sendvol == 0.0f) {
					strcpy(txt,"Off");
				} else {
					sprintf(txt,"%.0f%%", sendvol * 100.0f);
				}
				return 1;
			} else
			if (row == self->sends.count + 1) {				
				psy_snprintf(txt, 20, "%d%%", (int) (channel->drymix * 100));
				return 1;
			} else
			if (row == self->sends.count + 2) {
				psy_audio_WireSocketEntry* input_entry;

				input_entry = wiresocketentry(self, col - inputcolumn(self));
				if (input_entry) {
					float db;

					db = (psy_dsp_amp_t)(20 * log10(input_entry->volume));
					psy_snprintf(txt, 10, "%.2f dB", db);
					return 1;
				}
			} else
			if (row == self->sends.count + 3) {
				if (channel->panning == 0.f) {
					strcpy(txt,"left");
				} else
				if (channel->panning == 1.f) {
					strcpy(txt,"right");
				} else
				if (channel->panning == 0.5f) {
					strcpy(txt,"center");
				} else {
					psy_snprintf(txt, 128, "%.0f%%", channel->panning * 100);
				}
				return 1;
			} else
			if (row == self->sends.count + 8) {
				float db;

				db = (psy_dsp_amp_t)(20 * log10(channel->volume));
				psy_snprintf(txt, 10, "%.2f dB", db);
				return 1;
			}
		}		
	} else
	if (col < returncolumn(self) + self->returns.count) {
		psy_audio_ReturnChannel* channel;
		uintptr_t index;

		index = col - returncolumn(self);
		channel = (psy_audio_ReturnChannel*) psy_table_at(&self->returns, index);		
		if (channel) {
			if (row == 0) {
				psy_audio_Machine* fx;		
				
				fx = machines_at(base->vtable->machines(base),
					channel->fxslot);
				if (fx && fx->vtable->info(fx)) {
					strcpy(txt, fx->vtable->info(fx)->ShortName);					
				}
				return 1;
			}
			if (row == self->sends.count + 1) {
				if (channel->mastersend == 0) {
					strcpy(txt,"off");
				} else {
					strcpy(txt,"on");
				}
			} else
			if (row == self->sends.count + 3) {
				if (channel->panning == 0.f) {
					strcpy(txt,"left");
				} else
				if (channel->panning == 1.f) {
					strcpy(txt,"right");
				} else
				if (channel->panning == 0.5f) {
					strcpy(txt,"center");
				} else {
					sprintf(txt,"%.0f%%", channel->panning * 100);
				}
				return 1;
			} else
			if (row == self->sends.count + 8) {
				float db;

				db = (psy_dsp_amp_t)(20 * log10(channel->volume));
				psy_snprintf(txt, 10, "%.2f dB", db);
				return 1;
			}
		}
	}
	return 0;
}

uintptr_t numparameters(psy_audio_Mixer* self)
{	
	return (uintptr_t)( numparametercols(self) * (10  + psy_table_size(&self->sends) + 
		psy_table_size(&self->sends)));
}

uintptr_t numparametercols(psy_audio_Mixer* self)
{
	return returncolumn(self) + self->returns.count + 1;
}

int parametername(psy_audio_Mixer* self, char* txt, uintptr_t param)
{		
	uintptr_t col;
	uintptr_t row;
	uintptr_t rows;

	txt[0] = '\0';
	rows = numparameters(self) / numparametercols(self);
	row = param % rows;
	col = param / rows;	
	if (col < mastercolumn(self)) {
		if (row > 0 && row < self->sends.count + 1) {
			psy_snprintf(txt, 128, "Send %u", (unsigned int)row);
			return 1;
		}
	} else
	if (col == mastercolumn(self)) {
		if (row == 0) {
			psy_snprintf(txt, 128, "%s", "MasterOut");
			return 1;
		}
	} else	
	if (col < returncolumn(self)) {
		uintptr_t index;
		psy_audio_MixerChannel* channel;

		index = col - inputcolumn(self);
		channel = (psy_audio_MixerChannel*) psy_table_at(&self->inputs, index);
		if (row == 0) {			
			psy_snprintf(txt, 128, "Input %u", (unsigned int)index + 1);			
		}
	} else
	if (col < returncolumn(self) + self->returns.count) {
		uintptr_t index;
		psy_audio_ReturnChannel* channel;		
		
		index = col - returncolumn(self);
		channel = (psy_audio_ReturnChannel*) psy_table_at(&self->returns, index);
		if (row == 0) {			
			psy_snprintf(txt, 128, "Return %u", (unsigned int) index + 1);
		}
	}
	return *txt != '\0';
}

int parametertype(psy_audio_Mixer* self, uintptr_t param)
{
	uintptr_t col;
	uintptr_t row;
	uintptr_t rows;
	
	rows = numparameters(self) / numparametercols(self);
	row = param % rows;
	col = param / rows;


	if (col < mastercolumn(self)) {
		if (row == 0) {
			return 1;
		} else
		if (row > 0 && row < self->sends.count + 1) {
			return 1;
		}
	} else
	if (col == mastercolumn(self)) {
		if (row == 0) {			
			return 1;
		}
	} else	
	if (col < returncolumn(self)) {
		uintptr_t index;
		psy_audio_MixerChannel* channel;

		index = col - inputcolumn(self);
		channel = (psy_audio_MixerChannel*) psy_table_at(&self->inputs, index);
		if (row == 0) {			
			return 1;
		}
	} else
	if (col < returncolumn(self) + self->returns.count) {
		uintptr_t index;
		psy_audio_ReturnChannel* channel;		
		
		index = col - returncolumn(self);
		channel = (psy_audio_ReturnChannel*) psy_table_at(&self->returns, index);
		if (row == 0) {			
			return 1;
		}
	}	
	return MPF_STATE;
}

void parameterrange(psy_audio_Mixer* self, uintptr_t param, int* minval, int* maxval)
{
	*minval = 0;
	*maxval = 65535;
}

int parameterlabel(psy_audio_Mixer* self, char* txt, uintptr_t param)
{
	return parametername(self, txt, param);	
}

uintptr_t inputcolumn(psy_audio_Mixer* self)
{
	return mastercolumn(self) + 1;
}

uintptr_t returncolumn(psy_audio_Mixer* self)
{
	return inputcolumn(self) + psy_table_size(&self->inputs);
}

psy_audio_WireSocketEntry* wiresocketentry(psy_audio_Mixer* self, uintptr_t input)
{	
	psy_audio_WireSocketEntry* rv = 0;
	psy_audio_MachineSockets* sockets;
	WireSocket* p;
	psy_audio_Machine* base = (psy_audio_Machine*)self;
	psy_audio_Machines* machines = base->vtable->machines(base);

	sockets = connections_at(&machines->connections, self->slot);
	if (sockets) {
		uintptr_t c = 0;

		for (p = sockets->inputs; p != 0 && c != input; p = p->next, ++c);
		if (p) {				
			rv = (psy_audio_WireSocketEntry*) p->entry;
		}
	}
	return rv;
}

void loadspecific(psy_audio_Mixer* self, struct psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	uint32_t filesize;
	int32_t numins = 0;
	uint32_t numrets = 0;
	uint32_t i;
	
	psyfile_read(songfile->file, &filesize, sizeof(filesize));
	psyfile_read(songfile->file, &self->solocolumn, sizeof(self->solocolumn));
	psyfile_read(songfile->file, &self->master.volume, sizeof(float));
	psyfile_read(songfile->file, &self->master.gain, sizeof(float));
	psyfile_read(songfile->file, &self->master.drymix, sizeof(float));
	
	psyfile_read(songfile->file, &numins,sizeof(int));
	psyfile_read(songfile->file, &numrets,sizeof(int));
	self->slot = slot;
	if ( numins >0 ) insertinputchannels(self, numins, &songfile->song->machines);
//	if ( numrets >0 ) InsertReturn(numrets - 1);
//	if ( numrets >0 ) InsertSend(numrets-1, NULL);
	for (i = 0; i < (unsigned int) psy_table_size(&self->inputs); ++i) {
		psy_audio_MixerChannel* channel;
		unsigned int j;

		channel = (psy_audio_MixerChannel*) psy_table_at(&self->inputs, i);
		for (j = 0; j < numrets; ++j) {
			float send = 0.0f;
			psyfile_read(songfile->file, &send,sizeof(float));
			psy_table_insert(&channel->sendvols, j, (void*)(ptrdiff_t)(send * 65535));
		}
		psyfile_read(songfile->file, &channel->volume, sizeof(float));
		psyfile_read(songfile->file, &channel->panning, sizeof(float));
		psyfile_read(songfile->file, &channel->drymix, sizeof(float));
		psyfile_read(songfile->file, &channel->mute, sizeof(unsigned char));
		psyfile_read(songfile->file, &channel->dryonly, sizeof(unsigned char));
		psyfile_read(songfile->file, &channel->wetonly, sizeof(unsigned char));
	}
	//legacyReturn_.resize(numrets);
	//legacySend_.resize(numrets);

	for (i = 0; i < numrets; ++i) {
		psy_audio_ReturnChannel* channel;

		channel = returnchannel_allocinit(-1);
		psy_table_insert(&self->returns, i, channel);		
	}

	for (i = 0; i < numrets; ++i) {
		unsigned int j;
		psy_audio_ReturnChannel* channel;

		channel = (psy_audio_ReturnChannel*) psy_table_at(&self->returns, i);
		{			
			// LegacyWire& leg = legacyReturn_[i];
			int inputmachine;
			float inputconvol;
			float wiremultiplier;
			psyfile_read(songfile->file, &inputmachine, sizeof(inputmachine));	// Incoming (Return) connections psy_audio_Machine number
			psyfile_read(songfile->file, &inputconvol, sizeof(inputconvol));	// /volume value for the current return wire. Range 0.0..1.0. (As opposed to the standard wires)
			psyfile_read(songfile->file, &wiremultiplier, sizeof(wiremultiplier));	// Ignore. (value to divide returnVolume for work. The reason is because natives output at -32768.0f..32768.0f range )
			if (inputconvol > 8.0f) { //bugfix on 1.10.1 alpha
				inputconvol /= 32768.f;
			}
			psy_table_insert(&self->sends, i, (void*)(intptr_t)inputmachine);
			channel->fxslot = inputmachine;
			psy_table_insert(&songfile->song->machines.connections.sends, inputmachine, (void*)1);
		}
		{
			// LegacyWire& leg2 = legacySend_[i];
			int inputmachine;
			float inputconvol;
			float wiremultiplier;
			
			psyfile_read(songfile->file, &inputmachine, sizeof(inputmachine));	// Outgoing (Send) connections psy_audio_Machine number
			psyfile_read(songfile->file, &inputconvol, sizeof(inputconvol));	//volume value for the current send wire. Range 0.0..1.0. (As opposed to the standard wires)
			psyfile_read(songfile->file, &wiremultiplier, sizeof(wiremultiplier));	// Ignore. (value to divide returnVolume for work. The reason is because natives output at -32768.0f..32768.0f range )
			if (inputconvol > 0.f && inputconvol < 0.0002f) { //bugfix on 1.10.1 alpha
				inputconvol *= 32768.f;
			}			
		}		
				
		for (j = 0; j < numrets; ++j)
		{
			unsigned char send = 0;
			psyfile_read(songfile->file, &send, sizeof(unsigned char));
			if (send) {				
				psy_table_insert(&channel->sendsto, j,
					psy_table_at(&self->returns, j));
			}			
		}
		{
			// Return(i)
			unsigned char mastersend;
			float volume;
			float panning;
			unsigned char mute;

			psyfile_read(songfile->file, &mastersend, sizeof(unsigned char));
			psyfile_read(songfile->file, &volume, sizeof(float));
			psyfile_read(songfile->file, &panning, sizeof(float));
			psyfile_read(songfile->file, &mute, sizeof(unsigned char));
			channel->mastersend = mastersend;
			channel->volume = volume;
			channel->panning = panning;
			channel->mute = mute;
		}
	}

	psy_signal_connect(&songfile->song->machines.connections.signal_connected,
		self, onconnected);
	psy_signal_connect(&songfile->song->machines.connections.signal_disconnected,
		self, ondisconnected);
	// return true;
}

void savespecific(psy_audio_Mixer* self, struct psy_audio_SongFile* songfile,
	uintptr_t slot)
{
	float volume_;
	float drywetmix_;
	float gain_;	
	uint8_t temp8;
	uint32_t temp32;
	int32_t numins;
	int32_t numrets;
	uint32_t size;
	uint32_t i;
	uint32_t j;
	
	size = (sizeof(self->solocolumn) + sizeof(volume_) + sizeof(drywetmix_) + sizeof(gain_) +		
		2 * sizeof(uint32_t));
	size += (3 * sizeof(float) + 3 * sizeof(unsigned char) + self->sends.count * sizeof(float)) * self->inputs.count;
	size += (2 * sizeof(float) + 2 * sizeof(unsigned char) + self->sends.count * sizeof(unsigned char) + 2 * sizeof(float) + sizeof(uint32_t)) * self->returns.count;
	size += (2 * sizeof(float) + sizeof(uint32_t)) * self->sends.count;
	psyfile_write(songfile->file, &size, sizeof(size));

	psyfile_write(songfile->file, &self->solocolumn, sizeof(self->solocolumn));
	psyfile_write(songfile->file, &self->master.volume, sizeof(float));
	psyfile_write(songfile->file, &self->master.gain, sizeof(float));
	psyfile_write(songfile->file, &self->master.drymix, sizeof(float));

	numins = self->inputs.count;
	numrets = self->returns.count;
	psyfile_write(songfile->file, &numins, sizeof(int32_t));
	psyfile_write(songfile->file, &numrets, sizeof(int32_t));
	
	for (i = 0; i < (uint32_t) (self->inputs.count); i++)
	{
		psy_audio_MixerChannel* channel;

		channel = (psy_audio_MixerChannel*) psy_table_at(&self->inputs, i);
		for (j = 0; j < self->sends.count; j++)
		{
			float sendvol;

			sendvol = (int)(intptr_t)psy_table_at(&channel->sendvols, j) / 65535.f;
			psyfile_write(songfile->file,  &sendvol, sizeof(float));
		}
		psyfile_write(songfile->file, &channel->volume, sizeof(float));
		psyfile_write(songfile->file, &channel->panning, sizeof(float));
		psyfile_write(songfile->file, &channel->drymix, sizeof(float));
		psyfile_write(songfile->file, &channel->mute, sizeof(unsigned char));
		psyfile_write(songfile->file, &channel->dryonly, sizeof(unsigned char));
		psyfile_write(songfile->file, &channel->wetonly, sizeof(unsigned char));
	}
	for (i = 0; i < self->returns.count; i++)
	{
		float volume, volMultiplier;
		uint32_t wMacIdx;

		psy_audio_ReturnChannel* channel;

		channel = psy_table_at(&self->returns, i);		
		//Returning machines and values
		//const psy_audio_Wire& wireRet = Return(i).GetWire();
		//wMacIdx = (wireRet.Enabled()) ? wireRet.GetSrcMachine()._macIndex : -1;
		//volume = wireRet.GetVolume();
		volMultiplier = 1.0f; // wireRet.GetVolMultiplier();
		psyfile_write(songfile->file, &channel->fxslot, sizeof(int32_t));	// Incoming connections psy_audio_Machine number
		psyfile_write(songfile->file, &channel->volume, sizeof(float));	// Incoming connections psy_audio_Machine vol
		psyfile_write(songfile->file, &volMultiplier, sizeof(float));	// Value to multiply _inputConVol[] to have a 0.0...1.0 range

		//Sending machines and values
		if (psy_table_exists(&channel->sendsto, i)) {
			psy_audio_ReturnChannel* sendto;

			sendto = psy_table_at(&channel->sendsto, i);
			wMacIdx = sendto->fxslot;
			volume = sendto->volume;
			volMultiplier = 1.0f;
		}
		else {
			wMacIdx = -1;
			volume = 1.0f;
			volMultiplier = 1.0f;
		}
		psyfile_write(songfile->file, &wMacIdx, sizeof(int));	// send connections psy_audio_Machine number
		psyfile_write(songfile->file, &volume, sizeof(float));	// send connections psy_audio_Machine vol
		psyfile_write(songfile->file, &volMultiplier, sizeof(float));	// Value to multiply _inputConVol[] to have a 0.0...1.0 range

		//Rewiring of returns to sends and mix values
		for (j = 0; j < self->sends.count; j++)
		{
			unsigned char send;

			send = psy_table_exists(&channel->sendsto, j);
			psyfile_write(songfile->file, &send, sizeof(unsigned char));
		}
		psyfile_write(songfile->file, &channel->mastersend, sizeof(unsigned char));
		psyfile_write(songfile->file, &channel->volume, sizeof(float));
		psyfile_write(songfile->file, &channel->panning, sizeof(float));
		psyfile_write(songfile->file, &channel->mute, sizeof(unsigned char));
	}
}

void insertinputchannels(psy_audio_Mixer* self, uintptr_t num, psy_audio_Machines* machines)
{
	psy_audio_WireSocketEntry* rv = 0;
	psy_audio_MachineSockets* sockets;
	WireSocket* p;	

	sockets = connections_at(&machines->connections, self->slot);
	if (sockets) {
		int c = 0;

		for (p = sockets->inputs; p != 0 && c != num; p = p->next, ++c) {		
			psy_audio_WireSocketEntry* entry;

			entry = (psy_audio_WireSocketEntry*) p->entry;			
			psy_table_insert(&self->inputs, c, mixerchannel_allocinit(c));
		}
	}	
}

int intparamvalue(float value)
{	
	return (int)((value * 65535.f));	
}

float floatparamvalue(int value)
{
	return value / 65535.f;	
}

uintptr_t mastercolumn(psy_audio_Mixer* self)
{
	return 1;
}
