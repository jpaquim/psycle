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
static void drawlevel(ParamView*, psy_ui_Graphics*, uintptr_t param, uintptr_t row, uintptr_t col);
static void drawheader(ParamView*, psy_ui_Graphics*, uintptr_t param, uintptr_t row, uintptr_t col);
static void drawinfolabel(ParamView*, psy_ui_Graphics*, uintptr_t param, uintptr_t row, uintptr_t col);
static void drawknob(ParamView*, psy_ui_Graphics*, uintptr_t param, uintptr_t row, uintptr_t col);
static void cellsize(ParamView*, int* width, int* height);
static void cellposition(ParamView*, uintptr_t row, uintptr_t col, int* x, int* y);
static void onmousedown(ParamView*, psy_ui_MouseEvent*);
static void onmouseup(ParamView*, psy_ui_MouseEvent*);
static void onmousemove(ParamView*, psy_ui_MouseEvent*);
static int hittest(ParamView*, int x, int y);
static void ontimer(ParamView*, psy_ui_Component* sender, int timerid);
static uintptr_t paramview_numrows(ParamView*);

static int paramview_refcount = 0;
static int paramskin_initialized = 0;
static ParamSkin paramskin;

static void paramskin_init(ParamView*);
static void paramskin_release(ParamView*);

void paramskin_init(ParamView* self)
{
	if (!paramskin_initialized) {
		psy_Properties* theme;		

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
		paramskin_initialized = 1;
	}
	self->skin = &paramskin;
}

