// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "paramview.h"
#include "skingraphics.h"
#include "resources/resource.h"
#include "skincoord.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "../../detail/portable.h"

static void onpreferredsize(ParamView*, psy_ui_Size* limit, psy_ui_Size* rv);
static void ondestroy(ParamView*, psy_ui_Component* sender);
static void ondraw(ParamView*, psy_ui_Graphics*);
static void drawparameter(ParamView*, psy_ui_Graphics*, psy_audio_MachineParam*, uintptr_t paramnum, uintptr_t row, uintptr_t col);
static void drawslider(ParamView*, psy_ui_Graphics*, psy_audio_MachineParam*, uintptr_t paramnum, uintptr_t row, uintptr_t col);
static void drawsliderlevel(ParamView*, psy_ui_Graphics*, psy_audio_MachineParam*, uintptr_t paramnum, uintptr_t row, uintptr_t col);
static void drawslidercheck(ParamView*, psy_ui_Graphics*, psy_audio_MachineParam*, uintptr_t paramnum, uintptr_t row, uintptr_t col);
static void drawswitch(ParamView*, psy_ui_Graphics*, psy_audio_MachineParam*, uintptr_t paramnum, uintptr_t row, uintptr_t col);
static void drawheader(ParamView*, psy_ui_Graphics*, psy_audio_MachineParam*, uintptr_t paramnum, uintptr_t row, uintptr_t col);
static void drawinfolabel(ParamView*, psy_ui_Graphics*, psy_audio_MachineParam*, uintptr_t paramnum, uintptr_t row, uintptr_t col);
static void drawknob(ParamView*, psy_ui_Graphics*, psy_audio_MachineParam*, uintptr_t paramnum, uintptr_t row, uintptr_t col);
static void drawblank(ParamView*, psy_ui_Graphics*, psy_audio_MachineParam*, uintptr_t paramnum, uintptr_t row, uintptr_t col);
static void mpfsize(ParamView*, uintptr_t paramtype, bool small, double* width, double* height);
static void cellsize(ParamView*, uintptr_t row, uintptr_t col, double* width, double* height);
static void cellposition(ParamView*, uintptr_t row, uintptr_t col, double* x, double* y);
static void onmousedown(ParamView*, psy_ui_MouseEvent*);
static void onmouseup(ParamView*, psy_ui_MouseEvent*);
static void onmousemove(ParamView*, psy_ui_MouseEvent*);
static void onmousewheel(ParamView*, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void onmousedoubleclick(ParamView*, psy_ui_MouseEvent*);
static uintptr_t hittest(ParamView*, double x, double y);
static void ontimer(ParamView*, psy_ui_Component* sender, uintptr_t timerid);
static uintptr_t paramview_numrows(ParamView*);
static void paramview_computepositions(ParamView*);
static void paramview_clearpositions(ParamView*);
static void mixer_vumeterdraw(ParamView*, psy_ui_Graphics*, double x, double y,
	float value);
static void drawsliderlevelback(ParamView*, psy_ui_Graphics*, double x, double y);
static psy_audio_MachineParam* tweakparam(ParamView*);

static intptr_t paramview_refcount = 0;
static intptr_t paramskin_initialized = 0;
static ParamSkin paramskin;

static void paramskin_init(ParamView*);
static void paramskin_release(ParamView*);

void paramskin_init(ParamView* self)
{
	if (!paramskin_initialized) {
		psy_Property* theme;
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
		theme = self->workspace->config.macparam.theme;
		paramskin.topcolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguitopcolour", 0x00555555));
		paramskin.fonttopcolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguifonttopcolour", 0x00CDCDCD));
		paramskin.bottomcolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguibottomcolour", 0x00444444));
		paramskin.fontbottomcolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguifontbottomcolour", 0x00E7BD18));
		//highlighted param colours
		paramskin.htopcolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguihtopcolour", 0x00555555));
		paramskin.fonthtopcolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguihfonttopcolour", 0x00CDCDCD));
		paramskin.hbottomcolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguihbottomcolour", 0x00292929));
		paramskin.fonthbottomcolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguihfontbottomcolour", 0x00E7BD18));

		paramskin.titlecolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguititlecolour", 0x00292929));
		paramskin.fonttitlecolour = psy_ui_colour_make(psy_property_at_colour(theme, "machineguititlefontcolour", 0x00B4B4B4));
		psy_ui_bitmap_init(&paramskin.knobbitmap);
		if (!machineparamconfig_dialbpm(psycleconfig_macparam(workspace_conf(self->workspace)))) {
			psy_ui_bitmap_loadresource(&paramskin.knobbitmap, IDB_PARAMKNOB);
		} else if (psy_ui_bitmap_load(&paramskin.knobbitmap,
			machineparamconfig_dialbpm(psycleconfig_macparam(workspace_conf(self->workspace)))) != PSY_OK) {
			psy_ui_bitmap_loadresource(&paramskin.knobbitmap, IDB_PARAMKNOB);			
		}
		psy_ui_bitmap_init(&paramskin.mixerbitmap);
		psy_ui_bitmap_loadresource(&paramskin.mixerbitmap, IDB_MIXERSKIN);		
	}
	self->skin = &paramskin;
	psy_ui_component_setbackgroundcolour(&self->component, self->skin->bottomcolour);
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
static bool vtable_initialized = FALSE;

static void vtable_init(ParamView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw = (psy_ui_fp_component_ondraw)ondraw;
		vtable.onmousedown = (psy_ui_fp_component_onmousedown)onmousedown;
		vtable.onmousemove = (psy_ui_fp_component_onmousemove)onmousemove;
		vtable.onmouseup = (psy_ui_fp_component_onmouseup)onmouseup;
		vtable.onmousedoubleclick = (psy_ui_fp_component_onmousedoubleclick)onmousedoubleclick;
		vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)onpreferredsize;
		vtable_initialized = TRUE;
	}
}

