// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "paramview.h"
#include <bitmap.h>
#include "resources/resource.h"
#include "skincoord.h"

#define TIMERID_PARAMVIEW 410

static void OnDraw(ParamView* self, ui_component* sender, ui_graphics* g);
static void DrawBackground(ParamView* self, ui_graphics* g);
static void OnSize(ParamView* self, ui_component* sender, int width, int height);
static void DrawParam(ParamView* self, ui_graphics* g, int param, int row, int col);
static void DrawSlider(ParamView* self, ui_graphics* g, int param, int row, int col);
static void DrawLevel(ParamView* self, ui_graphics* g, int param, int row, int col);
static void DrawHeader(ParamView* self, ui_graphics* g, int param, int row, int col);
static void DrawInfoLabel(ParamView*, ui_graphics* g, int param, int row, int col);
static void DrawKnob(ParamView* self, ui_graphics* g, int param, int row, int col);
static void cellsize(ParamView* self, int* width, int* height);
static void cellposition(ParamView* self, int row, int col, int* x, int* y);
static void OnMouseDown(ParamView* self, ui_component* sender, int x, int y, int button);
static void OnMouseUp(ParamView* self, ui_component* sender, int x, int y, int button);
static void OnMouseMove(ParamView* self, ui_component* sender, int x, int y, int button);
static int HitTest(ParamView* self, int x, int y);
static void OnTimer(ParamView*, ui_component* sender, int timerid);
static void blitskinpart(ui_graphics*, ui_bitmap*, int x, int y, SkinCoord*);
static int intparamvalue(float value);
static float floatparamvalue(int value);

static ui_bitmap knobs;
static ui_bitmap mixer;

void InitParamView(ParamView* self, ui_component* parent, Machine* machine)
{		
	if (knobs.hBitmap == NULL) {
		ui_bitmap_loadresource(&knobs, IDB_PARAMKNOB);		
	}
	if (mixer.hBitmap == NULL) {
		ui_bitmap_loadresource(&mixer, IDB_MIXERSKIN);
	}
	self->machine = machine;
	ui_component_init(&self->component, parent);	
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_size, self, OnSize);
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	signal_connect(&self->component.signal_mouseup, self, OnMouseUp);
	signal_connect(&self->component.signal_mousemove,self, OnMouseMove);
	signal_connect(&self->component.signal_timer,self, OnTimer);
	ui_component_move(&self->component, 0, 0);
	ui_component_resize(&self->component, 800, 400);
	if (self->machine) {
		self->numparams = self->machine->numparameters(self->machine);
		self->numcols = self->machine->numcols(self->machine);
		if (self->numcols > 0) {
			self->numrows = self->numparams / self->numcols;
		} else {
			self->numrows = self->numparams;
		}
	} else {		
		self->numparams = 0;
		self->numcols = 0;
		self->numrows = 0;
	}
	self->tweak = -1;
	self->component.doublebuffered = TRUE;
	SetTimer(self->component.hwnd, TIMERID_PARAMVIEW, 100, 0);
}

