// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "paramview.h"
#include "skingraphics.h"
#include "resources/resource.h"
#include "skincoord.h"
#include <portable.h>

#define TIMERID_PARAMVIEW 410

static void OnDraw(ParamView* self, ui_component* sender, ui_graphics* g);
static void DrawBackground(ParamView* self, ui_graphics* g);
static void DrawParam(ParamView* self, ui_graphics* g, int param, int row, int col);
static void DrawSlider(ParamView* self, ui_graphics* g, int param, int row, int col);
static void DrawLevel(ParamView* self, ui_graphics* g, int param, int row, int col);
static void DrawHeader(ParamView* self, ui_graphics* g, int param, int row, int col);
static void DrawInfoLabel(ParamView*, ui_graphics* g, int param, int row, int col);
static void DrawKnob(ParamView* self, ui_graphics* g, int param, int row, int col);
static void cellsize(ParamView* self, int* width, int* height);
static void cellposition(ParamView* self, int row, int col, int* x, int* y);
static void OnMouseDown(ParamView* self, ui_component* sender, MouseEvent*);
static void OnMouseUp(ParamView* self, ui_component* sender, MouseEvent*);
static void OnMouseMove(ParamView* self, ui_component* sender, MouseEvent*);
static int HitTest(ParamView* self, int x, int y);
static void OnTimer(ParamView*, ui_component* sender, int timerid);
static int intparamvalue(float value);
static float floatparamvalue(int value);

static ui_bitmap knobs;
static ui_bitmap mixer;

void InitParamView(ParamView* self, ui_component* parent, Machine* machine,
	Workspace* workspace)
{	
	if (knobs.hBitmap == NULL) {
		ui_bitmap_init(&knobs);
		ui_bitmap_loadresource(&knobs, IDB_PARAMKNOB);		
	}
	if (mixer.hBitmap == NULL) {
		ui_bitmap_init(&mixer);
		ui_bitmap_loadresource(&mixer, IDB_MIXERSKIN);
	}
	self->machine = machine;
	self->workspace = workspace;
	ui_component_init(&self->component, parent);	
	psy_signal_connect(&self->component.signal_draw, self, OnDraw);	
	psy_signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	psy_signal_connect(&self->component.signal_mouseup, self, OnMouseUp);
	psy_signal_connect(&self->component.signal_mousemove,self, OnMouseMove);
	psy_signal_connect(&self->component.signal_timer,self, OnTimer);	
	ui_component_resize(&self->component, 800, 400);
	if (self->machine) {
		self->numparams = self->machine->vtable->numparameters(self->machine);
		self->numparametercols =
			self->machine->vtable->numparametercols(self->machine);
		if (self->numparametercols > 0) {
			self->numrows = self->numparams / self->numparametercols;
		} else {
			self->numrows = self->numparams;
		}
	} else {		
		self->numparams = 0;
		self->numparametercols = 0;
		self->numrows = 0;
	}
	self->tweak = -1;
	self->component.doublebuffered = TRUE;
	ui_component_starttimer(&self->component, TIMERID_PARAMVIEW, 100);
}

void OnDraw(ParamView* self, ui_component* sender, ui_graphics* g)
{			
	if (self->machine) {								
		int row = 0;
		int col = 0;
		int param;		

		self->numparams = self->machine->vtable->numparameters(self->machine);
		self->numparametercols = self->machine->vtable->numparametercols(self->machine);

		if (self->numparams > 0 && self->numparametercols > 0) {
			self->numrows = self->numparams / self->numparametercols;						
			for (param = 0; param < self->numparams; ++param) {
				DrawParam(self, g, param, row, col);
				++row;
				if (row >= self->numrows) {
				  row = 0;
				  ++col;
				}
			}
		}
	}			
}

void DrawBackground(ParamView* self, ui_graphics* g)
{

}

void DrawParam(ParamView* self, ui_graphics* g, int par, int row, int col)
{	
	switch (self->machine->vtable->parametertype(self->machine, par)) {
		case 1:
			DrawHeader(self, g, par, row, col);
		break;
		case 3:
			DrawInfoLabel(self, g, par, row, col);
		break;
		case MPF_STATE:
			DrawKnob(self, g, par, row, col);
		break;
		case MPF_SLIDER:
			DrawSlider(self, g, par, row, col);
		break;
		case MPF_LEVEL:
			DrawLevel(self, g, par, row, col);
		break;
		default:
		break;
	}
}

