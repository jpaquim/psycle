// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "clipbox.h"
#include <rms.h>
#include <songio.h>

static void clipbox_ondraw(ClipBox*, psy_ui_Graphics*);
static void clipbox_ontimer(ClipBox*, int timerid);
static void clipbox_onmousedown(ClipBox*, psy_ui_MouseEvent*);
static void clipbox_onmasterworked(ClipBox*, psy_audio_Machine* master, uintptr_t slot,
	psy_audio_BufferContext* bc);
static void clipbox_onsongchanged(ClipBox*, Workspace*, int flag, psy_audio_SongFile*);
static void clipbox_connectmachinessignals(ClipBox*, Workspace*);
static void clipbox_onpreferredsize(ClipBox*, psy_ui_Size* limit, psy_ui_Size* rv);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(ClipBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize) clipbox_onpreferredsize;
		vtable.ondraw = (psy_ui_fp_ondraw) clipbox_ondraw;
		vtable.onmousedown = (psy_ui_fp_onmousedown) clipbox_onmousedown;
		vtable.ontimer = (psy_ui_fp_ontimer) clipbox_ontimer;
		vtable_initialized = 1;
	}
}

#define TIMER_ID_CLIPBOX 700

void clipbox_init(ClipBox* self, psy_ui_Component* parent, Workspace* workspace)
{
	self->clip = 0;	
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	psy_signal_connect(&workspace->signal_songchanged, self, clipbox_onsongchanged);	
	clipbox_connectmachinessignals(self, workspace);	
}

void clipbox_ontimer(ClipBox* self, int timerid)
{	
	if (self->clip) {		
		psy_ui_component_invalidate(&self->component);
		psy_ui_component_stoptimer(&self->component, TIMER_ID_CLIPBOX);
	}
}

void clipbox_onmasterworked(ClipBox* self, psy_audio_Machine* master,
	uintptr_t slot, psy_audio_BufferContext* bc)
{	
	if (bc->output->rms) {		
		if (bc->output->rms->data.previousLeft >= 32767.f ||
			bc->output->rms->data.previousLeft < -32768.f) {
			self->clip = 1;
			psy_ui_component_starttimer(&self->component, TIMER_ID_CLIPBOX, 50);
		}
		if (bc->output->rms->data.previousRight >= 32767.f ||
			bc->output->rms->data.previousRight < -32768.f) {
			self->clip = 1;
			psy_ui_component_starttimer(&self->component, TIMER_ID_CLIPBOX, 50);
		}
	}
}

void clipbox_onmousedown(ClipBox* self, psy_ui_MouseEvent* ev)
{
	self->clip = 0;	
	psy_ui_component_invalidate(&self->component);
}

void clipbox_onsongchanged(ClipBox* self, Workspace* workspace, int flag, psy_audio_SongFile* songfile)
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

void clipbox_ondraw(ClipBox* self, psy_ui_Graphics* g)
{
	psy_ui_Rectangle r;
	psy_ui_Size size;

	size = psy_ui_component_size(&self->component);
	if (size.height > 40) {
		size.height = 40;
	}
	psy_ui_setrectangle(&r, 1, 5, size.width - 1, size.height - 5);
	if (self->clip) {
		psy_ui_drawsolidrectangle(g, r, 0x000000FF);
	}	
	psy_ui_setcolor(g, 0x00333333);
	psy_ui_drawrectangle(g, r);
}

void clipbox_onpreferredsize(ClipBox* self, psy_ui_Size* limit, psy_ui_Size* rv)
{	
	rv->width = 10;
	rv->height = 20;
}
