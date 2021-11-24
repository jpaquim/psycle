/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "patternviewtabbar.h"
/* host */
#include "styles.h"
/* platform */
#include "../../detail/portable.h"

/* PatternViewTabBar */

/* prototypes*/
static void patternviewtabbar_ondestroy(PatternViewTabBar*);
static void patternviewtabbar_ontabbarchange(PatternViewTabBar*,
	psy_ui_Component* sender, uintptr_t tabindex);

/* vtable */
static psy_ui_ComponentVtable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(PatternViewTabBar* self)
{
	if (!vtable_initialized) {
		vtable = *(self->component.vtable);
		vtable.ondestroy =
			(psy_ui_fp_component_event)
			patternviewtabbar_ondestroy;		
		vtable_initialized = TRUE;
	}
	self->component.vtable = &vtable;
}
/* implementation */
void patternviewtabbar_init(PatternViewTabBar* self, psy_ui_Component* parent,
	Workspace* workspace)
{
	psy_ui_Tab* tab;

	assert(self);

	psy_ui_component_init_align(&self->component, parent, NULL,
		psy_ui_ALIGN_LEFT);
	vtable_init(self);
	self->workspace = workspace;
	psy_signal_init(&self->signal_toggle_properties);
	psy_ui_tabbar_init(&self->tabbar, &self->component, NULL);
	psy_ui_component_setalign(psy_ui_tabbar_base(&self->tabbar),
		psy_ui_ALIGN_LEFT);
	psy_ui_tabbar_append_tabs(&self->tabbar, "patternview.tracker",
		"patternview.roll", "patternview.split", "patternview.vert",
		"patternview.horz", "patternview.properties", NULL);
	tab = psy_ui_tabbar_tab(&self->tabbar, 2);
	if (tab) {
		psy_ui_tab_setmode(tab, psy_ui_TABMODE_LABEL);
	}
	tab = psy_ui_tabbar_tab(&self->tabbar, 0);
	if (tab) {
		psy_ui_Margin margin;

		margin = psy_ui_component_margin(&tab->component);
		margin.left = psy_ui_value_make_ew(1.0);
		psy_ui_component_setmargin(&tab->component, margin);
	}
	tab = psy_ui_tabbar_tab(&self->tabbar, 5);
	if (tab) {
		tab->istoggle = TRUE;
	}
	psy_ui_tabbar_select(&self->tabbar, 0);
	psy_ui_button_init(&self->contextbutton, &self->component, NULL);
	psy_ui_button_seticon(&self->contextbutton, psy_ui_ICON_MORE);
	psy_ui_component_setalign(psy_ui_button_base(&self->contextbutton),
		psy_ui_ALIGN_RIGHT);
	psy_signal_connect(&self->tabbar.signal_change, self,
		patternviewtabbar_ontabbarchange);
}

void patternviewtabbar_ondestroy(PatternViewTabBar* self)
{
	assert(self);
	
	psy_signal_dispose(&self->signal_toggle_properties);
}

void patternviewtabbar_ontabbarchange(PatternViewTabBar* self,
	psy_ui_Component* sender, uintptr_t tabindex)
{
	if (tabindex <= PATTERN_DISPLAYMODE_NUM) {
		static PatternDisplayMode display[] = {
			PATTERN_DISPLAYMODE_TRACKER, PATTERN_DISPLAYMODE_PIANOROLL,
			PATTERN_DISPLAYMODE_NUM,
			PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_VERTICAL,
			PATTERN_DISPLAYMODE_TRACKER_PIANOROLL_HORIZONTAL };
		workspace_selectpatterndisplay(self->workspace, display[tabindex]);
	} else if (tabindex == 5) {
		psy_signal_emit(&self->signal_toggle_properties, self, 0);		
	}
}
