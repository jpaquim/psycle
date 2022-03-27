/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "parammap.h"

/* host */
#include "styles.h"
/* platform */
#include "../../detail/portable.h"

/* implementation */
void parammap_init(ParamMap* self, psy_ui_Component* parent,
	psy_audio_Machine* machine, MachineParamConfig* config)
{	
	assert(config);

	psy_ui_component_init(&self->component, parent, NULL);
	psy_ui_component_set_style_type(&self->component, STYLE_MACPARAM_TITLE);	
	self->machine = machine;
	parammap_setmachine(self, self->machine);
	psy_ui_label_init_text(&self->construction, &self->component,
		"under construction");
	psy_ui_component_set_align(psy_ui_label_base(&self->construction),
		psy_ui_ALIGN_TOP);
}

void parammap_setmachine(ParamMap* self, psy_audio_Machine* machine)
{
	self->machine = machine;
}
