/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"

#include "titlebar.h"
/* host */
#include "styles.h"
/* std */
#include <stdlib.h>
/* platform */
#include "../../detail/portable.h"

/* prototypes */
static void titlebar_ondestroy(TitleBar*);
static void titlebar_onhide(TitleBar*);
static void titlebar_ondragstart(TitleBar*, psy_ui_DragEvent*);

/* vtable  */
static psy_ui_ComponentVtable titlebar_vtable;
static bool titlebar_vtable_initialized = FALSE;

static void titlebar_vtable_init(TitleBar* self)
{
	if (!titlebar_vtable_initialized) {
		titlebar_vtable = *(self->component.vtable);		
		titlebar_vtable.ondestroy =
			(psy_ui_fp_component_event)
			titlebar_ondestroy;
		titlebar_vtable.ondragstart =
			(psy_ui_fp_component_ondragstart)
			titlebar_ondragstart;
		titlebar_vtable_initialized = TRUE;
	}
	self->component.vtable = &titlebar_vtable;
}

/* implementation */
void titlebar_init(TitleBar* self, psy_ui_Component* parent,	
	const char* title)
{
	psy_ui_Margin margin;

	psy_ui_component_init_align(&self->component, parent, NULL,
		psy_ui_ALIGN_TOP);
	titlebar_vtable_init(self);
	psy_ui_component_setstyletype(&self->component, STYLE_HEADER);
	psy_ui_component_init_align(&self->client, &self->component, NULL,
		psy_ui_ALIGN_CLIENT);
	psy_ui_label_init_text(&self->title, &self->client, title);
	psy_ui_component_setalign(&self->title.component, psy_ui_ALIGN_LEFT);
	psy_ui_button_init(&self->hide, &self->component);
	psy_ui_button_preventtranslation(&self->hide);
	psy_ui_button_settext(&self->hide, "X");	
	psy_ui_component_setalign(&self->hide.component, psy_ui_ALIGN_RIGHT);
	psy_ui_margin_init_em(&margin, 0.0, 2.0, 0.0, 0.0);
	psy_ui_component_setmargin(&self->hide.component, margin);
	self->component.draggable = TRUE;
	self->dragid = NULL;
}

void titlebar_ondestroy(TitleBar* self)
{
	free(self->dragid);
	self->dragid = NULL;
}

void titlebar_hideonclose(TitleBar* self)
{
	psy_signal_connect(&self->hide.signal_clicked, self,
		titlebar_onhide);
}

void titlebar_onhide(TitleBar* self)
{
	psy_ui_component_hide_align(psy_ui_component_parent(&self->component));
}

void titlebar_ondragstart(TitleBar* self, psy_ui_DragEvent* ev)
{
	if (self->dragid) {
		psy_Property* data;

		ev->dataTransfer = psy_property_allocinit_key(NULL);
		data = psy_property_append_section(ev->dataTransfer, "data");
		psy_property_append_str(data, "dragview", self->dragid);
	}
}

void titlebar_enabledrag(TitleBar* self, const char* dragid)
{
	psy_strreset(&self->dragid, dragid);
}