void paramview_init(ParamView* self, psy_ui_Component* parent, psy_audio_Machine* machine,
	Workspace* workspace)
{
	psy_Property* pv;

	psy_ui_component_init(&self->component, parent);	
	vtable_init(self);
	self->component.vtable = &vtable;	
	++paramview_refcount;
	self->workspace = workspace;
	self->machine = machine;
	self->numparams = 0;
	psy_ui_component_doublebuffer(&self->component);
	pv = psy_property_findsection(&self->workspace->config.config, "visual.paramview");
	if (pv) {
		psy_ui_Font font;

		psy_ui_fontinfo_init_string(&self->fontinfo,
			psy_property_at_str(pv, "font", "tahoma;-16"));
		psy_ui_font_init(&font, &self->fontinfo);
		psy_ui_component_setfont(&self->component, &font);
		psy_ui_font_dispose(&font);
	} else {
		psy_ui_fontinfo_init_string(&self->fontinfo, "tahoma;-16");
	}
	paramskin_init(self);	
	psy_table_init(&self->positions);
	self->tweak = psy_INDEX_INVALID;
	self->lasttweak = psy_INDEX_INVALID;
	self->sizechanged = 1;
	psy_signal_connect(&self->component.signal_destroy, self, ondestroy);		
	psy_signal_connect(&self->component.signal_timer, self, ontimer);
	psy_signal_connect(&self->component.signal_mousewheel, self,
		onmousewheel);
	psy_ui_component_starttimer(&self->component, 0, 50);
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
		uintptr_t paramnum;		
						
		if (self->numparams != psy_audio_machine_numparameters(self->machine)) {
				self->numparams = psy_audio_machine_numparameters(self->machine);
			paramview_computepositions(self);
			self->sizechanged = 1;
			psy_signal_emit(&self->component.signal_preferredsizechanged, self, 0);
		}
		numrows = paramview_numrows(self);
		for (paramnum = 0; paramnum < psy_audio_machine_numparameters(self->machine);
				++paramnum) {			
			psy_audio_MachineParam* machineparam;

			machineparam = psy_audio_machine_parameter(self->machine, paramnum);
			if (machineparam) {
				drawparameter(self, g, machineparam, paramnum, row, col);
			}
			++row;
			if (row >= numrows) {
			  row = 0;
			  ++col;			
			}
		}
	}			
}

void drawparameter(ParamView* self, psy_ui_Graphics* g, psy_audio_MachineParam* param,
	uintptr_t paramnum, uintptr_t row, uintptr_t col)
{		
	switch (psy_audio_machine_parameter_type(self->machine, param) & ~MPF_SMALL) {
		case MPF_HEADER:
			drawheader(self, g, param, paramnum, row, col);
		break;
		case MPF_INFOLABEL:
			drawinfolabel(self, g, param, paramnum, row, col);
		break;
		case MPF_STATE:
			drawknob(self, g, param, paramnum, row, col);
		break;
		case MPF_SLIDER:				
			drawslider(self, g, param, paramnum, row, col);
		break;
		case MPF_SLIDERCHECK:
			drawslidercheck(self, g, param, paramnum, row, col);
		break;
		case MPF_SWITCH:
			drawswitch(self, g, param, paramnum, row, col);
		break;
		case MPF_SLIDERLEVEL:
			drawsliderlevel(self, g, param, paramnum, row, col);
		break;
		case MPF_NULL:
			drawblank(self, g, param, paramnum, row, col);
		break;
		default:
		break;
	}
}

