// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "songtrackbar.h"

static void Buildtrackbox(SongTrackBar* self);
static void OnTrackBoxSelChange(SongTrackBar*, ui_component* sender, int sel);
static void OnSongTracksNumChanged(SongTrackBar*, Workspace*,
	unsigned int numsongtracks);
static void OnSongChanged(SongTrackBar*, Workspace*);

void InitSongTrackBar(SongTrackBar* self, ui_component* parent, Workspace* workspace)
{	
	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	ui_label_init(&self->headerlabel, &self->component);	
	ui_label_settext(&self->headerlabel, "Tracks");		
	ui_combobox_init(&self->trackbox, &self->component);	
	ui_combobox_setcharnumber(&self->trackbox, 3);
	Buildtrackbox(self);	
	signal_connect(&self->trackbox.signal_selchanged, self, OnTrackBoxSelChange);	
	signal_connect(&workspace->player.signal_numsongtrackschanged, self, OnSongTracksNumChanged);
	signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
	{		
		ui_margin margin = { 0, 3, 3, 0 };
		
		list_free(ui_components_setalign(
			ui_component_children(&self->component, 0),
			UI_ALIGN_LEFT,
			&margin));
	}
}

void Buildtrackbox(SongTrackBar* self)
{
	int track;
	char text[20];

	for (track = 0; track < 65; ++track) {
		_snprintf(text, 20, "%d", track);		
		ui_combobox_addstring(&self->trackbox, text);
	}	
	ui_combobox_setcursel(&self->trackbox,
		player_numsongtracks(&self->workspace->player));
}

void OnTrackBoxSelChange(SongTrackBar* self, ui_component* sender, int sel)
{		
	player_setnumsongtracks(&self->workspace->player, sel);
}

void OnSongTracksNumChanged(SongTrackBar* self, Workspace* workspace,
	unsigned int numsongtracks)
{
	ui_combobox_setcursel(&self->trackbox, numsongtracks);
}

void OnSongChanged(SongTrackBar* self, Workspace* workspace)
{	
	ui_combobox_setcursel(&self->trackbox,
		player_numsongtracks(&workspace->player));
}
