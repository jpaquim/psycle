// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "machine.h"
#include "pattern.h"


static void work(Machine* self, List* events, int numsamples, int tracks);
static void generateaudio(Machine* self, Buffer* input, Buffer* output, int numsamples, int tracks) { }
static int hostevent(Machine* self, int const eventNr, int const val1, float const val2) { return 0; }
static void seqtick(Machine* self, int channel, const PatternEvent* event) { }
static void sequencertick(Machine* self) { }
static CMachineInfo* info(Machine* self) { return 0; }
static void parametertweak(Machine* self, int par, int val) { }
static int describevalue(Machine* self, char* txt, int const param, int const value) { return 0; }
static int value(Machine* self, int const param) { return 0; }
static void setvalue(Machine* self, int const param, int const value) { }
static void dispose(Machine* self);
static int mode(Machine* self) { return MACHMODE_FX; }

static int master_mode(Machine* self) { return MACHMODE_MASTER; }
static void master_dispose(Master* self);

void machine_init(Machine* self)
{	
	memset(self, 0, sizeof(Machine));
	self->dispose = machine_dispose;
	self->work = work;
	self->mode = mode;
	self->hostevent = hostevent;
	self->seqtick = seqtick;
	self->sequencertick = sequencertick;
	self->info = info;
	self->parametertweak = parametertweak;
	self->describevalue = describevalue;
	self->setvalue = setvalue;
	self->value = value;
	self->generateaudio = generateaudio;

	signal_init(&self->signal_worked);
}

void machine_dispose(Machine* self)
{
	signal_dispose(&self->signal_worked);
}

void work(Machine* self, List* events, int numsamples, int tracks)
{			
	List* p = events;
	unsigned int amount = numsamples;
	unsigned int pos = 0;
	while (p) {					
		int numworksamples;
		PatternEntry* entry = (PatternEntry*)p->entry;		
		numworksamples = (unsigned int)entry->delta - pos;		
		if (numworksamples > 0) {
			if (self->generateaudio) {
				self->inputs.pos = pos;
				self->outputs.pos = pos;
				self->generateaudio(self, &self->inputs, &self->outputs,
					numworksamples, tracks);				
			}			
			amount -= numworksamples;
		}
		self->seqtick(self, entry->track, &entry->event);		
		pos = (unsigned int)entry->delta;
		p = p->next;
	}
	if (amount > 0 && self->generateaudio) {
		self->inputs.pos = pos;
		self->outputs.pos = pos;
		self->generateaudio(self, &self->inputs, &self->outputs, amount, tracks);		
	}
	self->inputs.pos = 0;
	self->outputs.pos = 0;
}

int machine_supports(Machine* self, int option)
{
	if (self->info(self)) {
		return (self->info(self)->Flags & option) == option;
	}
	return 0;
}

void master_init(Master* self)
{
	memset(self, 0, sizeof(Master));
	machine_init(&self->machine);	
	self->machine.mode = master_mode;
	self->machine.dispose = master_dispose;
	buffer_init(&self->machine.inputs, 2);
	buffer_init(&self->machine.outputs, 2);	
}

void master_dispose(Master* self)
{	
	buffer_dispose(&self->machine.inputs);
	buffer_dispose(&self->machine.outputs);	
	machine_dispose(&self->machine);
}