void drawknob(ParamView* self, psy_ui_Graphics* g, psy_audio_MachineParam* param,
	uintptr_t paramnum, uintptr_t row, uintptr_t col)
{	
	char label[128];
	char str[128];
	psy_ui_RealRectangle r;
	psy_ui_RealRectangle r_top;
	psy_ui_RealRectangle r_bottom;	
	double top;
	double left;
	double width;
	double height;
	double knob_cx;
	double knob_cy;
	double w;
	double h;
	
	knob_cx = psy_ui_realrectangle_width(&self->skin->knob.dest);
	knob_cy = psy_ui_realrectangle_height(&self->skin->knob.dest);
	mpfsize(self, MPF_STATE, FALSE, &w, &h);	
	cellposition(self, row, col, &left, &top);
	cellsize(self, row, col, &width, &height);
	psy_ui_setrectangle(&r, left, top, width, height);
	psy_ui_setrectangle(&r_top, left + knob_cx, top, width - knob_cx,
		height / 2);
	psy_ui_drawsolidrectangle(g, r, self->skin->topcolour);
	psy_ui_setrectangle(&r_bottom, left + knob_cx, top + height / 2,
		width - knob_cx, height / 2);				
	psy_ui_drawsolidrectangle(g, r, self->skin->bottomcolour);
	if (!psy_audio_machine_parameter_name(self->machine, param, label)) {
		if (!psy_audio_machine_parameter_label(self->machine, param, label)) {
			psy_snprintf(label, 128, "%s", "");
		}
	}
	if (psy_audio_machine_parameter_describe(self->machine, param, str) == FALSE) {
		psy_snprintf(str, 128, "%d",
			(int) psy_audio_machineparam_scaledvalue(param));
	}
	psy_ui_setbackgroundcolour(g, (self->tweak == paramnum)
		? self->skin->htopcolour : self->skin->topcolour);
	psy_ui_settextcolour(g, (self->tweak == paramnum)
		? self->skin->fonthtopcolour : self->skin->fonttopcolour);
	psy_ui_textoutrectangle(g, r_top.left, r_top.top,
		psy_ui_ETO_OPAQUE, r_top, label, strlen(label));	
	psy_ui_setbackgroundcolour(g, (self->tweak == paramnum)
		? self->skin->hbottomcolour : self->skin->bottomcolour);
	psy_ui_settextcolour(g, (self->tweak == paramnum)
		? self->skin->fonthbottomcolour : self->skin->fontbottomcolour);
	psy_ui_textoutrectangle(g, r_bottom.left, r_bottom.top,
		psy_ui_ETO_OPAQUE, r_bottom, str, strlen(str));
	if (!psy_ui_bitmap_empty(&self->skin->knobbitmap)) {
		intptr_t knob_frame;

		knob_frame = (intptr_t)
			(psy_audio_machine_parameter_normvalue(self->machine, param) * 63.f);
		if (h < knob_cy) {
			double ratio;

			ratio = h / (double)knob_cy;
			w = ratio * knob_cx;
			psy_ui_drawstretchedbitmap(g, &self->skin->knobbitmap,
				psy_ui_realrectangle_make(
					psy_ui_realrectangle_topleft(&r),
					psy_ui_realsize_make(w, h)),
					psy_ui_realpoint_make(knob_frame * knob_cx, 0.0),
					psy_ui_realsize_make(knob_cx, knob_cy));
		} else {
			psy_ui_drawbitmap(g, &self->skin->knobbitmap,
				psy_ui_realrectangle_make(
					psy_ui_realrectangle_topleft(&r),
					psy_ui_realsize_make(knob_cx, knob_cy)),
				psy_ui_realpoint_make(knob_frame * knob_cx, 0));
		}

	}
	psy_ui_setcolour(g, psy_ui_colour_make(0x00232323));
	psy_ui_drawline(g, psy_ui_realpoint_make(r.left, r.bottom - 1),
		psy_ui_realpoint_make(r.right, r.bottom - 1));
	psy_ui_drawline(g, psy_ui_realpoint_make(r.right - 1, r.top),
		psy_ui_realpoint_make(r.right - 1, r.bottom - 1));
}

