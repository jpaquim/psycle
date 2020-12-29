// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "stereophase.h"

#include <songio.h>

#include <exclusivelock.h>
#include <operations.h>
#include <quantize.h>
#include <rms.h>

#include <math.h>
#include <string.h>

#include "../../detail/portable.h"

#define SCOPE_SPEC_BANDS 256

static const int SCOPE_BARS_WIDTH = 256 / SCOPE_SPEC_BANDS;
static const uint32_t CLBARDC = 0x1010DC;
static const uint32_t CLBARPEAK = 0xC0C0C0;
static const uint32_t CLLEFT = 0xC06060;
static const uint32_t CLRIGHT = 0x60C060;
static const uint32_t CLBOTH = 0xC0C060;
static const uint32_t linepenbL = 0x705050;
static const uint32_t linepenbR = 0x507050;
static const uint32_t linepenL = 0xc08080;
static const uint32_t linepenR = 0x80c080;

static void stereophase_ondestroy(StereoPhase*);
static void stereophase_ondraw(StereoPhase*, psy_ui_Component* sender, psy_ui_Graphics*);
static void stereophase_drawbackground(StereoPhase*, psy_ui_Graphics*);
static void stereophase_drawphase(StereoPhase*, psy_ui_Graphics*);
static void stereophase_drawscale(StereoPhase*, psy_ui_Graphics*);
static void stereophase_drawbars(StereoPhase*, psy_ui_Graphics*);
static void stereophase_ontimer(StereoPhase*, psy_ui_Component* sender, uintptr_t timerid);
static void stereophase_onsrcmachineworked(StereoPhase*, psy_audio_Machine*, uintptr_t slot, psy_audio_BufferContext*);
static void stereophase_onsongchanged(StereoPhase*, Workspace*, int flag, psy_audio_SongFile*);
static void stereophase_connectmachinessignals(StereoPhase*, Workspace*);
static void stereophase_disconnectmachinessignals(StereoPhase*, Workspace*);
static psy_dsp_amp_t dB(psy_dsp_amp_t amplitude);

void stereophase_init(StereoPhase* self, psy_ui_Component* parent, psy_audio_Wire wire,
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
	self->scope_phase_rate = 20;
	self->hold = 0;
	self->o_mvc = 0.0f;
	self->o_mvpc = 0.0f;
	self->o_mvl = 0.0f;
	self->o_mvdl = 0.0f;
	self->o_mvpl = 0.0f;
	self->o_mvdpl = 0.0f;
	self->o_mvr = 0.0f;
	self->o_mvdr = 0.0f;
	self->o_mvpr = 0.0f;
	self->o_mvdpr = 0.0f;

	self->peakL = self->peakR = (psy_dsp_amp_t) 128.0f;
	self->peakLifeL = self->peakLifeR = 0;
	self->workspace = workspace;
	// self->pSamplesL = dsp.memory_alloc(SCOPE_BUF_SIZE, sizeof(float));
	// self->pSamplesR = dsp.memory_alloc(SCOPE_BUF_SIZE, sizeof(float));
	// dsp.clear(self->pSamplesL, SCOPE_BUF_SIZE);
	// dsp.clear(self->pSamplesR, SCOPE_BUF_SIZE);
	psy_signal_connect(&self->component.signal_destroy, self, stereophase_ondestroy);
	psy_signal_connect(&self->component.signal_draw, self, stereophase_ondraw);	
	psy_signal_connect(&self->component.signal_timer, self, stereophase_ontimer);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		stereophase_onsongchanged);
	stereophase_connectmachinessignals(self, workspace);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void stereophase_ondestroy(StereoPhase* self)
{
	psy_signal_disconnect(&self->workspace->signal_songchanged, self,
		stereophase_onsongchanged);
	stereophase_disconnectmachinessignals(self, self->workspace);
	// dsp.memory_dealloc(self->pSamplesL);
	// dsp.memory_dealloc(self->pSamplesR);
	// self->pSamplesL = 0;
	// self->pSamplesR = 0;
}

