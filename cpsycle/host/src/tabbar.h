// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(TABBAR_H)
#define TABBAR_H

#include "uicomponent.h"
#include "list.h"

typedef enum {
	TABMODE_SINGLESEL,
	TABMODE_MULTISEL,
} TabMode;

typedef struct {
	char* text;
	psy_ui_Size size;
	bool istoggle;
	TabMode mode;
	int checkstate;
	psy_ui_Margin margin;
} Tab;

void tab_init(Tab*, const char* text, psy_ui_Size*, const psy_ui_Margin*);
void tab_settext(Tab*, const char* text);

typedef struct {
	psy_ui_Component component;
	psy_List* tabs;	
	int selected;	
	psy_Signal signal_change;	
	int hover;
	int hoverindex;
	int tabalignment;
	psy_ui_Margin defaulttabmargin;
} TabBar;

void tabbar_init(TabBar*, psy_ui_Component* parent);
Tab* tabbar_append(TabBar*, const char* label);
void tabbar_append_tabs(TabBar*, const char* label, ...);
void tabbar_clear(TabBar*);
void tabbar_rename_tabs(TabBar*, const char* label, ...);
void tabbar_select(TabBar*, int tabindex);
int tabbar_selected(TabBar*);
void tabbar_settabmargin(TabBar*, int tab, const psy_ui_Margin*);
void tabbar_settabmode(TabBar* self, int tab, TabMode);
void tabbar_setdefaulttabmargin(TabBar*, const psy_ui_Margin*);
Tab* tabbar_tab(TabBar*, int tabindex);
int tabbar_checkstate(TabBar*, int tabindex);
int tabbar_numchecked(TabBar*);

INLINE psy_ui_Component* tabbar_base(TabBar* self)
{
	return &self->component;
}

#endif
