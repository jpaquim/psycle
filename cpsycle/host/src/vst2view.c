// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "vst2view.h"

#define TIMERID_VST2VIEW 420

static void onpreferredsize(Vst2View* self, psy_ui_Size* limit, psy_ui_Size* rv);
static void ontimer(Vst2View*, psy_ui_Component* sender, int id);

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(Vst2View* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize) onpreferredsize;
	}
}

void vst2view_init(Vst2View* self, psy_ui_Component* parent, psy_audio_Machine* machine,
	Workspace* workspace)
{		
	self->machine = machine;
	ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
	machine->vtable->seteditorhandle(machine,
		(void*) self->component.hwnd);
//		(void*)ui_component_platformhandle(&self->component));	
	ui_component_starttimer(&self->component, TIMERID_VST2VIEW, 50);
}

Vst2View* vst2view_alloc(void)
{
	return (Vst2View*) malloc(sizeof(Vst2View));
}

Vst2View* vst2view_allocinit(psy_ui_Component* parent, psy_audio_Machine* machine,
	Workspace* workspace)
{
	Vst2View* rv;

	rv = vst2view_alloc();
	if (rv) {
		vst2view_init(rv, parent, machine, workspace);
	}
	return rv;	
}

void ontimer(Vst2View* self, psy_ui_Component* sender, int timerid)
{	
	machine_editoridle(self->machine);
}

void onpreferredsize(Vst2View* self, psy_ui_Size* limit, psy_ui_Size* rv)
{
	if (rv) {		
		machine_editorsize(self->machine, &rv->width, &rv->height);		
	} else {
		*rv = ui_component_size(&self->component);
	}
}
