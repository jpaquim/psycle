#include "vumeter.h"
#include <math.h>

static void OnDraw(Vumeter*, ui_component* sender, ui_graphics*);
static void OnSize(Vumeter*, ui_component* sender, int width, int height);
static void OnDestroy(Vumeter*, ui_component* sender);
static void OnMouseDown(Vumeter*, ui_component* sender, int x, int y, int button);
static void OnTimer(Vumeter*, ui_component* sender, int timerid);
static void OnMasterWorked(Vumeter*, Machine*, BufferContext*);
static void OnSongChanged(Vumeter*, Workspace*);
static void ConnectMachinesSignals(Vumeter*, Workspace*);

void InitVumeter(Vumeter* self, ui_component* parent, Workspace* workspace)
{			
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_destroy, self, OnDestroy);
	signal_connect(&self->component.signal_size, self, OnSize);	
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);	
	signal_connect(&self->component.signal_timer, self, OnTimer);
	SetTimer(self->component.hwnd, 300, 50, 0);	
	self->leftavg = 0;
	self->rightavg = 0;	
	signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
	ConnectMachinesSignals(self, workspace);
}

void OnDestroy(Vumeter* self, ui_component* component)
{		
}

void OnDraw(Vumeter* self, ui_component* sender, ui_graphics* g)
{	
	ui_rectangle left;
	ui_rectangle right;
	ui_size size = ui_component_size(&self->component);
	ui_setrectangle(&left, 0, 5, size.width, 5);
	right = left;
	right.top += 6;
	right.bottom += 6;
	ui_drawsolidrectangle(g, left, 0xFF000000);
	ui_drawsolidrectangle(g, right, 0xFF000000);
	
	left.right = (int) (self->leftavg * size.width);
	right.right = (int) (self->rightavg * size.width);
	ui_drawsolidrectangle(g, left, 0xFF00FF00);
	ui_drawsolidrectangle(g, right, 0xFF00FF00);

	ui_setrectangle(&left, left.right, left.top, size.width - left.right, 5);
	ui_setrectangle(&right, right.right, right.top, size.width - right.right, 5);
	ui_drawsolidrectangle(g, left, 0x003E3E3E);
	ui_drawsolidrectangle(g, right, 0x003E3E3E);
}

void OnSize(Vumeter* self, ui_component* sender, int width, int height)
{
}

void OnMouseDown(Vumeter* self, ui_component* sender, int x, int y, int button)
{
}

void OnTimer(Vumeter* self, ui_component* sender, int timerid)
{	
	ui_invalidate(&self->component);	
}

void OnMasterWorked(Vumeter* self, Machine* master, BufferContext* bc)
{
	real* left = bc->output->samples[0];
	real* right = bc->output->samples[1];
	real leftavg = 0;
	real rightavg = 0;
	char buffer[20];
	unsigned int sample = 0;
	for ( ; sample < bc->numsamples; ++sample) {
		leftavg += (real) fabs(left[sample]);
		rightavg += (real) fabs(right[sample]);
	}
	self->leftavg = leftavg / bc->numsamples / 32768;
	self->rightavg = rightavg / bc->numsamples / 32768;
	_snprintf(buffer, 10, "%.4f, ", self->leftavg);	
}

void OnSongChanged(Vumeter* self, Workspace* workspace)
{
	self->leftavg = 0;
	self->rightavg = 0;
	ConnectMachinesSignals(self, workspace);
}

void ConnectMachinesSignals(Vumeter* self, Workspace* workspace)
{
	if (workspace && workspace->song &&
			machines_master(&workspace->song->machines)) {
		signal_connect(
			&machines_master(&workspace->song->machines)->signal_worked, self,
			OnMasterWorked);
	}
}