/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "songtrackbar.h"
/* audio */
#include <songio.h>
/* platform */
#include "../../detail/portable.h"

#define MIN_TRACKS 4

/* prototypes */
static void songtrackbar_build(SongTrackBar*);
static void songtrackbar_onselchange(SongTrackBar*, psy_ui_Component* sender,
	intptr_t index);
static void songtrackbar_onsongtracknumchanged(SongTrackBar*,
	psy_audio_Patterns*);
static void songtrackbar_on_song_changed(SongTrackBar*, psy_audio_Player* sender);

/* implementation */
void songtrackbar_init(SongTrackBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{		
	psy_ui_component_init(&self->component, parent, NULL);
	self->workspace = workspace;
	psy_ui_component_set_align_expand(songtrackbar_base(self), psy_ui_HEXPAND);
	psy_ui_component_set_default_align(songtrackbar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_label_init(&self->desc, songtrackbar_base(self));
	psy_ui_label_set_text(&self->desc, "trackbar.tracks");
	psy_ui_combobox_init(&self->tracknumbers, songtrackbar_base(self));
	psy_ui_combobox_set_char_number(&self->tracknumbers, 3.0);
	songtrackbar_build(self);	
	psy_signal_connect(&self->tracknumbers.signal_selchanged, self,
		songtrackbar_onselchange);
	psy_signal_connect(&workspace->song->patterns.signal_numsongtrackschanged,
		self, songtrackbar_onsongtracknumchanged);
	psy_signal_connect(&workspace->player.signal_song_changed, self,
		songtrackbar_on_song_changed);	
}

void songtrackbar_build(SongTrackBar* self)
{
	int track;
	char text[20];

	for (track = MIN_TRACKS; track < 65; ++track) {
		psy_snprintf(text, 20, "%d", track);
		psy_ui_combobox_add_text(&self->tracknumbers, text);
	}	
	if (workspace_song(self->workspace)) {
		psy_ui_combobox_select(&self->tracknumbers,
			psy_audio_song_num_song_tracks(workspace_song(self->workspace)) -
			MIN_TRACKS);
	}
}

void songtrackbar_onselchange(SongTrackBar* self, psy_ui_Component* sender,
	intptr_t index)
{		
	if (workspace_song(self->workspace)) {
		psy_audio_song_set_num_song_tracks(workspace_song(self->workspace),
			index + MIN_TRACKS);
	}
}

void songtrackbar_onsongtracknumchanged(SongTrackBar* self,
	psy_audio_Patterns* patterns)
{	
	assert(self);
	assert(patterns);
	
	psy_ui_combobox_select(&self->tracknumbers,  psy_audio_patterns_num_tracks(
		patterns) - MIN_TRACKS);
}

void songtrackbar_on_song_changed(SongTrackBar* self, psy_audio_Player* sender)
{	
	if (psy_audio_player_song(sender)) {
		psy_ui_combobox_select(&self->tracknumbers,
			psy_audio_song_num_song_tracks(psy_audio_player_song(sender)) - MIN_TRACKS);		
		psy_signal_connect(
			&psy_audio_player_song(sender)->patterns.signal_numsongtrackschanged,
			self, songtrackbar_onsongtracknumchanged);
		songtrackbar_onsongtracknumchanged(self, psy_audio_song_patterns(
			psy_audio_player_song(sender)));
	}
}
