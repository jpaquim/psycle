/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "vubar.h"

/* implementation */
void vubar_init(VuBar* self, psy_ui_Component* parent, Workspace* workspace)
{
	assert(self);
	assert(workspace);

	psy_ui_component_init(vubar_base(self), parent, NULL);
	clipbox_init(&self->clipbox, vubar_base(self), workspace);
	psy_ui_component_set_align(clipbox_base(&self->clipbox),
		psy_ui_ALIGN_RIGHT);
	vumeter_init(&self->vumeter, vubar_base(self));
	psy_ui_component_set_align(vumeter_base(&self->vumeter),
		psy_ui_ALIGN_TOP);
	volslider_init(&self->volslider, vubar_base(self), workspace);
	psy_ui_component_set_align(volslider_base(&self->volslider),
		psy_ui_ALIGN_TOP);
}

void vubar_reset(VuBar* self)
{
	assert(self);

	clipbox_deactivate(&self->clipbox);
}

void vubar_idle(VuBar* self)
{
	vumeter_idle(&self->vumeter);
}

void vubar_set_machine(VuBar* self, psy_audio_Machine* machine)
{
	assert(self);
	
	vumeter_set_machine(&self->vumeter, machine);
}
