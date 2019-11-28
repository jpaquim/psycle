// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "vst2view.h"

#define TIMERID_VST2VIEW 420

static void ontimer(Vst2View*, ui_component* sender, int id);

void InitVst2View(Vst2View* self, ui_component* parent, Machine* plugin)
{		
	self->plugin = plugin;
	ui_component_init(&self->component, parent);
	plugin->vtable->seteditorhandle(plugin, (void*)self->component.hwnd);
	signal_connect(&self->component.signal_timer, self, ontimer);
	ui_component_starttimer(&self->component, TIMERID_VST2VIEW, 50);
}

void ontimer(Vst2View* self, ui_component* sender, int id)
{	
	self->plugin->vtable->editoridle(self->plugin);
}
