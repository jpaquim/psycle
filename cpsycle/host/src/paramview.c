// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "paramview.h"
#include "skingraphics.h"
#include "resources/resource.h"
#include "skincoord.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "../../detail/portable.h"

#define TIMERID_PARAMVIEW 410

static void onpreferredsize(ParamView*, psy_ui_Size* limit, psy_ui_Size* rv);
static void ondestroy(ParamView*, psy_ui_Component* sender);
static void ondraw(ParamView*, psy_ui_Graphics*);
static void drawparameter(ParamView*, psy_ui_Graphics*, uintptr_t param, uintptr_t row, uintptr_t col);
static void drawslider(ParamView*, psy_ui_Graphics*, uintptr_t param, uintptr_t row, uintptr_t col);
static void drawsliderlevel(ParamView*, psy_ui_Graphics*, uintptr_t param, uintptr_t row, uintptr_t col);
static void drawslidercheck(ParamView*, psy_ui_Graphics*, uintptr_t param, uintptr_t row, uintptr_t col);
static void drawswitch(ParamView*, psy_ui_Graphics*, uintptr_t param, uintptr_t row, uintptr_t col);
static void drawheader(ParamView*, psy_ui_Graphics*, uintptr_t param, uintptr_t row, uintptr_t col);
static void drawinfolabel(ParamView*, psy_ui_Graphics*, uintptr_t param, uintptr_t row, uintptr_t col);
static void drawknob(ParamView*, psy_ui_Graphics*, uintptr_t param, uintptr_t row, uintptr_t col);
static void drawblank(ParamView*, psy_ui_Graphics*, uintptr_t param, uintptr_t row, uintptr_t col);
static void cellsize(ParamView*, uintptr_t param, int* width, int* height);
static void cellposition(ParamView*, uintptr_t param, uintptr_t row, uintptr_t col, int* x, int* y);
static void onmousedown(ParamView*, psy_ui_MouseEvent*);
static void onmouseup(ParamView*, psy_ui_MouseEvent*);
static void onmousemove(ParamView*, psy_ui_MouseEvent*);
static int hittest(ParamView*, int x, int y);
static void ontimer(ParamView*, psy_ui_Component* sender, int timerid);
static uintptr_t paramview_numrows(ParamView*);
static void paramview_computepositions(ParamView*);
static void paramview_clearpositions(ParamView*);
static void mixer_vumeterdraw(ParamView*, psy_ui_Graphics*, int x, int y,
	float value);
static void drawsliderlevelback(ParamView*, psy_ui_Graphics*, int x, int y);

static int paramview_refcount = 0;
static int paramskin_initialized = 0;
static ParamSkin paramskin;

static void paramskin_init(ParamView*);
static void paramskin_release(ParamView*);

