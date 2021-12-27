/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"

#include "metronomebar.h"
/* host */
#include "resources/resource.h"
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void metronomebar_fillprecount(MetronomeBar*);
static void metronomebar_ontogglemetronomestate(MetronomeBar*);
static void metronomebar_onconfigure(MetronomeBar*, psy_ui_Button* sender);
static void metronomebar_onprecountchanged(MetronomeBar*,
	psy_ui_Component* sender, int index);

/* implementation */
void metronomebar_init(MetronomeBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{				
	psy_ui_component_init(metronomebar_base(self), parent, NULL);
	psy_ui_component_setdefaultalign(metronomebar_base(self),
		psy_ui_ALIGN_LEFT, psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->workspace = workspace;
	self->player = &workspace->player;
	/* activated */
	psy_ui_button_init_text_connect(&self->activated, metronomebar_base(self),
		"metronome.metronome", self, metronomebar_ontogglemetronomestate);
	/* precount */
	psy_ui_label_init_text(&self->desc, metronomebar_base(self),
		"metronome.precount");
	psy_ui_combobox_init(&self->precount, metronomebar_base(self));
	psy_ui_combobox_setcharnumber(&self->precount, 6);
	metronomebar_fillprecount(self);
	psy_ui_combobox_setcursel(&self->precount, 0);
	psy_signal_connect(&self->precount.signal_selchanged, self,
		metronomebar_onprecountchanged);
	/* configure */
	psy_ui_button_init_connect(&self->configure, metronomebar_base(self),
		self, metronomebar_onconfigure);
	psy_ui_button_loadresource(&self->configure, IDB_SETTINGS_LIGHT,
		IDB_SETTINGS_DARK, psy_ui_colour_white());
}

void metronomebar_fillprecount(MetronomeBar* self)
{
	uintptr_t i;

	psy_ui_combobox_clear(&self->precount);
	for (i = 0; i < 6; ++i) {
		char str[4];

		psy_snprintf(str, 4, "%d", (int)i);
		psy_ui_combobox_addtext(&self->precount, str);
	}	
}

void metronomebar_ontogglemetronomestate(MetronomeBar* self)
{
	if (psy_ui_button_highlighted(&self->activated)) {
		psy_ui_button_disablehighlight(&self->activated);
		psy_audio_player_deactivatemetronome(self->player);		
	} else {
		psy_ui_button_highlight(&self->activated);
		psy_audio_player_activatemetronome(self->player);		
	}
}

void metronomebar_onconfigure(MetronomeBar* self, psy_ui_Button* sender)
{
	workspace_selectview(self->workspace, VIEW_ID_SETTINGSVIEW, 10, 0);
}

void metronomebar_onprecountchanged(MetronomeBar* self,
	psy_ui_Component* sender, int index)
{
	self->player->sequencer.metronome.precount = (double)index;
}
