/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "metronomebar.h"
/* host */
#include "resources/resource.h"
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void metronomebar_fill_precount(MetronomeBar*);
static void metronomebar_on_toggle_metronome_state(MetronomeBar*);
static void metronomebar_on_configure(MetronomeBar*, psy_ui_Button* sender);
static void metronomebar_on_precount_changed(MetronomeBar*,
	psy_ui_Component* sender, int index);

/* implementation */
void metronomebar_init(MetronomeBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{				
	psy_ui_component_init(metronomebar_base(self), parent, NULL);
	psy_ui_component_set_default_align(metronomebar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_component_set_align_expand(metronomebar_base(self), psy_ui_HEXPAND);
	self->workspace = workspace;
	self->player = &workspace->player;
	/* activated */
	psy_ui_button_init_text_connect(&self->activated, metronomebar_base(self),
		"metronome.metronome", self, metronomebar_on_toggle_metronome_state);
	/* precount */
	psy_ui_label_init_text(&self->desc, metronomebar_base(self),
		"metronome.precount");
	psy_ui_combobox_init(&self->precount, metronomebar_base(self));
	psy_ui_combobox_set_char_number(&self->precount, 6);
	metronomebar_fill_precount(self);
	psy_ui_combobox_select(&self->precount, 0);
	psy_signal_connect(&self->precount.signal_selchanged, self,
		metronomebar_on_precount_changed);
	/* configure */
	psy_ui_button_init_connect(&self->configure, metronomebar_base(self),
		self, metronomebar_on_configure);
	psy_ui_button_load_resource(&self->configure, IDB_SETTINGS_LIGHT,
		IDB_SETTINGS_DARK, psy_ui_colour_white());
}

void metronomebar_fill_precount(MetronomeBar* self)
{
	uintptr_t i;

	psy_ui_combobox_clear(&self->precount);
	for (i = 0; i < 6; ++i) {
		char str[4];

		psy_snprintf(str, 4, "%d", (int)i);
		psy_ui_combobox_add_text(&self->precount, str);
	}	
}

void metronomebar_on_toggle_metronome_state(MetronomeBar* self)
{
	if (psy_ui_button_highlighted(&self->activated)) {
		psy_ui_button_disable_highlight(&self->activated);
		psy_audio_player_deactivatemetronome(self->player);		
	} else {
		psy_ui_button_highlight(&self->activated);
		psy_audio_player_activatemetronome(self->player);		
	}
}

void metronomebar_on_configure(MetronomeBar* self, psy_ui_Button* sender)
{
	workspace_select_view(self->workspace,
		viewindex_make(VIEW_ID_SETTINGSVIEW, 10, 0, psy_INDEX_INVALID));
}

void metronomebar_on_precount_changed(MetronomeBar* self,
	psy_ui_Component* sender, int index)
{
	self->player->sequencer.metronome.precount = (double)index;
}
