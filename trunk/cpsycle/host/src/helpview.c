// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "helpview.h"

static void OnShow(HelpView*, ui_component* sender);
static void OnHide(HelpView*, ui_component* sender);

void InitHelpView(HelpView* self, ui_component* parent,
	ui_component* tabbarparent, Workspace* workspace)
{
	ui_component_init(&self->component, parent);	
	ui_component_enablealign(&self->component);
	ui_notebook_init(&self->notebook, &self->component);	
	ui_component_setalign(&self->notebook.component, UI_ALIGN_CLIENT);	
	about_init(&self->about, &self->notebook.component);
	greet_init(&self->greet, &self->notebook.component);
	tabbar_init(&self->tabbar, tabbarparent);
	ui_component_setalign(&self->tabbar.component, UI_ALIGN_LEFT);
	ui_component_hide(&self->tabbar.component);	
	tabbar_append(&self->tabbar, "About");
	tabbar_append(&self->tabbar, "Greetings");
	ui_notebook_connectcontroller(&self->notebook, &self->tabbar.signal_change);
	ui_notebook_setpageindex(&self->notebook, 0);
	signal_connect(&self->component.signal_show, self, OnShow);
	signal_connect(&self->component.signal_hide, self, OnHide);
}

void OnShow(HelpView* self, ui_component* sender)
{	
	self->tabbar.component.visible = 1;
	ui_component_align(ui_component_parent(&self->tabbar.component));
	ui_component_show(&self->tabbar.component);
}

void OnHide(HelpView* self, ui_component* sender)
{
	ui_component_hide(&self->tabbar.component);
}
