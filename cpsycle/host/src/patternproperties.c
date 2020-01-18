// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "patternproperties.h"
#include "../../detail/portable.h"

static void patternproperties_onapply(PatternProperties*,
	psy_ui_Component* sender);

void patternproperties_init(PatternProperties* self, psy_ui_Component* parent,
	psy_audio_Pattern* pattern)
{	
	psy_ui_Margin margin;

	self->pattern = pattern;
	ui_component_init(&self->component, parent);
	ui_component_enablealign(&self->component);
	psy_ui_label_init(&self->namelabel, &self->component);	
	psy_ui_label_settext(&self->namelabel, "Name");	
	psy_ui_edit_init(&self->nameedit, &self->component, 0);
	psy_ui_edit_settext(&self->nameedit, "No Pattern");		
	psy_ui_label_init(&self->lengthlabel, &self->component);
	psy_ui_label_settext(&self->lengthlabel, "Length");	
	psy_ui_edit_init(&self->lengthedit, &self->component, 0);
	psy_ui_button_init(&self->applybutton, &self->component);
	psy_ui_button_settext(&self->applybutton, "Apply");	
	psy_signal_connect(&self->applybutton.signal_clicked, self,
		patternproperties_onapply);
	psy_ui_margin_init(&margin, psy_ui_value_makepx(0), psy_ui_value_makepx(0),
		psy_ui_value_makeeh(0.5), psy_ui_value_makeew(1.0));
	psy_list_free(ui_components_setalign(
		ui_component_children(&self->component, 0),
		psy_ui_ALIGN_TOP,
		&margin));	
	
}

void patternproperties_setpattern(PatternProperties* self,
	psy_audio_Pattern* pattern)
{
	char buffer[20];
	self->pattern = pattern;
	if (self->pattern) {
		psy_ui_edit_settext(&self->nameedit, pattern->label);
		psy_snprintf(buffer, 20, "%.4f", self->pattern->length);		
	} else {
		psy_ui_edit_settext(&self->nameedit, "");
		psy_snprintf(buffer, 10, "");
	}
	psy_ui_edit_settext(&self->lengthedit, buffer);
}

void patternproperties_onapply(PatternProperties* self,
	psy_ui_Component* sender)
{
	if (self->pattern) {
		pattern_setlabel(self->pattern, psy_ui_edit_text(&self->nameedit));
		pattern_setlength(self->pattern,
			(psy_dsp_beat_t)atof(psy_ui_edit_text(&self->lengthedit)));
	}
}
