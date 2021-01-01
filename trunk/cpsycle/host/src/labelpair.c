// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "labelpair.h"

void labelpair_init(LabelPair* self, psy_ui_Component* parent, const char* desc)
{
	psy_ui_component_init(&self->component, parent);	
	psy_ui_label_init_text(&self->desc, &self->component, desc);
	psy_ui_label_settextalignment(&self->desc, psy_ui_ALIGNMENT_LEFT);
	psy_ui_label_setcharnumber(&self->desc, 25);
	psy_ui_component_setalign(&self->desc.component, psy_ui_ALIGN_LEFT);
	psy_ui_label_init(&self->value, &self->component);
	psy_ui_label_settextalignment(&self->value, psy_ui_ALIGNMENT_LEFT);
	psy_ui_label_setcharnumber(&self->value, 10);
	psy_ui_component_setalign(&self->value.component, psy_ui_ALIGN_LEFT);
}
