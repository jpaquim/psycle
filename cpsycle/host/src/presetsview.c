/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "presetsview.h"
/* host */
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void presetsview_on_destroyed(PresetsView*);
static void presetsview_init_buttons(PresetsView*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(PresetsView* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component)
			presetsview_on_destroyed;		
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void presetsview_init(PresetsView* self, psy_ui_Component* parent)
{	
	assert(self);
	
	psy_ui_component_init(&self->component, parent, NULL);
	vtable_init(self);
	psy_ui_component_set_preferred_width(&self->component,
		psy_ui_value_make_ew(40.0));
	titlebar_init(&self->title_bar, &self->component, "Preset");
	presetsview_init_buttons(self);
	psy_ui_component_init_align(&self->client, &self->component, NULL,
		psy_ui_ALIGN_CLIENT);
	psy_ui_component_set_default_align(&self->client, psy_ui_ALIGN_TOP,		
		psy_ui_margin_zero());	
	psy_ui_combobox_init_simple(&self->presets, &self->client);
	psy_ui_component_set_align(&self->presets.component, psy_ui_ALIGN_CLIENT);	
}

void presetsview_on_destroyed(PresetsView* self)
{	
}

void presetsview_init_buttons(PresetsView* self)
{
	psy_ui_component_init_align(&self->buttons, &self->component, NULL,
		psy_ui_ALIGN_RIGHT);
	psy_ui_component_set_default_align(&self->buttons, psy_ui_ALIGN_TOP,		
		psy_ui_margin_zero());
	psy_ui_button_init_text(&self->savepresets, &self->buttons,
		"machineframe.saveas");
	psy_ui_button_init_text(&self->importpresets, &self->buttons,
		"machineframe.import");
	psy_ui_button_init_text(&self->exportpresets, &self->buttons,
		"machineframe.export");		
}
