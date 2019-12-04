// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(TABBAR_H)
#define TABBAR_H

#include "uicomponent.h"
#include "list.h"

typedef struct {
	char* text;
	ui_size size;
	ui_margin margin;
} Tab;

void InitTab(Tab*, const char* text, ui_size* size);

typedef struct {
	List* tabs;
	ui_component component;
	int selected;	
	Signal signal_change;
	ui_font font;
	int hover;
	int hoverindex;
	int tabalignment;
} TabBar;

void tabbar_init(TabBar*, ui_component* parent);
Tab* tabbar_append(TabBar*, const char* label);
void tabbar_select(TabBar*, int tabindex);
int tabbar_selected(TabBar*);
void tabbar_settabmargin(TabBar*, int tab, const ui_margin* margin);
Tab* tabbar_tab(TabBar*, int tabindex);

#endif
