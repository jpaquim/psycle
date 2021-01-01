// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "kbdhelp.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void kbdhelp_appendtabbarsections(KbdHelp*);
static void kbdhelp_ontabbarchange(KbdHelp*, psy_ui_Component* sender,
	int tabindex);
static void kbdhelp_markpatterncmds(KbdHelp* self, const char* section);

void kbdhelp_init(KbdHelp* self, psy_ui_Component* parent, Workspace* workspace)
{	
	psy_ui_Margin tabmargin;

	self->workspace = workspace;
	psy_ui_component_init(kbdhelp_base(self), parent);		
	psy_ui_component_enablealign(kbdhelp_base(self));
	kbdbox_init(&self->kbdbox, kbdhelp_base(self), workspace);	
	psy_ui_component_setalign(kbdbox_base(&self->kbdbox),
		psy_ui_ALIGN_CLIENT);
	tabbar_init(&self->tabbar, &self->component);
	psy_ui_component_setalign(tabbar_base(&self->tabbar), psy_ui_ALIGN_RIGHT);
	self->tabbar.tabalignment = psy_ui_ALIGN_RIGHT;
	psy_ui_margin_init_all(&tabmargin, psy_ui_value_makepx(0),
		psy_ui_value_makeew(1),
		psy_ui_value_makeeh(0.5),
		psy_ui_value_makeew(2));
	tabbar_setdefaulttabmargin(&self->tabbar, &tabmargin);
	kbdhelp_markpatterncmds(self, "notes");
	kbdhelp_appendtabbarsections(self);	
}

void kbdhelp_markpatterncmds(KbdHelp* self, const char* sectionname)
{
	psy_EventDriver* kbd;	

	kbdbox_cleardescriptions(&self->kbdbox);
	kbd = workspace_kbddriver(self->workspace);
	if (kbd && psy_eventdriver_configuration(kbd)) {
		const psy_Property* section;

		section = psy_property_find_const(psy_eventdriver_configuration(kbd),
			sectionname, PSY_PROPERTY_TYPE_SECTION);
		if (section) {
			const psy_List* p;

			for (p = psy_property_children_const(section); p != NULL;
					psy_list_next_const(&p)) {
				const psy_Property* property;
				uint32_t keycode;
				bool shift;
				bool ctrl;
				bool alt;

				property = (const psy_Property*)psy_list_entry_const(p);
				psy_audio_decodeinput((uint32_t)psy_property_item_int(property),
					&keycode, &shift, &ctrl, &alt);
				kbdbox_setcolour(&self->kbdbox, keycode, psy_ui_colour_make(0x00B1C8B0));
				kbdbox_setdescription(&self->kbdbox, keycode, shift,
					ctrl, psy_property_shorttext(property));
			}
		}
	}
}

void kbdhelp_appendtabbarsections(KbdHelp* self)
{	
	psy_EventDriver* kbd;

	kbd = workspace_kbddriver(self->workspace);
	if (kbd && psy_eventdriver_configuration(kbd)) {
		const psy_Property* cmds;

		cmds = psy_property_findsection_const(psy_eventdriver_configuration(kbd),
			"cmds");
		if (cmds) {
			const psy_List* p;

			for (p = psy_property_children_const(cmds); p != NULL;
					psy_list_next_const(&p)) {
				const psy_Property* property;

				property = (const psy_Property*)psy_list_entry_const(p);
				if (psy_property_type(property) ==
					PSY_PROPERTY_TYPE_SECTION) {
					tabbar_append(&self->tabbar,
						psy_property_translation(property));
				}
			}
		}
	}
	tabbar_select(&self->tabbar, 0);
	psy_signal_connect(&self->tabbar.signal_change, self,
		kbdhelp_ontabbarchange);
}

void kbdhelp_ontabbarchange(KbdHelp* self, psy_ui_Component* sender,
	int tabindex)
{	
	Tab* tab;
	psy_EventDriver* kbd;

	kbd = workspace_kbddriver(self->workspace);
	if (kbd && psy_eventdriver_configuration(kbd)) {
		const psy_Property* property;
		const psy_Property* cmds;
		
		property = NULL;
		cmds = psy_property_findsection_const(psy_eventdriver_configuration(kbd),
			"cmds");
		tab = tabbar_tab(&self->tabbar, tabindex);
		if (cmds && tab) {				
			const psy_List* p = 0;

			p = psy_property_children_const(cmds);
			while (p) {
				property = (const psy_Property*)psy_list_entry_const(p);
				if (psy_property_type(property) == PSY_PROPERTY_TYPE_SECTION) {
					if (strcmp(psy_property_translation(property), tab->text) == 0) {
						break;
					}
				}
				property = NULL;
				psy_list_next_const(&p);
			}
		}
		if (property) {
			kbdhelp_markpatterncmds(self, psy_property_key(property));
			psy_ui_component_invalidate(&self->component);
		}
	}
}