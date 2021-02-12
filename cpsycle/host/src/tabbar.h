// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

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

typedef enum {
	TABCHECKSTATE_OFF = 0,
	TABCHECKSTATE_ON = 1
} TabCheckState;

typedef struct Tab {
	char* text;
	char* translation;
	TabMode mode;
	psy_ui_Size size;
	psy_ui_Margin margin;
	bool istoggle;	
	TabCheckState checkstate;
	psy_ui_RealPoint position;
	psy_ui_Bitmap icon;
} Tab;

void tab_init(Tab*, const char* text, psy_ui_Size*, const psy_ui_Margin*);
void tab_settext(Tab*, const char* text);
psy_ui_RealRectangle tab_position(const Tab*, const psy_ui_TextMetric*);
psy_ui_Size tab_preferredsize(Tab*, psy_ui_Component* base);

typedef struct TabBarSkin {
	psy_ui_Colour text;			
	psy_ui_Colour linesel;	
} TabBarSkin;

void tabbarskin_init(TabBarSkin*);

typedef struct TabBar {
	// inherits
	psy_ui_Component component;
	// data members
	psy_List* tabs;	
	uintptr_t selected;
	bool hover;
	uintptr_t hoverindex;
	psy_ui_AlignType tabalignment;
	psy_ui_Margin defaulttabmargin;
	psy_ui_Value defaulttabheight;
	TabBarSkin skin;
	psy_ui_Style style_tab;
	psy_ui_Style style_tab_hover;
	psy_ui_Style style_tab_select;
	psy_ui_Style style_tab_label;
	// Signals
	psy_Signal signal_change;	
} TabBar;

void tabbar_init(TabBar*, psy_ui_Component* parent);
Tab* tabbar_append(TabBar*, const char* label);
void tabbar_append_tabs(TabBar*, const char* label, ...);
void tabbar_clear(TabBar*);
void tabbar_rename_tabs(TabBar*, const char* label, ...);
void tabbar_select(TabBar*, uintptr_t tabindex);
void tabbar_updatealign(TabBar*);

INLINE uintptr_t tabbar_selected(const TabBar* self)
{	
	assert(self);

	return self->selected;	
}

void tabbar_settabalignment(TabBar*, psy_ui_AlignType);
void tabbar_settabmargin(TabBar*, uintptr_t tab, const psy_ui_Margin*);
void tabbar_settabmode(TabBar*, uintptr_t tab, TabMode);
void tabbar_setdefaulttabmargin(TabBar*, const psy_ui_Margin*);
Tab* tabbar_tab(TabBar*, uintptr_t tabindex);
const Tab* tabbar_tab_const(const TabBar*, uintptr_t tabindex);
TabCheckState tabbar_checkstate(const TabBar*, uintptr_t tabindex);
uintptr_t tabbar_numchecked(const TabBar*);

INLINE psy_ui_Component* tabbar_base(TabBar* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* TABBAR_H */
