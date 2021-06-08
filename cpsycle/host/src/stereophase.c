/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

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

static void stereophase_initbackground(StereoPhase*);
static void stereophase_ondraw(StereoPhase*, psy_ui_Graphics*);
static void stereophase_drawbackground(StereoPhase*, psy_ui_Graphics*);
static void stereophase_drawphase(StereoPhase*, psy_ui_Graphics*);
static void stereophase_drawscale(StereoPhase*, psy_ui_Graphics*);
static void stereophase_drawbars(StereoPhase*, psy_ui_Graphics*);
static psy_dsp_amp_t dB(psy_dsp_amp_t amplitude);
/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(StereoPhase* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);		
		vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			stereophase_ondraw;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}
/* implementation */
void stereophase_init(StereoPhase* self, psy_ui_Component* parent, psy_audio_Wire wire,
	Workspace* workspace)
{					
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_px(256.0, 128.0));
	self->wire = wire;	
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
	stereophase_initbackground(self);	
}

void stereophase_initbackground(StereoPhase* self)
{
	psy_ui_Font font;
	psy_ui_FontInfo fontinfo;
	psy_ui_Graphics g;

	psy_ui_bitmap_init_size(&self->bg, psy_ui_realsize_make(256.0, 128.0));
	psy_ui_graphics_init_bitmap(&g, &self->bg);
	psy_ui_fontinfo_init(&fontinfo, "tahoma", 12);
	psy_ui_font_init(&font, &fontinfo);
	psy_ui_setfont(&g, &font);
	psy_ui_font_dispose(&font);
	stereophase_drawbackground(self, &g);
	psy_ui_graphics_dispose(&g);
}

void stereophase_ondraw(StereoPhase* self, psy_ui_Graphics* g)
{
	psy_ui_drawfullbitmap(g, &self->bg, psy_ui_realpoint_zero());
	stereophase_drawphase(self, g);
}

