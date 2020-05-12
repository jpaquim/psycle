// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "oscilloscope.h"

#include <exclusivelock.h>
#include <operations.h>

#include <songio.h>
#include <math.h>
#include <string.h>

#include "../../detail/portable.h"

#define TIMERID_MASTERVU 400
#define SCOPE_SPEC_BANDS 256
#define SCOPE_BUF_SIZE_LOG 13
#define SCOPE_BUF_SIZE 1 << SCOPE_BUF_SIZE_LOG

static const int SCOPE_BARS_WIDTH = 256 / SCOPE_SPEC_BANDS;
static const uint32_t CLBARDC = 0x1010DC;
static const uint32_t CLBARPEAK = 0xC0C0C0;
static const uint32_t CLLEFT = 0xC06060;
static const uint32_t CLRIGHT = 0x60C060;
static const uint32_t CLBOTH = 0xC0C060;

static void oscilloscope_init_memory(Oscilloscope*);
static void oscilloscope_ondestroy(Oscilloscope*);
static void oscilloscope_deallocate_holdbuffer(Oscilloscope*);
static void oscilloscope_ondraw(Oscilloscope*, psy_ui_Component* sender, psy_ui_Graphics*);
static void oscilloscope_ontimer(Oscilloscope*, psy_ui_Component* sender, int timerid);
static void oscilloscope_onsrcmachineworked(Oscilloscope*, psy_audio_Machine*, unsigned int slot, psy_audio_BufferContext*);
static void oscilloscope_onsongchanged(Oscilloscope*, Workspace*, int flag, psy_audio_SongFile*);
static void oscilloscope_connectmachinessignals(Oscilloscope*, Workspace*);
static void oscilloscope_disconnectmachinessignals(Oscilloscope*, Workspace*);
static psy_audio_Buffer* oscilloscope_buffer(Oscilloscope*, uintptr_t* numsamples);

void oscilloscope_init(Oscilloscope* self, psy_ui_Component* parent, psy_audio_Wire wire,
	Workspace* workspace)
{					
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_doublebuffer(&self->component);
	self->wire = wire;
	self->invol = (psy_dsp_amp_t)1.0f;
	self->mult = 1.0f;
	self->scope_peak_rate = 20;
	self->scope_spec_samples = SCOPE_BUF_SIZE;
	self->scope_spec_begin = 0; // (hold mode scroll)
	self->hold = FALSE;
	self->peakL = self->peakR = (psy_dsp_amp_t) 128.0f;
	self->peakLifeL = self->peakLifeR = 0;
	self->workspace = workspace;
	self->hold_buffer = 0;
	psy_signal_connect(&self->component.signal_destroy, self, oscilloscope_ondestroy);
	psy_signal_connect(&self->component.signal_draw, self, oscilloscope_ondraw);	
	psy_signal_connect(&self->component.signal_timer, self, oscilloscope_ontimer);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		oscilloscope_onsongchanged);
	oscilloscope_connectmachinessignals(self, workspace);
	oscilloscope_init_memory(self);
	psy_ui_component_starttimer(&self->component, TIMERID_MASTERVU, 50);
}

void oscilloscope_init_memory(Oscilloscope* self)
{
	psy_audio_Machine* machine;
	psy_audio_Buffer* memory;

	machine = machines_at(&self->workspace->song->machines, self->wire.src);
	if (machine) {
		memory = psy_audio_machine_buffermemory(machine);
		if (memory) {
			psy_audio_exclusivelock_enter();
			psy_audio_machine_setbuffermemorysize(machine, SCOPE_BUF_SIZE);
			psy_audio_exclusivelock_leave();
		}
	}
}