void paramskin_init(ParamView* self)
{
	if (!paramskin_initialized) {
		psy_Properties* theme;
		SkinCoord knob = { 0, 0, 28, 28, 0, 0, 28, 28, 0 };
		SkinCoord slider = { 0, 0, 30, 182, 0, 0, 30, 182, 0 };
		SkinCoord sliderknob = { 0, 182, 22, 10, 0, 0, 22, 10, 0 };
		SkinCoord vuoff = { 30, 0, 16, 90, 0, 0, 16, 90, 0 };
		SkinCoord vuon = { 46, 0, 16, 90, 0, 0, 16, 90, 0 };
		SkinCoord switchon = { 30, 118, 28, 28, 0, 0, 28, 28, 0 };
		SkinCoord switchoff = { 30, 90, 28, 28, 0, 0, 28, 28, 0 };
		SkinCoord checkon = { 30, 159, 13, 13, 0, 0, 13, 13, 0 };
		SkinCoord checkoff = { 30, 146, 13, 13, 0, 0, 13, 13, 0 };		
		
		paramskin.slider = slider;
		paramskin.sliderknob = sliderknob;
		paramskin.knob = knob;
		paramskin.vuoff = vuoff;
		paramskin.vuon = vuon;
		paramskin.switchon = switchon;
		paramskin.switchoff = switchoff;
		paramskin.checkon = checkon;
		paramskin.checkoff = checkoff;
		paramskin_initialized = 1;		
		theme = self->workspace->paramtheme;
		paramskin.topcolor = psy_properties_int(theme, "machineguitopcolor", 0x00555555);
		paramskin.fonttopcolor = psy_properties_int(theme, "machineguifonttopcolor", 0x00CDCDCD);
		paramskin.bottomcolor = psy_properties_int(theme, "machineguibottomcolor", 0x00444444);
		paramskin.fontbottomcolor = psy_properties_int(theme, "machineguifontbottomcolor", 0x00E7BD18);
		//highlighted param colours
		paramskin.htopcolor = psy_properties_int(theme, "machineguihtopcolor", 0x00555555);
		paramskin.fonthtopcolor = psy_properties_int(theme, "machineguihfonttopcolor", 0x00CDCDCD);
		paramskin.hbottomcolor = psy_properties_int(theme, "machineguihbottomcolor", 0x00292929);
		paramskin.fonthbottomcolor = psy_properties_int(theme, "machineguihfontbottomcolor", 0x00E7BD18);

		paramskin.titlecolor = psy_properties_int(theme, "machineguititlecolor", 0x00292929);
		paramskin.fonttitlecolor = psy_properties_int(theme, "machineguititlefontcolor", 0x00B4B4B4);
		psy_ui_bitmap_init(&paramskin.knobbitmap);
		if (!workspace_dialbitmap_path(self->workspace)) {
			psy_ui_bitmap_loadresource(&paramskin.knobbitmap, IDB_PARAMKNOB);
		} else
		if (psy_ui_bitmap_load(&paramskin.knobbitmap,
				workspace_dialbitmap_path(self->workspace))) {
			psy_ui_bitmap_loadresource(&paramskin.knobbitmap, IDB_PARAMKNOB);			
		}
		psy_ui_bitmap_init(&paramskin.mixerbitmap);
		psy_ui_bitmap_loadresource(&paramskin.mixerbitmap, IDB_MIXERSKIN);		
	}
	self->skin = &paramskin;
}

void paramskin_release(ParamView* self)
{
	if (paramskin_initialized && paramview_refcount == 0) {
		psy_ui_bitmap_dispose(&paramskin.knobbitmap);
		psy_ui_bitmap_dispose(&paramskin.mixerbitmap);
		paramskin_initialized = 0;
	}
}

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(ParamView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw = (psy_ui_fp_ondraw) ondraw;
		vtable.onmousedown = (psy_ui_fp_onmousedown) onmousedown;
		vtable.onmousemove = (psy_ui_fp_onmousemove) onmousemove;
		vtable.onmouseup = (psy_ui_fp_onmouseup) onmouseup;
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize) onpreferredsize;
	}
}

void paramview_init(ParamView* self, psy_ui_Component* parent, psy_audio_Machine* machine,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_doublebuffer(&self->component);
	++paramview_refcount;
	self->workspace = workspace;
	self->machine = machine;
	self->numparams = 0;
	paramskin_init(self);	
	psy_table_init(&self->positions);
	self->tweak = -1;
	self->sizechanged = 1;
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);		
	psy_signal_connect(&self->component.signal_timer, self, ontimer);	
	psy_ui_component_starttimer(&self->component, TIMERID_PARAMVIEW, 100);
}

ParamView* paramview_alloc(void)
{
	return (ParamView*) malloc(sizeof(ParamView));
}

ParamView* paramview_allocinit(psy_ui_Component* parent, psy_audio_Machine* machine,
	Workspace* workspace)
{
	ParamView* rv;

	rv = paramview_alloc();
	if (rv) {
		paramview_init(rv, parent, machine, workspace);
	}
	return rv;
}

void ondestroy(ParamView* self, psy_ui_Component* sender)
{
	--paramview_refcount;
	paramskin_release(self);
	paramview_clearpositions(self);
	psy_table_dispose(&self->positions);
}

