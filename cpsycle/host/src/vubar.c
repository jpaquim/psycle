// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "vubar.h"

void vubar_init(VuBar* self, psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_component_init(&self->component, parent);
	clipbox_init(&self->clipbox, &self->component, workspace);	
	psy_ui_component_setalign(&self->clipbox.component, psy_ui_ALIGN_RIGHT);
	vumeter_init(&self->vumeter, &self->component, workspace);
	psy_ui_component_setalign(&self->vumeter.component, psy_ui_ALIGN_TOP);	
	volslider_init(&self->volslider, &self->component, workspace);
	psy_ui_component_setalign(&self->volslider.component, psy_ui_ALIGN_TOP);		
}
