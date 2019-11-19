// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

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

const MachineInfo* mixer_info(void)
{
	static MachineInfo const macinfo = {
		MI_VERSION,
		0x0250,
		GENERATOR | 32 | 64,
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

static const MachineInfo* info(Mixer*);
static void mixer_dispose(Mixer*);
static int mixer_mode(Mixer* self) { return MACHMODE_FX; }
static void mixer_seqtick(Mixer*, int channel, const PatternEvent*);
static unsigned int numinputs(Mixer*);
static unsigned int numoutputs(Mixer*);
static Buffer* mix(Mixer*, uintptr_t slot, unsigned int amount, MachineSockets*, Machines*);
static void addsamples(Buffer* dst, Buffer* source, unsigned int numsamples, float vol);
static void loadspecific(Mixer*, struct SongFile*, unsigned int slot);
static void savespecific(Mixer*, struct SongFile*, unsigned int slot);
static void onconnected(Mixer*, Connections*, uintptr_t outputslot, uintptr_t inputslot);
static void ondisconnected(Mixer*, Connections*, uintptr_t outputslot, uintptr_t inputslot);
static int parametertype(Mixer*, int par);
static void parameterrange(Mixer*, int param, int* minval, int* maxval);
static int parameterlabel(Mixer*, char* txt, int param);
static int parametername(Mixer*, char* txt, int param);
static void parametertweak(Mixer*, int param, int value);
static void patterntweak(Mixer* self, int par, int val);
static int parametervalue(Mixer*, int const param);
static int describevalue(Mixer*, char* txt, int const param, int const value);
static unsigned int numparameters(Mixer*);
static unsigned int numparametercols(Mixer*);

static int intparamvalue(float value);
static float floatparamvalue(int value);
static WireSocketEntry* wiresocketentry(Mixer*, uintptr_t input);
static void insertinputchannels(Mixer*, uintptr_t num, Machines* machines);
static int paramviewoptions(Machine* self) { return MACHINE_PARAMVIEW_COMPACT; }
static uintptr_t slot(Mixer* self) { return self->slot; }
static void setslot(Mixer* self, uintptr_t slot) { self->slot = slot; }
static uintptr_t mastercolumn(Mixer*);
static uintptr_t inputcolumn(Mixer*);
static uintptr_t returncolumn(Mixer*);
static void preparemix(Mixer*, Machines*, unsigned int amount);
static void mixinputs(Mixer*, Machines*, unsigned int amount);
static void workreturns(Mixer*, Machines*, unsigned int amount);
static void mixreturns(Mixer*, Machines*, unsigned int amount);
static void tickrms(Mixer*, unsigned int amount);
static void levelmaster(Mixer*, unsigned int amount);

static MixerChannel* mixerchannel_allocinit(uintptr_t inputslot);
static void mixerchannel_dispose(MixerChannel*);

static void returnchannel_init(ReturnChannel*, uintptr_t fxslot);
static ReturnChannel* returnchannel_allocinit(uintptr_t fxslot);
static void returnchannel_dispose(ReturnChannel*);

void mixerchannel_init(MixerChannel* self, uintptr_t inputslot)
{		
	self->inputslot = inputslot;
	self->drymix = 1.0f;
	self->dryonly = 0;
	self->mute = 0;
	self->panning = 0.5f;
	table_init(&self->sendvols);
	self->volume = 1.0f;
	self->gain = 1.f;
	self->wetonly = 0;
}

MixerChannel* mixerchannel_allocinit(uintptr_t inputslot)
{
	MixerChannel* rv;

	rv = (MixerChannel*) malloc(sizeof(MixerChannel));
	if (rv) {
		mixerchannel_init(rv, inputslot);
	}	
	return rv;
}

void mixerchannel_dispose(MixerChannel* self)
{	
	table_dispose(&self->sendvols);	
}

void returnchannel_init(ReturnChannel* self, uintptr_t fxslot)
{		
	self->fxslot = fxslot;
	self->mute = 0;
	self->panning = 0.5f;	
	self->volume = 1.0f;	
	self->mastersend = 1;
	table_init(&self->sendsto);
}

ReturnChannel* returnchannel_allocinit(uintptr_t fxslot)
{
	ReturnChannel* rv;

	rv = (ReturnChannel*) malloc(sizeof(ReturnChannel));
	if (rv) {
		returnchannel_init(rv, fxslot);
	}	
	return rv;
}

void returnchannel_dispose(ReturnChannel* self)
{		
	table_dispose(&self->sendsto);
}

void mixer_init(Mixer* self, MachineCallback callback)
{
	Machine* base = (Machine*)self;
	Machines* machines;

	custommachine_init(&self->custommachine, callback);
	base->info = info;
	base->numinputs = numinputs;
	base->numoutputs = numoutputs;
	base->dispose = mixer_dispose;
	base->seqtick = mixer_seqtick;
	base->mode = mixer_mode;
	base->mix = mix;
	base->parametertype = parametertype;
	base->parameterrange = parameterrange;
	base->parametername = parametername;
	base->parameterlabel = parameterlabel;
	base->parametertweak = parametertweak;
	base->patterntweak = patterntweak;
	base->parametervalue = parametervalue;
	base->describevalue = describevalue;
	base->numparameters = numparameters;
	base->numparametercols = numparametercols;	
	base->paramviewoptions = paramviewoptions;
	base->loadspecific = loadspecific;
	base->savespecific = savespecific;
	base->setslot = setslot;
	base->slot = slot;	
	table_init(&self->inputs);
	table_init(&self->sends);
	table_init(&self->returns);
	machines = base->machines(base);
	signal_connect(&machines->connections.signal_connected, self, onconnected);
	signal_connect(&machines->connections.signal_disconnected, self, ondisconnected);
	rmsvol_init(&self->masterrmsvol);
	mixerchannel_init(&self->master, 0);	
	self->slot = 65535;
	base->seteditname(base, "Mixer");
}

void mixer_dispose(Mixer* self)
{
	{ // dispose inputs
		TableIterator it;

		for (it = table_begin(&self->inputs);
			!tableiterator_equal(&it, table_end()); tableiterator_inc(&it)) {
			MixerChannel* channel;

			channel = (MixerChannel*)tableiterator_value(&it);
			mixerchannel_dispose(channel);
			free(channel);		
		}	
		table_dispose(&self->inputs);
	}
	table_dispose(&self->sends);
	{ // dispose returns
		TableIterator it;

		for (it = table_begin(&self->returns);
			!tableiterator_equal(&it, table_end()); tableiterator_inc(&it)) {
			ReturnChannel* channel;

			channel = (ReturnChannel*)tableiterator_value(&it);
			returnchannel_dispose(channel);
			free(channel);		
		}	
		table_dispose(&self->returns);
	}	
	custommachine_dispose(&self->custommachine);	
}

void mixer_seqtick(Mixer* self, int channel, const PatternEvent* event)
{	
	if(event->note == NOTECOMMANDS_TWEAK)
	{
		int nv = (event->cmd<<8)+event->parameter;
		int param =  event->inst; // translate_param(event->inst);

		// if(param < GetNumParams()) {
		//	SetParameter(param,nv);
		// }
	}
	else if(event->note == NOTECOMMANDS_TWEAKSLIDE)
	{
		//\todo: Tweaks and tweak slides should not be a per-machine thing, but rather be player centric.
		// doing simply "tweak" for now..
		int nv = (event->cmd<<8)+event->parameter;
		int param = event->inst; // translate_param(event->inst);

		//if(param < GetNumParams()) {
		//	SetParameter(param,nv);
		//}
	}
}

unsigned int numinputs(Mixer* self)
{
	return 2;
}

unsigned int numoutputs(Mixer* self)
{
	return 2;
}

Buffer* mix(Mixer* self, uintptr_t slot, unsigned int amount, MachineSockets* connected_machine_sockets, Machines* machines)
{							
	preparemix(self, machines, amount);
	mixinputs(self, machines, amount);
	workreturns(self, machines, amount);	
	mixreturns(self, machines, amount);
	tickrms(self, amount);
	levelmaster(self, amount);	
	return self->master.buffer;
}

void preparemix(Mixer* self, Machines* machines, unsigned int amount)
{			
	TableIterator iter;

	self->master.buffer = machines_outputs(machines, self->slot);
	buffer_clearsamples(self->master.buffer, amount);	
	for (iter = table_begin(&self->returns);
		!tableiterator_equal(&iter, table_end());
			tableiterator_inc(&iter)) {	
		ReturnChannel* channel;

		channel = (ReturnChannel*) tableiterator_value(&iter);
		channel->buffer = machines_outputs(machines, channel->fxslot);
		channel->fx = machines_at(machines, channel->fxslot);
		if (channel->buffer) {
			buffer_clearsamples(channel->buffer, amount);
		}
	}
}

void mixinputs(Mixer* self, Machines* machines, unsigned int amount)
{		
	TableIterator iter;	
	
	for (iter = table_begin(&self->inputs);
			!tableiterator_equal(&iter, table_end());
			tableiterator_inc(&iter)) {		
		MixerChannel* channel;		
		
		channel = (MixerChannel*) tableiterator_value(&iter);
		if (channel) {						
			WireSocketEntry* input_entry;
			float wirevol = 1.f;
			TableIterator fx_iter;

			channel->buffer = machines_outputs(machines, channel->inputslot);
			input_entry = wiresocketentry(self, tableiterator_key(&iter));
			if (input_entry) {
				wirevol = input_entry->volume;	
			}
			addsamples(self->master.buffer, channel->buffer, amount,
				channel->volume * channel->drymix * wirevol);
			for (fx_iter = table_begin(&self->returns);
					!tableiterator_equal(&fx_iter, table_end());
					tableiterator_inc(&fx_iter)) {
				ReturnChannel* fxchannel;
				int sendvol;			

				fxchannel = (ReturnChannel*)tableiterator_value(&fx_iter);
				if (fxchannel && fxchannel->buffer) {
					sendvol = (int)(uintptr_t) table_at(&channel->sendvols,
						tableiterator_key(&fx_iter));
					addsamples(fxchannel->buffer, channel->buffer, amount,
						(sendvol / 65535.f) * wirevol);
				}
			}
		}
	}
}

void mixreturns(Mixer* self, Machines* machines, unsigned int amount)
{		
	TableIterator iter;
	
	for (iter = table_begin(&self->returns);
		!tableiterator_equal(&iter, table_end());
			tableiterator_inc(&iter)) {	
		ReturnChannel* channel;		

		channel = tableiterator_value(&iter);
		if (channel && channel->mastersend) {			
			addsamples(self->master.buffer, channel->buffer, amount,
				channel->volume);
		}
	}
}

void workreturns(Mixer* self, Machines* machines, unsigned int amount)
{	
	TableIterator iter;
	
	for (iter = table_begin(&self->returns);
		!tableiterator_equal(&iter, table_end());
			tableiterator_inc(&iter)) {	
		ReturnChannel* channel;		

		channel = tableiterator_value(&iter);
		if (channel && channel->fx && channel->buffer) {
			BufferContext bc;
			List* events = 0;			
			TableIterator sendsto_iter;			
			
			buffercontext_init(&bc, events, channel->buffer, channel->buffer,
				amount, 16, 0);
			channel->fx->work(channel->fx, &bc);
		//	buffer_pan(fxbuffer, fx->panning(fx), amount);
		//	buffer_pan(fxbuffer, channel->panning, amount);
			signal_emit(&channel->fx->signal_worked, channel->fx, 2, 
				channel->fxslot, &bc);
			if (channel->sendsto.count >=0 ) {
				for (sendsto_iter = table_begin(&channel->sendsto);
					!tableiterator_equal(&sendsto_iter, table_end());
						tableiterator_inc(&sendsto_iter)) {
					ReturnChannel* sendto;

					sendto = (ReturnChannel*)tableiterator_value(&sendsto_iter);
					if (sendto) {										
						addsamples(sendto->buffer, channel->buffer, amount, 
							channel->volume);
					}
				}
			}
		}
	}
}

void addsamples(Buffer* dst, Buffer* source, unsigned int numsamples, float vol)
{
	unsigned int channel;

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

void tickrms(Mixer* self, unsigned int amount)
{
	rmsvol_tick(&self->masterrmsvol, 
		self->master.buffer->samples[0],
		self->master.buffer->samples[1],
		amount);
}

void levelmaster(Mixer* self, unsigned int amount)
{
	dsp.mul(self->master.buffer->samples[0], amount, self->master.volume);
	dsp.mul(self->master.buffer->samples[1], amount, self->master.volume);
}

void onconnected(Mixer* self, Connections* connections, uintptr_t outputslot, uintptr_t inputslot)
{				
	Machine* base = (Machine*)self;
	if (inputslot == (int)self->slot) {		
		if (outputslot != (int)self->slot) {
			Machine* machine;
			Machines* machines = base->machines(base);

			machine = machines_at(machines, outputslot);
			if (machine->mode(machine) == MACHMODE_GENERATOR) {
				MixerChannel* channel;

				channel = mixerchannel_allocinit(outputslot);
				table_insert(&self->inputs, self->inputs.count, (void*)channel);
			} else {
				table_insert(&self->sends, self->sends.count, (void*)outputslot);
				table_insert(&self->returns, self->returns.count, 
					returnchannel_allocinit(outputslot));				
			}
		}
	}
}

void ondisconnected(Mixer* self, Connections* connections, uintptr_t outputslot, uintptr_t inputslot)
{
	Machine* base = (Machine*)self;
	if (inputslot == (int)self->slot) {
		Machine* machine;
		Machines* machines = base->machines(base);

		machine = machines_at(machines, outputslot);
		if (machine->mode(machine) == MACHMODE_GENERATOR) {
			TableIterator it;
			int c = 0;

			for (it = table_begin(&self->inputs);
				!tableiterator_equal(&it, table_end()); tableiterator_inc(&it), ++c) {
				MixerChannel* channel;
		
				channel = (MixerChannel*)tableiterator_value(&it);
				if (channel->inputslot == outputslot) {
					table_remove(&self->inputs, c);
					mixerchannel_dispose(channel);
					free(channel);
					break;
				}
			}			
		} else {
			TableIterator it;			

			for (it = table_begin(&self->returns);
					!tableiterator_equal(&it, table_end());
					tableiterator_inc(&it)) {				
				TableIterator sendsto_iter;	
				ReturnChannel* channel;				

				channel = table_at(&self->returns, tableiterator_key(&it));

				sendsto_iter = table_begin(&channel->sendsto);
				while (!tableiterator_equal(&sendsto_iter, table_end())) {
					ReturnChannel* sendto;
					uintptr_t key;

					key = tableiterator_key(&sendsto_iter);
					sendto = (ReturnChannel*)tableiterator_value(&sendsto_iter);
					tableiterator_inc(&sendsto_iter);
					if (sendto) {										
						if (sendto->fxslot == outputslot) {
							table_remove(&channel->sendsto, key);								
						}
					}					
				}
			}

			for (it = table_begin(&self->sends);
				!tableiterator_equal(&it, table_end()); tableiterator_inc(&it)) {
				uintptr_t sendslot;
				uintptr_t c;

				c = tableiterator_key(&it);
				sendslot = (uintptr_t)tableiterator_value(&it);
				if (sendslot == outputslot) {
					ReturnChannel* returnchannel;
					table_remove(&self->sends, c);					
					returnchannel = table_at(&self->returns, c);
					table_remove(&self->returns, c);
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

const MachineInfo* info(Mixer* self)
{	
	return mixer_info();
}

static amp_t dB2Amp(amp_t db)
{	
	return (amp_t) pow(10.0f, db / 20.0f);
}

void patterntweak(Mixer* self, int numparam, int value)
{
	Machine* base = (Machine*)self;
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
				amp_t dbs = (value/42.67f)-96.0f;
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
			base->setpanning(base, (value >> 1) / 256.f);
		}
	} else
	// Inputs
	if (channelindex <= self->inputs.count) {
		MixerChannel* channel;

		channel = table_at(&self->inputs, channelindex - 1);
		if (channel) {
			if (param == 0) { 
				channel->drymix = (value == 256) 
					? 1.0f
					: ((value&0xFF)/256.0f);
			} else
			if (param <= 12) {				 
				if (param - 1 < self->sends.count) {														
					table_insert(&channel->sendvols, param - 1, 
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
				WireSocketEntry* input_entry;

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
					amp_t dbs = (value/42.67f)-96.0f;
					channel->volume = dB2Amp(dbs);
				}
			}
		}
	}
}


void parametertweak(Mixer* self, int param, int value)
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
		MixerChannel* channel;
			
		channel = (MixerChannel*) table_at(&self->inputs,
			col - inputcolumn(self));
		if (channel) {
			if (row == self->sends.count + 1) {
				channel->drymix = value / 65535.f;
			} else
			if (row > 0 && row < self->sends.count + 1) {						
				table_insert(&channel->sendvols, row - 1, (void*)(ptrdiff_t)value);
			} else
			if (row == self->sends.count + 2) {
				WireSocketEntry* input_entry;

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
		ReturnChannel* channel;
	
		channel = (ReturnChannel*) table_at(&self->returns,
			col - returncolumn(self));
		if (channel) {
			if (row > 0 && row < self->sends.count + 1) {
				if (value) {
					table_insert(&channel->sendsto, row - 1,
						(ReturnChannel*) table_at(&self->returns, 
							col - returncolumn(self) + 1));
				} else {
					table_remove(&channel->sendsto, row - 1);
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

int parametervalue(Mixer* self, int const param)
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
			return (int)(rmsvol_value(&self->masterrmsvol) / 32768.f  * 65535);
		}
	} else 
	// Input Column
	if (col < returncolumn(self)) {
		MixerChannel* channel;
	
		channel = (MixerChannel*) table_at(&self->inputs, 
			col - inputcolumn(self));
		if (channel) {
			if (row == self->sends.count + 1) {
				return (int)(channel->drymix * 65535);
			} else
			if (row > 0 && row < self->sends.count + 1) {			
				return (int)(ptrdiff_t)table_at(&channel->sendvols, row - 1);
			} else
			if (row == self->sends.count + 2) {
				WireSocketEntry* input_entry;

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
		ReturnChannel* channel;
	
		channel = (ReturnChannel*) table_at(&self->returns,
			col - returncolumn(self));
		if (channel) {
			if (row > 0 && row < self->sends.count + 1) {
				return table_exists(&channel->sendsto, row - 1);				
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

int describevalue(Mixer* self, char* txt, int const param, int const value)
{ 	
	Machine* base = (Machine*)self;
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
			channel = (uintptr_t) table_at(&self->sends, index);
			//if (channel) {
			{
				Machine* machine;		
				
				machine = machines_at(base->machines(base), channel);		
				if (machine && machine->info(machine)) {
					strcpy(txt, machine->info(machine)->ShortName);
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

			db = (amp_t)(20 * log10(self->master.gain));
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

			db = (amp_t)(20 * log10(self->master.volume));
			psy_snprintf(txt, 10, "%.2f dB", db);
			return 1;
		}
	} else
	if (col < returncolumn(self)) {
		MixerChannel* channel;
	
		channel = (MixerChannel*) table_at(&self->inputs,
			col - inputcolumn(self));
		if (channel) {
			if (row == 0) {
				Machine* machine;		
				
				machine = machines_at(base->machines(base), channel->inputslot);		
				if (machine && machine->info(machine)) {
					strcpy(txt, machine->info(machine)->ShortName);					
				}
				return 1;								
			} else
			if (row > 0 && row < self->sends.count + 1) {
				amp_t sendvol;
				sendvol = (int) (ptrdiff_t)table_at(&channel->sendvols, row - 1) / 65535.f;
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
				WireSocketEntry* input_entry;

				input_entry = wiresocketentry(self, col - inputcolumn(self));
				if (input_entry) {
					float db;

					db = (amp_t)(20 * log10(input_entry->volume));
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

				db = (amp_t)(20 * log10(channel->volume));
				psy_snprintf(txt, 10, "%.2f dB", db);
				return 1;
			}
		}		
	} else
	if (col < returncolumn(self) + self->returns.count) {
		ReturnChannel* channel;
		uintptr_t index;

		index = col - returncolumn(self);
		channel = (ReturnChannel*) table_at(&self->returns, index);		
		if (channel) {
			if (row == 0) {
				Machine* fx;		
				
				fx = machines_at(base->machines(base), channel->fxslot);
				if (fx && fx->info(fx)) {
					strcpy(txt, fx->info(fx)->ShortName);					
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

				db = (amp_t)(20 * log10(channel->volume));
				psy_snprintf(txt, 10, "%.2f dB", db);
				return 1;
			}
		}
	}
	return 0;
}

unsigned int numparameters(Mixer* self)
{	
	return (unsigned int)( numparametercols(self) * (10  + table_size(&self->sends) + 
		table_size(&self->sends)));
}

unsigned int numparametercols(Mixer* self)
{
	return (unsigned int) (returncolumn(self) + self->returns.count + 1);
}

int parametername(Mixer* self, char* txt, int param)
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
		MixerChannel* channel;

		index = col - inputcolumn(self);
		channel = (MixerChannel*) table_at(&self->inputs, index);
		if (row == 0) {			
			psy_snprintf(txt, 128, "Input %u", (unsigned int)index + 1);			
		}
	} else
	if (col < returncolumn(self) + self->returns.count) {
		uintptr_t index;
		ReturnChannel* channel;		
		
		index = col - returncolumn(self);
		channel = (ReturnChannel*) table_at(&self->returns, index);
		if (row == 0) {			
			psy_snprintf(txt, 128, "Return %u", (unsigned int) index + 1);
		}
	}
	return *txt != '\0';
}

int parametertype(Mixer* self, int param)
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
		MixerChannel* channel;

		index = col - inputcolumn(self);
		channel = (MixerChannel*) table_at(&self->inputs, index);
		if (row == 0) {			
			return 1;
		}
	} else
	if (col < returncolumn(self) + self->returns.count) {
		uintptr_t index;
		ReturnChannel* channel;		
		
		index = col - returncolumn(self);
		channel = (ReturnChannel*) table_at(&self->returns, index);
		if (row == 0) {			
			return 1;
		}
	}	
	return MPF_STATE;
}

void parameterrange(Mixer* self, int param, int* minval, int* maxval)
{
	*minval = 0;
	*maxval = 65535;
}

int parameterlabel(Mixer* self, char* txt, int param)
{
	return parametername(self, txt, param);	
}

uintptr_t inputcolumn(Mixer* self)
{
	return mastercolumn(self) + 1;
}

uintptr_t returncolumn(Mixer* self)
{
	return inputcolumn(self) + table_size(&self->inputs);
}

WireSocketEntry* wiresocketentry(Mixer* self, uintptr_t input)
{	
	WireSocketEntry* rv = 0;
	MachineSockets* sockets;
	WireSocket* p;
	Machine* base = (Machine*)self;
	Machines* machines = base->machines(base);

	sockets = connections_at(&machines->connections, self->slot);
	if (sockets) {
		int c = 0;

		for (p = sockets->inputs; p != 0 && c != input; p = p->next, ++c);
		if (p) {				
			rv = (WireSocketEntry*) p->entry;
		}
	}
	return rv;
}

void loadspecific(Mixer* self, struct SongFile* songfile, unsigned int slot)
{
	unsigned int filesize;
	int numins = 0;
	unsigned int numrets = 0;
	unsigned int i;
	
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
	for (i = 0; i < (unsigned int) table_size(&self->inputs); ++i) {
		MixerChannel* channel;
		unsigned int j;

		channel = (MixerChannel*) table_at(&self->inputs, i);
		for (j = 0; j < numrets; ++j) {
			float send = 0.0f;
			psyfile_read(songfile->file, &send,sizeof(float));
			table_insert(&channel->sendvols, j, (void*)(ptrdiff_t)(send * 65535));
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
		ReturnChannel* channel;

		channel = returnchannel_allocinit(-1);
		table_insert(&self->returns, i, channel);		
	}

	for (i = 0; i < numrets; ++i) {
		unsigned int j;
		ReturnChannel* channel;

		channel = (ReturnChannel*) table_at(&self->returns, i);
		{			
			// LegacyWire& leg = legacyReturn_[i];
			int inputmachine;
			float inputconvol;
			float wiremultiplier;
			psyfile_read(songfile->file, &inputmachine, sizeof(inputmachine));	// Incoming (Return) connections Machine number
			psyfile_read(songfile->file, &inputconvol, sizeof(inputconvol));	// /volume value for the current return wire. Range 0.0..1.0. (As opposed to the standard wires)
			psyfile_read(songfile->file, &wiremultiplier, sizeof(wiremultiplier));	// Ignore. (value to divide returnVolume for work. The reason is because natives output at -32768.0f..32768.0f range )
			if (inputconvol > 8.0f) { //bugfix on 1.10.1 alpha
				inputconvol /= 32768.f;
			}
			table_insert(&self->sends, i, (void*)(intptr_t)inputmachine);
			channel->fxslot = inputmachine;
			table_insert(&songfile->song->machines.connections.sends, inputmachine, (void*)1);
		}
		{
			// LegacyWire& leg2 = legacySend_[i];
			int inputmachine;
			float inputconvol;
			float wiremultiplier;
			
			psyfile_read(songfile->file, &inputmachine, sizeof(inputmachine));	// Outgoing (Send) connections Machine number
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
				table_insert(&channel->sendsto, j,
					table_at(&self->returns, j));
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

	signal_connect(&songfile->song->machines.connections.signal_connected, self, onconnected);
	signal_connect(&songfile->song->machines.connections.signal_disconnected, self, ondisconnected);
	// return true;
}

void savespecific(Mixer* self, struct SongFile* songfile, unsigned int slot)
{
	float volume_;
	float drywetmix_;
	float gain_;
	int32_t numins;
	int32_t numrets;
	uint32_t size;
	int32_t i;
	int32_t j;
	
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
	for (i = 0; i < self->inputs.count; i++)
	{
		MixerChannel* channel;

		channel = (MixerChannel*) table_at(&self->inputs, i);
		for (j = 0; j < self->sends.count; j++)
		{
			float sendvol;

			sendvol = (int)(intptr_t)table_at(&channel->sendvols, j) / 65535.f;
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

		ReturnChannel* channel;

		channel = table_at(&self->returns, i);		
		//Returning machines and values
		//const Wire& wireRet = Return(i).GetWire();
		//wMacIdx = (wireRet.Enabled()) ? wireRet.GetSrcMachine()._macIndex : -1;
		//volume = wireRet.GetVolume();
		volMultiplier = 1.0f; // wireRet.GetVolMultiplier();
		psyfile_write(songfile->file, &channel->fxslot, sizeof(int32_t));	// Incoming connections Machine number
		psyfile_write(songfile->file, &channel->volume, sizeof(float));	// Incoming connections Machine vol
		psyfile_write(songfile->file, &volMultiplier, sizeof(float));	// Value to multiply _inputConVol[] to have a 0.0...1.0 range

		//Sending machines and values
		if (table_exists(&channel->sendsto, i)) {
			ReturnChannel* sendto;

			sendto = table_at(&channel->sendsto, i);
			wMacIdx = sendto->fxslot;
			volume = sendto->volume;
			volMultiplier = 1.0f;
		}
		else {
			wMacIdx = -1;
			volume = 1.0f;
			volMultiplier = 1.0f;
		}
		psyfile_write(songfile->file, &wMacIdx, sizeof(int));	// send connections Machine number
		psyfile_write(songfile->file, &volume, sizeof(float));	// send connections Machine vol
		psyfile_write(songfile->file, &volMultiplier, sizeof(float));	// Value to multiply _inputConVol[] to have a 0.0...1.0 range

		//Rewiring of returns to sends and mix values
		for (j = 0; j < self->sends.count; j++)
		{
			unsigned char send;

			send = table_exists(&channel->sendsto, j);
			psyfile_write(songfile->file, &send, sizeof(unsigned char));
		}
		psyfile_write(songfile->file, &channel->mastersend, sizeof(unsigned char));
		psyfile_write(songfile->file, &channel->volume, sizeof(float));
		psyfile_write(songfile->file, &channel->panning, sizeof(float));
		psyfile_write(songfile->file, &channel->mute, sizeof(unsigned char));
	}
}

void insertinputchannels(Mixer* self, uintptr_t num, Machines* machines)
{
	WireSocketEntry* rv = 0;
	MachineSockets* sockets;
	WireSocket* p;	

	sockets = connections_at(&machines->connections, self->slot);
	if (sockets) {
		int c = 0;

		for (p = sockets->inputs; p != 0 && c != num; p = p->next, ++c) {		
			WireSocketEntry* entry;

			entry = (WireSocketEntry*) p->entry;			
			table_insert(&self->inputs, c, mixerchannel_allocinit(c));
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

uintptr_t mastercolumn(Mixer* self)
{
	return 1;
}
