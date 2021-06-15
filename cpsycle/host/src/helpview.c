/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "helpview.h"
/* host */
#include "resources/resource.h"

/* prototypes */
static void helpview_initbase(HelpView*, psy_ui_Component* parent);
static void helpview_inittabbar(HelpView*, psy_ui_Component* tabbarparent,
	Workspace*);
static void helpview_initsections(HelpView*, Workspace* workspace);
static void helpview_initsectionfloated(HelpView*);
static void helpview_onselectsection(HelpView*, psy_ui_Component* sender,
	uintptr_t section, uintptr_t options);
static void helpview_onfocus(HelpView*, psy_ui_Component* sender);
static void helpview_onfloatsection(HelpView*, psy_ui_Button* sender);

/* implementation */
void helpview_init(HelpView* self, psy_ui_Component* parent,
	psy_ui_Component* tabbarparent, Workspace* workspace)
{	
	helpview_initbase(self, parent);
	self->workspace = workspace;
	helpview_inittabbar(self, tabbarparent, workspace);
	helpview_initsections(self, workspace);
	helpview_initsectionfloated(self);
	psy_ui_tabbar_select(&self->tabbar, HELPVIEWSECTION_ABOUT);
}
/* construction */
void helpview_initbase(HelpView* self, psy_ui_Component* parent)
{
	psy_ui_component_init(helpview_base(self), parent, NULL);
	psy_signal_connect(&helpview_base(self)->signal_focus, self,
		helpview_onfocus);
	psy_signal_connect(&helpview_base(self)->signal_selectsection, self,
		helpview_onselectsection);
}

void helpview_inittabbar(HelpView* self, psy_ui_Component* tabbarparent,
	Workspace* workspace)
{	
	psy_ui_component_init_align(&self->bar, tabbarparent, NULL,
		psy_ui_ALIGN_LEFT);
	psy_ui_tabbar_init(&self->tabbar, &self->bar);
	psy_ui_component_setalignexpand(&self->bar, psy_ui_HEXPAND);
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->tabbar),
		psy_ui_ALIGN_LEFT);
	psy_ui_tabbar_append_tabs(&self->tabbar, "help.help", "help.about",
		"help.greetings", NULL);	
	psy_ui_button_init_text_connect(&self->floatsection, &self->bar, NULL,
		"help.extract", self, helpview_onfloatsection);		
	psy_ui_button_loadresource(&self->floatsection,
		IDB_VSPLIT_LIGHT, IDB_VSPLIT_DARK, psy_ui_colour_white());
	psy_ui_button_settextalignment(&self->floatsection,
		psy_ui_ALIGNMENT_CENTER_VERTICAL);
	psy_ui_component_setalign(psy_ui_button_base(&self->floatsection),
		psy_ui_ALIGN_LEFT);	
}

void helpview_initsections(HelpView* self, Workspace* workspace)
{
	psy_ui_notebook_init(&self->notebook, helpview_base(self));
	psy_ui_component_setalign(psy_ui_notebook_base(&self->notebook),
		psy_ui_ALIGN_CLIENT);
	psy_ui_notebook_connectcontroller(&self->notebook,
		&self->tabbar.signal_change);
	help_init(&self->help, psy_ui_notebook_base(&self->notebook), workspace);
	about_init(&self->about, psy_ui_notebook_base(&self->notebook), workspace);
	greet_init(&self->greet, psy_ui_notebook_base(&self->notebook));	
}

void helpview_initsectionfloated(HelpView* self)
{
	psy_ui_component_init(&self->sectionfloated, helpview_base(self), NULL);
	psy_ui_component_hide(&self->sectionfloated);
	psy_ui_label_init(&self->floatdesc, &self->sectionfloated, NULL);		
	psy_ui_label_preventtranslation(&self->floatdesc);
	psy_ui_label_settext(&self->floatdesc, "This view is floated.");
	psy_ui_component_setalign(&self->floatdesc.component,
		psy_ui_ALIGN_CENTER);
}

/* events */
void helpview_onselectsection(HelpView* self, psy_ui_Component* sender,
	uintptr_t section, uintptr_t options)
{
	psy_ui_tabbar_select(&self->tabbar, (int)section);
}

void helpview_onfocus(HelpView* self, psy_ui_Component* sender)
{
	psy_ui_Component* view;

	view = psy_ui_notebook_activepage(&self->notebook);
	if (view) {
		psy_ui_component_setfocus(view);
	}
}

void helpview_float(HelpView* self, HelpViewSection section,
	psy_ui_Component* dest)
{
	if (section == HELPVIEWSECTION_HELP) {
		psy_ui_component_hide(&self->help.component);
		psy_ui_component_insert(&self->notebook.component,
			&self->sectionfloated, NULL);
		psy_ui_component_show(&self->sectionfloated);
		psy_ui_component_insert(dest, &self->help.component, NULL);		
		psy_ui_component_setalign(&self->help.component, psy_ui_ALIGN_CLIENT);
		psy_ui_component_setpreferredsize(&self->help.editor.component,
			psy_ui_size_make_em(120, 40));
		psy_ui_component_preventalign(&self->help.editor.component);
		psy_ui_component_show_align(&self->help.component);
		psy_ui_component_align(&self->help.component);
		psy_ui_button_settext(&self->floatsection, "help.combine");		
		psy_ui_component_align(psy_ui_component_parent(
			psy_ui_component_parent(&self->bar)));		
	}
}

void helpview_dock(HelpView* self, HelpViewSection section,
	psy_ui_Component* dest)
{
	if (section == HELPVIEWSECTION_HELP) {		
		psy_ui_component_hide(&self->sectionfloated);
		psy_ui_component_setparent(&self->sectionfloated, &self->component);		
		psy_ui_component_insert(&self->notebook.component,
			&self->help.component, NULL);
		psy_ui_component_setcontaineralign(&self->help.editor.component,
			psy_ui_CONTAINER_ALIGN_NONE);
		psy_ui_component_setalign(&self->help.component, psy_ui_ALIGN_CLIENT);			
		psy_ui_component_show_align(&self->help.component);
		psy_ui_component_align(&self->help.component);
		psy_ui_button_settext(&self->floatsection, "help.extract");
		psy_ui_component_align(psy_ui_component_parent(
			psy_ui_component_parent(&self->bar)));
	}
}

void helpview_onfloatsection(HelpView* self, psy_ui_Button* sender)
{
	if (psy_ui_component_visible(&self->sectionfloated)) {
		workspace_docksection(self->workspace, VIEW_ID_HELPVIEW,
			HELPVIEWSECTION_HELP);
	} else {
		workspace_floatsection(self->workspace, VIEW_ID_HELPVIEW,
			HELPVIEWSECTION_HELP);
	}	
}
