// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiterminal.h"

#include <string.h>

void psy_ui_terminal_init(psy_ui_Terminal* self, psy_ui_Component* parent)
{			
	psy_ui_component_init(&self->component, parent);
	psy_ui_component_enablealign(&self->component);
	psy_ui_editor_init(&self->output, &self->component);
	psy_ui_component_setalign(&self->output.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_resize(&self->component,
		psy_ui_size_make(psy_ui_value_makeew(20.0),
		psy_ui_value_makeeh(20.0)));
	psy_ui_component_preventpreferredsize(self);
}

void psy_ui_terminal_output(psy_ui_Terminal* self, const char* text)
{
	psy_ui_editor_addtext(&self->output, text);
}

void psy_ui_terminal_clear(psy_ui_Terminal* self)
{
	psy_ui_editor_clear(&self->output);
}
