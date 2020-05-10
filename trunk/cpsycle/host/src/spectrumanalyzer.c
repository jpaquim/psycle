// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "spectrumanalyzer.h"

#include <songio.h>
#include "../../detail/portable.h"
#include <math.h>
#include <rms.h>
#include <exclusivelock.h>
#include <operations.h>
#include <string.h>

#define TIMERID_MASTERVU 400
#define SCOPE_BUF_SIZE_LOG 13

static const int SCOPE_BARS_WIDTH = 256 / SCOPE_SPEC_BANDS;
static const int SCOPE_BUF_SIZE = 1 << SCOPE_BUF_SIZE_LOG;
static const uint32_t CLBARDC = 0x1010DC;
static const uint32_t CLBARPEAK = 0xC0C0C0;
static const uint32_t CLLEFT = 0xC06060;
static const uint32_t CLRIGHT = 0x60C060;
static const uint32_t CLBOTH = 0xC0C060;

static void spectrumanalyzer_ondestroy(SpectrumAnalyzer*);
static void spectrumanalyzer_ondraw(SpectrumAnalyzer*, psy_ui_Component* sender, psy_ui_Graphics*);
static void spectrumanalyzer_drawbackground(SpectrumAnalyzer*, psy_ui_Graphics*);
static void spectrumanalyzer_drawspectrum(SpectrumAnalyzer*, psy_ui_Graphics*);
static void spectrumanalyzer_ontimer(SpectrumAnalyzer*, psy_ui_Component* sender, int timerid);
static void spectrumanalyzer_onsrcmachineworked(SpectrumAnalyzer*, psy_audio_Machine*,
	unsigned int slot, psy_audio_BufferContext*);
static void spectrumanalyzer_onsongchanged(SpectrumAnalyzer*, Workspace*,
	int flag, psy_audio_SongFile* songfile);
static void spectrumanalyzer_connectmachinessignals(SpectrumAnalyzer*, Workspace*);
static void spectrumanalyzer_disconnectmachinessignals(SpectrumAnalyzer*, Workspace*);
static psy_dsp_amp_t dB(psy_dsp_amp_t amplitude);

void spectrumanalyzer_init(SpectrumAnalyzer* self, psy_ui_Component* parent, psy_audio_Wire wire,
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
	self->scope_spec_mode = 2;
	self->scope_spec_samples = 256;
	self->scope_spec_rate = 20;
	self->peakL = self->peakR = (psy_dsp_amp_t) 128.0f;
	self->peakLifeL = self->peakLifeR = 0;
	self->workspace = workspace;
	psy_signal_connect(&self->component.signal_destroy, self, spectrumanalyzer_ondestroy);
	psy_signal_connect(&self->component.signal_draw, self, spectrumanalyzer_ondraw);	
	psy_signal_connect(&self->component.signal_timer, self, spectrumanalyzer_ontimer);	
	psy_signal_connect(&workspace->signal_songchanged, self,
		spectrumanalyzer_onsongchanged);
	spectrumanalyzer_connectmachinessignals(self, workspace);
	fftclass_init(&self->fftSpec);
	fftclass_setup(&self->fftSpec, hann, self->scope_spec_samples, SCOPE_SPEC_BANDS);
	psy_ui_component_starttimer(&self->component, TIMERID_MASTERVU, 50);
}

void spectrumanalyzer_ondestroy(SpectrumAnalyzer* self)
{
	psy_signal_disconnect(&self->workspace->signal_songchanged, self,
		spectrumanalyzer_onsongchanged);
	spectrumanalyzer_disconnectmachinessignals(self, self->workspace);
	fftclass_dispose(&self->fftSpec);
}