void ondraw(ParamView* self, psy_ui_Graphics* g)
{				
	if (self->machine && psy_audio_machine_numparameters(self->machine) > 0
			&& psy_audio_machine_numparametercols(self->machine) > 0) {
		uintptr_t row = 0;
		uintptr_t col = 0;
		uintptr_t numrows = 0;
		uintptr_t param;		
						
		if (self->numparams != psy_audio_machine_numparameters(self->machine)) {
			self->numparams = psy_audio_machine_numparameters(self->machine);
			paramview_computepositions(self);
			self->sizechanged = 1;
			psy_signal_emit(&self->component.signal_preferredsizechanged, self, 0);
		}
		numrows = paramview_numrows(self);
		for (param = 0; param < psy_audio_machine_numparameters(self->machine);
				++param) {
			drawparameter(self, g, param, row, col);
			++row;
			if (row >= numrows) {
			  row = 0;
			  ++col;			
			}
		}
	}			
}

void drawparameter(ParamView* self, psy_ui_Graphics* g, uintptr_t param,
	uintptr_t row, uintptr_t col)
{		
	switch (psy_audio_machine_parametertype(self->machine, param) &
			~MPF_SMALL) {
		case MPF_HEADER:
			drawheader(self, g, param, row, col);
		break;
		case MPF_INFOLABEL:
			drawinfolabel(self, g, param, row, col);
		break;
		case MPF_STATE:
			drawknob(self, g, param, row, col);
		break;
		case MPF_SLIDER:				
			drawslider(self, g, param, row, col);					
		break;
		case MPF_SLIDERCHECK:
			drawslidercheck(self, g, param, row, col);
		break;
		case MPF_SWITCH:
			drawswitch(self, g, param, row, col);
		break;
		case MPF_SLIDERLEVEL:
			drawsliderlevel(self, g, param, row, col);
		break;
		case MPF_NULL:
			drawblank(self, g, param, row, col);
		break;
		default:
		break;
	}
}

void drawknob(ParamView* self, psy_ui_Graphics* g, uintptr_t param,
	uintptr_t row, uintptr_t col)
{	
	char label[128];
	char str[128];
	psy_ui_Rectangle r;
	psy_ui_Rectangle r_top;
	psy_ui_Rectangle r_bottom;	
	int top;
	int left;
	int width;
	int height;
	int knob_cx;
	int knob_cy;

	knob_cx = self->skin->knob.destwidth;
	knob_cy = self->skin->knob.destheight;		
	cellposition(self, param, row, col, &left, &top);
	cellsize(self, param, &width, &height);
	psy_ui_setrectangle(&r, left, top, width, height);
	psy_ui_setrectangle(&r_top, left + knob_cx, top, width - knob_cx,
		height / 2);
	psy_ui_drawsolidrectangle(g, r, self->skin->topcolor);
	psy_ui_setrectangle(&r_bottom, left + knob_cx, top + height/2,
		width - knob_cx, height / 2);				
	psy_ui_drawsolidrectangle(g, r, self->skin->bottomcolor);
	if (!psy_audio_machine_parametername(self->machine, label, param)) {
		if (!psy_audio_machine_parameterlabel(self->machine, label, param)) {
			psy_snprintf(label, 128, "%s", "");
		}
	}
	if (psy_audio_machine_describevalue(self->machine, str, param,
			machine_parametervalue_scaled(self->machine, param,
			psy_audio_machine_parametervalue(self->machine, param))) == FALSE) {
		psy_snprintf(str, 128, "%d",
			machine_parametervalue_scaled(self->machine, param,
				psy_audio_machine_parametervalue(self->machine, param)));
	}
	psy_ui_setbackgroundcolor(g, (self->tweak == param)
		? self->skin->htopcolor : self->skin->topcolor);
	psy_ui_settextcolor(g, (self->tweak == param)
		? self->skin->fonthtopcolor : self->skin->fonttopcolor);
	psy_ui_textoutrectangle(g, r_top.left, r_top.top,
		psy_ui_ETO_OPAQUE, r_top, label, strlen(label));	
	psy_ui_setbackgroundcolor(g, (self->tweak == param)
		? self->skin->hbottomcolor : self->skin->bottomcolor);
	psy_ui_settextcolor(g, (self->tweak == param)
		? self->skin->fonthbottomcolor : self->skin->fontbottomcolor);
	psy_ui_textoutrectangle(g, r_bottom.left, r_bottom.top,
		psy_ui_ETO_OPAQUE, r_bottom, str, strlen(str));
	if (!psy_ui_bitmap_empty(&self->skin->knobbitmap)) {
		int knob_frame;

		knob_frame = (int)
			(psy_audio_machine_parametervalue(self->machine, param) * 63.f);
		psy_ui_drawbitmap(g, &self->skin->knobbitmap, r.left, r.top, knob_cx,
			knob_cy, knob_frame * knob_cx, 0);
	}
	psy_ui_setcolor(g, 0x00232323);
	psy_ui_drawline(g, r.left, r.bottom - 1, r.right, r.bottom - 1);
	psy_ui_drawline(g, r.right - 1, r.top, r.right - 1, r.bottom - 1);
}

