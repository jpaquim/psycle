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
static void oscilloscope_ondraw(Oscilloscope*, psy_ui_Graphics*);
static void oscilloscope_ontimer(Oscilloscope*, psy_ui_Component* sender, uintptr_t timerid);
static void oscilloscope_onsrcmachineworked(Oscilloscope*, psy_audio_Machine*,
	uintptr_t slot, psy_audio_BufferContext*);
static void oscilloscope_onsongchanged(Oscilloscope*, Workspace*, int flag, psy_audio_SongFile*);
static void oscilloscope_connectmachinessignals(Oscilloscope*, Workspace*);
static void oscilloscope_disconnectmachinessignals(Oscilloscope*, Workspace*);
static psy_audio_Buffer* oscilloscope_buffer(Oscilloscope*, uintptr_t* numsamples);
static uintptr_t oscilloscope_channel(Oscilloscope*);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(Oscilloscope* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw = (psy_ui_fp_ondraw)oscilloscope_ondraw;
		vtable_initialized = 1;
	}
}

void oscilloscope_init(Oscilloscope* self, psy_ui_Component* parent, psy_audio_Wire wire,
	Workspace* workspace)
{					
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_doublebuffer(&self->component);
	self->wire = wire;
	self->invol = (psy_dsp_amp_t)1.0f;
	self->mult = 1.0f;
	self->scope_peak_rate = 20;
	self->scope_view_samples = SCOPE_BUF_SIZE;
	self->scope_begin = 0; // (hold mode scroll)
	self->hold = FALSE;
	self->peakL = self->peakR = (psy_dsp_amp_t) 128.0f;
	self->peakLifeL = self->peakLifeR = 0;
	self->workspace = workspace;
	self->hold_buffer = 0;
	self->channelmode = OSCILLOSCOPE_CHMODE_LEFT;
	self->ampzoom = (psy_dsp_amp_t)1.0f;
	self->running = TRUE;
	psy_signal_connect(&self->component.signal_destroy, self,
		oscilloscope_ondestroy);	
	psy_signal_connect(&self->component.signal_timer, self,
		oscilloscope_ontimer);	
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

	machine = psy_audio_machines_at(&self->workspace->song->machines, self->wire.src);
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
		machine = psy_audio_machines_at(&self->workspace->song->machines, self->wire.src);
		if (machine) {
			memory = psy_audio_machine_buffermemory(machine);
			if (memory) {
				psy_audio_exclusivelock_enter();
				psy_audio_machine_setbuffermemorysize(machine,
					psy_audio_MAX_STREAM_SIZE);
				psy_audio_exclusivelock_leave();
			}
		}
	}
}

void oscilloscope_deallocate_holdbuffer(Oscilloscope* self)
{
	if (self->hold_buffer) {
		uintptr_t channel;

		for (channel = 0; channel <
				psy_audio_buffer_numchannels(self->hold_buffer);
				++channel) {
			dsp.memory_dealloc(self->hold_buffer->samples[channel]);
		}
		psy_audio_buffer_dispose(self->hold_buffer);
		free(self->hold_buffer);
		self->hold_buffer = NULL;
	}
}

