/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "clipbox.h"
/* host */
#include "styles.h"
/* dsp */
#include <operations.h>

static bool check_peak(float peak)
{
	return (peak >= 32767.f || peak < -32768.f || peak >= 32767.f || peak < -32768.f);
}

/* prototypes */
static void clipbox_on_timer(ClipBox*, uintptr_t timerid);
static void clipbox_on_mouse_down(ClipBox*, psy_ui_MouseEvent*);
static bool clipbox_check(ClipBox*, psy_audio_Machine*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(ClipBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);		
		vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			clipbox_on_mouse_down;
		vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			clipbox_on_timer;
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void clipbox_init(ClipBox* self, psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_set_style_type(&self->component, STYLE_CLIPBOX);
	psy_ui_component_set_style_type_select(&self->component,
		STYLE_CLIPBOX_SELECT);
	self->workspace = workspace;	
	psy_ui_component_start_timer(&self->component, 0, 100);
}

void clipbox_on_timer(ClipBox* self, uintptr_t timerid)
{	
	if (!clipbox_ison(self) && workspace_song(self->workspace)) {
		psy_audio_Machine* master;

		master = psy_audio_machines_master(
			&workspace_song(self->workspace)->machines);
		if (master && clipbox_check(self, master)) {
			clipbox_activate(self);
		}				
	}
}

bool clipbox_check(ClipBox* self, psy_audio_Machine* machine)
{
	psy_audio_Buffer * memory;	

	memory = psy_audio_machine_buffermemory(machine);
	if (memory && memory->numsamples > 0) {
		if (memory->numchannels > 0 &&
			check_peak(dsp.maxvol(memory->samples[0], memory->numsamples))) {
			return TRUE;
		}
		if (memory->numchannels > 1 &&
			check_peak(dsp.maxvol(memory->samples[1], memory->numsamples))) {
			return TRUE;
		}
	}
	return FALSE;
}

void clipbox_on_mouse_down(ClipBox* self, psy_ui_MouseEvent* ev)
{
	clipbox_deactivate(self);	
}