void spectrumanalyzer_ondraw(SpectrumAnalyzer* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{
	spectrumanalyzer_drawbackground(self, g);
	spectrumanalyzer_drawspectrum(self, g);
}

void spectrumanalyzer_drawbackground(SpectrumAnalyzer* self, psy_ui_Graphics* g)
{
	psy_ui_Rectangle rect;
	char buf[64];
	int i;
	float invlog2;
	float thebar;

	for (i = 0; i < SCOPE_SPEC_BANDS; i++)
	{
		self->bar_heights[i] = 256;
	}
	
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolor(g, 0x505050);	
	rect.left = 0;
	rect.right = 256;
	psy_snprintf(buf, sizeof(buf), "db");
	psy_ui_textout(g, 3, 0, buf, strlen(buf));
	psy_ui_textout(g, 256 - 13, 0, buf, strlen(buf));
	for (i = 1; i < 6; i++) {
		rect.top = 20 * i;
		rect.bottom = rect.top + 1;
		psy_ui_drawsolidrectangle(g, rect, 0x00505050);

		psy_snprintf(buf, sizeof(buf), "-%d0", i);
		psy_ui_textout(g, 0, rect.top - 10, buf, strlen(buf));
		psy_ui_textout(g, 256 - 16, rect.top - 10, buf, strlen(buf));
	}
	rect.left = 128;
	rect.right = 256;
	rect.top = 120;
	rect.bottom = rect.top + 1;
	psy_ui_drawsolidrectangle(g, rect, 0x00505050);

	psy_snprintf(buf, sizeof(buf), "-60");
	psy_ui_textout(g, 256 - 16, rect.top - 10, buf, strlen(buf));

	rect.top = 0;
	rect.bottom = 256;
	invlog2 = (float)(1.0 / log10(2.0));
	thebar = 440.f * 2.f * 256.f / psy_audio_player_samplerate(&self->workspace->player);
	if (self->scope_spec_mode == 1) rect.left = (int) thebar;
	else if (self->scope_spec_mode == 2) rect.left = (int)(16 * sqrt(thebar));
	else if (self->scope_spec_mode == 3) rect.left = (int)(32 * log10(1 + thebar) * invlog2);
	rect.right = rect.left + 1;
	psy_ui_drawsolidrectangle(g, rect, 0x00606060);
	psy_snprintf(buf, sizeof(buf), "440");
	psy_ui_textout(g, rect.left, 0, buf, strlen(buf));
	psy_ui_textout(g, rect.left, 128 - 12, buf, strlen(buf));

	thebar = 7000 * 2.f * 256.f / psy_audio_player_samplerate(&self->workspace->player);
	if (self->scope_spec_mode == 1) rect.left = thebar;
	else if (self->scope_spec_mode == 2) rect.left = 16 * sqrt(thebar);
	else if (self->scope_spec_mode == 3) rect.left = 32 * log10(1 + thebar) * invlog2;
	rect.right = rect.left + 1;
	psy_ui_drawsolidrectangle(g, rect, 0x00606060);
	psy_snprintf(buf, sizeof(buf), "7K");
	psy_ui_textout(g, rect.left, 0, buf, strlen(buf));
	psy_ui_textout(g, rect.left, 128 - 12, buf, strlen(buf));

	thebar = 16000 * 2.f * 256.f / psy_audio_player_samplerate(&self->workspace->player);
	if (self->scope_spec_mode == 1) rect.left = thebar;
	else if (self->scope_spec_mode == 2) rect.left = 16 * sqrt(thebar);
	else if (self->scope_spec_mode == 3) rect.left = 32 * log10(1 + thebar) * invlog2;
	rect.right = rect.left + 1;
	psy_ui_drawsolidrectangle(g, rect, 0x00606060);
	sprintf(buf, "16K");
	psy_ui_textout(g, rect.left, 0, buf, strlen(buf));
	psy_ui_textout(g, rect.left, 128 - 12, buf, strlen(buf));
}

void spectrumanalyzer_drawspectrum(SpectrumAnalyzer* self, psy_ui_Graphics* g)
{
	float db_left[SCOPE_SPEC_BANDS];
	float db_right[SCOPE_SPEC_BANDS];
	const float multleft = self->invol * self->mult; // *srcMachine._lVol;
	const float multright = self->invol * self->mult; // *srcMachine._rVol;
	unsigned int scopesamples;
	float* pSamplesL;
	float* pSamplesR;
	psy_audio_Machine* machine;
	psy_audio_Buffer* buffer;
	int DCBar;
	psy_ui_Rectangle rect;
	char buf[64];
	int i;

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

#if 0
	if (FFTMethod == 0)
#endif
	{
		//schism mod FFT
		float temp[256];
		float tempout[256 >> 1];
		// FillLinearFromCircularBuffer(pSamplesL, temp, multleft);
		fftclass_calculatespectrum(&self->fftSpec, temp, tempout);
		fftclass_fillbandsfromfft(&self->fftSpec, tempout, db_left);
	}
#if 0
	else if (FFTMethod == 1)
	{
		//DM FFT
		float temp[SCOPE_BUF_SIZE];
		float tempout[SCOPE_BUF_SIZE];

		FillLinearFromCircularBuffer(pSamplesL, temp, multleft);
		helpers::dsp::dmfft::WindowFunc(3, scope_spec_samples, temp);
		helpers::dsp::dmfft::PowerSpectrum(scope_spec_samples, temp, tempout);
		fftSpec.FillBandsFromFFT(tempout, db_left);

		FillLinearFromCircularBuffer(pSamplesR, temp, multright);
		helpers::dsp::dmfft::WindowFunc(3, scope_spec_samples, temp);
		helpers::dsp::dmfft::PowerSpectrum(scope_spec_samples, temp, tempout);
		fftSpec.FillBandsFromFFT(tempout, db_right);
	}
	else if (FFTMethod == 2)
	{
		// Psycle's pseudo FT
		const float dbinvSamples = psycle::helpers::dsp::dB(1.0f / (scope_spec_samples >> 2)); // >>2, because else seems 12dB's off
		float aal[SCOPE_SPEC_BANDS] = { 0 };
		float aar[SCOPE_SPEC_BANDS] = { 0 };
		float bbl[SCOPE_SPEC_BANDS] = { 0 };
		float bbr[SCOPE_SPEC_BANDS] = { 0 };

		//scopeBufferIndex is the position where it will write new data.
		int index = (srcMachine._scopeBufferIndex == 0) SCOPE_BUF_SIZE - 1 : srcMachine._scopeBufferIndex - 1;
		for (int i = 0; i < scope_spec_samples; i++, index--, index &= (SCOPE_BUF_SIZE - 1))
		{
			const float wl = pSamplesL[index] * multleft;
			const float wr = pSamplesR[index] * multright;
			for (int h = 0; h < SCOPE_SPEC_BANDS; h++)
			{
				aal[h] += wl * cth[i][h];
				bbl[h] += wl * sth[i][h];
				aar[h] += wr * cth[i][h];
				bbr[h] += wr * sth[i][h];
			}
		}

		for (int h = 0; h < SCOPE_SPEC_BANDS; h++)
		{
			float out = aal[h] * aal[h] + bbl[h] * bbl[h];
			db_left[h] = helpers::dsp::powerdB(out + 0.0000001f) + dbinvSamples;
			out = aar[h] * aar[h] + bbr[h] * bbr[h];
			db_right[h] = helpers::dsp::powerdB(out + 0.0000001f) + dbinvSamples;
		}
	}
#endif
	// draw our bands
	DCBar = -1; // fftSpec.getDCBars();
	psy_ui_setrectangle(&rect, 0,0,SCOPE_BARS_WIDTH,0);
	for (i = 0; i < SCOPE_SPEC_BANDS;
		i++, rect.left += SCOPE_BARS_WIDTH, rect.right += SCOPE_BARS_WIDTH)
	{
		//Remember, 0 -> top of spectrum, 128 bottom of spectrum.
		int curpeak, halfpeak;
		uint32_t colour;
		int aml = -db_left[i] * 2; // Reducing visible range from 128dB to 64dB.
		int amr = -db_right[i] * 2; // Reducing visible range from 128dB to 64dB.
		//int aml = - db_left[i];
		//int amr = - db_right[i];
		aml = (aml < 0) ? 0 : (aml > 128) ? 128 : aml;
		amr = (amr < 0) ? 0 : (amr > 128) ? 128 : amr;
		if (i <= DCBar) {
			curpeak = min(aml, amr);
			halfpeak = 128;
			colour = CLBARDC;
		}
		else if (aml < amr) {
			curpeak = aml;
			halfpeak = amr;
			colour = CLLEFT;
		}
		else {
			curpeak = amr;
			halfpeak = aml;
			colour = CLRIGHT;
		}
		if (curpeak < self->bar_heights[i]) { self->bar_heights[i] = curpeak; }
		else if (!self->hold && self->bar_heights[i] < 128)
		{
			self->bar_heights[i] += 128 / self->scope_spec_rate * 0.5;//two seconds to decay
			if (self->bar_heights[i] > 128) { self->bar_heights[i] = 128; }
		}

		rect.top = curpeak;
		rect.bottom = halfpeak;
		psy_ui_drawsolidrectangle(g, rect, colour);

		rect.top = rect.bottom;
		rect.bottom = 128;
		psy_ui_drawsolidrectangle(g, rect, CLBOTH);

		rect.top = self->bar_heights[i];
		rect.bottom = self->bar_heights[i] + 1;
		psy_ui_drawsolidrectangle(g, rect, CLBARPEAK);
	}
	psy_snprintf(buf, sizeof(buf), "%d Samples Refresh %.2fhz", self->scope_spec_samples, 1000.0f / self->scope_spec_rate);
	//sprintf(buf,"%d Samples Refresh %.2fhz Window %d",scope_spec_samples,1000.0f/scope_spec_rate, FFTMethod);
	//CFont* oldFont = bufDC.SelectObject(&font);
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolor(g, 0x505050);
	psy_ui_textout(g, 4, 128 - 14, buf, strlen(buf));
}

void spectrumanalyzer_ontimer(SpectrumAnalyzer* self, psy_ui_Component* sender, int timerid)
{	
	if (timerid == TIMERID_MASTERVU) {
		psy_ui_component_invalidate(&self->component);
	}
}

void spectrumanalyzer_onsrcmachineworked(SpectrumAnalyzer* self, psy_audio_Machine* master, unsigned int slot,
	psy_audio_BufferContext* bc)
{	
	if (bc->output->rms) {
		psy_audio_Connections* connections;
		psy_audio_WireSocketEntry* input;	

		connections = &self->workspace->song->machines.connections;
		input = connection_input(connections, self->wire.src, self->wire.dst);
		if (input) {					
			self->leftavg = bc->output->rms->data.previousLeft / 32768;
			self->rightavg = bc->output->rms->data.previousRight / 32768;
			self->invol = input->volume;			
		}
	}
}

void spectrumanalyzer_onsongchanged(SpectrumAnalyzer* self, Workspace* workspace,
	int flag, psy_audio_SongFile* songfile)
{	
	self->leftavg = 0;
	self->rightavg = 0;
	spectrumanalyzer_connectmachinessignals(self, workspace);	
}

void spectrumanalyzer_connectmachinessignals(SpectrumAnalyzer* self, Workspace* workspace)
{
	if (workspace->song) {
		psy_audio_Machine* srcmachine;

		srcmachine = machines_at(&workspace->song->machines, self->wire.src);
		if (srcmachine) {
			psy_signal_connect(&srcmachine->signal_worked, self,
				spectrumanalyzer_onsrcmachineworked);
		}
	}
}

void spectrumanalyzer_disconnectmachinessignals(SpectrumAnalyzer* self, Workspace* workspace)
{
	if (workspace->song) {
		psy_audio_Machine* srcmachine;

		srcmachine = machines_at(&workspace->song->machines, self->wire.src);
		if (srcmachine) {
			psy_signal_disconnect(&srcmachine->signal_worked, self,
				spectrumanalyzer_onsrcmachineworked);
		}
	}
}

void spectrumanalyzer_stop(SpectrumAnalyzer* self)
{
	if (self->workspace && self->workspace->song) {
		psy_audio_Machine* srcmachine;		
		srcmachine = machines_at(&self->workspace->song->machines,
			self->wire.src);
		if (srcmachine) {
			psy_audio_exclusivelock_enter();
			psy_signal_disconnect(&srcmachine->signal_worked, self,
				spectrumanalyzer_onsrcmachineworked);
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
