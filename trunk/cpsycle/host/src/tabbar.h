// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(TABBAR_H)
#define TABBAR_H

// ui
#include "uicomponent.h"
// container
#include "list.h"

// TabBar
//
// Shows tabs in a bar. Can be used with a psy_ui_Notebook or independently.

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	TABMODE_SINGLESEL,
	TABMODE_MULTISEL,
	TABMODE_LABEL,
} TabMode;

typedef struct Tab {
	char* text;
	char* translation;
	TabMode mode;
	psy_ui_Size size;
	psy_ui_Margin margin;
	bool istoggle;	
	int checkstate;	
} Tab;

void tab_init(Tab*, const char* text, psy_ui_Size*, const psy_ui_Margin*);
void tab_settext(Tab*, const char* text);

typedef struct TabBar {
	// inherits
	psy_ui_Component component;
	// data members
	psy_List* tabs;	
	intptr_t selected;
	int hover;
	intptr_t hoverindex;
	int tabalignment;
	psy_ui_Margin defaulttabmargin;
	// Signals
	psy_Signal signal_change;
} TabBar;

void tabbar_init(TabBar*, psy_ui_Component* parent);
Tab* tabbar_append(TabBar*, const char* label);
void tabbar_append_tabs(TabBar*, const char* label, ...);
void tabbar_clear(TabBar*);
void tabbar_rename_tabs(TabBar*, const char* label, ...);
void tabbar_select(TabBar*, intptr_t tabindex);

INLINE intptr_t tabbar_selected(const TabBar* self)
{	
	assert(self);

	return self->selected;	
}

void tabbar_settabmargin(TabBar*, int tab, const psy_ui_Margin*);
void tabbar_settabmode(TabBar*, int tab, TabMode);
void tabbar_setdefaulttabmargin(TabBar*, const psy_ui_Margin*);
Tab* tabbar_tab(TabBar*, intptr_t tabindex);
int tabbar_checkstate(TabBar*, intptr_t tabindex);
int tabbar_numchecked(TabBar*);

INLINE psy_ui_Component* tabbar_base(TabBar* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* TABBAR_H */