void drawheader(ParamView* self, psy_ui_Graphics* g, psy_audio_MachineParam* param,
	uintptr_t paramnum, uintptr_t row, uintptr_t col)
{
	double top;
	double left;
	double width;
	double height;
	double half;
	double quarter;
	psy_ui_RealRectangle r;	
	const char* parValue;
	char str[128];

	cellposition(self, row, col, &left, &top);
	cellsize(self, row, col, &width, &height);
	parValue = str;	
	half = height / 2;
	quarter = half / 2;	
	psy_ui_setrectangle(&r, left, top, width, top + quarter);
	psy_ui_drawsolidrectangle(g, r, self->skin->topcolour);
	psy_ui_setrectangle(&r, left, top+ half + quarter, width, quarter);
	psy_ui_drawsolidrectangle(g, r, self->skin->bottomcolour);
	if (!psy_audio_machine_parameter_name(self->machine, param, str)) {
		if (!psy_audio_machine_parameter_label(self->machine, param, str)) {
			psy_snprintf(str, 128, "%s", "");
		}
	}	
	psy_ui_setrectangle(&r, left, top + quarter, width, half);
	psy_ui_setbackgroundcolour(g, self->skin->titlecolour);
	psy_ui_settextcolour(g, self->skin->fonttitlecolour);
	// font_bold
	psy_ui_textoutrectangle(g, left, top + quarter,
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED, r, str,
		strlen(str));
}

void drawinfolabel(ParamView* self, psy_ui_Graphics* g, psy_audio_MachineParam* param,
	uintptr_t paramnum, uintptr_t row, uintptr_t col)
{
	double top;
	double left;
	double width;
	double height;
	double half;
	psy_ui_RealRectangle r;
	char str[128];
			
	str[0] = '\0';
	cellposition(self, row, col, &left, &top);
	cellsize(self, row, col, &width, &height);
	half = height/2;
	psy_ui_setrectangle(&r, left, top, width, top + half);
	psy_ui_setbackgroundcolour(g, self->skin->titlecolour);
	psy_ui_settextcolour(g, self->skin->fonttitlecolour);
	if (!psy_audio_machine_parameter_name(self->machine, param, str)) {
		if (!psy_audio_machine_parameter_label(self->machine, param, str)) {
			psy_snprintf(str, 128, "%s", "");
		}
	}
	psy_ui_textoutrectangle(g, 
		left, top, psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, str, strlen(str));	
	if (psy_audio_machine_parameter_describe(self->machine, param, str) == FALSE) {
		psy_snprintf(str, 128, "%s", "");
	}
	psy_ui_setbackgroundcolour(g, self->skin->bottomcolour);
	psy_ui_settextcolour(g, self->skin->fontbottomcolour);
	psy_ui_setrectangle(&r, left, top + half, width, top + half);
	psy_ui_textoutrectangle(g, left, top + half,
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, str, strlen(str));	
}

void drawslider(ParamView* self, psy_ui_Graphics* g, psy_audio_MachineParam* param,
	uintptr_t paramnum, uintptr_t row, uintptr_t col)
{
	double top;
	double left;
	double width;
	double height;
	double xoffset;
	double yoffset;
	double value;
	
	psy_ui_RealRectangle r;
	char str[128];
	str[0] = '\0';

	cellposition(self, row, col, &left, &top);
	cellsize(self, row, col, &width, &height);
	psy_ui_setrectangle(&r, left, top, width, height);
	psy_ui_drawsolidrectangle(g, r, self->skin->bottomcolour);
	skin_blitpart(g, &self->skin->mixerbitmap,
		psy_ui_realpoint_make(left, top), &self->skin->slider);
	xoffset = (psy_ui_realrectangle_width(&self->skin->slider.dest) -
		psy_ui_realrectangle_width(&self->skin->knob.dest)) / 2;
	value =	psy_audio_machine_parameter_normvalue(self->machine, param);
	yoffset = ((1.0 - value) *
		(psy_ui_realrectangle_height(&self->skin->slider.dest) -
		psy_ui_realrectangle_height(&self->skin->sliderknob.dest)));
	skin_blitpart(g, &self->skin->mixerbitmap,
		psy_ui_realpoint_make(left + xoffset, top + yoffset),
		&self->skin->sliderknob);
			
	if (psy_audio_machine_parameter_name(self->machine, param, str) != FALSE) {
		psy_ui_setbackgroundcolour(g, self->skin->topcolour);
		psy_ui_settextcolour(g, self->skin->fonttopcolour);
		psy_ui_setrectangle(&r, left + 32, top + self->skin->slider.dest.bottom - self->skin->slider.dest.top - 48, width - 32, 24);
		psy_ui_textoutrectangle(g, left + 32, top + self->skin->slider.dest.bottom - self->skin->slider.dest.top - 48,
			psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
			r, str, strlen(str));
	}	
	psy_ui_setrectangle(&r, left + 32, top + psy_ui_realrectangle_height(&self->skin->slider.dest) - 24,
		width - 32, 24);
	if (psy_audio_machine_parameter_describe(self->machine, param, str) == FALSE) {
		psy_snprintf(str, 128, "%d",
			psy_audio_machine_parameter_normvalue(self->machine, param));
	}
	psy_ui_setbackgroundcolour(g, self->skin->bottomcolour);
	psy_ui_settextcolour(g, self->skin->fontbottomcolour);
	psy_ui_textoutrectangle(g, left + 32, top + psy_ui_realrectangle_height(&self->skin->slider.dest) - 24,
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, str, strlen(str));
}

