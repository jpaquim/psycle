/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "octavebar.h"
/* audio */
#include <songio.h>
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void octavebar_buildoctavebox(OctaveBar*);
static void octavebar_onoctaveboxselchange(OctaveBar*,
	psy_ui_Component* sender, intptr_t sel);
static void octavebar_onoctavechanged(OctaveBar*, Workspace*,
	intptr_t octave);
static void octavebar_onsongchanged(OctaveBar*, Workspace* sender);

/* implementation */
void octavebar_init(OctaveBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{	
	psy_ui_component_init(octavebar_base(self), parent, NULL);
	psy_ui_component_set_defaultalign(&self->component, psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_component_set_align_expand(octavebar_base(self),
		psy_ui_HEXPAND);
	self->workspace = workspace;
	psy_ui_label_init_text(&self->headerlabel, octavebar_base(self),
		"octavebar.octave");
	psy_ui_combobox_init(&self->octavebox, octavebar_base(self));
	psy_ui_combobox_setcharnumber(&self->octavebox, 3);	
	octavebar_buildoctavebox(self);
	psy_signal_connect(&self->octavebox.signal_selchanged, self,
		octavebar_onoctaveboxselchange);
	psy_signal_connect(&workspace->signal_octavechanged, self,
		octavebar_onoctavechanged);
	psy_signal_connect(&workspace->signal_songchanged, self,
		octavebar_onsongchanged);	
}

void octavebar_buildoctavebox(OctaveBar* self)
{
	int octave;
	char text[20];

	for (octave = 0; octave < 9; ++octave) {
		psy_snprintf(text, 20, "%d", octave);		
		psy_ui_combobox_addtext(&self->octavebox, text);
	}
	psy_ui_combobox_setcursel(&self->octavebox,
		workspace_octave(self->workspace));
}

void octavebar_onoctaveboxselchange(OctaveBar* self, psy_ui_Component* sender,
	intptr_t sel)
{	
	if (self >= 0 && sel <= 8) {
		workspace_set_octave(self->workspace, (uint8_t)sel);
	}
}

void octavebar_onoctavechanged(OctaveBar* self, Workspace* workspace,
	intptr_t octave)
{
	psy_ui_combobox_setcursel(&self->octavebox,
		workspace_octave(self->workspace));
}

void octavebar_onsongchanged(OctaveBar* self, Workspace* sender)
{	
	psy_ui_combobox_setcursel(&self->octavebox,
		workspace_octave(sender));
}
