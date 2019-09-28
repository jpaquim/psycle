// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "clipbox.h"
#include <math.h>

static void OnDestroy(ClipBox*);
static void OnSize(ClipBox*, ui_component* sender, int width, int height);
static void OnTimer(ClipBox* self, ui_component* sender, int timerid);
static void OnMouseDown(ClipBox* self, ui_component* sender, int x, int y, int button);
static void OnMasterWorked(ClipBox* self, Machine* master, BufferContext* bc);
static void OnSongChanged(ClipBox* self, Workspace* workspace);
static void ConnectMachinesSignals(ClipBox* self, Workspace* workspace);

static int timerid = 700;

void InitClipBox(ClipBox* self, ui_component* parent, Workspace* workspace)
{
	self->clip = 0;
	ui_component_init(&self->component, parent);	
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_component_setbackgroundcolor(&self->component, 0x00000000);
	signal_connect(&self->component.signal_mousedown, self, OnMouseDown);
	signal_connect(&self->component.signal_destroy, self, OnDestroy);
	signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
	ConnectMachinesSignals(self, workspace);
}

void OnDestroy(ClipBox* self)
{	
}

void OnTimer(ClipBox* self, ui_component* sender, int timerid)
{	
	if (self->clip) {
		ui_component_setbackgroundcolor(&self->component, 0x00FF0000);
		ui_invalidate(&self->component);
		self->clip = 0;
	}
}

void OnMasterWorked(ClipBox* self, Machine* master, BufferContext* bc)
{
	real* left = bc->output->samples[0];
	real* right = bc->output->samples[1];
	real leftavg = 0;
	real rightavg = 0;
	
	unsigned int sample = 0;
	for ( ; sample < bc->numsamples; ++sample) {
		leftavg += (real) fabs(left[sample]);
		rightavg += (real) fabs(right[sample]);
	}
	leftavg = leftavg / bc->numsamples / 32768;
	rightavg = rightavg / bc->numsamples / 32768;	

	if (leftavg >= 32767 || leftavg < -32768) {				
		self->clip = 1;
	}
	if (rightavg >= 32767 || rightavg < -32768) {				
		self->clip = 1;
	}
}

void OnMouseDown(ClipBox* self, ui_component* sender, int x, int y, int button)
{
	self->clip = 0;
	ui_component_setbackgroundcolor(&self->component, 0x00000000);
		ui_invalidate(&self->component);
}

void OnSongChanged(ClipBox* self, Workspace* workspace)
{
	ConnectMachinesSignals(self, workspace);
}

void ConnectMachinesSignals(ClipBox* self, Workspace* workspace)
{
	if (workspace && workspace->song &&
			machines_master(&workspace->song->machines)) {
		signal_connect(
			&machines_master(&workspace->song->machines)->signal_worked, self,
			OnMasterWorked);
	}
}
