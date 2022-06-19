/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "labelpair.h"
/* host */
#include "styles.h"

/* implementation */
void labelpair_init(LabelPair* self, psy_ui_Component* parent, const char* first,
	double desccharnumber)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_style_type(&self->component, STYLE_LABELPAIR);
	psy_ui_label_init_text(&self->first, &self->component, first);
	psy_ui_component_set_style_type(&self->first.component, STYLE_LABELPAIR_FIRST);
	psy_ui_label_set_textalignment(&self->first, psy_ui_ALIGNMENT_LEFT);
	psy_ui_label_set_charnumber(&self->first, desccharnumber);
	psy_ui_component_set_align(&self->first.component, psy_ui_ALIGN_LEFT);
	psy_ui_label_init(&self->second, &self->component);
	psy_ui_component_set_style_type(&self->second.component, STYLE_LABELPAIR_SECOND);
	psy_ui_label_prevent_translation(&self->second);
	psy_ui_label_set_textalignment(&self->second, psy_ui_ALIGNMENT_LEFT);
	psy_ui_label_set_charnumber(&self->second, 10.0);
	psy_ui_component_set_align(&self->second.component, psy_ui_ALIGN_LEFT);
}

void labelpair_init_right(LabelPair* self, psy_ui_Component* parent, const char* first,
	double desccharnumber)
{
	psy_ui_Margin margin;

	labelpair_init(self, parent, first, desccharnumber);
	psy_ui_label_set_textalignment(&self->first,
		psy_ui_ALIGNMENT_RIGHT);
	psy_ui_label_set_charnumber(&self->second, 0.0);
	psy_ui_component_set_align(&self->second.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_align(&self->component, psy_ui_ALIGN_BOTTOM);
	psy_ui_margin_init_em(&margin, 0.0, 0.0, 0.0, 2.0);
	psy_ui_component_set_margin(&self->second.component, margin);
}

void labelpair_init_top(LabelPair* self, psy_ui_Component* parent, const char* first,
	double desccharnumber)
{
	psy_ui_Margin margin;

	labelpair_init(self, parent, first, desccharnumber);
	psy_ui_label_set_textalignment(&self->first, psy_ui_ALIGNMENT_TOP);	
	psy_ui_label_set_charnumber(&self->second, 0.0);
	psy_ui_component_set_align(&self->second.component, psy_ui_ALIGN_TOP);	
	psy_ui_margin_init_em(&margin, 0.5, 0.0, 0.0, 0.0);
	psy_ui_component_set_margin(&self->second.component, margin);
}
