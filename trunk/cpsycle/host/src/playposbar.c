// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "playposbar.h"
#include <stdio.h>
#include <portable.h>

static void ontimer(PlayPosBar*, ui_component* sender, int timerid);

void playposbar_init(PlayPosBar* self, ui_component* parent, Player* player)
{					
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	self->player = player;		
	ui_label_init(&self->header, &self->component);		
	ui_label_settext(&self->header, "Player position");	
	ui_label_init(&self->position, &self->component);
	ui_label_setcharnumber(&self->position, 10);
	self->lastposition = -1.0f;
	psy_signal_connect(&self->component.signal_timer, self, ontimer);
	ui_component_starttimer(&self->component, 500, 50);
	{		
		ui_margin margin;

		ui_margin_init(&margin, ui_value_makepx(0), ui_value_makepx(0),
			ui_value_makepx(0), ui_value_makepx(0));			
		list_free(ui_components_setalign(		
			ui_component_children(&self->component, 0),
			UI_ALIGN_LEFT,
			&margin));		
	}
}

void ontimer(PlayPosBar* self, ui_component* sender, int timerid)
{		
	if (self->lastposition != player_position(self->player)) {
		char text[20];

		psy_snprintf(text, 10, "%.4f", player_position(self->player));
		ui_label_settext(&self->position, text);
		self->lastposition = player_position(self->player);
	}
}
