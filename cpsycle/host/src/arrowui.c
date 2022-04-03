/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "arrowui.h"
/* host */
#include "styles.h" 

/* ArrowUi */

/* prototypes */
static void arrowui_ondraw(ArrowUi*, psy_ui_Graphics*);
static void arrowui_on_mouse_down(ArrowUi*, psy_ui_MouseEvent*);
static bool arrowui_selected(const ArrowUi*);

/* vtable */
static psy_ui_ComponentVtable arrowui_vtable;
static bool arrowui_vtable_initialized = FALSE;

static void arrowui_vtable_init(ArrowUi* self)
{
	assert(self);

	if (!arrowui_vtable_initialized) {
		arrowui_vtable = *(self->component.vtable);		
		arrowui_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			arrowui_ondraw;		
		arrowui_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			arrowui_on_mouse_down;
		arrowui_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &arrowui_vtable);
}

/* implementation */
void arrowui_init(ArrowUi* self, psy_ui_Component* parent,
	psy_audio_Wire wire, Workspace* workspace)
{
	assert(self);
	assert(workspace);
	assert(workspace->song);	

	psy_ui_component_init(&self->component, parent, NULL);	
	arrowui_vtable_init(self);	
	psy_ui_component_set_style_type(&self->component, STYLE_MV_ARROW);
	self->wire = wire;
	self->workspace = workspace;	
}

ArrowUi* arrowui_alloc(void)
{
	return (ArrowUi*)malloc(sizeof(ArrowUi));
}

ArrowUi* arrowui_allocinit(psy_ui_Component* parent,
	psy_audio_Wire wire, Workspace* workspace)
{
	ArrowUi* rv;

	rv = arrowui_alloc();
	if (rv) {
		arrowui_init(rv, parent, wire, workspace);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void arrowui_ondraw(ArrowUi* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	psy_ui_Style* style;

	assert(self);
	
	if (arrowui_selected(self)) {		
		style = psy_ui_style(STYLE_MV_WIRE_SELECT);		
	} else {
		style = psy_ui_style(STYLE_MV_WIRE);		
	}
	psy_ui_setcolour(g, style->colour);
	size = psy_ui_component_size_px(&self->component);
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

void arrowui_on_mouse_down(ArrowUi* self, psy_ui_MouseEvent* ev)
{
	if (workspace_song(self->workspace)) {
		psy_audio_Machines* machines;

		machines = &self->workspace->song->machines;
		psy_audio_machines_selectwire(machines, self->wire);
	}
}