void drawheader(ParamView* self, psy_ui_Graphics* g, uintptr_t param,
	uintptr_t row, uintptr_t col)
{
	int top;
	int left;
	int width;
	int height;
	int half;
	int quarter;
	psy_ui_Rectangle r;	
	const char* parValue;
	char str[128];

	cellposition(self, param, row, col, &left, &top);
	cellsize(self, param, &width, &height);	
	parValue = str;	
	half = height / 2;
	quarter = half / 2;	
	psy_ui_setrectangle(&r, left, top, width, top + quarter);
	psy_ui_drawsolidrectangle(g, r, self->skin->topcolor);
	psy_ui_setrectangle(&r, left, top+ half + quarter, width, quarter);
	psy_ui_drawsolidrectangle(g, r, self->skin->bottomcolor);
	if (!psy_audio_machine_parametername(self->machine, str, param)) {
		if (!psy_audio_machine_parameterlabel(self->machine, str, param)) {
			psy_snprintf(str, 128, "%s", "");
		}
	}	
	psy_ui_setrectangle(&r, left, top + quarter, width, half);
	psy_ui_setbackgroundcolor(g, self->skin->titlecolor);
	psy_ui_settextcolor(g, self->skin->fonttitlecolor);
	// font_bold
	psy_ui_textoutrectangle(g, left, top + quarter,
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r, str,
		strlen(str));
}

void drawinfolabel(ParamView* self, psy_ui_Graphics* g, uintptr_t param,
	uintptr_t row, uintptr_t col)
{
	int top;
	int left;
	int width;
	int height;
	int half;
	psy_ui_Rectangle r;
	char str[128];
			
	str[0] = '\0';
	cellposition(self, param, row, col, &left, &top);
	cellsize(self, param, &width, &height);		
	half = height/2;
	psy_ui_setrectangle(&r, left, top, width, top + half);
	psy_ui_setbackgroundcolor(g, self->skin->titlecolor);
	psy_ui_settextcolor(g, self->skin->fonttitlecolor);
	if (!psy_audio_machine_parametername(self->machine, str, param)) {
		if (!psy_audio_machine_parameterlabel(self->machine, str, param)) {
			psy_snprintf(str, 128, "%s", "");
		}
	}
	psy_ui_textoutrectangle(g, 
		left, top, psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, str, strlen(str));	
	if (psy_audio_machine_describevalue(self->machine, str, param,
			machine_parametervalue_scaled(self->machine, param,
				psy_audio_machine_parametervalue(self->machine, param))) == FALSE) {
		psy_snprintf(str, 128, "%s", "");
	}
	psy_ui_setbackgroundcolor(g, self->skin->bottomcolor);
	psy_ui_settextcolor(g, self->skin->fontbottomcolor);
	psy_ui_setrectangle(&r, left, top + half, width, top + half);
	psy_ui_textoutrectangle(g, left, top + half,
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, str, strlen(str));	
}