void oscilloscope_ondestroy(Oscilloscope* self)
{
	psy_audio_Machine* machine;
	psy_audio_Buffer* memory;

	psy_signal_disconnect(&self->workspace->signal_songchanged, self,
		oscilloscope_onsongchanged);
	oscilloscope_disconnectmachinessignals(self, self->workspace);
	oscilloscope_deallocate_holdbuffer(self);
	if (self->workspace->song) {
		machine = machines_at(&self->workspace->song->machines, self->wire.src);
		if (machine) {
			memory = psy_audio_machine_buffermemory(machine);
			if (memory) {
				psy_audio_exclusivelock_enter();
				psy_audio_machine_setbuffermemorysize(machine, MAX_STREAM_SIZE);
				psy_audio_exclusivelock_leave();
			}
		}
	}
}

void oscilloscope_deallocate_holdbuffer(Oscilloscope* self)
{
	if (self->hold_buffer) {
		uintptr_t channel;

		for (channel = 0; channel < psy_audio_buffer_numchannels(self->hold_buffer);
				++channel) {
			dsp.memory_dealloc(self->hold_buffer->samples[channel]);
		}
		psy_audio_buffer_dispose(self->hold_buffer);
		free(self->hold_buffer);
		self->hold_buffer = NULL;
	}
}

