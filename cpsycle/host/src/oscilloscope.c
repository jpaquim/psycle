// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "oscilloscope.h"
#include "../../detail/portable.h"
#include <math.h>
#include <rms.h>
#include <exclusivelock.h>
#include <operations.h>
#include <string.h>

#define TIMERID_MASTERVU 400
#define SCOPE_SPEC_BANDS 256

static const int SCOPE_BARS_WIDTH = 256 / SCOPE_SPEC_BANDS;
static const uint32_t CLBARDC = 0x1010DC;
static const uint32_t CLBARPEAK = 0xC0C0C0;
static const uint32_t CLLEFT = 0xC06060;
static const uint32_t CLRIGHT = 0x60C060;
static const uint32_t CLBOTH = 0xC0C060;

static void oscilloscope_ondestroy(Oscilloscope*);
static void oscilloscope_ondraw(Oscilloscope*, psy_ui_Component* sender, psy_ui_Graphics*);
static void oscilloscope_ontimer(Oscilloscope*, psy_ui_Component* sender, int timerid);
static void oscilloscope_onsrcmachineworked(Oscilloscope*, psy_audio_Machine*, unsigned int slot, psy_audio_BufferContext*);
static void oscilloscope_onsongchanged(Oscilloscope*, Workspace*);
static void oscilloscope_connectmachinessignals(Oscilloscope*, Workspace*);
static void oscilloscope_disconnectmachinessignals(Oscilloscope* self, Workspace*);
static psy_dsp_amp_t dB(psy_dsp_amp_t amplitude);

void oscilloscope_init(Oscilloscope* self, psy_ui_Component* parent, psy_audio_Wire wire,
	Workspace* workspace)
{					
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_doublebuffer(&self->component);
	self->wire = wire;
	self->leftavg = 0;
	self->rightavg = 0;
	self->invol = 1.0f;
	self->mult = 1.0f;
	self->scope_peak_rate = 20;
	self->hold = 0;
	self->peakL = self->peakR = (psy_dsp_amp_t) 128.0f;
	self->peakLifeL = self->peakLifeR = 0;
	self->workspace = workspace;
	psy_signal_connect(&self->component.signal_destroy, self, oscilloscope_ondestroy);
	psy_signal_connect(&self->component.signal_draw, self, oscilloscope_ondraw);	
	psy_signal_connect(&self->component.signal_timer, self, oscilloscope_ontimer);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		oscilloscope_onsongchanged);
	oscilloscope_connectmachinessignals(self, workspace);
	psy_ui_component_starttimer(&self->component, TIMERID_MASTERVU, 50);
}

void oscilloscope_ondestroy(Oscilloscope* self)
{
	oscilloscope_disconnectmachinessignals(self, self->workspace);
}

void oscilloscope_ondraw(Oscilloscope* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{
	psy_audio_Machine* machine;
	psy_audio_Buffer* buffer;
	uintptr_t numsamples;
	uintptr_t frame;
	float px;
	float py;
	float cpx = 0;
	int x1, y1, x2, y2;
	static float epsilon = 0.01f;
	int centery;

	machine = machines_at(&self->workspace->song->machines, self->wire.src);
	if (!machine) {
		return;
	}

	buffer = psy_audio_machine_buffermemory(machine);
	if (!buffer) {
		return;
	}

	numsamples = psy_audio_machine_buffermemorysize(machine);
	if (numsamples > 0) {
		int zero = 1;
		psy_ui_Size size;

		size = psy_ui_component_size(&self->component);

		for (frame = 0; frame < numsamples; ++frame) {
			if (fabs(buffer->samples[0][frame]) > epsilon) {
				zero = 0;
				break;
			}
		}
		centery = size.height / 2 + 0;
		if (!zero) {
			px = size.width / (float)numsamples;
			py = size.height / 32768.f / 3;
			x1 = 0;
			y1 = (int)(buffer->samples[0][0] * py);
			x2 = 0;
			y2 = y1;
			for (frame = 0; frame < numsamples; ++frame, cpx += px) {
				x1 = x2;
				x2 = (int)(frame * px);
				if (frame == 0 || x1 != x2) {
					y1 = y2;
					y2 = (int)(buffer->samples[0][frame] * py);
					if (y2 > size.height / 2) {
						y2 = size.height / 2;
					}
					else
						if (y2 < -size.height / 2) {
							y2 = -size.height / 2;
						}
					psy_ui_drawline(g, 0 + x1, centery + y1, 0 + x2,
						centery + y2);
				}
			}
		}
		else {
			psy_ui_drawline(g, 0, centery, 0 + size.width, centery);
		}
	}
}

void oscilloscope_ontimer(Oscilloscope* self, psy_ui_Component* sender, int timerid)
{	
	if (timerid == TIMERID_MASTERVU) {
		psy_ui_component_invalidate(&self->component);
	}
}

void oscilloscope_onsrcmachineworked(Oscilloscope* self, psy_audio_Machine* master, unsigned int slot,
	psy_audio_BufferContext* bc)
{	
	if (bc->rmsvol) {	
		psy_audio_Connections* connections;
		psy_audio_WireSocketEntry* input;	

		connections = &self->workspace->song->machines.connections;
		input = connection_input(connections, self->wire.src, self->wire.dst);
		if (input) {					
			self->leftavg = bc->rmsvol->data.previousLeft / 32768;
			self->rightavg = bc->rmsvol->data.previousRight / 32768;
			self->invol = input->volume;			
		}
	}
}

void oscilloscope_onsongchanged(Oscilloscope* self, Workspace* workspace)
{	
	self->leftavg = 0;
	self->rightavg = 0;
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

/// linear -> deciBell
/// amplitude normalized to 1.0f.
psy_dsp_amp_t dB(psy_dsp_amp_t amplitude)
{
	///\todo merge with psycle::helpers::math::linear_to_deci_bell
	return (psy_dsp_amp_t) (20.0 * log10(amplitude));
}