void drawslider(ParamView* self, psy_ui_Graphics* g, uintptr_t param,
	uintptr_t row, uintptr_t col)
{
	int top;
	int left;
	int width;
	int height;	
	int xoffset;
	int yoffset;
	float value;
	
	psy_ui_Rectangle r;
	char str[128];
	str[0] = '\0';

	cellposition(self, param, row, col, &left, &top);
	cellsize(self, param, &width, &height);
	psy_ui_setrectangle(&r, left, top, width, height);
	psy_ui_drawsolidrectangle(g, r, self->skin->bottomcolor);
	skin_blitpart(g, &self->skin->mixerbitmap, left, top, &self->skin->slider);
	xoffset = (self->skin->slider.destwidth - self->skin->knob.destwidth) / 2;
	value =	psy_audio_machine_parametervalue(self->machine, param);
	yoffset = (int)((1 - value) *
		(self->skin->slider.destheight - self->skin->sliderknob.destheight));
	skin_blitpart(g, &self->skin->mixerbitmap, left + xoffset, top + yoffset,
		&self->skin->sliderknob);
			
	if (psy_audio_machine_parametername(self->machine, str, param) != FALSE) {
		psy_ui_setbackgroundcolor(g, self->skin->topcolor);
		psy_ui_settextcolor(g, self->skin->fonttopcolor);
		psy_ui_setrectangle(&r, left + 32, top + self->skin->slider.destheight - 48, width - 32, 24);
		psy_ui_textoutrectangle(g, left + 32, top + self->skin->slider.destheight - 48,
			psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
			r, str, strlen(str));
	}	
	psy_ui_setrectangle(&r, left + 32, top + self->skin->slider.destheight - 24,
		width - 32, 24);
	if (psy_audio_machine_describevalue(self->machine, str, param,
		machine_parametervalue_scaled(self->machine, param,
			psy_audio_machine_parametervalue(self->machine, param))) == FALSE) {
		psy_snprintf(str, 128, "%d", psy_audio_machine_parametervalue(
			self->machine, param));
	}
	psy_ui_setbackgroundcolor(g, self->skin->bottomcolor);
	psy_ui_settextcolor(g, self->skin->fontbottomcolor);
	psy_ui_textoutrectangle(g, left + 32, top + self->skin->slider.destheight - 24,
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, str, strlen(str));
}

void drawsliderlevel(ParamView* self, psy_ui_Graphics* g, uintptr_t param, uintptr_t row,
	uintptr_t col)
{
	int top;
	int left;
	int width;
	int height;	
		
	cellposition(self, param, row, col, &left, &top);
	cellsize(self, param, &width, &height);
	drawsliderlevelback(self, g, left, top);
	mixer_vumeterdraw(self, g, left, top,		
			psy_audio_machine_parametervalue(self->machine, param));		
}

void mixer_vumeterdraw(ParamView* self, psy_ui_Graphics* g, int x, int y, float value)
{	
	int ypos;

	if (value < 0.f) value = 0.f;
	if (value > 1.f) value = 1.f;
	ypos = (int)((1.f - value) * self->skin->vuoff.destheight);
	psy_ui_drawbitmap(g, &self->skin->mixerbitmap,
		x,
		y + ypos,
		self->skin->vuoff.destwidth,
		self->skin->vuoff.destheight - ypos,
		self->skin->vuon.srcx,
		self->skin->vuon.srcy + ypos);	
}

void drawsliderlevelback(ParamView* self, psy_ui_Graphics* g, int x, int y)
{
	psy_ui_drawbitmap(g, &self->skin->mixerbitmap,
		x,
		y,
		self->skin->vuoff.destwidth,
		self->skin->vuoff.destheight,
		self->skin->vuoff.srcx,
		self->skin->vuoff.srcy);
}

