// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "linesperbeatbar.h"
// std
#include <stdio.h>
// platform
#include "../../detail/portable.h"

// prototypes
static void linesperbeatbar_onlessclicked(LinesPerBeatBar*, psy_ui_Component* sender);
static void linesperbeatbar_onmoreclicked(LinesPerBeatBar*, psy_ui_Component* sender);
static void linesperbeatbar_ontimer(LinesPerBeatBar*, uintptr_t timerid);
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static psy_ui_ComponentVtable* vtable_init(LinesPerBeatBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ontimer = (psy_ui_fp_component_ontimer)
			linesperbeatbar_ontimer;
		vtable_initialized = TRUE;
	}
	return &vtable;
}
// implementation
void linesperbeatbar_init(LinesPerBeatBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{		
	assert(self);
	assert(workspace);

	psy_ui_component_init(linesperbeatbar_base(self), parent);
	psy_ui_component_setvtable(linesperbeatbar_base(self), vtable_init(self));
	self->lpb = 0;
	self->workspace = workspace;
	psy_ui_component_setalignexpand(linesperbeatbar_base(self),
		psy_ui_HORIZONTALEXPAND);
	psy_ui_component_setdefaultalign(linesperbeatbar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_label_init_text(&self->lpbdesclabel, linesperbeatbar_base(self),
		"lpb.lines-per-beat");
	psy_ui_label_init(&self->lpblabel, linesperbeatbar_base(self));
	psy_ui_label_preventtranslation(&self->lpblabel);
	psy_ui_label_setcharnumber(&self->lpblabel, 4);
	// less
	psy_ui_button_init_connect(&self->lessbutton, linesperbeatbar_base(self),
		self, linesperbeatbar_onlessclicked);
	psy_ui_button_seticon(&self->lessbutton, psy_ui_ICON_LESS);	
	// more
	psy_ui_button_init_connect(&self->morebutton, linesperbeatbar_base(self),
		self, linesperbeatbar_onmoreclicked);
	psy_ui_button_seticon(&self->morebutton, psy_ui_ICON_MORE);	
	psy_ui_component_starttimer(&self->component, 0, 200);	
}

void linesperbeatbar_onlessclicked(LinesPerBeatBar* self, psy_ui_Component* sender)
{
	assert(self);

	psy_audio_player_setlpb(workspace_player(self->workspace),
		psy_audio_player_lpb(workspace_player(self->workspace)) - 1);
}

void linesperbeatbar_onmoreclicked(LinesPerBeatBar* self, psy_ui_Component* sender)
{
	assert(self);

	psy_audio_player_setlpb(workspace_player(self->workspace),
		psy_audio_player_lpb(workspace_player(self->workspace)) + 1);
}

void linesperbeatbar_ontimer(LinesPerBeatBar* self, uintptr_t timerid)
{
	assert(self);

	if (self->lpb != psy_audio_player_lpb(workspace_player(self->workspace))) {
		char text[20];

		self->lpb = psy_audio_player_lpb(workspace_player(self->workspace));
		psy_snprintf(text, 10, "%2d", self->lpb);
		psy_ui_label_settext(&self->lpblabel, text);		
	}
}
