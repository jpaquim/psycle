// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "vuscope.h"

#include <songio.h>

#include <math.h>
#include <rms.h>
#include <exclusivelock.h>
#include <operations.h>
#include <string.h>

#include "../../detail/trace.h"
#include "../../detail/portable.h"

#define SCOPE_SPEC_BANDS 256

static const int SCOPE_BARS_WIDTH = 256 / SCOPE_SPEC_BANDS;
static const uint32_t CLBARDC = 0x001010DC;
static const uint32_t CLBARPEAK = 0x00C0C0C0;
static const uint32_t CLLEFT = 0x00C06060;
static const uint32_t CLRIGHT = 0x0060C060;
static const uint32_t CLBOTH = 0x00C0C060;
static const uint32_t linepenbL = 0x00705050;
static const uint32_t linepenbR = 0x00507050;
static const uint32_t linepenL = 0x00c08080;
static const uint32_t linepenR = 0x0080c080;

static void vuscope_ondestroy(VuScope*);
static void vuscope_ondraw(VuScope*, psy_ui_Component* sender, psy_ui_Graphics*);
static void vuscope_drawscale(VuScope*, psy_ui_Graphics*);
static void vuscope_drawbars(VuScope*, psy_ui_Graphics*);
static void vuscope_ontimer(VuScope*, psy_ui_Component* sender, uintptr_t timerid);
static void vuscope_onsrcmachineworked(VuScope*, psy_audio_Machine*,
	uintptr_t slot, psy_audio_BufferContext*);
static void vuscope_onsongchanged(VuScope*, Workspace*, int flag,
	psy_audio_Song*);
static void vuscope_connectmachinessignals(VuScope*, Workspace*);
static void vuscope_disconnectmachinessignals(VuScope*, Workspace*);
static psy_dsp_amp_t dB(psy_dsp_amp_t amplitude);
static psy_dsp_amp_t vuscope_wirevolume(VuScope*);
static void vuscope_drawlabel(VuScope*, psy_ui_Graphics*, const char* text,
	double x, double y, double width, double height);
static void vuscope_drawlabel_right(VuScope*, psy_ui_Graphics*, const char* text,
	double x, double y, double width, double height);

