// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net
#include "paramview.h"
#include <bitmap.h>
#include "resources/resource.h"

static void Draw(ParamView* self, ui_graphics* g);
static void DrawBackground(ParamView* self, ui_graphics* g);
static void OnSize(ParamView* self, int width, int height);
static void DrawParam(ParamView* self, ui_graphics* g, int param, int row, int col);
static void DrawHeader(ParamView* self, ui_graphics* g, int param, int row, int col);
static void DrawKnob(ParamView* self, ui_graphics* g, int param, int row, int col);
static void cellsize(ParamView* self, int* width, int* height);
static void cellposition(ParamView* self, int row, int col, int* x, int* y);
static void OnMouseDown(ParamView* self, int x, int y, int button);
static void OnMouseUp(ParamView* self, int x, int y, int button);
static void OnMouseMove(ParamView* self, int x, int y, int button);
static int HitTest(ParamView* self, int x, int y);

static ui_bitmap knobs;
static HFONT hfont;
extern HINSTANCE appInstance;

void InitParamView(ParamView* self, ui_component* parent, Machine* machine)
{		
	if (knobs.hBitmap == NULL) {
		knobs.hBitmap = LoadBitmap (appInstance, IDB_PARAMKNOB);		
	}
	if (hfont == NULL) {
		hfont = ui_createfont("Tahoma", 12);
	}
	self->machine = machine;
	ui_component_init(self, &self->component, parent);
	self->component.events.draw = Draw;
	self->component.events.size = OnSize;
	self->component.events.mousedown = OnMouseDown;
	self->component.events.mouseup = OnMouseUp;
	self->component.events.mousemove = OnMouseMove;
	ui_component_move(&self->component, 0, 0);
	ui_component_resize(&self->component, 800, 400);

	if (self->machine && self->machine->info && self->machine->info(self->machine)) {		
		self->params = self->machine->info(self->machine)->Parameters;
		self->numparams = self->machine->info(self->machine)->numParameters;
		self->numcols = self->machine->info(self->machine)->numCols;
		self->numrows = self->numparams / self->numcols;
	} else {
		self->params = 0;
		self->numparams = 0;
		self->numcols = 0;
		self->numrows = 0;
	}
	self->tweak = -1;
	self->component.doublebuffered = TRUE;
}

void Draw(ParamView* self, ui_graphics* g)
{			
	if (self->params) {								
		int row = 0;
		int col = 0;
		int param;

		if (hfont) {
			ui_setfont(g, hfont);
		}
					
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

void DrawBackground(ParamView* self, ui_graphics* g)
{

}

void DrawParam(ParamView* self, ui_graphics* g, int param, int row, int col)
{	
	if (self->params[param]->Flags & MPF_STATE) {
		DrawKnob(self, g, param, row, col);
	} else {
		DrawKnob(self, g, param, row, col);
	}
}

void DrawHeader(ParamView* self, ui_graphics* g, int param, int row, int col)
{
	
}

void DrawKnob(ParamView* self, ui_graphics* g, int param, int row, int col)
{
	const CMachineParameter* par;	
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

	knob_cx = 28;
	knob_cy = 28;
	cellposition(self, row, col, &left, &top);
	cellsize(self, &width, &height);	

	ui_setrectangle(&r, left, top, width, height);
	ui_setrectangle(&r_top, left + knob_cx, top, width - knob_cx, height / 2);
	ui_setrectangle(&r_bottom, left + knob_cx, top + height/2, width - knob_cx, height / 2);
		
	ui_drawrectangle(g, r);
	par = self->params[param];
	if (self->machine->describevalue(
		self->machine, str, param, self->machine->value(self->machine, param)) == FALSE) {
		_snprintf(str, 128, "%d", self->machine->value(self->machine, param));
	}
	ui_setbackgroundcolor(g, 0x00788D93); // + nc*2);
	ui_settextcolor(g, 0x00CCDDEE); // + nc);	
	ui_textoutrectangle(g, r_top.left, r_top.top,
		ETO_OPAQUE, r_top, par->Name, strlen(par->Name));	
	ui_setbackgroundcolor(g, 0x00687D83); // + nc*2);
	ui_settextcolor(g, 0x0044EEFF); // + nc);
	ui_textoutrectangle(g, r_bottom.left, r_bottom.top,
		ETO_OPAQUE, r_bottom, str, strlen(str));		
	knob_frame = (int)((self->machine->value(self->machine, param) - par->MinValue)* 63.0/(par->MaxValue - par->MinValue));
	ui_drawbitmap(g, &knobs, r.left, r.top, knob_cx, knob_cy, knob_frame*knob_cx, 0);	
}

void cellsize(ParamView* self, int* width, int* height)
{
	*width = 134;
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

	cellsize(self, &cellwidth, &cellheight);
	*width = self->numcols * cellwidth;
	*height = self->numrows * cellheight;
}

void OnSize(ParamView* self, int width, int height)
{
	self->cx = width;
	self->cy = height;
}

void OnMouseDown(ParamView* self, int x, int y, int button)
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

void OnMouseUp(ParamView* self, int x, int y, int button)
{
	if (self->tweak != -1) {
		ui_component_releasecapture();
	}
	self->tweak = -1;
}

void OnMouseMove(ParamView* self, int x, int y, int button)
{
	if (self->tweak != -1) {
		const CMachineParameter* param;
		int dy;
		int val;		
				
		self->my = y;
		param = self->params[self->tweak];
		dy = self->tweakbase - y;
		val = (int)(self->tweakval + (param->MaxValue - param->MinValue) / 100.0 * dy);
		if (val > param->MaxValue) {
			val = param->MaxValue;
		}
		if (val < param->MinValue) {
			val = param->MinValue;
		}
		self->machine->parametertweak(self->machine, self->tweak, val);				
		InvalidateRect(self->component.hwnd, NULL, FALSE);
	}
}