void drawsliderlevel(ParamView* self, psy_ui_Graphics* g, psy_audio_MachineParam* param,
	uintptr_t paramnum, uintptr_t row, uintptr_t col)
{
	double top;
	double left;
	double width;
	double height;
		
	cellposition(self, row, col, &left, &top);
	cellsize(self, row, col, &width, &height);
	drawsliderlevelback(self, g, left, top);
	mixer_vumeterdraw(self, g, left, top,
		psy_audio_machineparam_normvalue(param));		
}

void mixer_vumeterdraw(ParamView* self, psy_ui_Graphics* g, double x, double y, float value)
{	
	double ypos;

	if (value < 0.f) value = 0.f;
	if (value > 1.f) value = 1.f;
	ypos = (1.f - value) * psy_ui_realrectangle_height(&self->skin->vuoff.dest);
	psy_ui_drawbitmap(g, &self->skin->mixerbitmap,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(x, y + ypos),
			psy_ui_realsize_make(
				psy_ui_realrectangle_width(&self->skin->vuoff.dest),
				psy_ui_realrectangle_height(&self->skin->vuoff.dest) - ypos)),
		psy_ui_realrectangle_topleft(&self->skin->vuon.src));
}

void drawsliderlevelback(ParamView* self, psy_ui_Graphics* g, double x, double y)
{
	psy_ui_drawbitmap(g, &self->skin->mixerbitmap,
		psy_ui_realrectangle_make(
			psy_ui_realpoint_make(x, y),
			psy_ui_realrectangle_size(&self->skin->vuoff.dest)),
		psy_ui_realrectangle_topleft(&self->skin->vuoff.src));
}

void drawslidercheck(ParamView* self, psy_ui_Graphics* g, psy_audio_MachineParam* param,
	uintptr_t paramnum, uintptr_t row, uintptr_t col)
{
	double top;
	double left;
	double width;
	double height;
	double centery;
	psy_ui_RealRectangle r;
	char label[512];
	const psy_ui_TextMetric* tm;

	cellposition(self, row, col, &left, &top);
	cellsize(self, row, col, &width, &height);
	centery = (height - psy_ui_realrectangle_height(&self->skin->checkoff.dest)) / 2;
	if (psy_audio_machine_parameter_normvalue(self->machine, param) == 0.f) {
		skin_blitpart(g, &self->skin->mixerbitmap,
			psy_ui_realpoint_make(left, top + centery), &self->skin->checkoff);
	} else {
		skin_blitpart(g, &self->skin->mixerbitmap,
			psy_ui_realpoint_make(left, top + centery), & self->skin->checkon);
	}
	psy_ui_setrectangle(&r, left + 20, top, width, height);
	if (!psy_audio_machine_parameter_name(self->machine, param, label)) {
		if (!psy_audio_machineparam_label(param, label)) {
			psy_snprintf(label, 128, "%s", "");
		}
	}
	tm = psy_ui_component_textmetric(&self->component);
	centery = (height - tm->tmHeight) / 2;
	psy_ui_textoutrectangle(g, left + 20, top + centery,
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, label, strlen(label));
}