void OnDraw(ParamView* self, ui_component* sender, ui_graphics* g)
{			
	if (self->machine) {								
		int row = 0;
		int col = 0;
		int param;		

		self->numparams = self->machine->numparameters(self->machine);
		self->numcols = self->machine->numcols(self->machine);

		if (self->numparams > 0 && self->numcols > 0) {
			self->numrows = self->numparams / self->numcols;						
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
	const CMachineParameter* param;

	param = self->machine->parameter(self->machine, par);
	if (param) {
		if (param->Flags == 1) {
			DrawHeader(self, g, par, row, col);
		} else
		if (param->Flags == 3) {
			DrawInfoLabel(self, g, par, row, col);
		} else
		if (param->Flags & MPF_STATE) {
			DrawKnob(self, g, par, row, col);
		} else
		if (param->Flags & MPF_SLIDER) {
			DrawSlider(self, g, par, row, col);
		}
		if (param->Flags & MPF_LEVEL) {
			DrawLevel(self, g, par, row, col);
		}
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

	const CMachineParameter* par;
	par = self->machine->parameter(self->machine, param);
	knob_cx = 28;
	knob_cy = 28;
	cellposition(self, row, col, &left, &top);
	cellsize(self, &width, &height);	

	ui_setrectangle(&r, left, top, width, height);
	ui_setrectangle(&r_top, left + knob_cx, top, width - knob_cx, height / 2);
	ui_setrectangle(&r_bottom, left + knob_cx, top + height/2, width - knob_cx, height / 2);
				
	if (!self->machine->parametername(self->machine, label, param)) {
		if (!self->machine->parameterlabel(self->machine, label, param)) {
			_snprintf(label, 128, "%s", par->Name);
		}
	}
	if (self->machine->describevalue(
		self->machine, str, param, self->machine->value(self->machine, param)) == FALSE) {
		_snprintf(str, 128, "%d", self->machine->value(self->machine, param));
	}
	ui_setbackgroundcolor(g, 0x00555555); // + nc*2);
	ui_settextcolor(g, 0x00CDCDCD); // + nc);	
	ui_textoutrectangle(g, r_top.left, r_top.top,
		ETO_OPAQUE, r_top, label, strlen(label));	
	ui_setbackgroundcolor(g, 0x00444444); // + nc*2);
	ui_settextcolor(g, 0x00E7BD18); // + nc);
	ui_textoutrectangle(g, r_bottom.left, r_bottom.top,
		ETO_OPAQUE, r_bottom, str, strlen(str));		
	knob_frame = (int)((self->machine->value(self->machine, param) - par->MinValue)* 63.0/(par->MaxValue - par->MinValue));
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
	const CMachineParameter* par;
	const char *parName;
	const char *parValue;
	char str[128];
	
	par = self->machine->parameter(self->machine, param);
	if (par) {
		str[0] = '\0';
		parName = par->Name;	
		parValue = str;

		cellposition(self, row, col, &left, &top);
		cellsize(self, &width, &height);		
		half = height/2;
		ui_setrectangle(&r, left, top, width, top + half);

		ui_setbackgroundcolor(g, 0x00232323);
		ui_settextcolor(g, 0x00FFFFFF);
		ui_textoutrectangle(g, 
			left, top, ETO_OPAQUE | ETO_CLIPPED,
			r, parName, strlen(parName));
		ui_setrectangle(&r, left, top + half, width, top + half);
		ui_textoutrectangle(g, 
			left, top, ETO_OPAQUE | ETO_CLIPPED,
			r, parValue, strlen(parValue));	
	}
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
	const CMachineParameter* par;
	const char *parName;
	const char *parValue;
	char str[128];
	
	par = self->machine->parameter(self->machine, param);
	if (par) {
		str[0] = '\0';
		parName = par->Name;	
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
		ui_textoutrectangle(g, 
			left, top, ETO_OPAQUE | ETO_CLIPPED,
			r, parName, strlen(parName));
	}
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
	blitskinpart(g, &mixer, left, top, &slider);
	xoffset = (slider.destwidth - knob.destwidth) / 2;
	value =	(self->machine->value(self->machine, param) / 65535.f);	
	yoffset = (int)((1 - value) * slider.destheight);
	blitskinpart(g, &mixer, left + xoffset, top + yoffset, &knob);	
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
	blitskinpart(g, &mixer, left + slider.destwidth, 
		slider.destheight - vuoff.destheight, &vuoff);			
	value =	(self->machine->value(self->machine, param) / 65535.f);	
	vuonheight = (int)(vuon.srcheight * value);
	vuon.srcy += (vuon.srcheight - vuonheight);
	vuon.srcheight = vuonheight;
	vuon.destheight = vuon.srcheight;
	blitskinpart(g, &mixer, left + slider.destwidth, 
		slider.destheight - vuon.destheight, &vuon);
}

void cellsize(ParamView* self, int* width, int* height)
{
	*width = (self->machine->paramviewoptions(self->machine) & 
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
		self->numparams = self->machine->numparameters(self->machine);
		self->numcols = self->machine->numcols(self->machine);
		if (self->numcols > 0) {
			self->numrows = self->numparams / self->numcols;
		} else {
			self->numrows = self->numparams;
		}
	}
	cellsize(self, &cellwidth, &cellheight);
	*width = self->numcols * cellwidth;
	*height = self->numrows * cellheight;
}

void OnSize(ParamView* self, ui_component* sender, int width, int height)
{
	self->cx = width;
	self->cy = height;
}

void OnMouseDown(ParamView* self, ui_component* sender, int x, int y, int button)
{
	self->tweak = HitTest(self, x, y);
	if (self->tweak != -1) {
		self->tweakbase = y;
		self->tweakval = self->machine->value(self->machine, self->tweak);
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

void OnMouseUp(ParamView* self, ui_component* sender, int x, int y, int button)
{
	if (self->tweak != -1) {
		ui_component_releasecapture();
	}
	self->tweak = -1;
}

void OnMouseMove(ParamView* self, ui_component* sender, int x, int y, int button)
{
	if (self->tweak != -1) {
		const CMachineParameter* param;
		int dy;
		int val;		
				
		self->my = y;
		param = self->machine->parameter(self->machine, self->tweak);
		dy = self->tweakbase - y;
		val = (int)(self->tweakval + (param->MaxValue - param->MinValue) / 100.0 * dy);
		if (val > param->MaxValue) {
			val = param->MaxValue;
		}
		if (val < param->MinValue) {
			val = param->MinValue;
		}
		self->machine->parametertweak(self->machine, self->tweak, val);
		ui_invalidate(&self->component);		
	}
}

void OnTimer(ParamView* self, ui_component* sender, int timerid)
{
	ui_invalidate(&self->component);
}

void blitskinpart(ui_graphics* g, ui_bitmap* bitmap, int x, int y, SkinCoord* coord)
{
	ui_drawbitmap(g, bitmap, x + coord->destx, y + coord->desty,
		coord->destwidth, coord->destheight, coord->srcx, coord->srcy);
}

int intparamvalue(float value)
{	
	return (int)((value * 65535.f));	
}

float floatparamvalue(int value)
{
	return value / 65535.f;	
}
