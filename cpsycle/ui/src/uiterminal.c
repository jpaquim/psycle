// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiterminal.h"
#include <string.h>

static void ondestroy(psy_ui_Terminal*, psy_ui_Component* sender);
static void onpreferredsize(psy_ui_Terminal*, psy_ui_Component* sender,
	ui_size* limit, ui_size* rv);

void ui_terminal_init(psy_ui_Terminal* self, psy_ui_Component* parent)
{			
	ui_component_init(&self->component, parent);	
	ui_component_enablealign(&self->component);
	ui_editor_init(&self->output, &self->component);
	ui_component_setalign(&self->output.component, UI_ALIGN_CLIENT);
	ui_component_resize(&self->component, 200, 200);
	psy_signal_disconnectall(&self->component.signal_preferredsize);	
	psy_signal_connect(&self->component.signal_preferredsize, self,
		onpreferredsize);
}

void ondestroy(psy_ui_Terminal* self, psy_ui_Component* sender)
{	
}

void onpreferredsize(psy_ui_Terminal* self, psy_ui_Component* sender,
	ui_size* limit, ui_size* rv)
{		
	if (rv) {
		ui_size size;

		size = ui_component_size(&self->component);
		*rv = size;		
	}
}

void ui_terminal_output(psy_ui_Terminal* self, const char* text)
{
	ui_editor_addtext(&self->output, text);
}

void ui_terminal_clear(psy_ui_Terminal* self)
{
	ui_editor_clear(&self->output);
}