void drawslidercheck(ParamView* self, psy_ui_Graphics* g, uintptr_t param, uintptr_t row,
	uintptr_t col)
{
	int top;
	int left;
	int width;
	int height;	
	psy_ui_Rectangle r;
	char label[512];
	
	cellposition(self, param, row, col, &left, &top);
	cellsize(self, param, &width, &height);
	if (psy_audio_machine_parametervalue(self->machine, param) == 0.f) {
		skin_blitpart(g, &self->skin->mixerbitmap, left,
			top, &self->skin->checkoff);
	} else {
		skin_blitpart(g, &self->skin->mixerbitmap, left,
			top, &self->skin->checkon);
	}
	psy_ui_setrectangle(&r, left + 20, top, width, height);
	if (!psy_audio_machine_parametername(self->machine, label, param)) {
		if (!psy_audio_machine_parameterlabel(self->machine, label, param)) {
			psy_snprintf(label, 128, "%s", "");
		}
	}
	psy_ui_textoutrectangle(g, left + 20, top,
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, label, strlen(label));
}

void drawswitch(ParamView* self, psy_ui_Graphics* g, uintptr_t param, uintptr_t row,
	uintptr_t col)
{
	int top;
	int left;
	int width;
	int height;	
	psy_ui_Rectangle r;
	char label[512];

	cellposition(self, param, row, col, &left, &top);
	cellsize(self, param, &width, &height);
	psy_ui_setrectangle(&r, left, top, width, height);
	psy_ui_drawsolidrectangle(g, r, self->skin->bottomcolor);
	if (psy_audio_machine_parametervalue(self->machine, param) == 0.f) {
		skin_blitpart(g, &self->skin->mixerbitmap, left,
			top, &self->skin->switchoff);
	}
	else {
		skin_blitpart(g, &self->skin->mixerbitmap, left,
			top, &self->skin->switchon);
	}	
	if (!psy_audio_machine_parametername(self->machine, label, param)) {
		if (!psy_audio_machine_parameterlabel(self->machine, label, param)) {
			psy_snprintf(label, 128, "%s", "");
		}
	}
	psy_ui_setbackgroundcolor(g, self->skin->topcolor);
	psy_ui_settextcolor(g, self->skin->fonttopcolor);
	psy_ui_setrectangle(&r, left + self->skin->switchon.destwidth, top, width, height / 2);
	psy_ui_textoutrectangle(g, left + self->skin->switchon.destwidth, top,
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, label, strlen(label));
}

void drawblank(ParamView* self, psy_ui_Graphics* g, uintptr_t param,
	uintptr_t row, uintptr_t col)
{
	int top;
	int left;
	int width;
	int height;
	psy_ui_Rectangle r;

	cellposition(self, param, row, col, &left, &top);
	cellsize(self, param, &width, &height);
	psy_ui_setrectangle(&r, left, top, width, height);
	psy_ui_drawsolidrectangle(g, r, self->skin->bottomcolor);
}

void cellsize(ParamView* self, uintptr_t param, int* width, int* height)
{
	psy_ui_TextMetric tm;
	uintptr_t paramtype;
	bool small;

	*width = (psy_audio_machine_paramviewoptions(self->machine) & 
		MACHINE_PARAMVIEW_COMPACT) == MACHINE_PARAMVIEW_COMPACT 
			? 67
			: 134;
	*height = 28;
	small = (psy_audio_machine_parametertype(self->machine, param) & MPF_SMALL) == MPF_SMALL;
	paramtype = psy_audio_machine_parametertype(self->machine, param) & ~MPF_SMALL;
	if (paramtype == MPF_IGNORE) {
		*height = 0;
		*width = 0;
	} else
	if (paramtype == MPF_SLIDERCHECK) {
		*height = self->skin->checkoff.destheight;
		*width = self->skin->checkoff.destwidth;
	} else
	if (paramtype == MPF_SLIDER) {
		*height = self->skin->slider.destheight;
		*width = self->skin->slider.destwidth;
		tm = psy_ui_component_textmetric(&self->component);
		if (*width < tm.tmAveCharWidth * 30) {
			*width = tm.tmAveCharWidth * 30;
		}
	} else
	if (paramtype == MPF_SLIDERLEVEL) {
		*height = self->skin->vuon.destheight;
		*width = self->skin->slider.destwidth;
	} else {
		tm = psy_ui_component_textmetric(&self->component);
		if (*width < tm.tmAveCharWidth * 30) {
			*width = tm.tmAveCharWidth * 30;
		}
		if (*height < tm.tmHeight * 2) {
			*height = tm.tmHeight * 2;
		}		
	}
	if (small) {
		*width /= 2;
	}
}