void stereophase_drawbackground(StereoPhase* self, psy_ui_Graphics* g)
{
	char buf[64];

	psy_ui_setlinewidth(g, 8);
	psy_ui_setcolour(g, psy_ui_colour_make(0x00303030));

	psy_ui_drawline(g,
		psy_ui_realpoint_make(32, 32),
		psy_ui_realpoint_make(128, 128));
	psy_ui_drawline(g,
		psy_ui_realpoint_make(128, 128),
		psy_ui_realpoint_make(128, 0));
	psy_ui_drawline(g,
		psy_ui_realpoint_make(128, 128),
		psy_ui_realpoint_make(256 - 32, 32));

	psy_ui_drawarc(g,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(0, 0),
			psy_ui_realsize_make(256, 256)),
		0, 180);	
	psy_ui_drawarc(g,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(48, 48),
			psy_ui_realsize_make(160, 160)),
		0, 180);
	psy_ui_drawarc(g,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(96, 96),
			psy_ui_realsize_make(64, 64)),
		0, 180);
	psy_ui_setlinewidth(g, 4);
	psy_ui_setcolour(g, psy_ui_colour_make(0x00404040));
	psy_ui_drawline(g,
		psy_ui_realpoint_make(32, 32),
		psy_ui_realpoint_make(128, 128));
	psy_ui_drawline(g,
		psy_ui_realpoint_make(128, 128),
		psy_ui_realpoint_make(128, 0));
	psy_ui_drawline(g,
		psy_ui_realpoint_make(128, 128),
		psy_ui_realpoint_make(256 - 32, 32));
	psy_ui_setlinewidth(g, 1);

	sprintf(buf, "Refresh %.2fhz", 1000.0f / self->scope_phase_rate);
	// oldFont = bufDC.SelectObject(&font);
	// bufDC.SetBkMode(TRANSPARENT);
	// bufDC.SetTextColour(0x505050);
	psy_ui_setbackgroundmode(g, psy_ui_TRANSPARENT);
	psy_ui_settextcolour(g, psy_ui_colour_make(0x606060));
	psy_ui_textout(g, 4, 128 - 14, buf, psy_strlen(buf));
	// bufDC.TextOut(4, 128 - 14, buf);
	// bufDC.SelectObject(oldFont);
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
	uintptr_t sr;	
	int x, y;
	float* pSamplesL;
	float* pSamplesR;
	psy_audio_Machine* machine;
	psy_audio_Buffer* buffer;
	intptr_t scopesamples;
	int index = 0;
	float maxval;
	float quarterpi;
	
	sr = (uintptr_t)psy_audio_player_samplerate(workspace_player(self->workspace));
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
	psy_ui_drawline(g, psy_ui_realpoint_make(x, y), psy_ui_realpoint_make(128, 128));
	//	bufDC.LineTo(128,128-helpers::math::round<int,float>(o_mvpc*128.0f));
	//	bufDC.MoveTo(128,128);
	x = psy_dsp_fround((float)sin(quarterpi + (self->o_mvdpr * quarterpi)) * self->o_mvpr * 128.0f) + 128;
	y = psy_dsp_fround(-(float)cos(quarterpi + (self->o_mvdpr * quarterpi)) * self->o_mvpr * 128.0f) + 128;
	psy_ui_drawline(g, psy_ui_realpoint_make(128, 128), psy_ui_realpoint_make(x, y));

	// panning data
	// bufDC.SelectObject(&linepenbR);

	x = psy_dsp_fround((float)sin(-(self->o_mvdl * quarterpi)) * self->o_mvl * 128.0f) + 128;
	y = psy_dsp_fround(-(float)cos(-(self->o_mvdl * quarterpi)) * self->o_mvl * 128.0f) + 128;
	psy_ui_drawline(g, psy_ui_realpoint_make(x, y), psy_ui_realpoint_make(128, 128));

	//						bufDC.LineTo(128,128-helpers::math::round<int,float>(o_mvc*128.0f));
	//						bufDC.MoveTo(128,128);
	x = psy_dsp_fround((float)sin((self->o_mvdr * quarterpi)) * self->o_mvr * 128.0f) + 128;
	y = psy_dsp_fround(-(float)cos((self->o_mvdr * quarterpi)) * self->o_mvr * 128.0f) + 128;
	psy_ui_drawline(g, psy_ui_realpoint_make(128, 128), psy_ui_realpoint_make(x, y));

	// bufDC.SelectObject(&linepenL);

	x = psy_dsp_fround((float)sin(-quarterpi - (mvdpl * quarterpi)) * mvpl * 128.0f) + 128;
	y = psy_dsp_fround(-(float)cos(-quarterpi - (mvdpl * quarterpi)) * mvpl * 128.0f) + 128;
	psy_ui_drawline(g, psy_ui_realpoint_make(x, y), psy_ui_realpoint_make(128, 128));
	//						bufDC.LineTo(128,128-helpers::math::round<int,float>(mvpc*128.0f));
	//						bufDC.MoveTo(128,128);
	x = psy_dsp_fround((float)sin(quarterpi + (mvdpr * quarterpi)) * mvpr * 128.0f) + 128;
	y = psy_dsp_fround(-(float)cos(quarterpi + (mvdpr * quarterpi)) * mvpr * 128.0f) + 128;
	psy_ui_drawline(g, psy_ui_realpoint_make(128, 128), psy_ui_realpoint_make(x, y));

	// panning data
	// bufDC.SelectObject(&linepenR);

	x = psy_dsp_fround((float)sin(-(mvdl * quarterpi)) * mvl * 128.0f) + 128;
	y = psy_dsp_fround(-(float)cos(-(mvdl * quarterpi)) * mvl * 128.0f) + 128;
	psy_ui_drawline(g, psy_ui_realpoint_make(x, y), psy_ui_realpoint_make(128, 128));
	//						bufDC.LineTo(128,128-helpers::math::round<int,float>(mvc*128.0f));
	//						bufDC.MoveTo(128,128);
	x = psy_dsp_fround((float)sin((mvdr * quarterpi)) * mvr * 128.0f) + 128;
	y = psy_dsp_fround(-(float)cos((mvdr * quarterpi)) * mvr * 128.0f) + 128;
	psy_ui_drawline(g, psy_ui_realpoint_make(128, 128), psy_ui_realpoint_make(x, y));

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
}

void stereophase_idle(StereoPhase* self)
{
	self->invol = psy_audio_connections_wirevolume(
		&workspace_song(self->workspace)->machines.connections,
		self->wire);
	psy_ui_component_invalidate(&self->component);
}

void stereophase_stop(StereoPhase* self)
{	
}

/// linear -> deciBell
/// amplitude normalized to 1.0f.
psy_dsp_amp_t dB(psy_dsp_amp_t amplitude)
{
	///\todo merge with psycle::helpers::math::linear_to_deci_bell
	return (psy_dsp_amp_t) (20.0 * log10(amplitude));
}
