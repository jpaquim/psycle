// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "vst2view.h"

#define TIMERID_VST2VIEW 420

static void onpreferredsize(Vst2View* self, psy_ui_Component* sender,
	ui_size* limit, ui_size* rv);
static void ontimer(Vst2View*, psy_ui_Component* sender, int id);

void vst2view_init(Vst2View* self, psy_ui_Component* parent, psy_audio_Machine* machine,
	Workspace* workspace)
{		
	self->machine = machine;
	ui_component_init(&self->component, parent);
	machine->vtable->seteditorhandle(machine, (void*)self->component.hwnd);
	psy_signal_connect(&self->component.signal_timer, self, ontimer);
		psy_signal_disconnectall(&self->component.signal_preferredsize);
	psy_signal_connect(&self->component.signal_preferredsize, self,
		onpreferredsize);
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

void onpreferredsize(Vst2View* self, psy_ui_Component* sender, ui_size* limit,
	ui_size* rv)
{
	if (rv) {		
		machine_editorsize(self->machine, &rv->width, &rv->height);		
	} else {
		*rv = ui_component_size(&self->component);
	}
}