void cellposition(ParamView* self, uintptr_t param, uintptr_t row, uintptr_t col, int* x, int* y)
{
	psy_ui_Rectangle* position;

	position = (psy_ui_Rectangle*)psy_table_at(&self->positions, param);
	if (position) {
		*x = position->left;
		*y = position->top;
	} else {
		*x = 0;
		*y = 0;
	}
}

void onmousedown(ParamView* self, psy_ui_MouseEvent* ev)
{
	self->tweak = hittest(self, ev->x, ev->y);
	if (self->tweak != -1) {
		uintptr_t paramtype;

		self->tweakbase = ev->y;		
		self->tweakval = psy_audio_machine_parametervalue(self->machine,
			self->tweak);
		paramtype = psy_audio_machine_parametertype(self->machine, self->tweak) & ~MPF_SMALL;
		if (paramtype == MPF_SLIDERCHECK || paramtype == MPF_SWITCH) {
			if (psy_audio_machine_parametervalue(self->machine, self->tweak) == 0.f) {
				psy_audio_machine_parametertweak(self->machine, self->tweak,
					1 / (float)0xFFFF);
			} else {
				psy_audio_machine_parametertweak(self->machine, self->tweak,
					0.f);
			}			
		}
		psy_ui_component_capture(&self->component);
	}
}

int hittest(ParamView* self, int x, int y)
{
	int rv = 0;
	if (self->machine) {
		uintptr_t param;

		for (param = 0;	param < psy_table_size(&self->positions); ++param) {				
			psy_ui_Rectangle* position;
			uintptr_t type;

			position = (psy_ui_Rectangle*) psy_table_at(&self->positions,
				param);
			type = psy_audio_machine_parametertype(self->machine, param);			
			if (psy_ui_rectangle_intersect(position, x, y)) {							
				rv = param;
				break;
			}
		}
	}	
	return rv;
}

void onmousemove(ParamView* self, psy_ui_MouseEvent* ev)
{
	if (self->tweak != -1) {		
		float val;
				
		val = self->tweakval + (self->tweakbase - ev->y) / 200.f;
		if (val > 1.f) {
			val = 1.f;
		} else
		if (val < 0.f) {
			val = 0.f;
		}		
		psy_audio_machine_parametertweak(self->machine, self->tweak, val);
		workspace_parametertweak(self->workspace,
			psy_audio_machine_slot(self->machine),
			self->tweak, val);
		psy_ui_component_invalidate(&self->component);
	}
}

void onmouseup(ParamView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_component_releasecapture(&self->component);
	self->tweak = -1;
}

void ontimer(ParamView* self, psy_ui_Component* sender, int timerid)
{
	psy_ui_component_invalidate(&self->component);
}

void onpreferredsize(ParamView* self, psy_ui_Size* limit, psy_ui_Size* rv)
{
	assert(rv);	
	if (self->machine && psy_audio_machine_numparameters(self->machine) > 0) {				
		paramview_computepositions(self);
		*rv = self->cpmax;
		self->sizechanged = 0;
	} else {
		*rv = psy_ui_component_size(&self->component);
		self->sizechanged = 0;
	}
}

