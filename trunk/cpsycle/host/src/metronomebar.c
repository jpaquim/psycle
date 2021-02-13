// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "metronomebar.h"
// platform
#include "../../detail/portable.h"

// prototypes
static void metronomebar_fillprecount(MetronomeBar*);
static void metronomebar_ontogglemetronomestate(MetronomeBar*);
static void metronomebar_onconfigure(MetronomeBar*, psy_ui_Button* sender);

// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static psy_ui_ComponentVtable* vtable_init(MetronomeBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable_initialized = TRUE;
	}
	return &vtable;
}
// implementation
void metronomebar_init(MetronomeBar* self, psy_ui_Component* parent, Workspace* workspace)
{				
	psy_ui_component_init(metronomebar_base(self), parent);
	psy_ui_component_setvtable(metronomebar_base(self), vtable_init(self));
	psy_ui_component_setdefaultalign(metronomebar_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	self->workspace = workspace;
	self->player = &workspace->player;
	// activated
	psy_ui_button_init_text_connect(&self->activated, metronomebar_base(self),
		"metronome.metronome", self, metronomebar_ontogglemetronomestate);
	// precount
	psy_ui_label_init_text(&self->desc, metronomebar_base(self),
		"metronome.precount");
	psy_ui_combobox_init(&self->precount, metronomebar_base(self));
	psy_ui_combobox_setcharnumber(&self->precount, 6);
	metronomebar_fillprecount(self);
	psy_ui_combobox_setcursel(&self->precount, 2);
	// configure
	psy_ui_button_init_text_connect(&self->configure, metronomebar_base(self),
		"metronome.configure", self, metronomebar_onconfigure);
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
		self->workspace->player.sequencer.metronome = FALSE;
	} else {
		psy_ui_button_highlight(&self->activated);
		self->workspace->player.sequencer.metronome = TRUE;
	}
}

void metronomebar_onconfigure(MetronomeBar* self, psy_ui_Button* sender)
{
	workspace_selectview(self->workspace, VIEW_ID_SETTINGSVIEW, 10, 0);
}