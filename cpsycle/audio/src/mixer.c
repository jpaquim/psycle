// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "mixer.h"
#include "machines.h"
#include <operations.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <dsptypes.h>

static CMachineParameter const paraInputHeader = 
{
	"Input",	
	"Input",								// description
	0,										// MinValue	
	65535,									// MaxValue
	3,										// Flags
	0
};

static CMachineParameter const paraMixKnob = 
{ 
	"Mix",	
	"Mix",									// description
	0,										// MinValue	
	65535,									// MaxValue
	MPF_STATE,								// Flags
	0
};

static CMachineParameter const paraMixHeader = 
{ 
	"Mix",	
	"Mix",									// description
	0,										// MinValue	
	65535,									// MaxValue
	1,										// Flags
	0
};

static CMachineParameter const paraGainKnob = 
{ 
	"Gain",	
	"Gain",									// description
	0,										// MinValue	
	65535,									// MaxValue
	MPF_STATE,								// Flags
	0
};

static CMachineParameter const paraGainHeader = 
{ 
	"Gain",	
	"Gain",									// description
	0,										// MinValue	
	65535,									// MaxValue
	1,										// Flags
	0
};

static CMachineParameter const paraPanKnob = 
{ 
	"Pan",	
	"Pan",									// description
	0,										// MinValue	
	65535,									// MaxValue
	MPF_STATE,								// Flags
	0
};

static CMachineParameter const paraPanHeader = 
{ 
	"Pan",	
	"Pan",									// description
	0,										// MinValue	
	65535,									// MaxValue
	1,										// Flags
	0
};

static CMachineParameter const paraLevelKnob = 
{ 
	"Level",	
	"Level",									// description
	0,										// MinValue	
	65535,									// MaxValue
	MPF_STATE,								// Flags
	0
};

static CMachineParameter const paraVuInfo = 
{ 
	"VU",	
	"VU",									// description
	0,										// MinValue	
	65535,									// MaxValue
	1,										// Flags
	0
};

static CMachineParameter const paraReturnHeader = 
{ 
	"Return",
	"Return",								// description
	0,										// MinValue	
	65535,									// MaxValue
	3,										// Flags
	0
};

static CMachineParameter const paraMasterHeader = 
{ 
	"Master Out",
	"Master Out",							// description
	0,										// MinValue	
	65535,									// MaxValue
	1,										// Flags
	0
};

static CMachineParameter const paraSendVol = 
{ 
	"Send",	
	"Send",									// description
	0,										// MinValue	
	65535,									// MaxValue
	MPF_STATE,								// Flags
	0
};


static CMachineParameter const paraRouteKnob = 
{ 
	"Route",	
	"Route",								// description
	0,										// MinValue	
	1,										// MaxValue
	MPF_STATE,								// Flags
	0
};

static CMachineParameter const paraMasterSendKnob = 
{ 
	"Master",	
	"Master",								// description
	0,										// MinValue	
	1,										// MaxValue
	MPF_STATE,								// Flags
	0
};

static CMachineParameter const paraSoloKnob = 
{ 
	"Solo",	
	"Solo",									// description
	0,										// MinValue	
	1,										// MaxValue
	MPF_STATE,								// Flags
	0
};

static CMachineParameter const paraMuteKnob = 
{ 
	"Mute",	
	"Mute",									// description
	0,										// MinValue	
	1,										// MaxValue
	MPF_STATE,								// Flags
	0
};

static CMachineParameter const paraDryKnob = 
{ 
	"Dry",	
	"Dry",									// description
	0,										// MinValue	
	1,										// MaxValue
	MPF_STATE,								// Flags
	0
};

static CMachineParameter const paraWetKnob = 
{ 
	"Wet",	
	"Wet",									// description
	0,										// MinValue	
	1,										// MaxValue
	MPF_STATE,								// Flags
	0
};