void drawswitch(ParamView* self, psy_ui_Graphics* g, psy_audio_MachineParam* param,
	uintptr_t paramnum, uintptr_t row, uintptr_t col)
{
	double top;
	double left;
	double width;
	double height;
	psy_ui_RealRectangle r;
	char label[512];

	cellposition(self, row, col, &left, &top);
	cellsize(self, row, col, &width, &height);
	psy_ui_setrectangle(&r, left, top, width, height);
	psy_ui_drawsolidrectangle(g, r, self->skin->bottomcolour);
	if (psy_audio_machine_parameter_normvalue(self->machine, param) == 0.f) {
		skin_blitpart(g, &self->skin->mixerbitmap,
			psy_ui_realpoint_make(left, top), &self->skin->switchoff);
	}
	else {
		skin_blitpart(g, &self->skin->mixerbitmap,
			psy_ui_realpoint_make(left, top), &self->skin->switchon);
	}	
	if (!psy_audio_machine_parameter_name(self->machine, param, label)) {
		if (!psy_audio_machine_parameter_label(self->machine, param, label)) {
			psy_snprintf(label, 128, "%s", "");
		}
	}
	psy_ui_setbackgroundcolour(g, self->skin->topcolour);
	psy_ui_settextcolour(g, self->skin->fonttopcolour);
	psy_ui_setrectangle(&r, left + psy_ui_realrectangle_width(&self->skin->switchon.dest), top, width, height / 2);
	psy_ui_textoutrectangle(g, left + psy_ui_realrectangle_width(&self->skin->switchon.dest), top,
		psy_ui_ETO_OPAQUE | psy_ui_ETO_CLIPPED,
		r, label, strlen(label));
}

void drawblank(ParamView* self, psy_ui_Graphics* g, psy_audio_MachineParam* param,
	uintptr_t paramnum, uintptr_t row, uintptr_t col)
{
	double top;
	double left;
	double width;
	double height;
	psy_ui_RealRectangle r;

	cellposition(self, row, col, &left, &top);
	cellsize(self, row, col, &width, &height);
	psy_ui_setrectangle(&r, left, top, width, height);
	psy_ui_drawsolidrectangle(g, r, self->skin->bottomcolour);
}

void mpfsize(ParamView* self, uintptr_t paramtype, bool small, double* width, double* height)
{	
	const psy_ui_TextMetric* tm;
	static float SMALLDIV = 2.f;

	switch (paramtype) {
	case MPF_IGNORE:
		*height = 0;
		*width = 0;
		break;
	case MPF_SLIDERCHECK:
		tm = psy_ui_component_textmetric(&self->component);
		*height = psy_max(psy_ui_realrectangle_height(&self->skin->checkoff.dest),
			tm->tmHeight);
		*width = psy_ui_realrectangle_height(&self->skin->checkoff.dest) +
			tm->tmAveCharWidth * 5;
		break;
	case MPF_SLIDER:
		*height = psy_ui_realrectangle_height(&self->skin->slider.dest);
		*width = psy_ui_realrectangle_width(&self->skin->slider.dest);
		tm = psy_ui_component_textmetric(&self->component);
		if (*width < tm->tmAveCharWidth * 30) {
			*width = tm->tmAveCharWidth * 30;
		}
		if (small) {
			*width = *width / SMALLDIV;
		}
		if (*width < psy_ui_realrectangle_width(&self->skin->vuon.dest) +
			psy_ui_realrectangle_width(&self->skin->checkoff.dest) + 50 +
			tm->tmAveCharWidth * 5) {
			*width = psy_ui_realrectangle_width(&self->skin->vuon.dest) +
				psy_ui_realrectangle_width(&self->skin->checkoff.dest) + 50 +
				tm->tmAveCharWidth * 5;
		}
		break;
		case MPF_SLIDERLEVEL:
			*height = psy_ui_realrectangle_height(&self->skin->vuon.dest);
			*width = psy_ui_realrectangle_width(&self->skin->slider.dest);
			if (small) {
				*width = *width / SMALLDIV;
			}
		break;		
		default:
			tm = psy_ui_component_textmetric(&self->component);
			*width = tm->tmAveCharWidth * 30;			
			*height = tm->tmHeight * 2;			
			if (small) {
				*width = *width / SMALLDIV;
			}			
		break;
	}
}

void cellsize(ParamView* self, uintptr_t row, uintptr_t col, double* width, double* height)
{
	psy_ui_RealRectangle* position;

	position = (psy_ui_RealRectangle*) psy_table_at(&self->positions,
		paramview_numrows(self) * col + row);
	if (position) {
		*width = position->right - position->left;
		*height = position->bottom - position->top;
	} else {
		*width = 0;
		*height = 0;
	}	
}

void cellposition(ParamView* self, uintptr_t row, uintptr_t col, double* x, double* y)
{
	psy_ui_RealRectangle* position;

	position = (psy_ui_RealRectangle*) psy_table_at(&self->positions,
		paramview_numrows(self) * col + row);
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
	self->lasttweak = hittest(self, ev->x, ev->y);
	if (ev->button == 1) {
		psy_audio_MachineParam* param;
		
		self->tweak = self->lasttweak;
		param = tweakparam(self);
		if (param) {
			uintptr_t paramtype;

			self->tweakbase = (float)ev->y;
			self->tweakval = psy_audio_machine_parameter_normvalue(self->machine, param);
			paramtype = psy_audio_machine_parameter_type(self->machine, param) & ~MPF_SMALL;
			if (paramtype == MPF_SLIDERCHECK || paramtype == MPF_SWITCH) {
				if (self->tweakval == 0.f) {
					psy_audio_machine_parameter_tweak(self->machine, param, 1.f);
				} else {
					psy_audio_machine_parameter_tweak(self->machine, param, 0.f);
				}
			}
			psy_ui_component_capture(&self->component);
		}
	}
}