void DrawKnob(ParamView* self, ui_graphics* g, int param, int row, int col)
{	
	char label[128];
	char str[128];
	ui_rectangle r;
	ui_rectangle r_top;
	ui_rectangle r_bottom;	
	int top;
	int left;
	int width;
	int height;
	int knob_cx;
	int knob_cy;
	int knob_frame;
	int minval;
	int maxval;
	
	knob_cx = 28;
	knob_cy = 28;
	cellposition(self, row, col, &left, &top);
	cellsize(self, &width, &height);	

	ui_setrectangle(&r, left, top, width, height);
	ui_setrectangle(&r_top, left + knob_cx, top, width - knob_cx, height / 2);
	ui_setrectangle(&r_bottom, left + knob_cx, top + height/2, width - knob_cx, height / 2);
				
	if (!self->machine->vtable->parametername(self->machine, label, param)) {
		if (!self->machine->vtable->parameterlabel(self->machine, label, param)) {
			psy_snprintf(label, 128, "%s", "");
		}
	}
	if (self->machine->vtable->describevalue(
		self->machine, str, param, self->machine->vtable->parametervalue(self->machine, param)) == FALSE) {
		psy_snprintf(str, 128, "%d", self->machine->vtable->parametervalue(self->machine, param));
	}
	ui_setbackgroundcolor(g, 0x00555555); // + nc*2);
	ui_settextcolor(g, 0x00CDCDCD); // + nc);	
	ui_textoutrectangle(g, r_top.left, r_top.top,
		ETO_OPAQUE, r_top, label, strlen(label));	
	ui_setbackgroundcolor(g, 0x00444444); // + nc*2);
	ui_settextcolor(g, 0x00E7BD18); // + nc);
	ui_textoutrectangle(g, r_bottom.left, r_bottom.top,
		ETO_OPAQUE, r_bottom, str, strlen(str));
	self->machine->vtable->parameterrange(self->machine, param, &minval, &maxval);
	knob_frame = (int)((self->machine->vtable->parametervalue(self->machine, param) - minval) *
		63.0 / (max(1, maxval - minval)));
	ui_drawbitmap(g, &knobs, r.left, r.top, knob_cx, knob_cy, knob_frame*knob_cx, 0);	
}

void DrawInfoLabel(ParamView* self, ui_graphics* g, int param, int row, int col)
{
	int top;
	int left;
	int width;
	int height;
	int half;
	ui_rectangle r;	
	char str[128];
			
	str[0] = '\0';		

	cellposition(self, row, col, &left, &top);
	cellsize(self, &width, &height);		
	half = height/2;
	ui_setrectangle(&r, left, top, width, top + half);

	ui_setbackgroundcolor(g, 0x00232323);
	ui_settextcolor(g, 0x00FFFFFF);
	if (!self->machine->vtable->parametername(self->machine, str, param)) {
		if (!self->machine->vtable->parameterlabel(self->machine, str, param)) {
			psy_snprintf(str, 128, "%s", "");
		}
	}
	ui_textoutrectangle(g, 
		left, top, ETO_OPAQUE | ETO_CLIPPED,
		r, str, strlen(str));
	ui_setrectangle(&r, left, top + half, width, top + half);
	if (self->machine->vtable->describevalue(
		self->machine, str, param, self->machine->vtable->parametervalue(self->machine, param)) == FALSE) {
		psy_snprintf(str, 128, "%d", self->machine->vtable->parametervalue(self->machine, param));
	}
	ui_textoutrectangle(g, 
		left, top + half, ETO_OPAQUE | ETO_CLIPPED,
		r, str, strlen(str));	
}

void DrawHeader(ParamView* self, ui_graphics* g, int param, int row, int col)
{
	int top;
	int left;
	int width;
	int height;
	int half;
	int quarter;
	ui_rectangle r;	
	const char *parValue;
	char str[128];
			
	parValue = str;

	cellposition(self, row, col, &left, &top);
	cellsize(self, &width, &height);		
	half = height/2;
	quarter = half/2;

	// dc.FillSolidRect(x, y, width, quarter,uiSetting->topColor);
	// dc.FillSolidRect(x, y+half+quarter, width, quarter,uiSetting->bottomColor);

	// CFont *oldfont = dc.SelectObject(&uiSetting->font_bold);
	// dc.SetBkColor(uiSetting->titleColor);
	// dc.SetTextColor(uiSetting->fonttitleColor);
	// dc.ExtTextOut(x + xoffset, y + quarter, ETO_OPAQUE | ETO_CLIPPED, CRect(x, y + quarter, x+width, y+half+quarter), CString(parName), 0);
	// dc.SelectObject(oldfont);
	ui_setrectangle(&r, left, top, width, top + half);

	ui_setbackgroundcolor(g, 0x00232323);
	ui_settextcolor(g, 0x00FFFFFF);
	if (!self->machine->vtable->parametername(self->machine, str, param)) {
		if (!self->machine->vtable->parameterlabel(self->machine, str, param)) {
			psy_snprintf(str, 128, "%s", "");
		}
	}

	ui_textoutrectangle(g, 
		left, top, ETO_OPAQUE | ETO_CLIPPED,
		r, str, strlen(str));	
}

