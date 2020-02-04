// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "helpview.h"

void helpview_init(HelpView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{
	psy_ui_component_init(helpview_base(self), parent);	
	psy_ui_component_enablealign(helpview_base(self));
	psy_ui_notebook_init(&self->notebook, helpview_base(self));	
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	help_init(&self->help, &self->notebook.component, workspace);
	about_init(&self->about, psy_ui_notebook_base(&self->notebook));
	greet_init(&self->greet, psy_ui_notebook_base(&self->notebook));	
	tabbar_init(&self->tabbar, tabbarparent);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);
	psy_ui_component_hide(tabbar_base(&self->tabbar));
	tabbar_append(&self->tabbar, "Help");
	tabbar_append(&self->tabbar, "About");
	tabbar_append(&self->tabbar, "Greetings");	
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	tabbar_select(&self->tabbar, 1);
}

psy_ui_Component* helpview_base(HelpView* self)
{
	return &self->component;
}