psy_audio_MachineParam* tweakparam(ParamView* self)
{
	psy_audio_MachineParam* rv = NULL;
	
	if (self->machine && self->tweak != psy_INDEX_INVALID) {
		rv = psy_audio_machine_parameter(self->machine, self->tweak);
	}
	return rv;
}

uintptr_t hittest(ParamView* self, double x, double y)
{
	uintptr_t rv = psy_INDEX_INVALID;

	if (self->machine) {
		uintptr_t paramnum;

		for (paramnum = 0;	paramnum < psy_table_size(&self->positions);
				++paramnum) {			
			psy_audio_MachineParam* param;
			
			param = psy_audio_machine_parameter(self->machine, paramnum);
			if (param && ((psy_audio_machine_parameter_type(self->machine, param) & MPF_IGNORE)
					!= MPF_IGNORE)) {
				psy_ui_RealRectangle* position;

				position = (psy_ui_RealRectangle*) psy_table_at(&self->positions,
					paramnum);
				if (position && psy_ui_realrectangle_intersect(position, x, y)) {
					rv = paramnum;
				}
				// break;
			}
		}
	}	
	return rv;
}

void onmousemove(ParamView* self, psy_ui_MouseEvent* ev)
{
	psy_audio_MachineParam* param;

	param = tweakparam(self);
	if (param != NULL) {
		float val = 0;
						
		val = self->tweakval + (self->tweakbase - (float)ev->y) / 200.f;
		if (val > 1.f) {
			val = 1.f;
		} else
		if (val < 0.f) {
			val = 0.f;
		}		
		psy_audio_machine_parameter_tweak(self->machine, param, val);
		psy_ui_component_invalidate(&self->component);
	}
}

void onmouseup(ParamView* self, psy_ui_MouseEvent* ev)
{	
	psy_ui_component_releasecapture(&self->component);
	self->tweak = psy_INDEX_INVALID;
}

void onmousewheel(ParamView* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	psy_audio_MachineParam* param;

	self->tweak = hittest(self, ev->x, ev->y);
	param = tweakparam(self);
	if (param != NULL) {
		self->tweakval = psy_audio_machine_parameter_normvalue(self->machine, param);
		if (ev->delta > 0) {
			float val;
			
			val = self->tweakval + 1 / 200.f;
			if (val > 1.f) {
				val = 1.f;
			}
			else
				if (val < 0.f) {
					val = 0.f;
				}
			psy_audio_machine_parameter_tweak(self->machine, param, val);			
			psy_ui_component_invalidate(&self->component);
		} else
		if (ev->delta < 0) {
			float val;

			val = self->tweakval - 1 / 200.f;
			if (val > 1.f) {
				val = 1.f;
			} else
			if (val < 0.f) {
				val = 0.f;
			}
			psy_audio_machine_parameter_tweak(self->machine, param, val);			
			psy_ui_component_invalidate(&self->component);
		}
	}
	ev->preventdefault = 1;
	self->tweak = psy_INDEX_INVALID;
}

void onmousedoubleclick(ParamView* self, psy_ui_MouseEvent* ev)
{
	if (self->machine) {
		psy_audio_MachineParam* tweakpar;
		uintptr_t paramindex;

		paramindex = hittest(self, ev->x, ev->y);
		if (paramindex != psy_INDEX_INVALID) {
			tweakpar = psy_audio_machine_parameter(self->machine, paramindex);
			if (tweakpar) {
				psy_audio_machine_parameter_reset(self->machine, tweakpar);
			}
		}
	}
}

void ontimer(ParamView* self, psy_ui_Component* sender, uintptr_t timerid)
{
	psy_ui_component_invalidate(&self->component);
}

