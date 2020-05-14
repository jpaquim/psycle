// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "playposbar.h"
#include <stdio.h>
#include "../../detail/portable.h"

#define TIMERID_PLAYPOSBAR 500

static void playposbar_ontimer(PlayPosBar*, psy_ui_Component* sender, int timerid);

void playposbar_init(PlayPosBar* self, psy_ui_Component* parent,
	psy_audio_Player* player)
{		
	psy_ui_Margin margin;

	psy_ui_margin_init(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);
	self->player = player;		
	//psy_ui_label_init(&self->header, &self->component);		
	//psy_ui_label_settext(&self->header, "");	
	psy_ui_label_init(&self->position, &self->component);
	psy_ui_label_setcharnumber(&self->position, 10);
	self->lastposition = -1.0f;
	psy_signal_connect(&self->component.signal_timer, self,
		playposbar_ontimer);
	psy_list_free(psy_ui_components_setalign(		
		psy_ui_component_children(&self->component, 0),
		psy_ui_ALIGN_LEFT,
		&margin));
	psy_ui_component_starttimer(&self->component, TIMERID_PLAYPOSBAR, 50);
}

void playposbar_ontimer(PlayPosBar* self, psy_ui_Component* sender, int timerid)
{		
	if (self->lastposition != psy_audio_player_position(self->player)) {
		char text[20];

		psy_snprintf(text, 10, "%.3f",
			(float) psy_audio_player_position(self->player));
		psy_ui_label_settext(&self->position, text);
		self->lastposition = psy_audio_player_position(self->player);
	}
}
