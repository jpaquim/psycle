// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

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
	if (kbd) {
		psy_Property* section;

		section = psy_property_find(kbd->properties, sectionname,
			PSY_PROPERTY_TYPE_SECTION);
		if (section) {
			psy_List* p;

			for (p = psy_property_children(section); p != NULL;
					psy_list_next(&p)) {
				psy_Property* property;
				uintptr_t keycode;
				bool shift;
				bool ctrl;

				property = (psy_Property*)p->entry;
				psy_audio_decodeinput(property->item.value.i, &keycode, &shift, &ctrl);
				kbdbox_setcolor(&self->kbdbox, keycode, psy_ui_color_make(0x00B1C8B0));
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
	if (kbd) {
		psy_List* p;

		if (kbd->properties) {
			for (p = psy_property_children(kbd->properties); p != NULL;
					psy_list_next(&p)) {
				psy_Property* property;

				property = (psy_Property*)p->entry;
				if (psy_property_type(property) ==
						PSY_PROPERTY_TYPE_SECTION) {
					tabbar_append(&self->tabbar,
						psy_property_translation(property));
				}
			}
		}
		tabbar_select(&self->tabbar, 0);
		psy_signal_connect(&self->tabbar.signal_change, self,
			kbdhelp_ontabbarchange);
	}
}

void kbdhelp_ontabbarchange(KbdHelp* self, psy_ui_Component* sender,
	int tabindex)
{	
	Tab* tab;
	psy_EventDriver* kbd;

	kbd = workspace_kbddriver(self->workspace);
	if (kbd) {
		self->search = 0;
		if (kbd->properties) {
			psy_List* p = 0;
			psy_Property* property = NULL;

			p = kbd->properties->children;
			tab = tabbar_tab(&self->tabbar, tabindex);
			if (tab) {
				while (p) {
					property = (psy_Property*)p->entry;
					if (psy_property_type(property) == PSY_PROPERTY_TYPE_SECTION) {
						if (strcmp(psy_property_translation(property), tab->text) == 0) {
							break;
						}
					}
					property = NULL;
					psy_list_next(&p);
				}
			}
			self->search = property;	
			if (self->search) {
				kbdhelp_markpatterncmds(self, psy_property_key(self->search));
				psy_ui_component_invalidate(&self->component);
			}
		}
	}
}