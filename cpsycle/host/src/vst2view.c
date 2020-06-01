// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "vst2view.h"

#include <uiwincomponentimp.h>

#include <string.h>
#include <stdlib.h>

#define TIMERID_VST2VIEW 420

static void onpreferredsize(Vst2View* self, psy_ui_Size* limit, psy_ui_Size* rv);
static void ontimer(Vst2View*, int id);

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
static psy_ui_win_ComponentImp* psy_ui_win_component_details(psy_ui_Component* self)
{
	return (psy_ui_win_ComponentImp*)self->imp;
}
#endif

static psy_ui_ComponentVtable vtable;
static int vtable_initialized = 0;

static void vtable_init(Vst2View* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.onpreferredsize = (psy_ui_fp_onpreferredsize) onpreferredsize;
		vtable.ontimer = (psy_ui_fp_ontimer) ontimer;
	}
}

void vst2view_init(Vst2View* self, psy_ui_Component* parent, psy_audio_Machine* machine,
	Workspace* workspace)
{		
	self->machine = machine;
	psy_ui_component_init(&self->component, parent);
	vtable_init(self);
	self->component.vtable = &vtable;
#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
	psy_audio_machine_seteditorhandle(machine,
		(void*) psy_ui_win_component_details(&self->component)->hwnd);
#endif
	psy_ui_component_starttimer(&self->component, TIMERID_VST2VIEW, 50);
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

void ontimer(Vst2View* self, int timerid)
{	
	psy_audio_machine_editoridle(self->machine);
}

void onpreferredsize(Vst2View* self, psy_ui_Size* limit, psy_ui_Size* rv)
{
	if (rv) {		
		int width;
		int height;

		psy_audio_machine_editorsize(self->machine, &width, &height);
		rv->width = psy_ui_value_makepx(width);
		rv->height = psy_ui_value_makepx(height);
	}
}
