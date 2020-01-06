// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "vubar.h"

void vubar_init(VuBar* self, psy_ui_Component* parent, Workspace* workspace)
{
	ui_component_init(&self->component, parent);
	vumeter_init(&self->vumeter, &self->component, workspace);
	ui_component_setposition(&self->vumeter.component, 0, 00, 170, 20);	
	volslider_init(&self->volslider, &self->component, workspace);
	ui_component_setposition(&self->volslider.component, 0, 20, 170, 20);	
	clipbox_init(&self->clipbox, &self->component, workspace);
	ui_component_setposition(&self->clipbox.component, 175, 5, 10, 35);
}
