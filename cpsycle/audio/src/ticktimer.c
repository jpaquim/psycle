// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "ticktimer.h"
#include "buffercontext.h"

static void psy_audio_ticktimer_dowork(psy_audio_TickTimer*,
	uintptr_t numsamples, struct psy_audio_BufferContext*, uintptr_t offset);

void psy_audio_ticktimer_init(psy_audio_TickTimer* self,
	void* context,
	fp_samplerticktimer_ontick tick,
	fp_samplerticktimer_onwork work)
{	
	assert(self);

	self->context = context;
	self->tick = tick;
	self->work = work;
	self->samplesprotick = 882;
	self->counter = 0;
	self->tickcount = 0;
}

void psy_audio_ticktimer_reset(psy_audio_TickTimer* self,
	uintptr_t samplesprotick)
{
	assert(self);

	self->samplesprotick = samplesprotick;
	self->counter = 0;
	self->tickcount = 0;
}

void psy_audio_ticktimer_update(psy_audio_TickTimer* self,
	uintptr_t numsamples, psy_audio_BufferContext* bc)
{	
	uintptr_t j = 0;
	uintptr_t lastpos = 0;
	uintptr_t amount;

	assert(self);

	amount = numsamples;
	for (; j < numsamples; ++j) {
		if (self->counter == 0) {
			uintptr_t worknum;

			worknum = j - lastpos;
			if (worknum) {
				psy_audio_ticktimer_dowork(self, worknum, bc, lastpos);
				amount -= worknum;
				lastpos = j;
			}
			self->tick(self->context);			
			++self->tickcount;
		}
		if (self->counter >= self->samplesprotick) {
			self->counter = 0;
		} else {
			++self->counter;
		}
	}
	if (amount) {
		psy_audio_ticktimer_dowork(self, amount, bc, lastpos);		
	}
}

void psy_audio_ticktimer_dowork(psy_audio_TickTimer* self,
	uintptr_t amount, psy_audio_BufferContext* bc, uintptr_t offset)
{
	uintptr_t restorenumsamples;
	uintptr_t restoreoffset;

	assert(self);

	restorenumsamples = psy_audio_buffercontext_numsamples(bc);
	restoreoffset = bc->output->offset;
	psy_audio_buffercontext_setnumsamples(bc, amount);
	psy_audio_buffercontext_setoffset(bc, restoreoffset + offset);
	self->work(self->context, bc);
	psy_audio_buffercontext_setnumsamples(bc, restorenumsamples);
	psy_audio_buffercontext_setoffset(bc, restoreoffset);
}
