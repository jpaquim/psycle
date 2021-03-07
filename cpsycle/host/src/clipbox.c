// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "clipbox.h"
// dsp
#include <rms.h>
// audio
#include <songio.h>

#define TIMER_ID_CLIPBOX 700

// prototypes
static void clipbox_ondestroy(ClipBox*, psy_ui_Component* sender);
static void clipbox_ondraw(ClipBox*, psy_ui_Graphics*);
static void clipbox_currclipcolours(ClipBox*, psy_ui_Colour* currbackground,
	psy_ui_Colour* currborder);
static void clipbox_ontimer(ClipBox*, uintptr_t timerid);
static void clipbox_onmousedown(ClipBox*, psy_ui_MouseEvent*);
static void clipbox_onmasterworked(ClipBox*, psy_audio_Machine* master, uintptr_t slot,
	psy_audio_BufferContext*);
static bool clipbox_clipoccurred(ClipBox* self, psy_audio_Machine*);
static void clipbox_onsongchanged(ClipBox*, Workspace*, int flag, psy_audio_Song*);
static void clipbox_connectmachinessignals(ClipBox*, Workspace*);

static ClipBoxSkin clipboxdefaultskin;
static int clipboxdefaultskin_initialized = 0;

static void clipboxdefaultskin_init(ClipBox* self)
{
	if (!clipboxdefaultskin_initialized) {
		clipboxdefaultskin.on = psy_ui_colour_make(0x000000FF);
		clipboxdefaultskin.off = psy_ui_colour_make(0x00232323);
		clipboxdefaultskin.borderon = psy_ui_colour_make(0x00333333);
		clipboxdefaultskin.borderoff = psy_ui_colour_make(0x00333333);
		clipboxdefaultskin_initialized = 1;
	}
	self->skin = clipboxdefaultskin;
}
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(ClipBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondraw = (psy_ui_fp_component_ondraw)clipbox_ondraw;
		vtable.onmousedown = (psy_ui_fp_component_onmouseevent)clipbox_onmousedown;
		vtable.ontimer = (psy_ui_fp_component_ontimer)clipbox_ontimer;
		vtable_initialized = TRUE;
	}
}
// implementation
void clipbox_init(ClipBox* self, psy_ui_Component* parent, Workspace* workspace)
{	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	self->component.vtable = &vtable;
	self->workspace = workspace;
	self->isclipon = FALSE;
	clipboxdefaultskin_init(self);
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_makeem(2.0, 1.5));
	psy_signal_connect(&workspace->signal_songchanged, self,
		clipbox_onsongchanged);	
	clipbox_connectmachinessignals(self, workspace);
	psy_signal_connect(&self->component.signal_destroy, self,
		clipbox_ondestroy);
}

void clipbox_ondestroy(ClipBox* self, psy_ui_Component* sender)
{
	psy_signal_disconnect(&self->workspace->signal_songchanged, self,
		clipbox_onsongchanged);
	if (workspace_song(self->workspace)) {
		psy_signal_disconnect(&psy_audio_machines_master(
			&workspace_song(self->workspace)->machines)->signal_worked, self,
			clipbox_onmasterworked);
	}
}

void clipbox_ontimer(ClipBox* self, uintptr_t timerid)
{	
	if (clipbox_isclipon(self)) {
		psy_ui_component_stoptimer(&self->component, TIMER_ID_CLIPBOX);
		psy_ui_component_invalidate(&self->component);		
	}
}

void clipbox_onmasterworked(ClipBox* self, psy_audio_Machine* master,
	uintptr_t slot, psy_audio_BufferContext* bc)
{		
	if (clipbox_clipoccurred(self, master)) {
		clipbox_activate(self);
		psy_ui_component_starttimer(&self->component, TIMER_ID_CLIPBOX, 50);		
	}
}

bool clipbox_clipoccurred(ClipBox* self, psy_audio_Machine* machine)
{
	psy_audio_Buffer * memory;

	memory = psy_audio_machine_buffermemory(machine);
	return (memory && memory->rms &&
		(memory->rms->data.previousLeft >= 32767.f ||
		memory->rms->data.previousLeft < -32768.f ||
		memory->rms->data.previousRight >= 32767.f ||
		memory->rms->data.previousRight < -32768.f));
}

void clipbox_onmousedown(ClipBox* self, psy_ui_MouseEvent* ev)
{
	clipbox_deactivate(self);
	psy_ui_component_invalidate(&self->component);
}

void clipbox_onsongchanged(ClipBox* self, Workspace* workspace, int flag, psy_audio_Song* song)
{
	clipbox_connectmachinessignals(self, workspace);	
}

void clipbox_connectmachinessignals(ClipBox* self, Workspace* workspace)
{
	if (workspace && workspace->song &&
			psy_audio_machines_master(&workspace->song->machines)) {
		psy_signal_connect(&psy_audio_machines_master(
			&workspace->song->machines)->signal_worked, self,
			clipbox_onmasterworked);
	}
}

void clipbox_ondraw(ClipBox* self, psy_ui_Graphics* g)
{	
	psy_ui_Size size;
	const psy_ui_TextMetric* tm;
	psy_ui_RealRectangle rc;
	psy_ui_Colour currbackground;
	psy_ui_Colour currborder;

	size = psy_ui_component_size(&self->component);
	tm = psy_ui_component_textmetric(&self->component);
	if (psy_ui_value_px(&size.height, tm) > 40) {
		size.height = psy_ui_value_makepx(40);
	}
	psy_ui_setrectangle(&rc, 1, 5, psy_ui_value_px(&size.width, tm) - 1,
		psy_ui_value_px(&size.height, tm) - 5);
	clipbox_currclipcolours(self, &currbackground, &currborder);
	psy_ui_drawsolidrectangle(g, rc, currbackground);
	psy_ui_setcolour(g, currborder);			
	psy_ui_drawrectangle(g, rc);
}

void clipbox_currclipcolours(ClipBox* self, psy_ui_Colour* currbackground,
	psy_ui_Colour* currborder)
{
	if (clipbox_isclipon(self)) {
		*currbackground = self->skin.on;
		*currborder = self->skin.borderon;
	} else {
		*currbackground = self->skin.off;
		*currborder = self->skin.borderoff;
	}
}
