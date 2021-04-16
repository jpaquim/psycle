// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "arrowui.h"
// platform
#include "../../detail/portable.h"

// ArrowUi
static void arrowui_ondraw(ArrowUi*, psy_ui_Graphics*);
static void arrowui_drawbackground(ArrowUi*, psy_ui_Graphics*);
static void arrowui_onpreferredsize(ArrowUi*, const psy_ui_Size* limit,
	psy_ui_Size* rv);
static void arrowui_onmousedown(ArrowUi*, psy_ui_MouseEvent*);
static bool arrowui_selected(const ArrowUi*);
// vtable
static psy_ui_ComponentVtable arrowui_vtable;
static bool arrowui_vtable_initialized = FALSE;

static psy_ui_ComponentVtable* arrowui_vtable_init(ArrowUi* self)
{
	assert(self);

	if (!arrowui_vtable_initialized) {
		arrowui_vtable = *(self->component.vtable);		
		arrowui_vtable.ondraw = (psy_ui_fp_component_ondraw)arrowui_ondraw;		
		arrowui_vtable.onpreferredsize = (psy_ui_fp_component_onpreferredsize)
			arrowui_onpreferredsize;
		arrowui_vtable.onmousedown = (psy_ui_fp_component_onmouseevent)
			arrowui_onmousedown;
		arrowui_vtable_initialized = TRUE;
	}
	return &arrowui_vtable;
}
// implementation
void arrowui_init(ArrowUi* self, psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_Wire wire, MachineViewSkin* skin,
	Workspace* workspace)
{
	assert(self);
	assert(workspace);
	assert(workspace->song);
	assert(skin);
	assert(view);

	psy_ui_component_init(&self->component, parent, view);	
	arrowui_vtable_init(self);
	self->component.vtable = &arrowui_vtable;
	psy_ui_component_setbackgroundmode(&self->component,
		psy_ui_NOBACKGROUND);
	self->skin = skin;
	self->wire = wire;
	self->workspace = workspace;	
}

ArrowUi* arrowui_alloc(void)
{
	return (ArrowUi*)malloc(sizeof(ArrowUi));
}

ArrowUi* arrowui_allocinit(psy_ui_Component* parent,
	psy_ui_Component* view, psy_audio_Wire wire, MachineViewSkin* skin,
	Workspace* workspace)
{
	ArrowUi* rv;

	rv = arrowui_alloc();
	if (rv) {
		arrowui_init(rv, parent, view, wire, skin, workspace);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void arrowui_ondraw(ArrowUi* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	assert(self);
	
	if (arrowui_selected(self)) {
		psy_ui_setcolour(g, self->skin->selwirecolour);
	} else {
		psy_ui_setcolour(g, self->skin->wireaacolour2);
	}
	size = psy_ui_component_innersize_px(&self->component);
	psy_ui_drawline(g, 
		psy_ui_realpoint_make(0, size.height / 2),
		psy_ui_realpoint_make(size.width / 2, size.height / 2));
	psy_ui_drawline(g,
		psy_ui_realpoint_make(size.width / 2, size.height / 2),
		psy_ui_realpoint_make(size.width / 2, size.height));
}

bool arrowui_selected(const ArrowUi* self)
{
	if (workspace_song(self->workspace) &&
			psy_audio_wire_valid(&self->wire)) {
		psy_audio_Machines* machines;
		psy_audio_Wire selectedwire;

		machines = &self->workspace->song->machines;
		selectedwire = psy_audio_machines_selectedwire(machines);
		return psy_audio_wire_equal(&self->wire, &selectedwire);
	}
	return FALSE;
}

void arrowui_onpreferredsize(ArrowUi* self, const psy_ui_Size* limit,
	psy_ui_Size* rv)
{	
	psy_ui_size_setreal(rv, 		
		psy_ui_realrectangle_size(&
		self->skin->effect.background.dest));
}

void arrowui_onmousedown(ArrowUi* self, psy_ui_MouseEvent* ev)
{
	if (workspace_song(self->workspace)) {
		psy_audio_Machines* machines;

		machines = &self->workspace->song->machines;
		psy_audio_machines_selectwire(machines, self->wire);
	}
}