void onpreferredsize(ParamView* self, psy_ui_Size* limit, psy_ui_Size* rv)
{	
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
		double colmax = 0;
		double cpx = 0;
		double cpy = 0;
		double cpy_slidercheck;
		const psy_ui_TextMetric* tm;

		tm = psy_ui_component_textmetric(&self->component);
		numrows = paramview_numrows(self);
		self->cpmax.width = psy_ui_value_makepx(0);
		for (param = 0; param < psy_audio_machine_numparameters(self->machine);
			++param) {
			psy_ui_RealRectangle* position;
			psy_ui_RealRectangle* firstrow;
			uintptr_t paramtype;
			bool small;
			double width;
			double height;
			psy_audio_MachineParam* machineparam;

			machineparam = psy_audio_machine_parameter(self->machine, param);
			if (machineparam) {
				paramtype = psy_audio_machine_parameter_type(self->machine, machineparam) & ~MPF_SMALL;
				small = (psy_audio_machine_parameter_type(self->machine, machineparam) & MPF_SMALL) == MPF_SMALL;
			} else {
				paramtype = MPF_IGNORE;
				small = TRUE;
			}
			position = (psy_ui_RealRectangle*)malloc(sizeof(psy_ui_RealRectangle));
			psy_ui_setrectangle(position, cpx, 0, 0, 0);
			if (paramtype == MPF_SLIDERLEVEL) {
				position->left += psy_ui_realrectangle_width(&self->skin->slider.dest);
			} else
				if (paramtype == MPF_SLIDERCHECK) {
					position->left += 50;
				}
			psy_table_insert(&self->positions, param, position);
			mpfsize(self, paramtype, small, &width, &height);
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
			if (row >= numrows || param == psy_audio_machine_numparameters(self->machine) - 1) {
				uintptr_t i;

				for (i = paramrowbegin; i <= param; ++i) {
					psy_audio_MachineParam* machineparam;

					machineparam = psy_audio_machine_parameter(self->machine, i);
					position = psy_table_at(&self->positions, i);
					if (machineparam) {
						if (psy_audio_machine_parameter_type(self->machine, machineparam)
								== MPF_SLIDERCHECK ||
							psy_audio_machine_parameter_type(self->machine, machineparam)
								== MPF_SLIDERLEVEL) {
							cellsize(self, i, col, &width, &height);
							position->right = position->left + width;
						} else {
							position->right = cpx + colmax;
						}
					}
				}
				cpx += colmax;
				if (psy_ui_value_px(&self->cpmax.width, tm) < cpx) {
					self->cpmax.width = psy_ui_value_makepx(cpx);
				}
				++col;
				row = 0;				
				paramrowbegin = param + 1;
			}
		}
		cpy_slidercheck = 0;	
		row = 0;
		self->cpmax.height = psy_ui_value_makepx(0);
		for (param = 0; param < psy_audio_machine_numparameters(self->machine);
				++param) {
			psy_ui_RealRectangle* position;
			psy_ui_RealRectangle* firstrow;
			double w, h;
			double height;
			uintptr_t paramtype;
			bool small;
			psy_audio_MachineParam* machineparam;
			
			machineparam = psy_audio_machine_parameter(self->machine, param);
			position = psy_table_at(&self->positions, param);
			firstrow = psy_table_at(&self->positions, row);
			if (machineparam) {
				paramtype = psy_audio_machine_parameter_type(self->machine, machineparam) & ~MPF_SMALL;
				small = (psy_audio_machine_parameter_type(self->machine, machineparam) & MPF_SMALL) == MPF_SMALL;
			} else {
				paramtype = MPF_IGNORE;
				small = TRUE;
			}
			mpfsize(self, paramtype, small, &w, &h);
			height = firstrow->bottom - firstrow->top;
			position->top = cpy;
			if (paramtype == MPF_SLIDERCHECK) {								
				position->top += cpy_slidercheck;
				position->bottom = position->top + h;
			} else {
				position->bottom = cpy + height;				
			}
			if (psy_ui_value_px(&self->cpmax.height, tm) < cpy + height) {
				self->cpmax.height = psy_ui_value_makepx(cpy + height);
			}
			if (paramtype != MPF_IGNORE && paramtype != MPF_SLIDER &&
					paramtype != MPF_SLIDERCHECK) {
				cpy += height;
			}
			if (paramtype == MPF_SLIDERCHECK) {
				cpy_slidercheck += h;
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
	psy_table_disposeall(&self->positions, (psy_fp_disposefunc)NULL);	
	psy_table_init(&self->positions);
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

void paramview_setzoom(ParamView* self, double zoomrate)
{
	psy_ui_Font font;				
	psy_ui_FontInfo fontinfo;

	fontinfo = self->fontinfo;
	fontinfo.lfHeight = (int32_t)(self->fontinfo.lfHeight * zoomrate);
	psy_ui_font_init(&font, &fontinfo);
	psy_ui_component_setfont(&self->component, &font);
	psy_ui_font_dispose(&font);
	paramview_computepositions(self);
}