void stereophase_ondraw(StereoPhase* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{
	stereophase_drawbackground(self, g);
	stereophase_drawphase(self, g);
}

void stereophase_drawbackground(StereoPhase* self, psy_ui_Graphics* g)
{
	psy_ui_setlinewidth(g, 8);
	psy_ui_setcolour(g, psy_ui_colour_make(0x00303030));

	psy_ui_drawline(g, 32, 32, 128, 128);
	psy_ui_drawline(g, 128, 128, 128, 0);
	psy_ui_drawline(g, 128, 128, 256 - 32, 32);
	
	psy_ui_drawarc(g, 0, 0, 256, 256, 256, 128, 0, 128);
	psy_ui_drawarc(g, 96, 96, 256 - 96, 256 - 96, 256 - 96, 128, 96, 128);

	psy_ui_drawarc(g, 48, 48, 256 - 48, 256 - 48, 256 - 48, 128, 48, 128);

	psy_ui_setlinewidth(g, 4);
	psy_ui_setcolour(g, psy_ui_colour_make(0x00404040));
	psy_ui_drawline(g, 32, 32, 128, 128);
	psy_ui_drawline(g, 128, 128, 128, 0);
	psy_ui_drawline(g, 128, 128, 256 - 32, 32);
	psy_ui_setlinewidth(g, 1);
}

void stereophase_drawphase(StereoPhase* self, psy_ui_Graphics* g)
{
	// ok we need some points:

	// max vol center
	// max vol phase center
	// max vol left
	// max vol dif left
	// max vol phase left
	// max vol dif phase left
	// max vol right
	// max vol dif right
	// max vol phase right
	// max vol dif phase right

	float mvc, mvpc, mvl, mvdl, mvpl, mvdpl, mvr, mvdr, mvpr, mvdpr;
	const float multleft = self->invol * self->mult; //  *self->lsrcMachine._lVol;
	const float multright = self->invol * self->mult; //* srcMachine._rVol;
	uintptr_t sr = psy_audio_player_samplerate(workspace_player(self->workspace));
	char buf[64];
	int x, y;
	float* pSamplesL;
	float* pSamplesR;
	psy_audio_Machine* machine;
	psy_audio_Buffer* buffer;
	intptr_t scopesamples;
	int index = 0;
	float maxval;
	float quarterpi;
	
	mvc = mvpc = mvl = mvdl = mvpl = mvdpl = mvr = mvdr = mvpr = mvdpr = 0.0f;
	machine = psy_audio_machines_at(&workspace_song(self->workspace)->machines, self->wire.src);
	if (!machine) {
		return;
	}

	buffer = psy_audio_machine_buffermemory(machine);
	if (!buffer) {
		return;
	}
	scopesamples = psy_audio_machine_buffermemorysize(machine);
	//process the buffer that corresponds to the lapsed time. Also, force 16 bytes boundaries.
	scopesamples = psy_min(scopesamples, (int)(sr * self->scope_peak_rate * 0.001)) & (~3);
	pSamplesL = buffer->samples[0];
	pSamplesR = buffer->samples[1];
	//scopeBufferIndex is the position where it will write new data.
	index = 0; // (srcMachine._scopeBufferIndex == 0) ? SCOPE_BUF_SIZE - 1 : srcMachine._scopeBufferIndex - 1;
	// for (int i = 0; i < scopesamples; i++, index--, index &= (SCOPE_BUF_SIZE - 1))
	for (index = 0; index < scopesamples; ++index)
	{
		float wl = pSamplesL[index] * multleft;
		float wr = pSamplesR[index] * multright;
		float awl = (float)fabs(wl);
		float awr = (float)fabs(wr);
#if 0
		float mid = wl + wr;
		float side = wl - wr;
		float absmid = awl + awr;
		float absside = awl - awr;
		float abssideabs = fabsf(absside);
		float midabs = fabs(mid);
		float sideabs = fabs(side);
#endif

		if ((wl < 0.0f && wr > 0.0f) || (wl > 0.0f && wr < 0.0f)) {
			// phase difference
			if (awl > awr&& awl - awr > mvdpl) { mvdpl = awl - awr; }
			else if (awl < awr && awr - awl > mvdpr) { mvdpr = awr - awl; }
			if (awl + awr > mvpl) { mvpl = awl + awr; }
			if (awr + awl > mvpr) { mvpr = awr + awl; }
		}
		else if (awl > awr&& awl - awr > mvdl) {
			// left
			mvdl = awl - awr;
		}
		else if (awl < awr && awr - awl > mvdr) {
			// right
			mvdr = awr - awl;
		}
		if (awl > mvl) { mvl = awl; }
		if (awr > mvr) { mvr = awr; }

	}

	// ok we have some data, lets make some points and draw them
	maxval = psy_max(mvl, mvr);
	//Adapt difference range independently of max amplitude.
	if (maxval > 0.0f) {
		mvdl /= maxval;
		mvdr /= maxval;
		mvdpl /= maxval;
		mvdpr /= maxval;
		mvpl /= 2.f;
		mvpr /= 2.f;
	}


	// maintain peaks
	if (mvpl > self->o_mvpl)
	{
		self->o_mvpl = mvpl;
		self->o_mvdpl = mvdpl;
	}
	/*						if (mvpc > o_mvpc)
							{
								o_mvpc = mvpc;
							}
	*/
	if (mvpr > self->o_mvpr)
	{
		self->o_mvpr = mvpr;
		self->o_mvdpr = mvdpr;
	}
	if (mvl > self->o_mvl)
	{
		self->o_mvl = mvl;
		self->o_mvdl = mvdl;
	}
	/*						if (mvc > o_mvc)
							{
								o_mvc = mvc;
							}
	*/
	if (mvr > self->o_mvr)
	{
		self->o_mvr = mvr;
		self->o_mvdr = mvdr;
	}

	psy_ui_setcolour(g, psy_ui_colour_make(0x00FF0000));
	//CPen* oldpen = bufDC.SelectObject(&linepenbL);
	quarterpi = psy_dsp_PI_F * 0.25f;

	x = psy_dsp_fround((float)sin(-quarterpi - (self->o_mvdpl * quarterpi)) * self->o_mvpl * 128.0f) + 128;
	y = psy_dsp_fround(-(float)cos(-quarterpi - (self->o_mvdpl * quarterpi)) * self->o_mvpl * 128.0f) + 128;
	psy_ui_drawline(g, x, y, 128, 128);
	//	bufDC.LineTo(128,128-helpers::math::round<int,float>(o_mvpc*128.0f));
	//	bufDC.MoveTo(128,128);
	x = psy_dsp_fround((float)sin(quarterpi + (self->o_mvdpr * quarterpi)) * self->o_mvpr * 128.0f) + 128;
	y = psy_dsp_fround(-(float)cos(quarterpi + (self->o_mvdpr * quarterpi)) * self->o_mvpr * 128.0f) + 128;
	psy_ui_drawline(g, 128, 128, x, y);

	// panning data
	// bufDC.SelectObject(&linepenbR);

	x = psy_dsp_fround((float)sin(-(self->o_mvdl * quarterpi)) * self->o_mvl * 128.0f) + 128;
	y = psy_dsp_fround(-(float)cos(-(self->o_mvdl * quarterpi)) * self->o_mvl * 128.0f) + 128;
	psy_ui_drawline(g, x, y, 128, 128);

	//						bufDC.LineTo(128,128-helpers::math::round<int,float>(o_mvc*128.0f));
	//						bufDC.MoveTo(128,128);
	x = psy_dsp_fround((float)sin((self->o_mvdr * quarterpi)) * self->o_mvr * 128.0f) + 128;
	y = psy_dsp_fround(-(float)cos((self->o_mvdr * quarterpi)) * self->o_mvr * 128.0f) + 128;
	psy_ui_drawline(g, 128, 128, x, y);

	// bufDC.SelectObject(&linepenL);

	x = psy_dsp_fround((float)sin(-quarterpi - (mvdpl * quarterpi)) * mvpl * 128.0f) + 128;
	y = psy_dsp_fround(-(float)cos(-quarterpi - (mvdpl * quarterpi)) * mvpl * 128.0f) + 128;
	psy_ui_drawline(g, x, y, 128, 128);
	//						bufDC.LineTo(128,128-helpers::math::round<int,float>(mvpc*128.0f));
	//						bufDC.MoveTo(128,128);
	x = psy_dsp_fround((float)sin(quarterpi + (mvdpr * quarterpi)) * mvpr * 128.0f) + 128;
	y = psy_dsp_fround(-(float)cos(quarterpi + (mvdpr * quarterpi)) * mvpr * 128.0f) + 128;
	psy_ui_drawline(g, 128, 128, x, y);

	// panning data
	// bufDC.SelectObject(&linepenR);

	x = psy_dsp_fround((float)sin(-(mvdl * quarterpi)) * mvl * 128.0f) + 128;
	y = psy_dsp_fround(-(float)cos(-(mvdl * quarterpi)) * mvl * 128.0f) + 128;
	psy_ui_drawline(g, x, y, 128, 128);
	//						bufDC.LineTo(128,128-helpers::math::round<int,float>(mvc*128.0f));
	//						bufDC.MoveTo(128,128);
	x = psy_dsp_fround((float)sin((mvdr * quarterpi)) * mvr * 128.0f) + 128;
	y = psy_dsp_fround(-(float)cos((mvdr * quarterpi)) * mvr * 128.0f) + 128;
	psy_ui_drawline(g, 128, 128, x, y);

	if (!self->hold)
	{
		float rate = 2.f / self->scope_phase_rate;//Decay in half a second.
		self->o_mvpl -= rate;
		self->o_mvpc -= rate;
		self->o_mvpr -= rate;
		self->o_mvl -= rate;
		self->o_mvc -= rate;
		self->o_mvr -= rate;
	}
	//bufDC.SelectObject(oldpen);

	sprintf(buf, "Refresh %.2fhz", 1000.0f / self->scope_phase_rate);
	// oldFont = bufDC.SelectObject(&font);
	// bufDC.SetBkMode(TRANSPARENT);
	// bufDC.SetTextColour(0x505050);
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolour(g, psy_ui_colour_make(0x606060));
	psy_ui_textout(g, 4, 128 - 14, buf, strlen(buf));
	// bufDC.TextOut(4, 128 - 14, buf);
	// bufDC.SelectObject(oldFont);
}

void stereophase_ontimer(StereoPhase* self, psy_ui_Component* sender, uintptr_t timerid)
{	
	psy_ui_component_invalidate(&self->component);	
}

void stereophase_onsrcmachineworked(StereoPhase* self, psy_audio_Machine* machine,
	uintptr_t slot, psy_audio_BufferContext* bc)
{	
	if (bc->output->rms) {
		psy_audio_Connections* connections;
		psy_audio_WireSocket* input;	

		connections = &workspace_song(self->workspace)->machines.connections;
		input = psy_audio_connections_input(connections, self->wire);
		if (input) {					
			self->leftavg = bc->output->rms->data.previousLeft / 32768;
			self->rightavg = bc->output->rms->data.previousRight / 32768;
			self->invol = input->volume;			
		}
	}
}

void stereophase_onsongchanged(StereoPhase* self, Workspace* workspace,
	int flag, psy_audio_SongFile* songfile)
{	
	self->leftavg = 0;
	self->rightavg = 0;
	stereophase_connectmachinessignals(self, workspace);	
}

void stereophase_connectmachinessignals(StereoPhase* self, Workspace* workspace)
{
	if (workspace->song) {
		psy_audio_Machine* srcmachine;

		srcmachine = psy_audio_machines_at(&workspace->song->machines, self->wire.src);
		if (srcmachine) {
			psy_signal_connect(&srcmachine->signal_worked, self,
				stereophase_onsrcmachineworked);
		}
	}
}

void stereophase_disconnectmachinessignals(StereoPhase* self, Workspace* workspace)
{
	if (workspace->song) {
		psy_audio_Machine* srcmachine;

		srcmachine = psy_audio_machines_at(&workspace->song->machines, self->wire.src);
		if (srcmachine) {
			psy_signal_disconnect(&srcmachine->signal_worked, self,
				stereophase_onsrcmachineworked);
		}
	}
}

void stereophase_stop(StereoPhase* self)
{
	if (self->workspace && workspace_song(self->workspace)) {
		psy_audio_Machine* srcmachine;		
		srcmachine = psy_audio_machines_at(&workspace_song(self->workspace)->machines,
			self->wire.src);
		if (srcmachine) {
			psy_audio_exclusivelock_enter();
			psy_signal_disconnect(&srcmachine->signal_worked, self,
				stereophase_onsrcmachineworked);
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
