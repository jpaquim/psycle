/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "titlebar.h"
/* host */
#include "styles.h"
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void titlebar_on_destroyed(TitleBar*);
static void titlebar_on_hide(TitleBar*);
static void titlebar_on_drag_start(TitleBar*, psy_ui_DragEvent*);

/* vtable  */
static psy_ui_ComponentVtable titlebar_vtable;
static bool titlebar_vtable_initialized = FALSE;

static void titlebar_vtable_init(TitleBar* self)
{
	if (!titlebar_vtable_initialized) {
		titlebar_vtable = *(self->component.vtable);		
		titlebar_vtable.on_destroyed =
			(psy_ui_fp_component)
			titlebar_on_destroyed;
		titlebar_vtable.ondragstart =
			(psy_ui_fp_component_on_drag_event)
			titlebar_on_drag_start;
		titlebar_vtable_initialized = TRUE;
	}
	self->component.vtable = &titlebar_vtable;
}

/* implementation */
void titlebar_init(TitleBar* self, psy_ui_Component* parent,	
	const char* title)
{
	assert(self);

	psy_ui_component_init_align(&self->component, parent, NULL,
		psy_ui_ALIGN_TOP);
	titlebar_vtable_init(self);
	self->component.draggable = TRUE;
	self->dragid = NULL;
	psy_ui_component_set_style_type(&self->component, STYLE_TITLEBAR);
	psy_ui_component_init_align(&self->client, &self->component, NULL,
		psy_ui_ALIGN_CLIENT);
	psy_ui_label_init_text(&self->title, &self->client, title);
	psy_ui_label_set_text_alignment(&self->title,
		psy_ui_ALIGNMENT_CENTER);
	psy_ui_component_set_align(&self->title.component, psy_ui_ALIGN_CLIENT);
	psy_ui_button_init(&self->hide, &self->component);
	psy_ui_button_prevent_translation(&self->hide);
	psy_ui_button_set_text(&self->hide, "X");	
	psy_ui_component_set_align(&self->hide.component, psy_ui_ALIGN_RIGHT);		
}

void titlebar_on_destroyed(TitleBar* self)
{
	assert(self);

	free(self->dragid);
	self->dragid = NULL;
}

void titlebar_hide_on_close(TitleBar* self)
{
	assert(self);

	psy_signal_connect(&self->hide.signal_clicked, self,
		titlebar_on_hide);
}

void titlebar_on_hide(TitleBar* self)
{
	assert(self);

	psy_ui_component_hide_align(psy_ui_component_parent(&self->component));
}

void titlebar_on_drag_start(TitleBar* self, psy_ui_DragEvent* ev)
{
	assert(self);

	if (self->dragid) {
		psy_Property* data;

		ev->dataTransfer = psy_property_allocinit_key(NULL);
		data = psy_property_append_section(ev->dataTransfer, "data");
		psy_property_append_str(data, "dragview", self->dragid);
	}
}

void titlebar_enable_drag(TitleBar* self, const char* dragid)
{
	assert(self);

	psy_strreset(&self->dragid, dragid);
}
