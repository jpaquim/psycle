// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "vubar.h"

void vubar_init(VuBar* self, psy_ui_Component* parent, Workspace* workspace)
{
	psy_ui_component_init(vubar_base(self), parent, NULL);
	clipbox_init(&self->clipbox, vubar_base(self), workspace);
	psy_ui_component_setalign(clipbox_base(&self->clipbox),
		psy_ui_ALIGN_RIGHT);
	vumeter_init(&self->vumeter, vubar_base(self), workspace);
	psy_ui_component_setalign(vumeter_base(&self->vumeter), psy_ui_ALIGN_TOP);
	volslider_init(&self->volslider, vubar_base(self), workspace);
	psy_ui_component_setalign(volslider_base(&self->volslider),
		psy_ui_ALIGN_TOP);
}

void vubar_reset(VuBar* self)
{
	clipbox_deactivate(&self->clipbox);
}