void oscilloscope_ondraw(Oscilloscope* self, psy_ui_Graphics* g)
{
	if (self->running) {
		psy_ui_Size size;
		psy_ui_TextMetric tm;
		int centery;
		psy_audio_Buffer* buffer;
		uintptr_t numsamples;
		bool active = FALSE;

		size = psy_ui_component_size(&self->component);
		tm = psy_ui_component_textmetric(&self->component);
		centery = psy_ui_value_px(&size.height, &tm) / 2;
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
				uintptr_t channel;

				channel = oscilloscope_channel(self);
				active = TRUE;
				px = psy_ui_value_px(&size.width, &tm) / (float)self->scope_view_samples;
				py = psy_ui_value_px(&size.height, &tm) * psy_audio_buffer_rangefactor(buffer,
					PSY_DSP_AMP_RANGE_VST) / 3 * self->invol * self->ampzoom;
				readpos = buffer->writepos;
				if (readpos >= self->scope_begin) {
					readpos -= self->scope_begin;
				} else {
					readpos = numsamples - 1 - (self->scope_begin - readpos);
				}
				if (readpos >= self->scope_view_samples) {
					frame = readpos - self->scope_view_samples;
				} else {
					frame = numsamples - 1 - (self->scope_view_samples - readpos);
				}
				frame = min(frame, numsamples - 1);
				x1 = x2 = 0;
				y1 = y2 = (int)(buffer->samples[channel][frame] * py);
				for (i = 1; i < self->scope_view_samples; ++i) {
					x1 = x2;
					x2 = (int)(i * px);
					if (x1 != x2) {
						y1 = y2;
						y2 = (int)(buffer->samples[channel][frame] * py);
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
			psy_ui_drawline(g, 0, centery, psy_ui_value_px(&size.width, &tm), centery);
		}
	}
}

psy_audio_Buffer* oscilloscope_buffer(Oscilloscope* self,
	uintptr_t* numsamples)
{
	psy_audio_Machine* machine;
	psy_audio_Buffer* buffer;

	machine = psy_audio_machines_at(&self->workspace->song->machines, self->wire.src);
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

void oscilloscope_ontimer(Oscilloscope* self, psy_ui_Component* sender,
	uintptr_t timerid)
{	
	if (timerid == TIMERID_MASTERVU) {
		psy_ui_component_invalidate(&self->component);
	}
}

void oscilloscope_onsrcmachineworked(Oscilloscope* self,
	psy_audio_Machine* machine, uintptr_t slot,
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

		srcmachine = psy_audio_machines_at(&workspace->song->machines, self->wire.src);
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

		srcmachine = psy_audio_machines_at(&workspace->song->machines, self->wire.src);
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
		srcmachine = psy_audio_machines_at(&self->workspace->song->machines,
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
	self->scope_view_samples = (uintptr_t)((float)(SCOPE_BUF_SIZE) * rate);
}

void oscilloscope_setbegin(Oscilloscope* self, float begin)
{
	if (self->scope_view_samples > 0 &&
			self->scope_view_samples < SCOPE_BUF_SIZE) {
		self->scope_begin = (uintptr_t)(begin * ((float)(SCOPE_BUF_SIZE) -
			self->scope_view_samples));
	} else {
		self->scope_begin = 0;
	}
}

void oscilloscope_hold(Oscilloscope* self)
{
	self->scope_begin = 0;
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

uintptr_t oscilloscope_channel(Oscilloscope* self)
{
	uintptr_t rv;

	switch (self->channelmode) {
		case OSCILLOSCOPE_CHMODE_LEFT:
			rv = 0;
		break;
		case OSCILLOSCOPE_CHMODE_RIGHT:
			rv = 1;
		break;
		default:
			rv = 0;
		break;
	}	
	return rv;
}

static void oscilloscopecontrols_onchannelselect(OscilloscopeControls*,
	psy_ui_Component* sender);
static void oscilloscopecontrols_updatechannelselect(OscilloscopeControls*);
static void oscilloscopecontrols_onampzoomchanged(OscilloscopeControls*,
	ZoomBox* sender);

void oscilloscopecontrols_init(OscilloscopeControls* self, psy_ui_Component* parent,
	Oscilloscope* oscilloscope)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(0.5), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	self->oscilloscope = oscilloscope;
	psy_ui_label_init(&self->channellbl, &self->component);
	psy_ui_label_settext(&self->channellbl, "Channel");
	psy_ui_component_setmargin(&self->channellbl.component, &margin);
	psy_ui_button_init(&self->channelmode, &self->component);
	psy_ui_button_setcharnumber(&self->channelmode, 6);
	psy_ui_margin_setright(&margin, psy_ui_value_makeew(2.0));
	psy_ui_component_setmargin(&self->channelmode.component, &margin);
	psy_signal_connect(&self->channelmode.signal_clicked, self,
		oscilloscopecontrols_onchannelselect);
	oscilloscopecontrols_updatechannelselect(self);
	psy_ui_label_init(&self->amplbl, &self->component);
	psy_ui_label_settext(&self->amplbl, "Amp");
	psy_ui_margin_setright(&margin, psy_ui_value_makeew(0.5));
	psy_ui_component_setmargin(&self->amplbl.component, &margin);
	zoombox_init(&self->ampzoom, &self->component);
	psy_signal_connect(&self->ampzoom.signal_changed, self,
		oscilloscopecontrols_onampzoomchanged);	
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		NULL));
}

void oscilloscopecontrols_onchannelselect(OscilloscopeControls* self,
	psy_ui_Component* sender)
{	
	if (self->oscilloscope->channelmode == OSCILLOSCOPE_CHMODE_LEFT) {
		self->oscilloscope->channelmode = OSCILLOSCOPE_CHMODE_RIGHT;
	} else {
		self->oscilloscope->channelmode = OSCILLOSCOPE_CHMODE_LEFT;
	}
	oscilloscopecontrols_updatechannelselect(self);
}

void oscilloscopecontrols_updatechannelselect(OscilloscopeControls* self)
{
	if (self->oscilloscope->channelmode == OSCILLOSCOPE_CHMODE_LEFT) {
		psy_ui_button_settext(&self->channelmode, "Left");
	} else
	if (self->oscilloscope->channelmode == OSCILLOSCOPE_CHMODE_RIGHT) {
		psy_ui_button_settext(&self->channelmode, "Right");
	}
	psy_ui_component_align(&self->component);
}

void oscilloscopecontrols_onampzoomchanged(OscilloscopeControls* self, ZoomBox* sender)
{
	self->oscilloscope->ampzoom = (psy_dsp_amp_t)zoombox_rate(sender);
}

void oscilloscopeview_init(OscilloscopeView* self, psy_ui_Component* parent,
	psy_audio_Wire wire, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	oscilloscope_init(&self->oscilloscope, &self->component, wire,
		workspace);
	psy_ui_component_setalign(&self->oscilloscope.component,
		psy_ui_ALIGN_CLIENT);
	oscilloscopecontrols_init(&self->oscilloscopecontrols, &self->component,
		&self->oscilloscope);
	psy_ui_component_setalign(&self->oscilloscopecontrols.component,
		psy_ui_ALIGN_TOP);
}

void oscilloscopeview_start(OscilloscopeView* self)
{
	self->oscilloscope.running = TRUE;
}

void oscilloscopeview_stop(OscilloscopeView* self)
{
	self->oscilloscope.running = FALSE;
}