void DrawSlider(ParamView* self, ui_graphics* g, int param, int row, int col)
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
	value =	(self->machine->vtable->parametervalue(self->machine, param) / 65535.f);	
	yoffset = (int)((1 - value) * slider.destheight);
	skin_blitpart(g, &mixer, left + xoffset, top + yoffset, &knob);	
}

void DrawLevel(ParamView* self, ui_graphics* g, int param, int row, int col)
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
	value =	(self->machine->vtable->parametervalue(self->machine, param) / 65535.f);	
	vuonheight = (int)(vuon.srcheight * value);
	vuon.srcy += (vuon.srcheight - vuonheight);
	vuon.srcheight = vuonheight;
	vuon.destheight = vuon.srcheight;
	skin_blitpart(g, &mixer, left + slider.destwidth, 
		slider.destheight - vuon.destheight, &vuon);
}

void cellsize(ParamView* self, int* width, int* height)
{
	*width = (self->machine->vtable->paramviewoptions(self->machine) & 
		MACHINE_PARAMVIEW_COMPACT) == MACHINE_PARAMVIEW_COMPACT 
			? 67
			: 134;
	*height = 28;		
}

void cellposition(ParamView* self, int row, int col, int* x, int* y)
{
	int width;
	int height;
	cellsize(self, &width, &height);
	*x = col * width;
	*y = row * height;	
}

void ParamViewSize(ParamView* self, int* width, int* height)
{
	int cellwidth;
	int cellheight;

	if (self->machine) {
		self->numparams = self->machine->vtable->numparameters(self->machine);
		self->numparametercols = self->machine->vtable->numparametercols(self->machine);
		if (self->numparametercols > 0) {
			self->numrows = self->numparams / self->numparametercols;
		} else {
			self->numrows = self->numparams;
		}
	}
	cellsize(self, &cellwidth, &cellheight);
	*width = self->numparametercols * cellwidth;
	*height = self->numrows * cellheight;
}

void OnMouseDown(ParamView* self, ui_component* sender, MouseEvent* ev)
{
	self->tweak = HitTest(self, ev->x, ev->y);
	if (self->tweak != -1) {
		self->tweakbase = ev->y;
		self->tweakval =
			self->machine->vtable->parametervalue(self->machine, self->tweak);
		ui_component_capture(&self->component);
	}
}

int HitTest(ParamView* self, int x, int y)
{
	int width;
	int height;
	int row;
	int col;
	int rv;
	
	cellsize(self, &width, &height);
	row = y / height;
	col = x / width;
	rv = (row % self->numrows) + col*self->numrows;
	return (rv >= 0 && rv < self->numparams) ? rv : -1;
}

void OnMouseUp(ParamView* self, ui_component* sender, MouseEvent* ev)
{
	if (self->tweak != -1) {
		ui_component_releasecapture();
	}
	self->tweak = -1;
}

void OnMouseMove(ParamView* self, ui_component* sender, MouseEvent* ev)
{
	if (self->tweak != -1) {		
		int dy;
		int val;
		int minval;
		int maxval;		
				
		self->my = ev->y;
		self->machine->vtable->parameterrange(self->machine, self->tweak,
			&minval, &maxval);
		dy = self->tweakbase - ev->y;
		val = (int)(self->tweakval +
			(maxval - minval) / 100.0 * dy);
		if (val > maxval) {
			val = maxval;
		}
		if (val < minval) {
			val = minval;
		}
		self->machine->vtable->parametertweak(self->machine, self->tweak, val);		
		workspace_parametertweak(self->workspace,
			self->machine->vtable->slot(self->machine), self->tweak, val - minval);
		ui_component_invalidate(&self->component);		
	}
}

void OnTimer(ParamView* self, ui_component* sender, int timerid)
{
	ui_component_invalidate(&self->component);
}

int intparamvalue(float value)
{	
	return (int)((value * 65535.f));	
}

float floatparamvalue(int value)
{
	return value / 65535.f;	
}
