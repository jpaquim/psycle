// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "octavebar.h"

#include <songio.h>

#include "../../detail/portable.h"

static void octavebar_initalign(OctaveBar*);
static void octavebar_updatetext(OctaveBar*, Translator*);
static void octavebar_onlanguagechanged(OctaveBar*, Translator* sender);
static void octavebar_buildoctavebox(OctaveBar*);
static void octavebar_onoctaveboxselchange(OctaveBar*, psy_ui_Component* sender, int sel);
static void octavebar_onoctavechanged(OctaveBar*, Workspace*, int octave);
static void octavebar_onsongchanged(OctaveBar*, Workspace*, int flag, psy_audio_SongFile*);

void octavebar_init(OctaveBar* self, psy_ui_Component* parent, Workspace* workspace)
{
	self->workspace = workspace;
	psy_ui_component_init(octavebar_base(self), parent);
	psy_ui_component_enablealign(octavebar_base(self));
	psy_ui_component_setalignexpand(octavebar_base(self),
		psy_ui_HORIZONTALEXPAND);
	psy_ui_label_init(&self->headerlabel, octavebar_base(self));
	psy_ui_combobox_init(&self->octavebox, octavebar_base(self));
	psy_ui_combobox_setcharnumber(&self->octavebox, 2);	
	octavebar_buildoctavebox(self);
	psy_signal_connect(&self->octavebox.signal_selchanged, self,
		octavebar_onoctaveboxselchange);
	psy_signal_connect(&workspace->signal_octavechanged, self,
		octavebar_onoctavechanged);
	psy_signal_connect(&workspace->signal_songchanged, self,
		octavebar_onsongchanged);
	psy_signal_connect(&workspace->signal_languagechanged, self,
		octavebar_onlanguagechanged);
	octavebar_updatetext(self, &workspace->translator);
	octavebar_initalign(self);
}

void octavebar_updatetext(OctaveBar* self, Translator* translator)
{
	psy_ui_label_settext(&self->headerlabel,
		translator_translate(&self->workspace->translator,
			"octavebar.octave"));
}

void octavebar_onlanguagechanged(OctaveBar* self, Translator* sender)
{
	octavebar_updatetext(self, sender);
}

void octavebar_initalign(OctaveBar* self)
{
	psy_ui_Margin margin;

	psy_ui_margin_init_all(&margin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(2.0), psy_ui_value_makepx(0),
		psy_ui_value_makepx(0));
	psy_list_free(psy_ui_components_setalign(
		psy_ui_component_children(octavebar_base(self), psy_ui_NONRECURSIVE),
		psy_ui_ALIGN_LEFT,
		&margin));
}

void octavebar_buildoctavebox(OctaveBar* self)
{
	int octave;
	char text[20];

	for (octave = 0; octave < 9; ++octave) {
		psy_snprintf(text, 20, "%d", octave);		
		psy_ui_combobox_addtext(&self->octavebox, text);
	}
	psy_ui_combobox_setcursel(&self->octavebox, self->workspace->octave);
}

void octavebar_onoctaveboxselchange(OctaveBar* self, psy_ui_Component* sender, int sel)
{	
	workspace_setoctave(self->workspace, sel);
}

void octavebar_onoctavechanged(OctaveBar* self, Workspace* workspace, int octave)
{
	psy_ui_combobox_setcursel(&self->octavebox, workspace->octave);
}

void octavebar_onsongchanged(OctaveBar* self, Workspace* workspace, int flag, psy_audio_SongFile* songfile)
{	
	psy_ui_combobox_setcursel(&self->octavebox, workspace->octave);
}
