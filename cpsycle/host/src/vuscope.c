// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "vuscope.h"

#include <songio.h>
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

static void vuscope_ondestroy(VuScope*);
static void vuscope_ondraw(VuScope*, psy_ui_Component* sender, psy_ui_Graphics*);
static void vuscope_drawscale(VuScope*, psy_ui_Graphics*);
static void vuscope_drawbars(VuScope*, psy_ui_Graphics*);
static void vuscope_ontimer(VuScope*, psy_ui_Component* sender, int timerid);
static void vuscope_onsrcmachineworked(VuScope*, psy_audio_Machine*, unsigned int slot, psy_audio_BufferContext*);
static void vuscope_onsongchanged(VuScope*, Workspace*,
	int flag, psy_audio_SongFile* songfile);
static void vuscope_connectmachinessignals(VuScope*, Workspace*);
static void vuscope_disconnectmachinessignals(VuScope*, Workspace*);
static psy_dsp_amp_t dB(psy_dsp_amp_t amplitude);

void vuscope_init(VuScope* self, psy_ui_Component* parent, psy_audio_Wire wire,
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
	psy_signal_connect(&self->component.signal_destroy, self, vuscope_ondestroy);
	psy_signal_connect(&self->component.signal_draw, self, vuscope_ondraw);	
	psy_signal_connect(&self->component.signal_timer, self, vuscope_ontimer);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		vuscope_onsongchanged);
	vuscope_connectmachinessignals(self, workspace);
	psy_ui_component_starttimer(&self->component, TIMERID_MASTERVU, 50);
}

void vuscope_ondestroy(VuScope* self)
{
	vuscope_disconnectmachinessignals(self, self->workspace);
}