void paramskin_release(ParamView* self)
{
	if (paramskin_initialized && paramview_refcount == 0) {
		psy_ui_bitmap_dispose(&paramskin.knobbitmap);
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

static psy_ui_Bitmap mixer;
static int mixer_bmp_initialized = 0;

void paramview_init(ParamView* self, psy_ui_Component* parent, psy_audio_Machine* machine,
	Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_ui_component_doublebuffer(&self->component);
	++paramview_refcount;
	self->workspace = workspace;
	self->machine = machine;
	paramskin_init(self);	
	if (mixer_bmp_initialized) {
		psy_ui_bitmap_init(&mixer);
		psy_ui_bitmap_loadresource(&mixer, IDB_MIXERSKIN);
		mixer_bmp_initialized = 1;
	}	
	self->tweak = -1;	
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
}

void ondraw(ParamView* self, psy_ui_Graphics* g)
{			
	if (self->machine && psy_audio_machine_numparameters(self->machine) > 0
			&& psy_audio_machine_numparametercols(self->machine) > 0) {
		uintptr_t row = 0;
		uintptr_t col = 0;
		uintptr_t numrows = 0;
		uintptr_t param;		
						
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
	switch (psy_audio_machine_parametertype(self->machine, param)) {
		case 1:
			drawheader(self, g, param, row, col);
		break;
		case 3:
			drawinfolabel(self, g, param, row, col);
		break;
		case MPF_STATE:
			drawknob(self, g, param, row, col);
		break;
		case MPF_SLIDER:
			drawslider(self, g, param, row, col);
		break;
		case MPF_LEVEL:
			drawlevel(self, g, param, row, col);
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
	int knob_cx = 28;
	int knob_cy = 28;	
		
	cellposition(self, row, col, &left, &top);
	cellsize(self, &width, &height);
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

	cellposition(self, row, col, &left, &top);
	cellsize(self, &width, &height);
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
	cellposition(self, row, col, &left, &top);
	cellsize(self, &width, &height);		
	half = height/2;
	psy_ui_setrectangle(&r, left, top, width, top + half);
	psy_ui_setbackgroundcolor(g, 0x00232323);
	psy_ui_settextcolor(g, 0x00FFFFFF);
	if (!psy_audio_machine_parametername(self->machine, str, param)) {
		if (!psy_audio_machine_parameterlabel(self->machine, str, param)) {
			psy_snprintf(str, 128, "%s", "");
		}
	}
	psy_ui_textoutrectangle(g, 
		left, top, psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, str, strlen(str));
	psy_ui_setrectangle(&r, left, top + half, width, top + half);
	if (psy_audio_machine_describevalue(self->machine, str, param,
			machine_parametervalue_scaled(self->machine, param,
				psy_audio_machine_parametervalue(self->machine, param))) == FALSE) {
		psy_snprintf(str, 128, "%d", psy_audio_machine_parametervalue(
			self->machine, param));
	}
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
	SkinCoord slider = { 0, 0, 30, 182, 0, 0, 30, 182, 0 };
	SkinCoord knob = { 0, 182, 22, 10, 0, 0, 22, 10, 0 };
	SkinCoord vuoff = { 30, 0, 16, 90, 0, 0, 16, 90, 0 };
	SkinCoord vuon = { 46, 0, 16, 90, 0, 0, 16, 90, 0 };

	cellposition(self, row, col, &left, &top);
	cellsize(self, &width, &height);	
	skin_blitpart(g, &mixer, left, top, &slider);
	xoffset = (slider.destwidth - knob.destwidth) / 2;
	value =	psy_audio_machine_parametervalue(self->machine, param);
	yoffset = (int)((1 - value) * slider.destheight);
	skin_blitpart(g, &mixer, left + xoffset, top + yoffset, &knob);	
}

void drawlevel(ParamView* self, psy_ui_Graphics* g, uintptr_t param, uintptr_t row,
	uintptr_t col)
{
	int top;
	int left;
	int width;
	int height;	
	float value;
	int vuonheight;

	SkinCoord slider = { 0, 0, 30, 182, 0, 0, 30, 182, 0 };	
	SkinCoord vuoff = { 30, 0, 16, 90, 0, 0, 16, 90, 0 };
	SkinCoord vuon = { 46, 0, 16, 90, 0, 0, 16, 90, 0 };
	cellposition(self, row, col, &left, &top);
	cellsize(self, &width, &height);		
	skin_blitpart(g, &mixer, left + slider.destwidth, 
		slider.destheight - vuoff.destheight, &vuoff);			
	value =	psy_audio_machine_parametervalue(self->machine, param);
	vuonheight = (int)(vuon.srcheight * value);
	vuon.srcy += (vuon.srcheight - vuonheight);
	vuon.srcheight = vuonheight;
	vuon.destheight = vuon.srcheight;
	skin_blitpart(g, &mixer, left + slider.destwidth, 
		slider.destheight - vuon.destheight, &vuon);
}

void cellsize(ParamView* self, int* width, int* height)
{
	psy_ui_TextMetric tm;

	*width = (self->machine->vtable->paramviewoptions(self->machine) & 
		MACHINE_PARAMVIEW_COMPACT) == MACHINE_PARAMVIEW_COMPACT 
			? 67
			: 134;
	*height = 28;
	tm = psy_ui_component_textmetric(&self->component);
	if (*width < tm.tmAveCharWidth * 30) {
		*width = tm.tmAveCharWidth * 30;
	}
	if (*height < tm.tmHeight * 2) {
		*height = tm.tmHeight * 2;
	}
}

void cellposition(ParamView* self, uintptr_t row, uintptr_t col, int* x, int* y)
{
	int width;
	int height;

	cellsize(self, &width, &height);
	*x = (int) (col * width);
	*y = (int) (row * height);
}

void onmousedown(ParamView* self, psy_ui_MouseEvent* ev)
{
	self->tweak = hittest(self, ev->x, ev->y);
	if (self->tweak != -1) {
		self->tweakbase = ev->y;		
		self->tweakval = psy_audio_machine_parametervalue(self->machine,
			self->tweak);
		psy_ui_component_capture(&self->component);
	}
}

int hittest(ParamView* self, int x, int y)
{
	if (self->machine) {
		int width;
		int height;
		uintptr_t row;
		uintptr_t col;
		uintptr_t param;

		cellsize(self, &width, &height);
		row = y / height;
		col = x / width;
		param = (row % paramview_numrows(self)) + col * paramview_numrows(self);
		if (param < psy_audio_machine_numparameters(self->machine)) {
			return param;
		}
	}
	return 0;
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
	if (self->machine) {
		int cellwidth;
		int cellheight;
									
		cellsize(self, &cellwidth, &cellheight);
		rv->width = psy_audio_machine_numparametercols(self->machine) *
			cellwidth;
		rv->height = paramview_numrows(self) * cellheight;
	} else {
		*rv = psy_ui_component_size(&self->component);
	}
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
