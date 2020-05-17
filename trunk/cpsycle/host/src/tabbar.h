// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(TABBAR_H)
#define TABBAR_H

#include "uicomponent.h"
#include "list.h"

typedef struct {
	char* text;
	psy_ui_Size size;
	bool istoggle;
	int checkstate;
	psy_ui_Margin margin;
} Tab;

void tab_init(Tab*, const char* text, psy_ui_Size* size);
void tab_settext(Tab*, const char* text);

typedef struct {
	psy_ui_Component component;
	psy_List* tabs;	
	int selected;	
	psy_Signal signal_change;	
	int hover;
	int hoverindex;
	int tabalignment;
} TabBar;

void tabbar_init(TabBar*, psy_ui_Component* parent);
Tab* tabbar_append(TabBar*, const char* label);
void tabbar_append_tabs(TabBar*, const char* label, ...);
void tabbar_select(TabBar*, int tabindex);
int tabbar_selected(TabBar*);
void tabbar_settabmargin(TabBar*, int tab, const psy_ui_Margin*);
Tab* tabbar_tab(TabBar*, int tabindex);
psy_ui_Component* tabbar_base(TabBar*);

#endif
