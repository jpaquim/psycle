// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "labelpair.h"

void labelpair_init(LabelPair* self, psy_ui_Component* parent, const char* desc,
	double desccharnumber)
{
	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_label_init_text(&self->desc, &self->component, NULL, desc);
	psy_ui_label_settextalignment(&self->desc, psy_ui_ALIGNMENT_LEFT);
	psy_ui_label_setcharnumber(&self->desc, desccharnumber);
	psy_ui_component_setalign(&self->desc.component, psy_ui_ALIGN_LEFT);
	psy_ui_label_init(&self->value, &self->component, NULL);
	psy_ui_label_preventtranslation(&self->value);
	psy_ui_label_settextalignment(&self->value, psy_ui_ALIGNMENT_LEFT);
	psy_ui_label_setcharnumber(&self->value, 10.0);
	psy_ui_component_setalign(&self->value.component, psy_ui_ALIGN_LEFT);
}

void labelpair_init_right(LabelPair* self, psy_ui_Component* parent, const char* desc,
	double desccharnumber)
{
	psy_ui_Margin margin;

	labelpair_init(self, parent, desc, desccharnumber);
	psy_ui_label_settextalignment(&self->desc,
		psy_ui_ALIGNMENT_RIGHT);	
	psy_ui_label_setcharnumber(&self->value, 0.0);
	psy_ui_component_setalign(&self->value.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setalign(&self->component, psy_ui_ALIGN_BOTTOM);
	psy_ui_margin_init_all_em(&margin, 0.0, 0.0, 0.0, 2.0);
	psy_ui_component_setmargin(&self->value.component, margin);
}

void labelpair_init_top(LabelPair* self, psy_ui_Component* parent, const char* desc,
	double desccharnumber)
{
	psy_ui_Margin margin;

	labelpair_init(self, parent, desc, desccharnumber);
	psy_ui_label_settextalignment(&self->desc, psy_ui_ALIGNMENT_TOP);	
	psy_ui_label_setcharnumber(&self->value, 0.0);
	psy_ui_component_setalign(&self->value.component, psy_ui_ALIGN_TOP);	
	psy_ui_margin_init_all_em(&margin, 0.5, 0.0, 0.0, 0.0);
	psy_ui_component_setmargin(&self->value.component, margin);
}