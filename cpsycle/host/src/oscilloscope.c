// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "oscilloscope.h"
// audio
#include <exclusivelock.h>
#include <songio.h>
// std
#include <math.h>

#define SCOPE_SPEC_BANDS 256
#define SCOPE_BUF_SIZE_LOG 13
#define SCOPE_BUF_SIZE 1 << SCOPE_BUF_SIZE_LOG

static const int SCOPE_BARS_WIDTH = 256 / SCOPE_SPEC_BANDS;
static const uint32_t CLBARDC = 0x1010DC;
static const uint32_t CLBARPEAK = 0xC0C0C0;
static const uint32_t CLLEFT = 0xC06060;
static const uint32_t CLRIGHT = 0x60C060;
static const uint32_t CLBOTH = 0xC0C060;

static void oscilloscope_on_destroyed(Oscilloscope*);
static void oscilloscope_init_memory(Oscilloscope*);
static void oscilloscope_ondraw(Oscilloscope*, psy_ui_Graphics*);
static psy_audio_Buffer* oscilloscope_buffer(Oscilloscope*,
	uintptr_t* numsamples);
static uintptr_t oscilloscope_channel(Oscilloscope*);
/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(Oscilloscope* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component_event)
			oscilloscope_on_destroyed;
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			oscilloscope_ondraw;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}
/* implementation */
void oscilloscope_init(Oscilloscope* self, psy_ui_Component* parent, psy_audio_Wire wire,
	Workspace* workspace)
{					
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);	
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
	oscilloscope_init_memory(self);	
}

void oscilloscope_init_memory(Oscilloscope* self)
{
	psy_audio_Machine* machine;
	psy_audio_Buffer* memory;

	machine = psy_audio_machines_at(&workspace_song(self->workspace)->machines, self->wire.src);
	if (machine) {
		memory = psy_audio_machine_buffermemory(machine);
		if (memory) {
			psy_audio_exclusivelock_enter();
			psy_audio_machine_setbuffermemorysize(machine, SCOPE_BUF_SIZE);
			psy_audio_exclusivelock_leave();
		}
	}
}

void oscilloscope_on_destroyed(Oscilloscope* self)
{
	psy_signal_disconnect_context(&self->workspace->signal_songchanged, self);	
	if (self->hold_buffer) {
		psy_audio_buffer_deallocate(self->hold_buffer);
	}
	if (workspace_song(self->workspace)) {
		psy_audio_Machine* machine;

		machine = psy_audio_machines_at(
			&workspace_song(self->workspace)->machines, self->wire.src);
		if (machine && psy_audio_machine_buffermemory(machine)) {			
			psy_audio_exclusivelock_enter();
			psy_audio_machine_resetbuffermemory(machine);			
			psy_audio_exclusivelock_leave();			
		}
	}
}

