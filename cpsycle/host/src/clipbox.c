// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "clipbox.h"
#include <rms.h>

static void clipbox_ondraw(ClipBox*, psy_ui_Component* sender, psy_ui_Graphics*);
static void clipbox_ontimer(ClipBox* self, psy_ui_Component* sender, int timerid);
static void clipbox_onmousedown(ClipBox* self, psy_ui_Component* sender,
	psy_ui_MouseEvent*);
static void clipbox_onmasterworked(ClipBox* self, psy_audio_Machine* master, unsigned int slot, psy_audio_BufferContext* bc);
static void clipbox_onsongchanged(ClipBox* self, Workspace* workspace);
static void clipbox_connectmachinessignals(ClipBox* self, Workspace* workspace);

#define TIMER_ID_CLIPBOX 700

void clipbox_init(ClipBox* self, psy_ui_Component* parent, Workspace* workspace)
{
	self->clip = 0;	
	ui_component_init(&self->component, parent);	
	psy_signal_connect(&self->component.signal_draw, self, clipbox_ondraw);
	psy_signal_connect(&self->component.signal_mousedown, self, clipbox_onmousedown);
	psy_signal_connect(&self->component.signal_timer, self, clipbox_ontimer);
	psy_signal_connect(&workspace->signal_songchanged, self, clipbox_onsongchanged);	
	clipbox_connectmachinessignals(self, workspace);
	// ui_component_starttimer(&self->component, TIMER_ID_CLIPBOX, 200);
}

void clipbox_ontimer(ClipBox* self, psy_ui_Component* sender, int timerid)
{	
	if (self->clip) {
		ui_component_setbackgroundcolor(&self->component, 0x00FF0000);
		ui_component_invalidate(&self->component);
		self->clip = 0;
	}
}

void clipbox_onmasterworked(ClipBox* self, psy_audio_Machine* master, unsigned int slot, psy_audio_BufferContext* bc)
{	
	if (bc->rmsvol) {		
		if (bc->rmsvol->data.previousLeft >= 32767.f ||
			bc->rmsvol->data.previousLeft < -32768.f) {				
			self->clip = 1;
		}
		if (bc->rmsvol->data.previousRight >= 32767.f ||
			bc->rmsvol->data.previousRight < -32768.f) {				
			self->clip = 1;
		}
	}
}

void clipbox_onmousedown(ClipBox* self, psy_ui_Component* sender,
	psy_ui_MouseEvent* ev)
{
	self->clip = 0;
	ui_component_setbackgroundcolor(&self->component, 0x00000000);
	ui_component_invalidate(&self->component);
}

void clipbox_onsongchanged(ClipBox* self, Workspace* workspace)
{
	clipbox_connectmachinessignals(self, workspace);	
}

void clipbox_connectmachinessignals(ClipBox* self, Workspace* workspace)
{
	if (workspace && workspace->song &&
			machines_master(&workspace->song->machines)) {
		psy_signal_connect(&machines_master(
			&workspace->song->machines)->signal_worked, self,
			clipbox_onmasterworked);
	}
}

void clipbox_ondraw(ClipBox* self, psy_ui_Component* sender, psy_ui_Graphics* g)
{
	ui_rectangle r;
	ui_size size;

	size = ui_component_size(&self->component);
	ui_setrectangle(&r, 0, 0, size.width, size.height);
	ui_setcolor(g, 0x00333333);
	ui_drawrectangle(g, r);
}
