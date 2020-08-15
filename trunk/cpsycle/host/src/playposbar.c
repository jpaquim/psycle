// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "playposbar.h"

#include <songio.h>

#include <stdio.h>

#include "../../detail/portable.h"

static void playposbar_ontimer(PlayPosBar*, psy_ui_Component* sender,
	uintptr_t timerid);
static void playposbar_updatelabel(PlayPosBar*);
static void playposbar_onsongchanged(PlayPosBar*, Workspace*,
	int flag, psy_audio_SongFile*);

void playposbar_init(PlayPosBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{		
	psy_ui_Margin margin;

	psy_ui_component_init(&self->component, parent);
	self->workspace = workspace;	
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);	
	psy_ui_label_init(&self->position, &self->component);
	psy_ui_label_setcharnumber(&self->position, 20);
	psy_ui_label_settextalignment(&self->position, psy_ui_ALIGNMENT_LEFT);
	psy_signal_connect(&self->component.signal_timer, self,
		playposbar_ontimer);
	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_list_free(psy_ui_components_setalign(		
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		&margin));
	playposbar_updatelabel(self);
	psy_signal_connect(&workspace->signal_songchanged, self,
		playposbar_onsongchanged);
	psy_ui_component_starttimer(&self->component, 0, 50);
}

void playposbar_ontimer(PlayPosBar* self, psy_ui_Component* sender,
	uintptr_t timerid)
{
	if (psy_audio_player_playing(&self->workspace->player)) {
		playposbar_updatelabel(self);
	}
}

void playposbar_updatelabel(PlayPosBar* self)
{
	psy_dsp_seconds_t currplaytime;
	char text[80];

	currplaytime = psy_audio_sequencer_currplaytime(
		&self->workspace->player.sequencer);
	psy_snprintf(text, 40, "%02dm%02ds %.2fb",
		(int)(currplaytime / 60), ((int)currplaytime % 60),
		(float)psy_audio_player_position(&self->workspace->player));
	psy_ui_label_settext(&self->position, text);
}

void playposbar_onsongchanged(PlayPosBar* self, Workspace* sender, int flag,
	psy_audio_SongFile* songfile)
{
	playposbar_updatelabel(self);
}