void vuscope_ondraw(VuScope* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{		
	vuscope_drawscale(self, g);	
	vuscope_drawbars(self, g);
}

void vuscope_drawscale(VuScope* self, psy_ui_Graphics* g)
{	
	char buf[128];	
	int centerx;
	int right;
	int step;
	psy_ui_Size size;
	psy_ui_Rectangle rect;

	size = psy_ui_component_size(&self->component);
	right = size.width;
	centerx = size.width / 2;
	step = size.height / 7;
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolor(g, 0x606060);
	
	rect.left = 32 + 24;
	rect.right = right - 32 - 24;

	rect.top = 2;
	rect.bottom = rect.top + 1;
	sprintf(buf, "Peak");
	psy_ui_textout(g, centerx - 42, rect.top, buf, strlen(buf));
	sprintf(buf, "RMS");
	psy_ui_textout(g, centerx + 25, rect.top, buf, strlen(buf));

	rect.top = 2*step - step;
	rect.bottom = rect.top + 1;
	psy_ui_drawsolidrectangle(g, rect, 0x00606060);
	sprintf(buf, "+6 db");
	psy_ui_textout(g, 32 - 1, rect.top - 6, buf, strlen(buf));
	psy_ui_textout(g, right - 32 - 22, rect.top - 6, buf, strlen(buf));

	rect.top = 2*step + step;
	rect.bottom = rect.top + 1;
	psy_ui_drawsolidrectangle(g, rect, 0x00606060);

	sprintf(buf, "-6 db");
	psy_ui_textout(g, 32 - 1 + 4, rect.top - 6, buf, strlen(buf));
	psy_ui_textout(g, right - 32 - 22, rect.top - 6, buf, strlen(buf));

	rect.top = 4*step;
	rect.bottom = rect.top + 1;
	psy_ui_drawsolidrectangle(g, rect, 0x00606060);
	sprintf(buf, "-12 db");
	psy_ui_textout(g, 32 - 1 - 6 + 4, rect.top - 6, buf, strlen(buf));
	psy_ui_textout(g, right - 32 - 22, rect.top - 6, buf, strlen(buf));

	rect.top = 6*step;
	rect.bottom = rect.top + 1;
	psy_ui_drawsolidrectangle(g, rect, 0x00606060);
	sprintf(buf, "-24 db");
	psy_ui_textout(g, 32 - 1 - 6 + 4, rect.top - 6, buf, strlen(buf));
	psy_ui_textout(g, right - 32 - 22, rect.top - 6, buf, strlen(buf));

	rect.top = 2*step;
	rect.bottom = rect.top + 1;
	psy_ui_settextcolor(g, 0x00707070);
	psy_ui_drawsolidrectangle(g, rect, 0x00707070);
	sprintf(buf, "0 db");
	psy_ui_textout(g, 32 - 1 + 6, rect.top - 6, buf, strlen(buf));
	psy_ui_textout(g, right - 32 - 22, rect.top - 6, buf, strlen(buf));
}

void vuscope_drawbars(VuScope* self, psy_ui_Graphics* g)
{
	float maxL, maxR;
	int rmsL, rmsR;
	const float multleft = self->invol * self->mult * 1.0f; // self->srcMachine._lVol;
	const float multright = self->invol * self->mult * 1.0f; // srcMachine._rVol;
	unsigned int samplerate = 44100;
	unsigned int index = 0;
	int centerx;
	int right;
	int step;
	psy_ui_Size size;
	int scopesamples;
	psy_ui_Rectangle rect;
	char buf[64];
	float* pSamplesL;
	float* pSamplesR;
	psy_audio_Machine* machine;
	psy_audio_Buffer* buffer;

	machine = machines_at(&self->workspace->song->machines, self->wire.src);
	if (!machine) {
		return;
	}

	buffer = psy_audio_machine_buffermemory(machine);
	if (!buffer) {
		return;
	}
	scopesamples = psy_audio_machine_buffermemorysize(machine);
	//process the buffer that corresponds to the lapsed time. Also, force 16 bytes boundaries.
	scopesamples = min(scopesamples, (int)(psy_audio_machine_samplerate(machine) * self->scope_peak_rate * 0.001)) & (~3);
	pSamplesL = buffer->samples[0];
	pSamplesR = buffer->samples[1];

	size = psy_ui_component_size(&self->component);
	right = size.width;
	centerx = size.width / 2;
	step = size.height / 7;
	//process the buffer that corresponds to the lapsed time. Also, force 16 bytes boundaries.
	// scopesamples = min((int)(SCOPE_BUF_SIZE), (int)(samplerate * self->scope_peak_rate * 0.001)) & (~0x3);
	//scopeBufferIndex is the position where it will write new data.
	//int index = srcMachine._scopeBufferIndex & (~0x3); // & ~0x3 to ensure aligned to 16 bytes
	

	/*if (index > 0 && index < scopesamples) {
		
		int remaining = scopesamples - index; //Remaining samples at the end of the buffer.
		maxL = dsp_maxvol(self->pSamplesL + SCOPE_BUF_SIZE - remaining, remaining);
		maxL = fmax(maxL, dsp_maxvol(self->pSamplesL, index));
		maxR = dsp_maxvol(self->pSamplesR + SCOPE_BUF_SIZE - remaining, remaining);
		maxR = fmax(maxR, dsp_getmaxvol(self->pSamplesR, index));
#if PSYCLE__CONFIGURATION__RMS_VUS
		if (srcMachine.Bypass())
#endif
		//{
		//	psycle::helpers::dsp::GetRMSVol(srcMachine.rms, pSamplesL + SCOPE_BUF_SIZE - remaining,
			//	pSamplesR + SCOPE_BUF_SIZE - remaining, remaining);
			//psycle::helpers::dsp::GetRMSVol(srcMachine.rms, pSamplesL, pSamplesR, index);
		//}
	}
	else {*/
		// if (index == 0) { index = SCOPE_BUF_SIZE; }
	//for (index = 0; index < scopesamples; ++index) {
		maxL = dsp.maxvol(pSamplesL + index, scopesamples);
		maxR = dsp.maxvol(pSamplesR + index, scopesamples);
	//}
#if PSYCLE__CONFIGURATION__RMS_VUS
		if (srcMachine.Bypass())
#endif
		{
			//psycle::helpers::dsp::GetRMSVol(srcMachine.rms, pSamplesL + index - scopesamples,
				//pSamplesR + index - scopesamples, scopesamples);
		}

	// }
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
	if (self->peakL < centerx)
	{
		/*CPen* oldpen;
		if (peakL < 36) {
			oldpen = bufDC.SelectObject(&linepenbL);
		}
		else {
			oldpen = bufDC.SelectObject(&linepenL);
		}*/
		//ui_drawline(g, rect.left - 1, self->peakL, 		
			//rect.right - 1, self->peakL);
		// bufDC.SelectObject(oldpen);
	}

	rect.top = (int) maxL;
	rect.bottom = centerx;
	// ui_drawsolidrectangle(g, rect, 0xC08040);

	rect.left = centerx + 6;
	rect.right = rect.left + 24;
	rect.top = rmsL;
	rect.bottom = size.height;
	if (rect.top > rect.bottom) {
		rect.top = rect.bottom;
	}
	psy_ui_drawsolidrectangle(g, rect, 0xC08040);

	// RIGHT CHANNEL 
	rect.left = centerx - 30;
	rect.right = rect.left + 24;
	if (self->peakR < centerx)
	{
		/*CPen* oldpen;
		if (peakR < 36) {
			oldpen = bufDC.SelectObject(&linepenbR);
		}
		else {
			oldpen = bufDC.SelectObject(&linepenR);
		}*/
		//ui_drawline(g, rect.left - 1, self->peakR, rect.right - 1, self->peakR);
		//bufDC.SelectObject(oldpen);
	}

	rect.top = (int) maxR;
	rect.bottom = size.height;
	if (rect.top > rect.bottom) {
		rect.top = rect.bottom;
	}
	//ui_drawsolidrectangle(g, rect, 0x90D040);

	rect.left = centerx + 36;
	rect.right = rect.left + 24;
	rect.top = rmsR;
	rect.bottom = size.height;
	if (rect.top > rect.bottom) {
		rect.top = rect.bottom;
	}
	psy_ui_drawsolidrectangle(g, rect, 0x90D040);


	// update peak counter.
	if (!self->hold)
	{
		if (self->peakLifeL > 0 || self->peakLifeR > 0)
		{
			self->peakLifeL--;
			self->peakLifeR--;
			if (self->peakLifeL <= 0) { self->peakL = (psy_dsp_amp_t) centerx; }
			if (self->peakLifeR <= 0) { self->peakR = (psy_dsp_amp_t) centerx; }
		}
	}	
	psy_snprintf(buf, 64, "Refresh %.2fhz", 1000.0f / self->scope_peak_rate);
	//oldFont = bufDC.SelectObject(&font);
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolor(g, 0x505050);
	psy_ui_textout(g, 4, size.height - 14, buf, strlen(buf));
	// bufDC.SelectObject(oldFont);
}

void vuscope_ontimer(VuScope* self, psy_ui_Component* sender, int timerid)
{	
	if (timerid == TIMERID_MASTERVU) {
		psy_ui_component_invalidate(&self->component);
	}
}

void vuscope_onsrcmachineworked(VuScope* self, psy_audio_Machine* master, unsigned int slot,
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

void vuscope_onsongchanged(VuScope* self, Workspace* workspace, int flag, psy_audio_SongFile* songfile)
{	
	self->leftavg = 0;
	self->rightavg = 0;
	vuscope_connectmachinessignals(self, workspace);	
}

void vuscope_connectmachinessignals(VuScope* self, Workspace* workspace)
{
	if (workspace->song) {
		psy_audio_Machine* srcmachine;

		srcmachine = machines_at(&workspace->song->machines, self->wire.src);
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

		srcmachine = machines_at(&workspace->song->machines, self->wire.src);
		if (srcmachine) {
			psy_signal_disconnect(&srcmachine->signal_worked, self,
				vuscope_onsrcmachineworked);
		}
	}
}

void vuscope_stop(VuScope* self)
{
	if (self->workspace && self->workspace->song) {
		psy_audio_Machine* srcmachine;		
		srcmachine = machines_at(&self->workspace->song->machines,
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
