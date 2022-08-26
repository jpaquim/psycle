/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "arrowui.h"
/* host */
#include "styles.h" 


/* prototypes */
static void arrowui_on_destroyed(ArrowUi*);
static void arrowui_on_draw(ArrowUi*, psy_ui_Graphics*);
static void arrowui_on_mouse_down(ArrowUi*, psy_ui_MouseEvent*);
static void arrowui_on_wire_selected(ArrowUi*, psy_audio_Machines* sender);
static void arrowui_select_wire(ArrowUi*);

/* vtable */
static psy_ui_ComponentVtable arrowui_vtable;
static bool arrowui_vtable_initialized = FALSE;

static void arrowui_vtable_init(ArrowUi* self)
{
	assert(self);

	if (!arrowui_vtable_initialized) {
		arrowui_vtable = *(self->component.vtable);
		arrowui_vtable.on_destroyed =
			(psy_ui_fp_component)
			arrowui_on_destroyed;
		arrowui_vtable.ondraw =
			(psy_ui_fp_component_ondraw)
			arrowui_on_draw;
		arrowui_vtable.on_mouse_down =
			(psy_ui_fp_component_on_mouse_event)
			arrowui_on_mouse_down;
		arrowui_vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &arrowui_vtable);
}

/* implementation */
void arrowui_init(ArrowUi* self, psy_ui_Component* parent,
	psy_audio_Wire wire, psy_audio_Machines* machines)
{
	assert(self);
	assert(machines);	

	psy_ui_component_init(&self->component, parent, NULL);		
	arrowui_vtable_init(self);	
	psy_ui_component_set_style_type(&self->component, STYLE_MV_WIRE);
	psy_ui_component_set_style_type_select(&self->component,
		STYLE_MV_WIRE_SELECT);
	self->wire = wire;
	self->machines = machines;
	psy_signal_connect(&self->machines->signal_wireselected, self,
		arrowui_on_wire_selected);
	arrowui_select_wire(self);
}

ArrowUi* arrowui_alloc(void)
{
	return (ArrowUi*)malloc(sizeof(ArrowUi));
}

ArrowUi* arrowui_allocinit(psy_ui_Component* parent, psy_audio_Wire wire,
	psy_audio_Machines* machines)
{
	ArrowUi* rv;

	rv = arrowui_alloc();
	if (rv) {
		arrowui_init(rv, parent, wire, machines);
		rv->component.deallocate = TRUE;
	}
	return rv;
}

void arrowui_on_destroyed(ArrowUi* self)
{
	psy_signal_disconnect(&self->machines->signal_wireselected, self,
		arrowui_on_wire_selected);
}

void arrowui_on_draw(ArrowUi* self, psy_ui_Graphics* g)
{
	psy_ui_RealSize size;
	psy_ui_RealPoint center;

	assert(self);
		
	size = psy_ui_component_size_px(&self->component);
	center = psy_ui_realpoint_make(size.width / 2.0, size.height / 2.0);
	psy_ui_drawline(g, psy_ui_realpoint_make(0.0, center.y), center);
	psy_ui_drawline(g, center, psy_ui_realpoint_make(center.x, size.height));
}

void arrowui_on_mouse_down(ArrowUi* self, psy_ui_MouseEvent* ev)
{	
	assert(self);
	
	psy_audio_machines_selectwire(self->machines, self->wire);	
}

void arrowui_on_wire_selected(ArrowUi* self, psy_audio_Machines* sender)
{	
	assert(self);
	
	arrowui_select_wire(self);
}

void arrowui_select_wire(ArrowUi* self)
{		
	assert(self);		
	assert(self->machines);
		
	if (psy_audio_machines_wire_selected(self->machines, self->wire)) {		
		psy_ui_component_add_style_state(&self->component,
			psy_ui_STYLESTATE_SELECT);
	} else {
		psy_ui_component_remove_style_state(&self->component,
			psy_ui_STYLESTATE_SELECT);
	}
}
