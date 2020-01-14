// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "helpview.h"

static void helpview_onshow(HelpView*, psy_ui_Component* sender);
static void helpview_onhide(HelpView*, psy_ui_Component* sender);

void helpview_init(HelpView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{
	ui_component_init(helpview_base(self), parent);	
	ui_component_enablealign(helpview_base(self));
	psy_ui_notebook_init(&self->notebook, helpview_base(self));	
	ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	help_init(&self->help, &self->notebook.component, workspace);
	about_init(&self->about, psy_ui_notebook_base(&self->notebook));
	greet_init(&self->greet, psy_ui_notebook_base(&self->notebook));	
	tabbar_init(&self->tabbar, tabbarparent);
	ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);
	ui_component_hide(tabbar_base(&self->tabbar));
	tabbar_append(&self->tabbar, "Help");
	tabbar_append(&self->tabbar, "About");
	tabbar_append(&self->tabbar, "Greetings");	
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	tabbar_select(&self->tabbar, 1);
	psy_signal_connect(&helpview_base(self)->signal_show, self,
		helpview_onshow);
	psy_signal_connect(&helpview_base(self)->signal_hide, self,
		helpview_onhide);
}

void helpview_onshow(HelpView* self, psy_ui_Component* sender)
{	
	self->tabbar.component.visible = 1;
	ui_component_align(ui_component_parent(tabbar_base(&self->tabbar)));
	ui_component_show(tabbar_base(&self->tabbar));
}

void helpview_onhide(HelpView* self, psy_ui_Component* sender)
{
	ui_component_hide(tabbar_base(&self->tabbar));
}

psy_ui_Component* helpview_base(HelpView* self)
{
	return &self->component;
}
