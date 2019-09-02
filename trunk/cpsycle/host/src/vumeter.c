#include "vumeter.h"
#include <math.h>

static void OnDraw(Vumeter* self, ui_component* sender, ui_graphics* g);
static void OnSize(Vumeter* self, ui_component* sender, int width, int height);
static void OnDestroy(Vumeter* self, ui_component* component);
static void OnMouseDown(Vumeter* self, ui_component* sender, int x, int y, int button);
static void OnTimer(Vumeter* self, ui_component* sender, int timerid);
static void OnMasterWorked(Vumeter* self, Machine* sender, unsigned int numsamples);

void InitVumeter(Vumeter* self, ui_component* parent, Player* player)
{			
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_destroy, self, OnDestroy);
	signal_connect(&self->component.signal_size, self, OnSize);	
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	self->player = player;
	signal_connect(&self->component.signal_timer, self, OnTimer);
	SetTimer(self->component.hwnd, 300, 50, 0);	
	self->leftavg;
	self->rightavg;
	signal_connect(&machines_master(&self->player->song->machines)->signal_worked, self, OnMasterWorked);
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

void OnMasterWorked(Vumeter* self, Machine* master, unsigned int numsamples)
{
	real* left = master->outputs.samples[0];
	real* right = master->outputs.samples[1];
	real leftavg = 0;
	real rightavg = 0;
	char buffer[20];
	unsigned int sample = 0;
	for ( ; sample < numsamples; ++sample) {
		leftavg += (real) fabs(left[sample]);
		rightavg += (real) fabs(right[sample]);
	}
	self->leftavg = leftavg / numsamples / 32768;
	self->rightavg = rightavg / numsamples / 32768;
	_snprintf(buffer, 10, "%.4f, ", self->leftavg);	
}