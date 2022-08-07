/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


/* local */
#include "uiapp.h"
#include "uidropdownbox.h"
#include "uiimpfactory.h"

/* prototypes */
static void psy_ui_dropdownbox_on_mouse_up(psy_ui_DropDownBox*,
	psy_ui_MouseEvent* ev);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(psy_ui_DropDownBox* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);		
		vtable.on_mouse_up =
			(psy_ui_fp_component_on_mouse_event)
			psy_ui_dropdownbox_on_mouse_up;			
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}

/* implementation */
void psy_ui_dropdownbox_init(psy_ui_DropDownBox* self, psy_ui_Component* parent)
{  
	psy_ui_Component* view;
	
	if (parent && parent->view) {
		view = parent->view;
	} else {
		view = parent;
	}
	psy_ui_component_init_imp(&self->component, parent, NULL,
		psy_ui_impfactory_allocinit_popupimp(
			psy_ui_app_impfactory(psy_ui_app()), &self->component, view));
	vtable_init(self);
	self->component.dropdown = TRUE;
	psy_ui_component_doublebuffer(&self->component);	
}

void psy_ui_dropdownbox_show(psy_ui_DropDownBox* self, psy_ui_Component* field)
{
	psy_ui_Size size;
	psy_ui_Size preferred_size;
	psy_ui_RealRectangle position;	
	
	size = psy_ui_component_scroll_size(field);
	preferred_size = psy_ui_component_preferred_size(&self->component, NULL);
	preferred_size.height = psy_ui_min_values(preferred_size.height,
		psy_ui_value_make_eh(10.0), psy_ui_component_textmetric(
		&self->component), NULL);	
	position = psy_ui_component_screenposition(field);		
	psy_ui_component_setposition(&self->component,
		psy_ui_rectangle_make(
			psy_ui_point_make(
				psy_ui_value_make_px(position.left),
				psy_ui_value_make_px(position.bottom)),
			psy_ui_size_make(size.width, preferred_size.height)));		
	psy_ui_component_show(&self->component);
	if (psy_ui_component_parent(&self->component)) {
		psy_ui_component_parent(&self->component)->imp->vtable->dev_preventinput(
		psy_ui_component_parent(&self->component)->imp);
	}	
}

void psy_ui_dropdownbox_on_mouse_up(psy_ui_DropDownBox* self,
	psy_ui_MouseEvent* ev)
{
	psy_ui_RealRectangle position;
		
	position = psy_ui_component_position(&self->component);
	psy_ui_realrectangle_set_topleft(&position, psy_ui_realpoint_zero());	
	if (!psy_ui_realrectangle_intersect(&position,
			psy_ui_mouseevent_offset(ev))) {		
		psy_ui_dropdownbox_hide(self);		
	}
	psy_ui_mouseevent_stop_propagation(ev);
}

void psy_ui_dropdownbox_hide(psy_ui_DropDownBox* self)
{
	if (psy_ui_component_parent(&self->component)) {
		psy_ui_component_parent(&self->component)->imp->vtable->dev_enableinput(
			psy_ui_component_parent(&self->component)->imp);		
	}
	psy_ui_component_release_capture(&self->component);
	psy_ui_component_hide(&self->component);
}

	
