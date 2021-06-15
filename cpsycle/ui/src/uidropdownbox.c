/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


/* local */
#include "uiapp.h"
#include "uidropdownbox.h"
#include "uiimpfactory.h"

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
	self->component.dropdown = TRUE;
	self->component.visible = FALSE;
}

void psy_ui_dropdownbox_show(psy_ui_DropDownBox* self, psy_ui_Component* field)
{
	psy_ui_Size size;
	psy_ui_RealRectangle position;	
	
	size = psy_ui_component_scrollsize(field);
	position = psy_ui_component_screenposition(field);		
	psy_ui_component_setposition(&self->component,
		psy_ui_rectangle_make(
			psy_ui_point_make(
				psy_ui_value_make_px(position.left),
				psy_ui_value_make_px(position.bottom)),
			psy_ui_size_make(size.width, psy_ui_value_make_eh(10.0))));		
	psy_ui_component_show(&self->component);
	if (psy_ui_component_parent(&self->component)) {
		psy_ui_component_parent(&self->component)->imp->vtable->dev_preventinput(
		psy_ui_component_parent(&self->component)->imp);
	}
}

void psy_ui_dropdownbox_hide(psy_ui_DropDownBox* self)
{
	if (psy_ui_component_parent(&self->component)) {
		psy_ui_component_parent(&self->component)->imp->vtable->dev_enableinput(
			psy_ui_component_parent(&self->component)->imp);		
	}
	psy_ui_component_hide(&self->component);
}
