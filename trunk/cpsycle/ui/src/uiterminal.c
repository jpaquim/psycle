// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiterminal.h"
#include <string.h>

static void ondestroy(ui_terminal*, ui_component* sender);
static void ondraw(ui_terminal*, ui_component* sender, ui_graphics*);
static void onpreferredsize(ui_terminal*, ui_component* sender, ui_size* limit,
	ui_size* rv);

void ui_terminal_init(ui_terminal* self, ui_component* parent)
{			
	ui_component_init(&self->component, parent);	
}

void ondestroy(ui_terminal* self, ui_component* sender)
{	
}

void ondraw(ui_terminal* self, ui_component* sender, ui_graphics* g)
{	
}

void onpreferredsize(ui_terminal* self, ui_component* sender, ui_size* limit,
	ui_size* rv)
{		
	if (rv) {
		rv->width = 200;
		rv->height = 200;
	}
}

void ui_terminal_output(ui_terminal* self, char* text)
{

}
