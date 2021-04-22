// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiterminal.h"
// platform
#include "../../detail/portable.h"

// prototypes
static void psy_ui_terminal_ondestroy(psy_ui_Terminal*);
static void psy_ui_terminal_ontimer(psy_ui_Terminal* self, uintptr_t id);
// vtable
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static psy_ui_ComponentVtable* vtable_init(psy_ui_Terminal* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy = (psy_ui_fp_component_ondestroy)
			psy_ui_terminal_ondestroy;
		vtable.ontimer = (psy_ui_fp_component_ontimer)
			psy_ui_terminal_ontimer;
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;	
}

void psy_ui_terminal_init(psy_ui_Terminal* self, psy_ui_Component* parent)
{			
	psy_ui_component_init(&self->component, parent, NULL);	
	psy_ui_editor_init(&self->output, &self->component);	
	psy_ui_editor_enablewrap(&self->output);
	psy_ui_component_setalign(&self->output.component, psy_ui_ALIGN_CLIENT);
	psy_ui_component_setpreferredsize(&self->component,
		psy_ui_size_make_em(20.0, 20.0));	
	// psy_ui_component_starttimer(&self->component, 0, 50);
}

void psy_ui_terminal_ondestroy(psy_ui_Terminal* self)
{	
}

void psy_ui_terminal_output(psy_ui_Terminal* self, const char* text)
{
	// todo: thread syncronize
	psy_ui_editor_addtext(&self->output, text);
}

void psy_ui_terminal_clear(psy_ui_Terminal* self)
{
	psy_ui_editor_clear(&self->output);
}

void psy_ui_terminal_ontimer(psy_ui_Terminal* self, uintptr_t id)
{

}
