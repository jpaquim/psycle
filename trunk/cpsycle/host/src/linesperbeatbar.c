// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "linesperbeatbar.h"
#include <stdio.h>
#include "../../detail/portable.h"

static void linesperbeatbar_initalign(LinesPerBeatBar*);
static void linesperbeatbar_updatetext(LinesPerBeatBar*);
static void linesperbeatbar_onlessclicked(LinesPerBeatBar*, psy_ui_Component* sender);
static void linesperbeatbar_onmoreclicked(LinesPerBeatBar*, psy_ui_Component* sender);
static void linesperbeatbar_ontimer(LinesPerBeatBar*, psy_ui_Component* sender, uintptr_t timerid);
static void linesperbeatbar_onlanguagechanged(LinesPerBeatBar*, Translator* sender);

void linesperbeatbar_init(LinesPerBeatBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	self->lpb = 0;
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);	
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_setalignexpand(&self->component,
		psy_ui_HORIZONTALEXPAND);
	self->player = &workspace->player;		
	psy_ui_label_init(&self->lpbdesclabel, &self->component);		
	psy_ui_label_settext(&self->lpbdesclabel, 
		workspace_translate(self->workspace, "lines-per-beat"));	
	psy_ui_label_init(&self->lpblabel, &self->component);
	psy_ui_label_setcharnumber(&self->lpblabel, 4);	
	psy_ui_button_init(&self->lessbutton, &self->component);
	psy_ui_button_seticon(&self->lessbutton, psy_ui_ICON_LESS);
	psy_signal_connect(&self->lessbutton.signal_clicked, self,
		linesperbeatbar_onlessclicked);
	psy_ui_button_init(&self->morebutton, &self->component);
	psy_ui_button_seticon(&self->morebutton, psy_ui_ICON_MORE);
	psy_signal_connect(&self->morebutton.signal_clicked, self,
		linesperbeatbar_onmoreclicked);
	psy_signal_connect(&self->component.signal_timer, self,
		linesperbeatbar_ontimer);
	psy_signal_connect(&self->workspace->signal_languagechanged, self,
		linesperbeatbar_onlanguagechanged);
	linesperbeatbar_initalign(self);
	psy_ui_component_starttimer(&self->component, 500, 200);
	
}

void linesperbeatbar_updatetext(LinesPerBeatBar* self)
{
	psy_ui_label_settext(&self->lpbdesclabel,
		workspace_translate(self->workspace, "lines-per-beat"));
}

void linesperbeatbar_initalign(LinesPerBeatBar* self)
{	
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		NULL));
}

void linesperbeatbar_onlessclicked(LinesPerBeatBar* self, psy_ui_Component* sender)
{		
	psy_audio_player_setlpb(self->player,
		psy_audio_player_lpb(self->player) - 1);
}

void linesperbeatbar_onmoreclicked(LinesPerBeatBar* self, psy_ui_Component* sender)
{		
	psy_audio_player_setlpb(self->player,
		psy_audio_player_lpb(self->player) + 1);
}

void linesperbeatbar_ontimer(LinesPerBeatBar* self, psy_ui_Component* sender, uintptr_t timerid)
{		
	if (self->lpb != psy_audio_player_lpb(self->player)) {
		char text[20];

		self->lpb = psy_audio_player_lpb(self->player);
		psy_snprintf(text, 10, "%2d", self->lpb);
		psy_ui_label_settext(&self->lpblabel, text);		
	}
}

void linesperbeatbar_onlanguagechanged(LinesPerBeatBar* self, Translator* sender)
{
	linesperbeatbar_updatetext(self);
}