void oscilloscope_ondraw(Oscilloscope* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{
	psy_ui_Size size;
	int centery;
	psy_audio_Buffer* buffer;
	uintptr_t numsamples;
	bool active = FALSE;
	
	size = psy_ui_component_size(&self->component);
	centery = size.height / 2;
	buffer = oscilloscope_buffer(self, &numsamples);
	if (buffer && numsamples > 0) {		
		bool zero = FALSE;
		
		if (!zero) {			
			uintptr_t readpos;
			uintptr_t i;
			float px;
			float py;
			int x1, y1, x2, y2;			
			uintptr_t frame;
			intptr_t height2;
			
			active = TRUE;
			px = size.width / (float)self->scope_spec_samples;
			py = size.height * psy_audio_buffer_rangefactor(buffer,
				PSY_DSP_AMP_RANGE_VST) / 3 * self->invol;
			height2 = size.height / 2;
			readpos = buffer->writepos;
			if (readpos >= self->scope_spec_begin) {
				readpos -= self->scope_spec_begin;
			} else {
				readpos = numsamples - 1 - (self->scope_spec_begin - readpos);
			}
			if (readpos >= self->scope_spec_samples) {
				frame = readpos - self->scope_spec_samples;
			} else {
				frame = numsamples - 1 - (self->scope_spec_samples - readpos);
			}
			frame = min(frame, numsamples - 1);
			x1 = x2 = 0;
			y1 = y2 = (int)(buffer->samples[0][frame] * py);			
			if (y2 > height2) {
				y2 = height2;
			} else
			if (y2 < -height2) {
				y2 = -height2;
			}
			for (i = 1; i < self->scope_spec_samples; ++i) {
				x1 = x2;
				x2 = (int)(i * px);
				if (x1 != x2) {
					y1 = y2;
					y2 = (int)(buffer->samples[0][frame] * py);
					if (y2 > height2 / 2) {
						y2 = height2 / 2;
					} else
					if (y2 < -height2 / 2) {
						y2 = -height2 / 2;
					}
					psy_ui_drawline(g, x1, centery + y1, x2, centery + y2);
				}
				frame++;
				if (frame >= numsamples) {
					frame = 0;
				}
			}
		} 		
	}
	if (!active) {
		psy_ui_drawline(g, 0, centery, size.width, centery);
	}
}

psy_audio_Buffer* oscilloscope_buffer(Oscilloscope* self, uintptr_t* numsamples)
{
	psy_audio_Machine* machine;
	psy_audio_Buffer* buffer;

	machine = machines_at(&self->workspace->song->machines, self->wire.src);
	if (!machine) {
		*numsamples = 0;
		return NULL;
	}
	if (self->hold == FALSE) {
		buffer = psy_audio_machine_buffermemory(machine);
		*numsamples = psy_audio_machine_buffermemorysize(machine);
	} else {
		buffer = self->hold_buffer;
		*numsamples = self->hold_buffer_numsamples;
	}
	return buffer;
}

void oscilloscope_ontimer(Oscilloscope* self, psy_ui_Component* sender, int timerid)
{	
	if (timerid == TIMERID_MASTERVU) {
		psy_ui_component_invalidate(&self->component);
	}
}

void oscilloscope_onsrcmachineworked(Oscilloscope* self,
	psy_audio_Machine* machine, unsigned int slot,
	psy_audio_BufferContext* bc)
{	
	if (self->hold_buffer == NULL && self->hold == TRUE) {
		psy_audio_Buffer* memory;
		uintptr_t channel;

		memory = psy_audio_machine_buffermemory(machine);		
		if (!memory) {
			return;
		}

		self->hold_buffer_numsamples = psy_audio_machine_buffermemorysize(machine);
		self->hold_buffer = psy_audio_buffer_allocinit(psy_audio_buffer_numchannels(memory));
		for (channel = 0; channel < psy_audio_buffer_numchannels(memory); ++channel) {
			self->hold_buffer->samples[channel] = dsp.memory_alloc(
				self->hold_buffer_numsamples, sizeof(psy_dsp_amp_t));			
		}
		psy_audio_buffer_clearsamples(self->hold_buffer,
			self->hold_buffer_numsamples);
		psy_audio_buffer_addsamples(self->hold_buffer, memory,
			self->hold_buffer_numsamples, 1.0f);
		self->hold_buffer->writepos = memory->writepos;		
	}
	self->invol = connections_wirevolume(
		&self->workspace->song->machines.connections,
		self->wire.src, self->wire.dst);
}


void oscilloscope_onsongchanged(Oscilloscope* self, Workspace* workspace,
	int flag, psy_audio_SongFile* songfile)
{	
	self->invol = 1.f;
	oscilloscope_connectmachinessignals(self, workspace);	
}

void oscilloscope_connectmachinessignals(Oscilloscope* self, Workspace* workspace)
{
	if (workspace->song) {
		psy_audio_Machine* srcmachine;

		srcmachine = machines_at(&workspace->song->machines, self->wire.src);
		if (srcmachine) {
			psy_signal_connect(&srcmachine->signal_worked, self,
				oscilloscope_onsrcmachineworked);
		}
	}
}

void oscilloscope_disconnectmachinessignals(Oscilloscope* self, Workspace* workspace)
{
	if (workspace->song) {
		psy_audio_Machine* srcmachine;

		srcmachine = machines_at(&workspace->song->machines, self->wire.src);
		if (srcmachine) {
			psy_signal_disconnect(&srcmachine->signal_worked, self,
				oscilloscope_onsrcmachineworked);
		}
	}
}

void oscilloscope_stop(Oscilloscope* self)
{
	if (self->workspace && self->workspace->song) {
		psy_audio_Machine* srcmachine;		
		srcmachine = machines_at(&self->workspace->song->machines,
			self->wire.src);
		if (srcmachine) {
			psy_audio_exclusivelock_enter();
			psy_signal_disconnect(&srcmachine->signal_worked, self,
				oscilloscope_onsrcmachineworked);
			psy_audio_exclusivelock_leave();			
		}
	}
}

void oscilloscope_setzoom(Oscilloscope* self, float rate)
{
	self->scope_spec_samples = (uintptr_t)((float)(SCOPE_BUF_SIZE) * rate);
}

void oscilloscope_setspecbegin(Oscilloscope* self, float begin)
{
	if (self->scope_spec_samples > 0 && self->scope_spec_samples < SCOPE_BUF_SIZE) {		
		self->scope_spec_begin = (uintptr_t)(begin * self->scope_spec_samples);
	} else {
		self->scope_spec_begin = 0;
	}
}

void oscilloscope_hold(Oscilloscope* self)
{
	self->scope_spec_begin = 0;
	self->hold = TRUE;
}

void oscilloscope_continue(Oscilloscope* self)
{
	self->hold = FALSE;
	oscilloscope_deallocate_holdbuffer(self);
}

bool oscilloscope_stopped(Oscilloscope* self)
{
	return self->hold;
}

