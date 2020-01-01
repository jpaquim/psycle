// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "octavebar.h"
#include <portable.h>

static void OnDestroy(OctaveBar*, psy_ui_Component* component);
static void BuildOctaveBox(OctaveBar* self);
static void OnOctaveBoxSelChange(OctaveBar*, psy_ui_Component* sender, int sel);
static void OnOctaveChanged(OctaveBar*, Workspace*, int octave);
static void OnSongChanged(OctaveBar*, Workspace*);

void octavebar_init(OctaveBar* self, psy_ui_Component* parent, Workspace* workspace)
{	
	self->workspace = workspace;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	ui_component_setalignexpand(&self->component, UI_HORIZONTALEXPAND);
	psy_signal_connect(&self->component.signal_destroy, self, OnDestroy);	
	ui_label_init(&self->headerlabel, &self->component);	
	ui_label_settext(&self->headerlabel, "Octave");		
	ui_combobox_init(&self->octavebox, &self->component);
	ui_combobox_setcharnumber(&self->octavebox, 2);	
	BuildOctaveBox(self);	
	psy_signal_connect(&self->octavebox.signal_selchanged, self,
		OnOctaveBoxSelChange);	
	psy_signal_connect(&workspace->signal_octavechanged, self, OnOctaveChanged);
	psy_signal_connect(&workspace->signal_songchanged, self, OnSongChanged);
	{		
		ui_margin margin;

		ui_margin_init(&margin, ui_value_makepx(0), ui_value_makeew(2.0),
			ui_value_makepx(0), ui_value_makepx(0));				
		psy_list_free(ui_components_setalign(
			ui_component_children(&self->component, 0),
			UI_ALIGN_LEFT,
			&margin));		
	}
}

void OnDestroy(OctaveBar* self, psy_ui_Component* component)
{
}

void BuildOctaveBox(OctaveBar* self)
{
	int octave;
	char text[20];

	for (octave = 0; octave < 9; ++octave) {
		psy_snprintf(text, 20, "%d", octave);		
		ui_combobox_addstring(&self->octavebox, text);
	}
	ui_combobox_setcursel(&self->octavebox, self->workspace->octave);
}

void OnOctaveBoxSelChange(OctaveBar* self, psy_ui_Component* sender, int sel)
{	
	workspace_setoctave(self->workspace, sel);
}

void OnOctaveChanged(OctaveBar* self, Workspace* workspace, int octave)
{
	ui_combobox_setcursel(&self->octavebox, workspace->octave);
}

void OnSongChanged(OctaveBar* self, Workspace* workspace)
{	
	ui_combobox_setcursel(&self->octavebox, workspace->octave);
}
