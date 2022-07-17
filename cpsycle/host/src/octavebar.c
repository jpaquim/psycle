/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "octavebar.h"

/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void octavebar_build_octave_box(OctaveBar*);
static void octavebar_on_octavebox_sekected(OctaveBar*,
	psy_ui_Component* sender, intptr_t sel);
static void octavebar_on_octave_changed(OctaveBar*, psy_audio_Player*,
	intptr_t octave);
static void octavebar_on_song_changed(OctaveBar*, Workspace* sender);

/* implementation */
void octavebar_init(OctaveBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(octavebar_base(self), parent, NULL);
	psy_ui_component_set_default_align(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_component_set_align_expand(octavebar_base(self),
		psy_ui_HEXPAND);
	self->workspace = workspace;
	psy_ui_label_init_text(&self->desc, octavebar_base(self),
		"octavebar.octave");
	psy_ui_combobox_init(&self->octavebox, octavebar_base(self));
	psy_ui_combobox_set_char_number(&self->octavebox, 3);	
	octavebar_build_octave_box(self);
	psy_signal_connect(&self->octavebox.signal_selchanged, self,
		octavebar_on_octavebox_sekected);
	psy_signal_connect(&workspace->player.signal_octavechanged, self,
		octavebar_on_octave_changed);
	psy_signal_connect(&workspace->signal_songchanged, self,
		octavebar_on_song_changed);	
}

void octavebar_build_octave_box(OctaveBar* self)
{
	int octave;
	char text[20];

	for (octave = 0; octave < 9; ++octave) {
		psy_snprintf(text, 20, "%d", octave);		
		psy_ui_combobox_add_text(&self->octavebox, text);
	}
	psy_ui_combobox_select(&self->octavebox,
		psy_audio_player_octave(&self->workspace->player));
}

void octavebar_on_octavebox_sekected(OctaveBar* self, psy_ui_Component* sender,
	intptr_t index)
{	
	if (index >= 0 && index <= 8) {
		psy_audio_player_set_octave(&self->workspace->player, (uint8_t)index);
	}
}

void octavebar_on_octave_changed(OctaveBar* self, psy_audio_Player* sender,
	intptr_t octave)
{
	psy_ui_combobox_select(&self->octavebox, psy_audio_player_octave(sender));
}

void octavebar_on_song_changed(OctaveBar* self, Workspace* sender)
{	
	psy_ui_combobox_select(&self->octavebox, psy_audio_player_octave(&sender->player));
}