void paramview_computepositions(ParamView* self)
{
	paramview_clearpositions(self);
	if (self->machine) {				
		uintptr_t param;
		uintptr_t numrows;
		uintptr_t row = 0;
		uintptr_t col = 0;
		uintptr_t paramrowbegin = 0;
		int colmax = 0;
		int cpx = 0;
		int cpy = 0;
		int cpy_slidercheck;		

		numrows = paramview_numrows(self);
		self->cpmax.width = 0;
		for (param = 0; param < psy_audio_machine_numparameters(self->machine);
				++param) {
			psy_ui_Rectangle* position;
			psy_ui_Rectangle* firstrow;
			uintptr_t paramtype;
			int width;
			int height;			
			
			paramtype = psy_audio_machine_parametertype(self->machine, param) & ~MPF_SMALL;
			position = (psy_ui_Rectangle*) malloc(sizeof(psy_ui_Rectangle));
			psy_ui_setrectangle(position, cpx, 0, 0, 0);
			if (paramtype == MPF_SLIDERLEVEL) {
				position->left += self->skin->slider.destwidth;
			} else
			if (paramtype == MPF_SLIDERCHECK) {
					position->left += 50;
			}
			psy_table_insert(&self->positions, param, position);
			cellsize(self, param, &width, &height);
			if (colmax < width) {
				colmax = width;
			}
			firstrow = psy_table_at(&self->positions, row);
			if (paramtype != MPF_IGNORE && paramtype != MPF_SLIDERLEVEL &&
					paramtype != MPF_SLIDERCHECK) {
				if (height > (firstrow->bottom - firstrow->top)) {
					firstrow->bottom = firstrow->top + height;
				}				
			}			
			++row;
			if (row >= numrows) {
				uintptr_t i;

				for (i = paramrowbegin; i <= param; ++i) {
					position = psy_table_at(&self->positions, i);
					if (psy_audio_machine_parametertype(self->machine, i)
								== MPF_SLIDERCHECK ||
							psy_audio_machine_parametertype(self->machine, i)
								== MPF_SLIDERLEVEL) {
						cellsize(self, i, &width, &height);
						position->right = position->left + width;
					} else {						
						position->right = cpx + colmax;
					}
				}
				cpx += colmax;
				if (self->cpmax.width < cpx) {
					self->cpmax.width = cpx;
				}
				++col;
				row = 0;				
				paramrowbegin = param + 1;
			}
		}
		cpy_slidercheck = 0;	
		row = 0;
		self->cpmax.height = 0;
		for (param = 0; param < psy_audio_machine_numparameters(self->machine);
				++param) {
			psy_ui_Rectangle* position;
			psy_ui_Rectangle* firstrow;
			uintptr_t height;
			uintptr_t paramtype;

			position = psy_table_at(&self->positions, param);
			firstrow = psy_table_at(&self->positions, row);
			paramtype = psy_audio_machine_parametertype(self->machine, param) & ~MPF_SMALL;			
			height = firstrow->bottom - firstrow->top;
			position->top = cpy;
			if (paramtype == MPF_SLIDERCHECK) {
				position->top += cpy_slidercheck;
				position->bottom = position->top +
					self->skin->checkon.destheight;
			} else {
				position->bottom = cpy + height;
			}
			if (self->cpmax.height < (int) (cpy + height)) {
				self->cpmax.height = (int) (cpy + height);
			}
			if (paramtype != MPF_IGNORE && paramtype != MPF_SLIDER &&
					paramtype != MPF_SLIDERCHECK) {
				cpy += height;
			}
			if (paramtype == MPF_SLIDERCHECK) {
				cpy_slidercheck += self->skin->checkon.destheight;
			}
			++row;					
			if (row >= numrows) {
				++col;
				row = 0;
				cpy = 0;	
				cpy_slidercheck = 0;				
			}
		}
	}	
}

void paramview_clearpositions(ParamView* self)
{
	psy_TableIterator it;

	for (it = psy_table_begin(&self->positions); !psy_tableiterator_equal(&it, psy_table_end());
			psy_tableiterator_inc(&it)) {
		psy_ui_Rectangle* position;

		position = (psy_ui_Rectangle*)psy_tableiterator_value(&it);
		free(position);		
	}
	psy_table_clear(&self->positions);
}

uintptr_t paramview_numrows(ParamView* self)
{
	if (self->machine &&
			psy_audio_machine_numparametercols(self->machine) > 0) {
		return (psy_audio_machine_numparameters(self->machine) - 1) /
			psy_audio_machine_numparametercols(self->machine) + 1;
	}
	return 0;
}

void paramview_changecontrolskin(const char* path)
{		
	if (paramskin_initialized) {
		psy_ui_bitmap_dispose(&paramskin.knobbitmap);
	}
	paramskin_initialized = 0;
}
