// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "helpview.h"

// Prototypes
static void helpview_updatetext(HelpView*, Translator*);
static void helpview_onlanguagechanged(HelpView*, Translator* sender);
static void selectsection(HelpView*, psy_ui_Component* sender, uintptr_t section);

TabBar* helpview_init(HelpView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{
	self->workspace = workspace;
	psy_ui_component_init(helpview_base(self), parent);	
	psy_ui_component_enablealign(helpview_base(self));
	psy_ui_notebook_init(&self->notebook, helpview_base(self));	
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	help_init(&self->help, &self->notebook.component, workspace);
	about_init(&self->about, psy_ui_notebook_base(&self->notebook), workspace);
	greet_init(&self->greet, psy_ui_notebook_base(&self->notebook));	
	tabbar_init(&self->tabbar, tabbarparent);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_LEFT);
	tabbar_append_tabs(&self->tabbar, "", "", "", NULL);
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	psy_signal_connect(&self->component.signal_selectsection, self, selectsection);
	tabbar_select(&self->tabbar, 1);
	helpview_updatetext(self, &workspace->translator);
	psy_signal_connect(&workspace->signal_languagechanged, self,
		helpview_onlanguagechanged);
	return &self->tabbar;
}

void helpview_updatetext(HelpView* self, Translator* translator)
{
	tabbar_rename_tabs(&self->tabbar,
		translator_translate(translator, "help.help"),
		translator_translate(translator, "help.about"),
		translator_translate(translator, "help.greetings"),
		NULL);
}

void helpview_onlanguagechanged(HelpView* self, Translator* sender)
{
	helpview_updatetext(self, sender);
}

void selectsection(HelpView* self, psy_ui_Component* sender, uintptr_t section)
{
	tabbar_select(&self->tabbar, (int) section);
}
