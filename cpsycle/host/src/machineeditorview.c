// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "machineeditorview.h"

#include <imps/win32/uiwincomponentimp.h>

#include <stdlib.h>
#include <string.h>

static void machineeditorview_on_destroyed(MachineEditorView*);
static void machineeditorview_onpreferredsize(MachineEditorView*,
	psy_ui_Size* limit, psy_ui_Size* rv);
static void machineeditorview_on_timer(MachineEditorView*, uintptr_t id);
static void machineeditorview_onmachineeditresize(MachineEditorView*,
	Workspace* sender, psy_audio_Machine*, intptr_t w, intptr_t h);

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
static psy_ui_win_ComponentImp* psy_ui_win_component_details(psy_ui_Component* self)
{
	return (psy_ui_win_ComponentImp*)self->imp->vtable->dev_platform(self->imp);
}
#endif

static psy_ui_ComponentVtable machineeditorview_vtable;
static bool machineeditorview_vtable_initialized = FALSE;

static void machineeditorview_vtable_init(MachineEditorView* self)
{
	if (!machineeditorview_vtable_initialized) {
		machineeditorview_vtable = *(self->component.vtable);
		machineeditorview_vtable.on_destroyed =
			(psy_ui_fp_component_event)
			machineeditorview_on_destroyed;
		machineeditorview_vtable.onpreferredsize =
			(psy_ui_fp_component_on_preferred_size)
			machineeditorview_onpreferredsize;
		machineeditorview_vtable.on_timer =
			(psy_ui_fp_component_on_timer)
			machineeditorview_on_timer;
		machineeditorview_vtable_initialized = TRUE;
	}
	self->component.vtable = &machineeditorview_vtable;
}

void machineeditorview_init(MachineEditorView* self, psy_ui_Component* parent,
	psy_audio_Machine* machine, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent, NULL);
	machineeditorview_vtable_init(self);	
	self->machine = machine;
	self->workspace = workspace;
	psy_audio_machine_seteditorhandle(machine,
		(void*)self->component.imp->vtable->dev_platform(
			self->component.imp));
	psy_signal_connect(&workspace->signal_machineeditresize, self,
		machineeditorview_onmachineeditresize);	
}

void machineeditorview_on_destroyed(MachineEditorView* self)
{
	if (self->machine) {
		psy_audio_machine_seteditorhandle(self->machine, NULL);
		psy_signal_disconnect_context(&self->workspace->signal_machineeditresize, self);
	}
}

MachineEditorView* machineeditorview_alloc(void)
{
	return (MachineEditorView*) malloc(sizeof(MachineEditorView));
}

MachineEditorView* machineeditorview_allocinit(psy_ui_Component* parent,
	psy_audio_Machine* machine, Workspace* workspace)
{
	MachineEditorView* rv;

	rv = machineeditorview_alloc();
	if (rv) {
		machineeditorview_init(rv, parent, machine, workspace);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;	
}

void machineeditorview_on_timer(MachineEditorView* self, uintptr_t timerid)
{	
	psy_audio_machine_editoridle(self->machine);
}

void machineeditorview_onpreferredsize(MachineEditorView* self, psy_ui_Size* limit,
	psy_ui_Size* rv)
{		
	double width;
	double height;

	psy_audio_machine_editorsize(self->machine, &width, &height);
	rv->width = psy_ui_value_make_px(width);
	rv->height = psy_ui_value_make_px(height);	
}

void machineeditorview_onmachineeditresize(MachineEditorView* self, Workspace* sender,
	psy_audio_Machine* machine, intptr_t width, intptr_t height)
{	
	// In case to machineproxy self->machine and machine are different ptrs,
	// therefore the machine slots are compared
	if (self->machine && psy_audio_machine_slot(machine) ==
			psy_audio_machine_slot(self->machine)) {
		// change preferred size
		psy_ui_component_set_preferred_size(machineeditorview_base(self),
			psy_ui_size_make_px((double)width, (double)height));
		// signal will be catched by machineframe to resize the frame
		psy_signal_emit(&self->component.signal_preferredsizechanged, self, 0);
	}
}
