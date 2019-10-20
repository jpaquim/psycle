// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "linesperbeatbar.h"
#include <stdio.h>

static void OnLessClicked(LinesPerBeatBar*, ui_component* sender);
static void OnMoreClicked(LinesPerBeatBar*, ui_component* sender);
static void OnTimer(LinesPerBeatBar*, ui_component* sender, int timerid);

void InitLinesPerBeatBar(LinesPerBeatBar* self, ui_component* parent, Player* player)
{					
	ui_component_init(&self->component, parent);
	ui_component_setbackgroundmode(&self->component, BACKGROUND_SET);
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	self->player = player;		
	ui_label_init(&self->lpbdesclabel, &self->component);		
	ui_label_settext(&self->lpbdesclabel, "Lines per beat");	
	ui_label_init(&self->lpblabel, &self->component);
	ui_label_setcharnumber(&self->lpblabel, 3);	
	ui_button_init(&self->lessbutton, &self->component);
	ui_button_seticon(&self->lessbutton, UI_ICON_LESS);
	signal_connect(&self->lessbutton.signal_clicked, self, OnLessClicked);		
	ui_button_init(&self->morebutton, &self->component);
	ui_button_seticon(&self->morebutton, UI_ICON_MORE);
	signal_connect(&self->morebutton.signal_clicked, self, OnMoreClicked);	
	self->lpb = 0;
	signal_connect(&self->component.signal_timer, self, OnTimer);
	SetTimer(self->component.hwnd, 500, 200, 0);
	{		
		ui_margin margin = { 0, 3, 3, 0 };
				
		list_free(ui_components_setalign(
			ui_component_children(&self->component, 0),
			UI_ALIGN_LEFT,
			&margin));		
	}
}

void OnLessClicked(LinesPerBeatBar* self, ui_component* sender)
{		
	player_setlpb(self->player, player_lpb(self->player) - 1);
}

void OnMoreClicked(LinesPerBeatBar* self, ui_component* sender)
{		
	player_setlpb(self->player, player_lpb(self->player) + 1);
}

void OnTimer(LinesPerBeatBar* self, ui_component* sender, int timerid)
{		
	if (self->lpb != player_lpb(self->player)) {
		char text[20];

		self->lpb = player_lpb(self->player);
		_snprintf(text, 10, "%2d", self->lpb);
		ui_label_settext(&self->lpblabel, text);
		
	}
}