const CMachineInfo* mixer_info(void)
{
	static CMachineInfo const macinfo = {
		MI_VERSION,
		0x0250,
		GENERATOR | 32 | 64,
		0,
		0,
		"Mixer"
			#ifndef NDEBUG
			" (debug build)"
			#endif
			,
		"Mixer",
		"Psycledelics",
		"help",
		1
	};
	return &macinfo;
}

static const CMachineInfo* info(Mixer*);
static void mixer_dispose(Mixer*);
static int mixer_mode(Mixer* self) { return MACHMODE_FX; }
static void mixer_seqtick(Mixer*, int channel, const PatternEvent*);
static unsigned int numinputs(Mixer*);
static unsigned int numoutputs(Mixer*);
static Buffer* mix(Mixer*, int slot, unsigned int amount, MachineSockets*, Machines*);
static void addsamples(Buffer* dst, Buffer* source, unsigned int numsamples, float vol);
static void loadspecific(Mixer*, PsyFile* file, unsigned int slot, Machines*);
static void onconnected(Mixer*, Connections*, int outputslot, int inputslot);
static void ondisconnected(Mixer*, Connections*, int outputslot, int inputslot);
static int parametername(Mixer*, char* txt, int param);
static void parametertweak(Mixer*, int param, int value);
static void patterntweak(Mixer* self, int par, int val);
static int value(Mixer*, int const param);
static int describevalue(Mixer*, char* txt, int const param, int const value);
static unsigned int numparameters(Mixer*);
static unsigned int numcols(Mixer*);
static const CMachineParameter* parameter(Mixer*, unsigned int par);
static int intparamvalue(float value);
static float floatparamvalue(int value);
static WireSocketEntry* wiresocketentry(Mixer*, int input);
static void insertinputchannels(Mixer*, int num, Machines* machines);
static int paramviewoptions(Machine* self) { return MACHINE_PARAMVIEW_COMPACT; }
static unsigned int slot(Mixer* self) { return self->slot; }
static void setslot(Mixer* self, int slot) { self->slot = slot; }
static int mastercolumn(Mixer*);
static int inputcolumn(Mixer*);
static int returncolumn(Mixer*);
static void preparemix(Mixer*, Machines*, unsigned int amount);
static void mixinputs(Mixer*, Machines*, unsigned int amount);
static void workreturns(Mixer*, Machines*, unsigned int amount);
static void mixreturns(Mixer*, Machines*, unsigned int amount);
static void tickrms(Mixer*, unsigned int amount);
static void levelmaster(Mixer*, unsigned int amount);

static MixerChannel* mixerchannel_allocinit(unsigned int inputslot);
static void mixerchannel_dispose(MixerChannel*);

static ReturnChannel* returnchannel_allocinit(unsigned int fxslot);
static void returnchannel_dispose(ReturnChannel*);

void mixerchannel_init(MixerChannel* self, unsigned int inputslot)
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

MixerChannel* mixerchannel_allocinit(unsigned int inputslot)
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

void returnchannel_init(ReturnChannel* self, unsigned int fxslot)
{		
	self->fxslot = fxslot;
	self->mute = 0;
	self->panning = 0.5f;	
	self->volume = 1.0f;	
	self->mastersend = 1;
	table_init(&self->sendsto);
}

ReturnChannel* returnchannel_allocinit(unsigned int fxslot)
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
	Machines* machines;

	machine_init(&self->machine, callback);
	self->machine.info = info;
	self->machine.numinputs = numinputs;
	self->machine.numoutputs = numoutputs;
	self->machine.dispose = mixer_dispose;
	self->machine.seqtick = mixer_seqtick;
	self->machine.mode = mixer_mode;
	self->machine.mix = mix;
	self->machine.parametername = parametername;
	self->machine.parametertweak = parametertweak;
	self->machine.patterntweak = patterntweak;
	self->machine.value = value;
	self->machine.describevalue = describevalue;
	self->machine.numparameters = numparameters;
	self->machine.numcols = numcols;
	self->machine.parameter = parameter;
	self->machine.paramviewoptions = paramviewoptions;
	self->machine.loadspecific = loadspecific;
	self->machine.setslot = setslot;
	self->machine.slot = slot;
	table_init(&self->inputs);
	table_init(&self->sends);
	table_init(&self->returns);
	machines = self->machine.machines(self);
	signal_connect(&machines->connections.signal_connected, self, onconnected);
	signal_connect(&machines->connections.signal_disconnected, self, ondisconnected);
	rmsvol_init(&self->masterrmsvol);
	mixerchannel_init(&self->master, 0);	
	self->slot = 65535;
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
	machine_dispose(&self->machine);	
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

