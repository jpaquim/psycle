// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "mixer.h"
#include "machines.h"
#include <operations.h>
#include <windows.h>
#include <math.h>

#define TRUE 1
#define FALSE 0

static CMachineParameter const paraDummy = 
{ 
	"",	
	"",										// description
	0,										// MinValue	
	65535,									// MaxValue
	1,										// Flags
	0
};

static CMachineParameter const paraInputHeader = 
{ 
	"Input",	
	"Input",								// description
	0,										// MinValue	
	65535,									// MaxValue
	1,										// Flags
	0
};

static CMachineParameter const paraSendHeader = 
{ 
	"Send",	
	"Send",									// description
	0,										// MinValue	
	65535,									// MaxValue
	1,										// Flags
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

static CMachineParameter const paraGainKnob = 
{ 
	"Gain",	
	"Gain",									// description
	0,										// MinValue	
	65535,									// MaxValue
	MPF_STATE,								// Flags
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
	1,										// Flags
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
static void work_send(Mixer*, int slot, Machine*, Buffer* output, 
		unsigned int amount, unsigned int songtracks);
static void parametertweak(Mixer*, int param, int value);
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

static char* _psName;

static MixerChannel* mixerchannel_allocinit(unsigned int inputslot);
static void mixerchannel_dispose(MixerChannel*);

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
	self->machine.parametertweak = parametertweak;
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
	table_dispose(&self->returns);
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
	Buffer* master;
	TableIterator input_iter;
	int input = 0;

	master = machines_outputs(machines, slot);
	if (!master) {
		return 0;
	}
	buffer_clearsamples(master, amount);		
	for (input_iter = table_begin(&self->inputs);
			!tableiterator_equal(&input_iter, table_end());
			tableiterator_inc(&input_iter), ++input) {
		TableIterator send_iter;
		MixerChannel* inputchannel;
		Buffer* source;
		WireSocketEntry* input_entry;
		int sendindex = 0;
		float wirevol = 1.f;

		inputchannel = (MixerChannel*) tableiterator_value(&input_iter);
		source = machines_outputs(machines, inputchannel->inputslot);
		input_entry = wiresocketentry(self, input);				
		if (input_entry) {
			wirevol = input_entry->volume;	
		}
		for (send_iter = table_begin(&self->sends);
				!tableiterator_equal(&send_iter, table_end());
				tableiterator_inc(&send_iter), ++sendindex) {
			Buffer* dst;
			int dstslot;
			int sendvol;

			dstslot = (int)tableiterator_value(&send_iter);
			dst = machines_outputs(machines, dstslot);
			buffer_clearsamples(dst, amount);
			sendvol = (int) table_at(&inputchannel->sendvols, sendindex);
			addsamples(dst, source, amount, sendvol / 65535.f * wirevol);
			work_send(self, dstslot, machines_at(machines, dstslot),
				dst, amount, 16);
			addsamples(master, dst, amount, 1.0f);
		}
		addsamples(master, source, amount, inputchannel->volume * 
			inputchannel->drymix * wirevol);
	}
	rmsvol_tick(&self->masterrmsvol, master->samples[0], master->samples[1],
							amount);
	dsp_mul(master->samples[0], amount, self->master.volume);
	dsp_mul(master->samples[1], amount, self->master.volume);
	return master;
}

void work_send(Mixer* self, int slot, Machine* machine, Buffer* output, 
		unsigned int amount, unsigned int songtracks) {
	BufferContext bc;										
	List* events;	

	events = 0;
	buffercontext_init(&bc, events, output, output, amount,
		songtracks, 0);
	machine->work(machine, &bc);
	buffer_pan(output, machine_panning(machine), amount);	
	signal_emit(&machine->signal_worked, machine, 2, slot, &bc);	
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
				dsp_erase_all_nans_infinities_and_denormals(
					dst->samples[channel], numsamples);					
		}
	}
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
				table_insert(&self->returns, self->returns.count, (void*)outputslot);
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
			int c = 0;

			for (it = table_begin(&self->sends);
				!tableiterator_equal(&it, table_end()); tableiterator_inc(&it), ++c) {				
				int sendslot;

				sendslot = (int)tableiterator_value(&it);
				if (sendslot == outputslot) {
					table_remove(&self->sends, c);
					table_remove(&self->returns, c);
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

void parametertweak(Mixer* self, int param, int value)
{	
	int col;
	int row;
	int rows;

	rows = numparameters(self) / numcols(self);
	row = param % rows;
	col = param / rows;

	if (col == 0) {	// MASTER COLUMN
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
		if (row == self->sends.count + 4) {
			self->master.volume = floatparamvalue(value) * 
					floatparamvalue(value) * 4.f;			
		}
	} else 
	if (col < self->inputs.count + 1) {
		MixerChannel* channel;
			
		channel = (MixerChannel*) table_at(&self->inputs, col - 1);
		if (channel) {
			if (row == self->sends.count + 1) {
				channel->drymix = value / 65535.f;
			} else
			if (row > 0 && row < self->sends.count + 1) {						
				table_insert(&channel->sendvols, row - 1, (void*)value);
			} else
			if (row == self->sends.count + 2) {
				WireSocketEntry* input_entry;

				input_entry = wiresocketentry(self, col - 1);
				if (input_entry) {						
					input_entry->volume = floatparamvalue(value) * 
						floatparamvalue(value) * 4.f;
				}
			} else				
			if (row == self->sends.count + 3) {
				channel->panning = floatparamvalue(value); 
			} else
			if (row == self->sends.count + 4) {
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

	if (col == 0) {	// MASTER COLUMN
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
		if (row == self->sends.count + 4) {
			return intparamvalue(
				(float)sqrt(self->master.volume) * 0.5f);
		} else
		if (row == self->sends.count + 5) {
			return (int)(rmsvol_value(&self->masterrmsvol) / 32768.f  * 65535);
		}
	} else 
	if (col < self->inputs.count + 1) {
		MixerChannel* channel;
	
		channel = (MixerChannel*) table_at(&self->inputs, col - 1);
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
				return intparamvalue(
					(float)sqrt(channel->volume) * 0.5f);
			}
		}
	}
	/*
	if (param == 0) {
		return (int)(self->mastervol * 65535);
	} else
	if (param == 1) {				
		return (int)(rmsvol_value(&self->masterrmsvol) / 32768.f  * 65535);
		return 0;
	} */
	return 0;
}

int describevalue(Mixer* self, char* txt, int const param, int const value)
{ 	
	int col;
	int row;
	int rows;

	rows = numparameters(self) / numcols(self);
	row = param % rows;
	col = param / rows;

	if (col == 0) {	// MASTER COLUMN
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
		if (row == self->sends.count + 4) {
			float db;

			db = (amp_t)(20 * log10(self->master.volume));
			_snprintf(txt, 10, "%.2f dB", db);
			return 1;
		}
	} else
	if (col < self->inputs.count + 1) {
		MixerChannel* channel;
	
		channel = (MixerChannel*) table_at(&self->inputs, col - 1);
		if (channel) {
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
			if (row == self->sends.count + 4) {
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
	int cols;

	cols = numcols(self);	
	return cols * (6  + self->sends.count + self->sends.count);
}

unsigned int numcols(Mixer* self)
{
	return self->inputs.count + self->returns.count + 1;
}

const CMachineParameter* parameter(Mixer* self, unsigned int param)
{			
	int col;
	int row;
	int rows;

	rows = numparameters(self) / numcols(self);
	row = param % rows;
	col = param / rows;
	if (col == 0) {
		if (row == 0) {
			return &paraMasterHeader;
		} else
		if (row > 0 && row < self->sends.count + 1) {
			return &paraSendHeader;
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
			return &paraLevelKnob;
		} else
		if (row == self->sends.count + 5) {
			return &paraVuInfo;
		}
	} else
	if (col < self->inputs.count + 1) {
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
			return &paraLevelKnob;
		} else
		if (row == self->sends.count + 5) {
			return &paraVuInfo;
		}
	} else {
		if (row == 0) {
			return &paraReturnHeader;
		} else 		
		if (row == self->sends.count + 3) {
			return &paraPanKnob;		
		} else
		if (row == self->sends.count + 4) {
			return &paraLevelKnob;
		} else
		if (row == self->sends.count + 5) {
			return &paraVuInfo;
		} else {
			return &paraDummy;
		}
	}
	return &paraDummy;
}

int intparamvalue(float value)
{	
	return (int)((value * 65535.f));	
}

float floatparamvalue(int value)
{
	return value / 65535.f;	
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
		int j;

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
			table_insert(&self->returns, i, (void*)inputmachine);
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
			// Return(i).SendsTo(j,send);
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
