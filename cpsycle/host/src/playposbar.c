// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "playposbar.h"

#include <songio.h>

#include <stdio.h>

#include "../../detail/portable.h"

static void playposbar_ontimer(PlayPosBar*, uintptr_t timerid);
static void playposbar_updatelabel(PlayPosBar*);
static void playposbar_onsongchanged(PlayPosBar*, Workspace*,
	int flag, psy_audio_Song*);
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static psy_ui_ComponentVtable* vtable_init(PlayPosBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ontimer = (psy_ui_fp_component_ontimer)playposbar_ontimer;
		vtable_initialized = TRUE;
	}
	return &vtable;
}

void playposbar_init(PlayPosBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_setvtable(&self->component, vtable_init(self));
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);
	psy_ui_component_setdefaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->workspace = workspace;	
	psy_ui_label_init(&self->position, &self->component);
	psy_ui_label_preventtranslation(&self->position);
	psy_ui_label_setcharnumber(&self->position, 20);
	psy_ui_label_settextalignment(&self->position, psy_ui_ALIGNMENT_LEFT);	
	playposbar_updatelabel(self);
	psy_signal_connect(&workspace->signal_songchanged, self,
		playposbar_onsongchanged);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void playposbar_ontimer(PlayPosBar* self, uintptr_t timerid)
{
	//if (psy_audio_player_playing(&workspace_player(self->workspace))) {
	playposbar_updatelabel(self);
	//}
}

void playposbar_updatelabel(PlayPosBar* self)
{
	psy_dsp_seconds_t currplaytime;
	char text[80];

	currplaytime = psy_audio_sequencer_currplaytime(
		&workspace_player(self->workspace)->sequencer);
	psy_snprintf(text, 40, "%02dm%02ds %.2fb",
		(int)(currplaytime / 60), ((int)currplaytime % 60),
		(float)psy_audio_player_position(workspace_player(self->workspace)));
	psy_ui_label_settext(&self->position, text);
}

void playposbar_onsongchanged(PlayPosBar* self, Workspace* sender, int flag,
	psy_audio_Song* song)
{
	playposbar_updatelabel(self);
}