void vuscope_init(VuScope* self, psy_ui_Component* parent, psy_audio_Wire wire,
	Workspace* workspace)
{					
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_doublebuffer(&self->component);
	self->wire = wire;
	self->workspace = workspace;
	self->leftavg = 0;
	self->rightavg = 0;
	self->invol = 1.0f;
	self->mult = 1.0f;
	self->scope_peak_rate = 20;
	self->hold = 0;
	self->peakL = self->peakR = (psy_dsp_amp_t) INT16_MAX;
	self->peakLifeL = self->peakLifeR = 0;
	self->running = TRUE;
	psy_signal_connect(&self->component.signal_destroy, self, vuscope_ondestroy);
	psy_signal_connect(&self->component.signal_draw, self, vuscope_ondraw);	
	psy_signal_connect(&self->component.signal_timer, self, vuscope_ontimer);
	psy_signal_connect(&workspace->signal_songchanged, self,
		vuscope_onsongchanged);
	vuscope_connectmachinessignals(self, workspace);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void vuscope_ondestroy(VuScope* self)
{
	psy_signal_disconnect(&self->workspace->signal_songchanged, self,
		vuscope_onsongchanged);
	vuscope_disconnectmachinessignals(self, self->workspace);
}

void vuscope_ondraw(VuScope* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{		
	vuscope_drawscale(self, g);
	if (self->running) {
		vuscope_drawbars(self, g);
	}
}

void vuscope_drawscale(VuScope* self, psy_ui_Graphics* g)
{	
	char buf[128];	
	double centerx;
	double right;
	double step;
	psy_ui_Size size;
	psy_ui_RealRectangle rect;	
	const psy_ui_TextMetric* tm;
	double charwidth;

	tm = psy_ui_component_textmetric(&self->component);
	charwidth = tm->tmAveCharWidth * 8;
	size = psy_ui_component_offsetsize(&self->component);
	right = psy_ui_value_px(&size.width, tm);
	centerx = psy_ui_value_px(&size.width, tm) / 2;
	step = psy_ui_value_px(&size.height, tm) / 7;
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolour(g, psy_ui_colour_make(0x606060));

	rect.left = 32 + 24;
	rect.right = right - 32 - 24;
	rect.top = 2;
	rect.bottom = rect.top + 1;
	sprintf(buf, "Peak");
	psy_ui_textout(g, centerx - 42, rect.top, buf, strlen(buf));
	sprintf(buf, "RMS");
	psy_ui_textout(g, centerx + 25, rect.top, buf, strlen(buf));
	
	rect.top = 2 * step;
	rect.bottom = rect.top + 1;
	rect.left = charwidth + tm->tmAveCharWidth;
	rect.right = psy_ui_value_px(&size.width, tm) - charwidth - tm->tmAveCharWidth;
	vuscope_drawlabel_right(self, g, " 0 db", 0, rect.top, charwidth, tm->tmHeight);
	vuscope_drawlabel(self, g, " 0 db", psy_ui_value_px(&size.width, tm) - charwidth, rect.top,
		charwidth, tm->tmHeight);
	psy_ui_drawsolidrectangle(g, rect, psy_ui_colour_make(0x00606060));

	rect.top = 2 * step - step;
	rect.bottom = rect.top + 1;
	vuscope_drawlabel_right(self, g, "+6 db", 0, rect.top, charwidth, tm->tmHeight);
	vuscope_drawlabel(self, g, "+6 db", psy_ui_value_px(&size.width, tm) - charwidth, rect.top,
		charwidth, tm->tmHeight);		
	psy_ui_drawsolidrectangle(g, rect, psy_ui_colour_make(0x00606060));


	rect.top = 2 * step + step;
	rect.bottom = rect.top + 1;
	vuscope_drawlabel_right(self, g, "-6 db", 0, rect.top, charwidth, tm->tmHeight);
	vuscope_drawlabel(self, g, "-6 db", psy_ui_value_px(&size.width, tm) - charwidth, rect.top,
		charwidth, tm->tmHeight);
	psy_ui_drawsolidrectangle(g, rect, psy_ui_colour_make(0x00606060));

	rect.top = 4 * step;
	rect.bottom = rect.top + 1;
	vuscope_drawlabel_right(self, g, "-12 db", 0, rect.top, charwidth, tm->tmHeight);
	vuscope_drawlabel(self, g, "-12 db", psy_ui_value_px(&size.width, tm) - charwidth, rect.top,
		charwidth, tm->tmHeight);
	psy_ui_drawsolidrectangle(g, rect, psy_ui_colour_make(0x00606060));

	rect.top = 6 * step;
	rect.bottom = rect.top + 1;
	vuscope_drawlabel_right(self, g, "-24 db", 0, rect.top, charwidth, tm->tmHeight);
	vuscope_drawlabel(self, g, "-24 db", psy_ui_value_px(&size.width, tm) - charwidth, rect.top,
		charwidth, tm->tmHeight);
	psy_ui_drawsolidrectangle(g, rect, psy_ui_colour_make(0x00606060));

	psy_snprintf(buf, 64, "Refresh %.2fhz", 1000.0f / self->scope_peak_rate);
	//oldFont = bufDC.SelectObject(&font);
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolour(g, psy_ui_colour_make(0x505050));
	psy_ui_textout(g, tm->tmAveCharWidth, psy_ui_value_px(&size.height, tm) - tm->tmHeight, buf, strlen(buf));
}

void vuscope_drawlabel(VuScope* self, psy_ui_Graphics* g, const char* text,
	double x, double y, double width, double height)
{	
	psy_ui_textout(g, x, y - height / 2, text, strlen(text));
}

void vuscope_drawlabel_right(VuScope* self, psy_ui_Graphics* g, const char* text,
	double x, double y, double width, double height)
{
	psy_ui_Size size;
	const psy_ui_TextMetric* tm;

	size = psy_ui_textsize(g, text);
	tm = psy_ui_component_textmetric(&self->component);
	psy_ui_textout(g, x + (width - psy_ui_value_px(&size.width, tm)), y - height / 2,text, strlen(text));
}

void vuscope_drawbars(VuScope* self, psy_ui_Graphics* g)
{
	float maxL, maxR;
	int rmsL, rmsR;
	const float multleft = self->invol * self->mult * 1.0f; // self->srcMachine._lVol;
	const float multright = self->invol * self->mult * 1.0f; // srcMachine._rVol;
	uintptr_t samplerate = 44100;
	unsigned int index = 0;
	double centerx;
	double right;
	double step;
	psy_ui_Size size;
	const psy_ui_TextMetric* tm;
	intptr_t scopesamples;
	psy_ui_RealRectangle rect;
	float* pSamplesL;
	float* pSamplesR;
	psy_audio_Machine* machine;
	psy_audio_Buffer* buffer;

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
	scopesamples = psy_min(scopesamples, (int)(psy_audio_machine_samplerate(machine) *
		self->scope_peak_rate * 0.001)) & (~3);
	pSamplesL = buffer->samples[0];
	pSamplesR = buffer->samples[1];

	size = psy_ui_component_offsetsize(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	right = psy_ui_value_px(&size.width, tm);
	centerx = psy_ui_value_px(&size.width, tm) / 2;
	step = psy_ui_value_px(&size.height, tm) / 7;

	maxL = dsp.maxvol(buffer->samples[0], scopesamples) / 32768.f;
	maxR = dsp.maxvol(buffer->samples[1], scopesamples) / 32768.f;
	
	maxL = ((psy_dsp_amp_t)(2*step) - dB(maxL * multleft + 0.0000001f) * (psy_dsp_amp_t)step / 6.f);	
	maxR = ((psy_dsp_amp_t)(2*step) - dB(maxR * multright + 0.0000001f) * (psy_dsp_amp_t)step/ 6.f);
	rmsL = (int)((psy_dsp_amp_t)(2*step) - dB(self->leftavg * multleft + 0.0000001f) * (psy_dsp_amp_t)step / 6.f);
	rmsR = (int)((psy_dsp_amp_t)(2*step) - dB(self->rightavg * multright + 0.0000001f) * (psy_dsp_amp_t)step / 6.f);

	
	if (maxL < self->peakL) //  it is a cardinal value, so smaller means higher peak.
	{
		if (maxL < 0) maxL = 0;
		self->peakL = maxL;		self->peakLifeL = 2000 / self->scope_peak_rate; //2 seconds
	}

	if (maxR < self->peakR)//  it is a cardinal value, so smaller means higher peak.
	{
		if (maxR < 0) maxR = 0;
		self->peakR = maxR;		self->peakLifeR = 2000 / self->scope_peak_rate; //2 seconds
	}

	// now draw our scope
	// LEFT CHANNEL		
	rect.left = centerx - 60;
	rect.right = rect.left + 24;
	if (self->peakL < 2 * step) {
		psy_ui_setcolour(g, psy_ui_colour_make(linepenbL));
	} else {
		psy_ui_setcolour(g, psy_ui_colour_make(linepenL));
	}
	psy_ui_drawline(g, psy_ui_realpoint_make(rect.left - 1, (int)self->peakL),
		psy_ui_realpoint_make(rect.right - 1, (int)self->peakL));

	rect.top = (int) maxL;
	rect.bottom = centerx;
	rect.bottom = psy_ui_value_px(&size.height, tm);
	if (rect.top > rect.bottom) {
		rect.top = rect.bottom;
	}
	psy_ui_drawsolidrectangle(g, rect, psy_ui_colour_make(0xC08040));

	rect.left = centerx + 6;
	rect.right = rect.left + 24;
	rect.top = rmsL;
	rect.bottom = psy_ui_value_px(&size.height, tm);
	if (rect.top > rect.bottom) {
		rect.top = rect.bottom;
	}
	psy_ui_drawsolidrectangle(g, rect, psy_ui_colour_make(0xC08040));

	// RIGHT CHANNEL 
	rect.left = centerx - 30;
	rect.right = rect.left + 24;	
	if (self->peakR < 2 * step) {
		psy_ui_setcolour(g, psy_ui_colour_make(linepenbR));
	} else {
		psy_ui_setcolour(g, psy_ui_colour_make(linepenR));
	}
	psy_ui_drawline(g, psy_ui_realpoint_make(rect.left - 1, self->peakR),
		psy_ui_realpoint_make(rect.right - 1, self->peakR));

	rect.top = (int) maxR;
	rect.bottom = psy_ui_value_px(&size.height, tm);
	if (rect.top > rect.bottom) {
		rect.top = rect.bottom;
	}
	psy_ui_drawsolidrectangle(g, rect, psy_ui_colour_make(0x90D040));

	rect.left = centerx + 36;
	rect.right = rect.left + 24;
	rect.top = rmsR;
	rect.bottom = psy_ui_value_px(&size.height, tm);
	if (rect.top > rect.bottom) {
		rect.top = rect.bottom;
	}
	psy_ui_drawsolidrectangle(g, rect, psy_ui_colour_make(0x90D040));
	// update peak counter.
	if (!self->hold)
	{
		if (self->peakLifeL > 0 || self->peakLifeR > 0)
		{
			self->peakLifeL--;
			self->peakLifeR--;
			if (self->peakLifeL <= 0) { self->peakL = (psy_dsp_amp_t) INT16_MAX; }
			if (self->peakLifeR <= 0) { self->peakR = (psy_dsp_amp_t) INT16_MAX; }
		}
	}	
	
	// bufDC.SelectObject(oldFont);
}

void vuscope_ontimer(VuScope* self, psy_ui_Component* sender, uintptr_t timerid)
{	
	psy_ui_component_invalidate(&self->component);	
}

void vuscope_onsrcmachineworked(VuScope* self, psy_audio_Machine* master,
	uintptr_t slot, psy_audio_BufferContext* bc)
{	
	if (self->running) {
		psy_audio_Machine* machine;
		psy_audio_Buffer* memory;

		machine = psy_audio_machines_at(&workspace_song(self->workspace)->machines, self->wire.src);
		if (machine) {
			memory = psy_audio_machine_buffermemory(machine);
			if (memory && memory->rms) {
				self->leftavg = memory->rms->data.previousLeft / 32768;
				self->rightavg = memory->rms->data.previousRight / 32768;
				self->invol = vuscope_wirevolume(self);
			}
		}
	}
}

psy_dsp_amp_t vuscope_wirevolume(VuScope* self)
{
	return psy_audio_connections_wirevolume(&workspace_song(self->workspace)->machines.connections,
		self->wire);
}

void vuscope_onsongchanged(VuScope* self, Workspace* workspace, int flag,
	psy_audio_Song* song)
{	
	self->leftavg = 0;
	self->rightavg = 0;
	vuscope_connectmachinessignals(self, workspace);	
}

void vuscope_connectmachinessignals(VuScope* self, Workspace* workspace)
{
	if (workspace->song) {
		psy_audio_Machine* srcmachine;

		srcmachine = psy_audio_machines_at(&workspace->song->machines, self->wire.src);
		if (srcmachine) {
			psy_signal_connect(&srcmachine->signal_worked, self,
				vuscope_onsrcmachineworked);
		}
	}
}

void vuscope_disconnectmachinessignals(VuScope* self, Workspace* workspace)
{
	if (workspace->song) {
		psy_audio_Machine* srcmachine;

		srcmachine = psy_audio_machines_at(&workspace->song->machines, self->wire.src);
		if (srcmachine) {
			psy_signal_disconnect(&srcmachine->signal_worked, self,
				vuscope_onsrcmachineworked);
		}
	}
}

void vuscope_start(VuScope* self)
{
	self->running = TRUE;
}

void vuscope_stop(VuScope* self)
{
	self->running = FALSE;
}

void vuscope_disconnect(VuScope* self)
{
	if (self->workspace && workspace_song(self->workspace)) {
		psy_audio_Machine* srcmachine;
		srcmachine = psy_audio_machines_at(&workspace_song(self->workspace)->machines,
			self->wire.src);
		if (srcmachine) {
			psy_audio_exclusivelock_enter();
			psy_signal_disconnect(&srcmachine->signal_worked, self,
				vuscope_onsrcmachineworked);
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
