/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "rangeedit.h"
/* ui */
#include <uiapp.h>
/* audio */
#include <songio.h>
/* std */
#include <ctype.h>
/* platform */
#include "../../detail/portable.h"


/* prototypes */
static void rangeedit_on_destroyed(RangeEdit*);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(RangeEdit* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.on_destroyed =
			(psy_ui_fp_component_event)
			rangeedit_on_destroyed;		
		vtable_initialized = TRUE;
	}
	psy_ui_component_set_vtable(&self->component, &vtable);
}

/* implementation */
void rangeedit_init(RangeEdit* self, psy_ui_Component* parent)
{
	psy_ui_component_init(rangeedit_base(self), parent, NULL);
	vtable_init(self);
	psy_ui_component_set_align_expand(rangeedit_base(self), psy_ui_HEXPAND);
	psy_ui_component_set_default_align(rangeedit_base(self), psy_ui_ALIGN_LEFT,
		psy_ui_defaults_hmargin(psy_ui_defaults()));
	psy_ui_label_init(&self->from, rangeedit_base(self));
	psy_ui_label_set_text(&self->from, "from");
	psy_ui_textarea_init_single_line(&self->edit_from, rangeedit_base(self));	
	psy_ui_textarea_set_char_number(&self->edit_from, 5);
	psy_ui_textarea_enable_input_field(&self->edit_from);
	psy_ui_label_init(&self->to, rangeedit_base(self));
	psy_ui_label_set_text(&self->to, "to");
	psy_ui_textarea_init_single_line(&self->edit_to, rangeedit_base(self));
	psy_ui_textarea_set_char_number(&self->edit_to, 5);
	psy_ui_textarea_enable_input_field(&self->edit_to);
}

void rangeedit_on_destroyed(RangeEdit* self)
{	
}

RangeEdit* rangeedit_alloc(void)
{
	return (RangeEdit*)malloc(sizeof(RangeEdit));
}

RangeEdit* rangeedit_allocinit(psy_ui_Component* parent)
{
	RangeEdit* rv;

	rv = rangeedit_alloc();
	if (rv) {
		rangeedit_init(rv, parent);
		psy_ui_component_deallocate_after_destroyed(&rv->component);
	}
	return rv;
}

RangeEdit* rangeedit_allocinit_range(psy_ui_Component* parent, psy_Property* from,
	psy_Property* to)
{
	RangeEdit* rv;

	rv = rangeedit_allocinit(parent);
	if (rv) {
		rangeedit_data_exchange(rv, from, to);
	}
	return rv;
}

void rangeedit_data_exchange(RangeEdit* self, psy_Property* from,
	psy_Property* to)
{
	assert(self);

	psy_ui_textarea_data_exchange(&self->edit_from, from);
	psy_ui_textarea_data_exchange(&self->edit_to, to);
}
