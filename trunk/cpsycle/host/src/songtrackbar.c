// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "songtrackbar.h"
#include <portable.h>

static void songtrackbar_build(SongTrackBar*);
static void songtrackbar_onselchange(SongTrackBar*, psy_ui_Component* sender,
	int index);
static void songtrackbar_onsongtracknumchanged(SongTrackBar*, Workspace*,
	unsigned int numsongtracks);
static void songtrackbar_onsongchanged(SongTrackBar*, Workspace*);

void songtrackbar_init(SongTrackBar* self, psy_ui_Component* parent, Workspace*
	workspace)
{	
	ui_margin margin;

	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	ui_label_init(&self->headerlabel, &self->component);	
	ui_label_settext(&self->headerlabel, "Tracks");		
	ui_combobox_init(&self->trackbox, &self->component);	
	ui_combobox_setcharnumber(&self->trackbox, 4);
	songtrackbar_build(self);	
	psy_signal_connect(&self->trackbox.signal_selchanged, self,
		songtrackbar_onselchange);	
	psy_signal_connect(&workspace->player.signal_numsongtrackschanged, self,
		songtrackbar_onsongtracknumchanged);
	psy_signal_connect(&workspace->signal_songchanged, self,
		songtrackbar_onsongchanged);
	ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(1),
		ui_value_makepx(0), ui_value_makepx(0));		
	psy_list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		UI_ALIGN_LEFT,
		&margin));	
}

void songtrackbar_build(SongTrackBar* self)
{
	int track;
	char text[20];

	for (track = 0; track < 65; ++track) {
		psy_snprintf(text, 20, "%d", track);		
		ui_combobox_addstring(&self->trackbox, text);
	}	
	ui_combobox_setcursel(&self->trackbox,
		player_numsongtracks(&self->workspace->player));
}

void songtrackbar_onselchange(SongTrackBar* self, psy_ui_Component* sender,
	int index)
{		
	player_setnumsongtracks(&self->workspace->player, index);
	if (self->workspace->song) {
		patterns_setsongtracks(&self->workspace->song->patterns, index);
	}
}

void songtrackbar_onsongtracknumchanged(SongTrackBar* self,
	Workspace* workspace, unsigned int numsongtracks)
{
	ui_combobox_setcursel(&self->trackbox, numsongtracks);
}

void songtrackbar_onsongchanged(SongTrackBar* self, Workspace* workspace)
{	
	ui_combobox_setcursel(&self->trackbox,
		player_numsongtracks(&workspace->player));
}
