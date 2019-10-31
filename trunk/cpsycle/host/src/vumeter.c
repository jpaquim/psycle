#include "../../detail/prefix.h"

#include "vumeter.h"
#include <math.h>
#include <rms.h>

#define TIMERID_MASTERVU 400

static void OnDraw(Vumeter*, ui_component* sender, ui_graphics*);
static void OnDestroy(Vumeter*, ui_component* sender);
static void OnTimer(Vumeter*, ui_component* sender, int timerid);
static void OnMasterWorked(Vumeter*, Machine*, unsigned int slot, BufferContext*);
static void OnSongChanged(Vumeter*, Workspace*);
static void ConnectMachinesSignals(Vumeter*, Workspace*);

void InitVumeter(Vumeter* self, ui_component* parent, Workspace* workspace)
{			
	ui_component_init(&self->component, parent);	
	signal_connect(&self->component.signal_draw, self, OnDraw);
	signal_connect(&self->component.signal_destroy, self, OnDestroy);	
	signal_connect(&self->component.signal_timer, self, OnTimer);
	self->leftavg = 0;
	self->rightavg = 0;	
	signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
	ConnectMachinesSignals(self, workspace);
	ui_component_starttimer(&self->component, TIMERID_MASTERVU, 50);
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
	ui_drawsolidrectangle(g, left, 0x00000000);
	ui_drawsolidrectangle(g, right, 0x00000000);
	
	left.right = (int) (self->leftavg * size.width);
	right.right = (int) (self->rightavg * size.width);
	ui_drawsolidrectangle(g, left, 0x0000FF00);
	ui_drawsolidrectangle(g, right, 0x0000FF00);

	ui_setrectangle(&left, left.right, left.top, size.width - left.right, 5);
	ui_setrectangle(&right, right.right, right.top, size.width - right.right, 5);
	ui_drawsolidrectangle(g, left, 0x003E3E3E);
	ui_drawsolidrectangle(g, right, 0x003E3E3E);
}

void OnTimer(Vumeter* self, ui_component* sender, int timerid)
{	
	if (timerid == TIMERID_MASTERVU) {
		ui_invalidate(&self->component);
	}
}

void OnMasterWorked(Vumeter* self, Machine* master, unsigned int slot,
	BufferContext* bc)
{	
	if (bc->rmsvol) {
		self->leftavg = bc->rmsvol->data.previousLeft / 32768;
		self->rightavg = bc->rmsvol->data.previousRight / 32768;
	}
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