void oscilloscope_ondraw(Oscilloscope* self, psy_ui_Graphics* g)
{
	if (self->running) {
		psy_ui_RealSize size;		
		double centery;
		psy_audio_Buffer* buffer;
		uintptr_t numsamples;
		bool active = FALSE;

		size = psy_ui_component_size_px(&self->component);
		centery = size.height / 2;
		buffer = oscilloscope_buffer(self, &numsamples);
		if (buffer && numsamples > 0) {
			bool zero = FALSE;

			if (!zero) {
				uintptr_t readpos;
				uintptr_t i;
				double px;
				double py;
				int x1, y1, x2, y2;
				uintptr_t frame;
				uintptr_t channel;

				channel = oscilloscope_channel(self);
				active = TRUE;
				px = size.width / (double)self->scope_view_samples;
				py = size.height * (double)psy_audio_buffer_rangefactor(buffer,
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
				frame = psy_min(frame, numsamples - 1);
				x1 = x2 = 0;
				y1 = y2 = (int)(buffer->samples[channel][frame] * py);
				for (i = 1; i < self->scope_view_samples; ++i) {
					x1 = x2;
					x2 = (int)(i * px);
					if (x1 != x2) {
						y1 = y2;
						y2 = (int)(buffer->samples[channel][frame] * py);
						psy_ui_drawline(g, psy_ui_realpoint_make(x1, centery + y1),
							psy_ui_realpoint_make(x2, centery + y2));
					}
					frame++;
					if (frame >= numsamples) {
						frame = 0;
					}
				}
			}
		}
		if (!active) {
			psy_ui_drawline(g, psy_ui_realpoint_make(0, centery),
				psy_ui_realpoint_make(size.width, centery));
		}
	}
}

psy_audio_Buffer* oscilloscope_buffer(Oscilloscope* self,
	uintptr_t* numsamples)
{
	psy_audio_Machine* machine;
	psy_audio_Buffer* buffer;

	machine = psy_audio_machines_at(&workspace_song(self->workspace)->machines, self->wire.src);
	if (!machine) {
		*numsamples = 0;
		return NULL;
	}
	if (self->hold == FALSE) {
		buffer = psy_audio_machine_buffermemory(machine);
		*numsamples = psy_audio_machine_buffermemorysize(machine);
	} else if (self->hold_buffer) {
		buffer = self->hold_buffer;
		*numsamples = self->hold_buffer->numsamples;
	} else {
		buffer = NULL;
		*numsamples = 0;
	}
	return buffer;
}

void oscilloscope_idle(Oscilloscope* self)
{	
	self->invol = psy_audio_connections_wirevolume(
		&workspace_song(self->workspace)->machines.connections, self->wire);
	psy_ui_component_invalidate(&self->component);
}

void oscilloscope_stop(Oscilloscope* self)
{
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
	psy_audio_buffer_deallocate(self->hold_buffer);
	self->hold_buffer = NULL;
	self->hold = FALSE;
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

	psy_ui_margin_init_em(&margin, 0.0, 0.5, 0.0, 0.0);		
	psy_ui_component_init(&self->component, parent, NULL);
	self->oscilloscope = oscilloscope;
	psy_ui_label_init_text(&self->channellbl, &self->component,
		"Channel");
	psy_ui_component_set_margin(&self->channellbl.component, margin);
	psy_ui_button_init(&self->channelmode, &self->component);
	psy_ui_button_set_char_number(&self->channelmode, 6);
	psy_ui_margin_setright(&margin, psy_ui_value_make_ew(2.0));
	psy_ui_component_set_margin(&self->channelmode.component, margin);
	psy_signal_connect(&self->channelmode.signal_clicked, self,
		oscilloscopecontrols_onchannelselect);
	oscilloscopecontrols_updatechannelselect(self);
	psy_ui_label_init(&self->amplbl, &self->component);
	psy_ui_label_prevent_translation(&self->amplbl);
	psy_ui_label_set_text(&self->amplbl, "Amp");
	psy_ui_margin_setright(&margin, psy_ui_value_make_ew(0.5));
	psy_ui_component_set_margin(&self->amplbl.component, margin);
	zoombox_init(&self->ampzoom, &self->component);
	psy_signal_connect(&self->ampzoom.signal_changed, self,
		oscilloscopecontrols_onampzoomchanged);	
	psy_ui_component_setalign_children(&self->component, psy_ui_ALIGN_LEFT);		
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
		psy_ui_button_set_text(&self->channelmode, "Left");
	} else
	if (self->oscilloscope->channelmode == OSCILLOSCOPE_CHMODE_RIGHT) {
		psy_ui_button_set_text(&self->channelmode, "Right");
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
	psy_ui_component_init(&self->component, parent, NULL);	
	oscilloscope_init(&self->oscilloscope, &self->component, wire,
		workspace);
	psy_ui_component_set_align(&self->oscilloscope.component,
		psy_ui_ALIGN_CLIENT);
	oscilloscopecontrols_init(&self->oscilloscopecontrols, &self->component,
		&self->oscilloscope);
	psy_ui_component_set_align(&self->oscilloscopecontrols.component,
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