Buffer* mix(Mixer* self, int slot, unsigned int amount, MachineSockets* connected_machine_sockets, Machines* machines)
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
		buffer_clearsamples(channel->buffer, amount);
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
				if (fxchannel) {
					sendvol = (int) table_at(&channel->sendvols,
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
		if (channel) {
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
				dsp_add(
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
	dsp_mul(self->master.buffer->samples[0], amount, self->master.volume);
	dsp_mul(self->master.buffer->samples[1], amount, self->master.volume);
}

void onconnected(Mixer* self, Connections* connections, int outputslot, int inputslot)
{				
	if (inputslot == (int)self->slot) {		
		if (outputslot != (int)self->slot) {
			Machine* machine;
			Machines* machines = self->machine.machines(self);

			machine = machines_at(machines, outputslot);
			if (machine->mode(machine) == MACHMODE_GENERATOR) {
				MixerChannel* channel;

				channel = mixerchannel_allocinit(outputslot);
				table_insert(&self->inputs, self->inputs.count, (void*)channel);
			} else {
				table_insert(&self->sends, self->sends.count, (void*)outputslot);
				table_insert(&self->returns, self->returns.count, 
					returnchannel_allocinit(outputslot));
				machine->ismixersend = 1;
			}
		}
	}
}

void ondisconnected(Mixer* self, Connections* connections, int outputslot, int inputslot)
{
	if (inputslot == (int)self->slot) {
		Machine* machine;
		Machines* machines = self->machine.machines(self);

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
					int key;

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
				int sendslot;
				int c;

				c = tableiterator_key(&it);
				sendslot = (int)tableiterator_value(&it);
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

const CMachineInfo* info(Mixer* self)
{	
	return mixer_info();
}

static amp_t dB2Amp(amp_t db)
{	
	return (amp_t) pow(10.0f, db / 20.0f);
}

void patterntweak(Mixer* self, int numparam, int value)
{
	int channelindex = numparam/16;
	int param = numparam%16;
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
			self->machine.setpanning(&self->machine, (value >> 1) / 256.f);
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
						(void*) intparamvalue( 
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
	int col;
	int row;
	int rows;

	rows = numparameters(self) / numcols(self);
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
				table_insert(&channel->sendvols, row - 1, (void*)value);
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

int value(Mixer* self, int const param)
{	
	int col;
	int row;
	int rows;

	rows = numparameters(self) / numcols(self);
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
				return (int)table_at(&channel->sendvols, row - 1);				
			} else
			if (row == self->sends.count + 2) {
				WireSocketEntry* input_entry;

				input_entry = wiresocketentry(self, col - 1);
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
	int col;
	int row;
	int rows;

	*txt = '\0';
	rows = numparameters(self) / numcols(self);
	row = param % rows;
	col = param / rows;
	if (col < mastercolumn(self)) {
		if (row > 0 && row < self->sends.count + 1) {
			int index;
			int channel;
			
			index = row - 1;
			channel = (int) table_at(&self->sends, index);
			//if (channel) {
			{
				Machine* machine;		
				
				machine = machines_at(self->machine.machines(&self->machine),
					channel);		
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
			_snprintf(txt, 20, "%d%%", (int) (self->master.drymix * 100));
			return 1;
		} else
		if (row == self->sends.count + 2) {
			float db;

			db = (amp_t)(20 * log10(self->master.gain));
			_snprintf(txt, 10, "%.2f dB", db);
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
				sprintf(txt,"%.0f%%", self->master.panning * 100);
			}
			return 1;
		} else
		if (row == self->sends.count + 8) {
			float db;

			db = (amp_t)(20 * log10(self->master.volume));
			_snprintf(txt, 10, "%.2f dB", db);
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
				
				machine = machines_at(self->machine.machines(&self->machine),
					channel->inputslot);		
				if (machine && machine->info(machine)) {
					strcpy(txt, machine->info(machine)->ShortName);					
				}
				return 1;								
			} else
			if (row > 0 && row < self->sends.count + 1) {
				float sendvol;
				sendvol = (int) table_at(&channel->sendvols, row - 1) / 65535.f;
				if (sendvol == 0.0f) {
					strcpy(txt,"Off");
				} else {
					sprintf(txt,"%.0f%%", sendvol * 100.0f);
				}
				return 1;
			} else
			if (row == self->sends.count + 1) {				
				_snprintf(txt, 20, "%d%%", (int) (channel->drymix * 100));
				return 1;
			} else
			if (row == self->sends.count + 2) {
				WireSocketEntry* input_entry;

				input_entry = wiresocketentry(self, col - 1);
				if (input_entry) {
					float db;

					db = (amp_t)(20 * log10(input_entry->volume));
					_snprintf(txt, 10, "%.2f dB", db);
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
					sprintf(txt,"%.0f%%", channel->panning * 100);
				}
				return 1;
			} else
			if (row == self->sends.count + 8) {
				float db;

				db = (amp_t)(20 * log10(channel->volume));
				_snprintf(txt, 10, "%.2f dB", db);
				return 1;
			}
		}		
	} else
	if (col < returncolumn(self) + self->returns.count) {
		ReturnChannel* channel;
		int index;

		index = col - returncolumn(self);
		channel = (ReturnChannel*) table_at(&self->returns, index);		
		if (channel) {
			if (row == 0) {
				Machine* fx;		
				
				fx = machines_at(self->machine.machines(&self->machine),
					channel->fxslot);		
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
				_snprintf(txt, 10, "%.2f dB", db);
				return 1;
			}
		}
	}
	return 0;
}

unsigned int numparameters(Mixer* self)
{	
	return numcols(self) * (10  + self->sends.count + self->sends.count);
}

unsigned int numcols(Mixer* self)
{
	return returncolumn(self) + self->returns.count + 1;
}

int parametername(Mixer* self, char* txt, int param)
{		
	int col;
	int row;
	int rows;

	txt[0] = '\0';
	rows = numparameters(self) / numcols(self);
	row = param % rows;
	col = param / rows;
	if (col < mastercolumn(self)) {
		if (row > 0 && row < self->sends.count + 1) {
			_snprintf(txt, 128, "Send %d", row);
			return 1;
		}
	} else
	if (col == mastercolumn(self)) {
	} else	
	if (col < returncolumn(self)) {
		int index;
		MixerChannel* channel;

		index = col - inputcolumn(self);
		channel = (MixerChannel*) table_at(&self->inputs, index);
		if (row == 0) {			
			_snprintf(txt, 128, "Input %d", index + 1);			
		}
	} else
	if (col < returncolumn(self) + self->returns.count) {
		int index;
		ReturnChannel* channel;		
		
		index = col - returncolumn(self);
		channel = (ReturnChannel*) table_at(&self->returns, index);
		if (row == 0) {			
			_snprintf(txt, 128, "Return %d", index + 1);
		}
	}
	return *txt != '\0';
}

const CMachineParameter* parameter(Mixer* self, unsigned int param)
{			
	static CMachineParameter paraempty = { "", "", 0, 65535,	1, 0 };
	int col;
	int row;
	int rows;

	rows = numparameters(self) / numcols(self);
	row = param % rows;
	col = param / rows;
	if (col < mastercolumn(self)) {
		if (row > 0 && row < self->sends.count + 1) {
			static CMachineParameter par = { "Send", "Send", 0, 65535, 3, 0 };
			return &par;
		} else
		if (row == self->sends.count + 1) {
			return &paraMixHeader;
		} else
		if (row == self->sends.count + 2) {
			return &paraGainHeader;
		} else
		if (row == self->sends.count + 3) {
			return &paraPanHeader;
		}
	} else
	if (col == mastercolumn(self)) {
		if (row == 0) {
			return &paraMasterHeader;
		} else
		if (row > 0 && row < self->sends.count + 1) {			
		} else
		if (row == self->sends.count + 1) {
			return &paraMixKnob;
		} else
		if (row == self->sends.count + 2) {
			return &paraGainKnob;
		} else	
		if (row == self->sends.count + 3) {
			return &paraPanKnob;
		} else	
		if (row == self->sends.count + 8) {
			return &paraLevelKnob;
		} else
		if (row == self->sends.count + 9) {
			return &paraVuInfo;
		}
	} else
	// Input Column
	if (col < returncolumn(self)) {
		if (row == 0) {
			return &paraInputHeader;
		} else
		if (row > 0 && row < self->sends.count + 1) {
			return &paraSendVol;
		} else
		if (row == self->sends.count + 1) {
			return &paraMixKnob;
		} else
		if (row == self->sends.count + 2) {
			return &paraGainKnob;
		} else	
		if (row == self->sends.count + 3) {
			return &paraPanKnob;
		} else
		if (row == self->sends.count + 4) {
			return &paraSoloKnob;
		} else
		if (row == self->sends.count + 5) {
			return &paraMuteKnob;
		} else
		if (row == self->sends.count + 6) {
			return &paraDryKnob;
		} else
		if (row == self->sends.count + 7) {
			return &paraWetKnob;
		} else
		if (row == self->sends.count + 8) {
			return &paraLevelKnob;
		} else
		if (row == self->sends.count + 9) {
			return &paraVuInfo;
		}
	} else
	// Return Column
	if (col < returncolumn(self) + self->returns.count) {
		int returnindex;

		returnindex = col - returncolumn(self);
		if (row == 0) {
			return &paraReturnHeader;
		} else
		if (row > returnindex + 1 && row < self->sends.count + 1) {
			return &paraRouteKnob;
		} else
		if (row == self->sends.count + 1) {
			return &paraMasterSendKnob;
		} else
		if (row == self->sends.count + 3) {
			return &paraPanKnob;		
		} else
		if (row == self->sends.count + 4) {
			return &paraSoloKnob;
		} else
		if (row == self->sends.count + 5) {
			return &paraMuteKnob;
		} else		
		if (row == self->sends.count + 8) {
			return &paraLevelKnob;
		} else
		if (row == self->sends.count + 9) {
			return &paraVuInfo;
		} else {
			
			return &paraempty;
		}
	}
	return &paraempty;
}

int intparamvalue(float value)
{	
	return (int)((value * 65535.f));	
}

float floatparamvalue(int value)
{
	return value / 65535.f;	
}

int mastercolumn(Mixer* self)
{
	return 1;
}

int inputcolumn(Mixer* self)
{
	return mastercolumn(self) + 1;
}

int returncolumn(Mixer* self)
{
	return inputcolumn(self) + self->inputs.count;
}

WireSocketEntry* wiresocketentry(Mixer* self, int input)
{
	WireSocketEntry* rv = 0;
	MachineSockets* sockets;
	WireSocket* p;
	Machines* machines = self->machine.machines(self);

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

void loadspecific(Mixer* self, PsyFile* file, unsigned int slot, Machines* machines)
{
	unsigned int filesize;
	int numins = 0;
	int numrets = 0;
	int i;
	
	psyfile_read(file, &filesize, sizeof(filesize));
	psyfile_read(file, &self->solocolumn, sizeof(self->solocolumn));
	psyfile_read(file, &self->master.volume, sizeof(float));
	psyfile_read(file, &self->master.gain, sizeof(float));
	psyfile_read(file, &self->master.drymix, sizeof(float));
	
	psyfile_read(file, &numins,sizeof(int));
	psyfile_read(file, &numrets,sizeof(int));
	self->slot = slot;
	if ( numins >0 ) insertinputchannels(self, numins, machines);
//	if ( numrets >0 ) InsertReturn(numrets - 1);
//	if ( numrets >0 ) InsertSend(numrets-1, NULL);
	for (i = 0; i < self->inputs.count; ++i) {
		MixerChannel* channel;
		int j;

		channel = (MixerChannel*) table_at(&self->inputs, i);
		for (j = 0; j<numrets; ++j) {
			float send = 0.0f;
			psyfile_read(file, &send,sizeof(float));
			table_insert(&channel->sendvols, j, (void*)(int)(send * 65535));
		}
		psyfile_read(file, &channel->volume, sizeof(float));
		psyfile_read(file, &channel->panning, sizeof(float));
		psyfile_read(file, &channel->drymix, sizeof(float));
		psyfile_read(file, &channel->mute, sizeof(unsigned char));
		psyfile_read(file, &channel->dryonly, sizeof(unsigned char));
		psyfile_read(file, &channel->wetonly, sizeof(unsigned char));
	}
	//legacyReturn_.resize(numrets);
	//legacySend_.resize(numrets);

	for (i = 0; i < numrets; ++i) {
		ReturnChannel* channel;

		channel = returnchannel_allocinit(-1);
		table_insert(&self->returns, i, channel);		
	}

	for (i = 0; i < numrets; ++i) {
		int j;
		ReturnChannel* channel;

		channel = (ReturnChannel*) table_at(&self->returns, i);
		{			
			// LegacyWire& leg = legacyReturn_[i];
			int inputmachine;
			float inputconvol;
			float wiremultiplier;
			psyfile_read(file, &inputmachine, sizeof(inputmachine));	// Incoming (Return) connections Machine number
			psyfile_read(file, &inputconvol, sizeof(inputconvol));	// /volume value for the current return wire. Range 0.0..1.0. (As opposed to the standard wires)
			psyfile_read(file, &wiremultiplier, sizeof(wiremultiplier));	// Ignore. (value to divide returnVolume for work. The reason is because natives output at -32768.0f..32768.0f range )
			if (inputconvol > 8.0f) { //bugfix on 1.10.1 alpha
				inputconvol /= 32768.f;
			}
			table_insert(&self->sends, i, (void*)inputmachine);
			channel->fxslot = inputmachine;
			table_insert(&machines->connections.sends, inputmachine, (void*)1);
		}
		{
			// LegacyWire& leg2 = legacySend_[i];
			int inputmachine;
			float inputconvol;
			float wiremultiplier;
			
			psyfile_read(file, &inputmachine, sizeof(inputmachine));	// Outgoing (Send) connections Machine number
			psyfile_read(file, &inputconvol, sizeof(inputconvol));	//volume value for the current send wire. Range 0.0..1.0. (As opposed to the standard wires)
			psyfile_read(file, &wiremultiplier, sizeof(wiremultiplier));	// Ignore. (value to divide returnVolume for work. The reason is because natives output at -32768.0f..32768.0f range )
			if (inputconvol > 0.f && inputconvol < 0.0002f) { //bugfix on 1.10.1 alpha
				inputconvol *= 32768.f;
			}			
		}		
				
		for (j = 0; j < numrets; ++j)
		{
			unsigned char send = 0;
			psyfile_read(file, &send, sizeof(unsigned char));
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

			psyfile_read(file, &mastersend, sizeof(unsigned char));
			psyfile_read(file, &volume, sizeof(float));
			psyfile_read(file, &panning, sizeof(float));
			psyfile_read(file, &mute, sizeof(unsigned char));
			channel->mastersend = mastersend;
			channel->volume = volume;
			channel->panning = panning;
			channel->mute = mute;
		}
	}

	signal_connect(&machines->connections.signal_connected, self, onconnected);
	signal_connect(&machines->connections.signal_disconnected, self, ondisconnected);
	// return true;
}

void insertinputchannels(Mixer* self, int num, Machines* machines)
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
