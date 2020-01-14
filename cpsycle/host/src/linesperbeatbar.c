// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "linesperbeatbar.h"
#include <stdio.h>
#include <portable.h>

static void OnLessClicked(LinesPerBeatBar*, psy_ui_Component* sender);
static void OnMoreClicked(LinesPerBeatBar*, psy_ui_Component* sender);
static void OnTimer(LinesPerBeatBar*, psy_ui_Component* sender, int timerid);
void linesperbeatbar_initalign(LinesPerBeatBar*);

void linesperbeatbar_init(LinesPerBeatBar* self, psy_ui_Component* parent, psy_audio_Player* player)
{	
	self->lpb = 0;			
	ui_component_init(&self->component, parent);	
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);
	self->player = player;		
	psy_ui_label_init(&self->lpbdesclabel, &self->component);		
	psy_ui_label_settext(&self->lpbdesclabel, "Lines per beat");	
	psy_ui_label_init(&self->lpblabel, &self->component);
	psy_ui_label_setcharnumber(&self->lpblabel, 3);	
	psy_ui_button_init(&self->lessbutton, &self->component);
	psy_ui_button_seticon(&self->lessbutton, psy_ui_ICON_LESS);
	psy_signal_connect(&self->lessbutton.signal_clicked, self,
		OnLessClicked);
	psy_ui_button_init(&self->morebutton, &self->component);
	psy_ui_button_seticon(&self->morebutton, psy_ui_ICON_MORE);
	psy_signal_connect(&self->morebutton.signal_clicked, self,
		OnMoreClicked);
	psy_signal_connect(&self->component.signal_timer, self, OnTimer);
	ui_component_starttimer(&self->component, 500, 200);
	linesperbeatbar_initalign(self);
}

void linesperbeatbar_initalign(LinesPerBeatBar* self)
{
	psy_ui_Margin margin;

	psy_ui_margin_init(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		psy_ui_ALIGN_LEFT,
		&margin));
}

void OnLessClicked(LinesPerBeatBar* self, psy_ui_Component* sender)
{		
	player_setlpb(self->player, player_lpb(self->player) - 1);
}

void OnMoreClicked(LinesPerBeatBar* self, psy_ui_Component* sender)
{		
	player_setlpb(self->player, player_lpb(self->player) + 1);
}

void OnTimer(LinesPerBeatBar* self, psy_ui_Component* sender, int timerid)
{		
	if (self->lpb != player_lpb(self->player)) {
		char text[20];

		self->lpb = player_lpb(self->player);
		psy_snprintf(text, 10, "%2d", self->lpb);
		psy_ui_label_settext(&self->lpblabel, text);		
	}
}
