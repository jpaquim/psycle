// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "titlebar.h"
// host
#include "styles.h"

// prototypes
static void titlebar_onhide(TitleBar* self);
// implementation
void titlebar_init(TitleBar* self, psy_ui_Component* parent,
	psy_ui_Component* view, const char* title)
{
	psy_ui_Margin margin;

	psy_ui_component_init_align(&self->component, parent, NULL,
		psy_ui_ALIGN_TOP);
	psy_ui_component_setstyletype(&self->component, STYLE_HEADER);
	psy_ui_component_init_align(&self->client, &self->component, NULL,
		psy_ui_ALIGN_CLIENT);
	psy_ui_label_init_text(&self->title, &self->client, NULL, title);
	psy_ui_component_setalign(&self->title.component, psy_ui_ALIGN_LEFT);
	psy_ui_button_init(&self->hide, &self->component, NULL);
	psy_ui_button_preventtranslation(&self->hide);
	psy_ui_button_settext(&self->hide, "X");	
	psy_ui_component_setalign(&self->hide.component, psy_ui_ALIGN_RIGHT);
	psy_ui_margin_init_em(&margin, 0.0, 2.0, 0.0, 0.0);
	psy_ui_component_setmargin(&self->hide.component, margin);
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
