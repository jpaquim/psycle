// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "songtrackbar.h"

#include <songio.h>

#include "../../detail/portable.h"

#define MIN_TRACKS 4

static void songtrackbar_updatetext(SongTrackBar*);
static void songtrackbar_build(SongTrackBar*);
static void songtrackbar_onselchange(SongTrackBar*, psy_ui_Component* sender,
	int index);
static void songtrackbar_onsongtracknumchanged(SongTrackBar*, Workspace*,
	unsigned int numsongtracks);
static void songtrackbar_onsongchanged(SongTrackBar*, Workspace*,
	int flag, psy_audio_SongFile* songfile);

void songtrackbar_init(SongTrackBar* self, psy_ui_Component* parent, Workspace*
	workspace)
{	
	self->workspace = workspace;
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_component_setalignexpand(&self->component, psy_ui_HORIZONTALEXPAND);
	psy_ui_label_init(&self->headerlabel, &self->component);				
	psy_ui_combobox_init(&self->trackbox, &self->component);	
	psy_ui_combobox_setcharnumber(&self->trackbox, 4);
	songtrackbar_build(self);	
	psy_signal_connect(&self->trackbox.signal_selchanged, self,
		songtrackbar_onselchange);	
	psy_signal_connect(&workspace->player.signal_numsongtrackschanged, self,
		songtrackbar_onsongtracknumchanged);
	psy_signal_connect(&workspace->signal_songchanged, self,
		songtrackbar_onsongchanged);
	songtrackbar_updatetext(self);		
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(&self->component, psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		NULL));	
}

void songtrackbar_updatetext(SongTrackBar* self)
{
	psy_ui_label_settext(&self->headerlabel, 
		workspace_translate(self->workspace, "Tracks"));
}

void songtrackbar_build(SongTrackBar* self)
{
	int track;
	char text[20];

	for (track = MIN_TRACKS; track < 65; ++track) {
		psy_snprintf(text, 20, "%d", track);
		psy_ui_combobox_addtext(&self->trackbox, text);
	}	
	psy_ui_combobox_setcursel(&self->trackbox,
		psy_audio_player_numsongtracks(&self->workspace->player) - MIN_TRACKS);
}

void songtrackbar_onselchange(SongTrackBar* self, psy_ui_Component* sender,
	int index)
{		
	psy_audio_player_setnumsongtracks(&self->workspace->player, index + MIN_TRACKS);
	if (self->workspace->song) {
		patterns_setsongtracks(&self->workspace->song->patterns, index + MIN_TRACKS);
	}
}

void songtrackbar_onsongtracknumchanged(SongTrackBar* self,
	Workspace* workspace, unsigned int numsongtracks)
{
	psy_ui_combobox_setcursel(&self->trackbox, numsongtracks - MIN_TRACKS);
}

void songtrackbar_onsongchanged(SongTrackBar* self, Workspace* workspace,
	int flag, psy_audio_SongFile* songfile)
{	
	psy_ui_combobox_setcursel(&self->trackbox,
		psy_audio_player_numsongtracks(&workspace->player) - MIN_TRACKS);
}
