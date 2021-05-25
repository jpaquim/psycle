/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_TABBAR_H
#define psy_ui_TABBAR_H

/* local */
#include "uicomponent.h"
/* container */
#include "list.h"

/*
** psy_ui_TabBar
**
**  Shows tabs in a bar. Can be used with a psy_ui_Notebook or independently.
**
**  Structure:
**
**  psy_ui_Component <>----<> psy_ui_ComponentImp
**        ^
**         |
**         |            psy_ui_Component <>----<> psy_ui_ViewComponentImp
**         |                    ^
**         |                    |
**  psy_ui_TabBar <@>------ psy_ui_Tab
**                       *
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	psy_ui_TABMODE_SINGLESEL,	
	psy_ui_TABMODE_LABEL
} TabMode;

typedef enum {
	TABCHECKSTATE_OFF = 0,
	TABCHECKSTATE_ON = 1
} psy_ui_TabCheckState;

/* psy_ui_Tab */
typedef struct psy_ui_Tab {	
	/* inherits */
	psy_ui_Component component;
	/* Signals */
	psy_Signal signal_clicked;
	/* internal */
	char* text;
	char* translation;
	TabMode mode;		
	bool istoggle;	
	psy_ui_TabCheckState checkstate;
	psy_ui_Bitmap icon;
	double bitmapident;
	uintptr_t index;
	bool preventtranslation;
} psy_ui_Tab;

void psy_ui_tab_init(psy_ui_Tab*, psy_ui_Component* parent, psy_ui_Component* view,
	const char* text, uintptr_t index);

psy_ui_Tab* psy_ui_tab_alloc(void);
psy_ui_Tab* psy_ui_tab_allocinit(psy_ui_Component* parent, psy_ui_Component* view,
	const char* text, uintptr_t index);

void psy_ui_tab_settext(psy_ui_Tab*, const char* text);
void psy_ui_tab_setmode(psy_ui_Tab*, TabMode);
void psy_ui_tab_preventtranslation(psy_ui_Tab*);
void psy_ui_tab_loadresource(psy_ui_Tab*, uintptr_t resourceid,
	psy_ui_Colour transparency);

INLINE psy_ui_Component* psy_ui_tab_base(psy_ui_Tab* self)
{
	return &self->component;
}

/* psy_ui_TabBar */
typedef struct psy_ui_TabBar {
	/* inherits */
	psy_ui_Component component;
	/* Signals */
	psy_Signal signal_change;
	/* internal	*/
	uintptr_t selected;	
	uintptr_t numtabs;
	bool preventtranslation;
	psy_ui_AlignType tabalignment;
} psy_ui_TabBar;

void psy_ui_tabbar_init(psy_ui_TabBar*, psy_ui_Component* parent);

psy_ui_Tab* psy_ui_tabbar_append(psy_ui_TabBar*, const char* label);
void psy_ui_tabbar_append_tabs(psy_ui_TabBar*, const char* label, ...);
void psy_ui_tabbar_clear(psy_ui_TabBar*);
void psy_ui_tabbar_select(psy_ui_TabBar*, uintptr_t tabindex);
void psy_ui_tabbar_mark(psy_ui_TabBar*, uintptr_t tabindex);
void psy_ui_tabbar_unmark(psy_ui_TabBar*);
void psy_ui_tabbar_settabalign(psy_ui_TabBar*, psy_ui_AlignType align);
void psy_ui_tabbar_preventtranslation(psy_ui_TabBar*);

INLINE uintptr_t psy_ui_tabbar_selected(const psy_ui_TabBar* self)
{	
	assert(self);

	return self->selected;	
}

void psy_ui_tabbar_settabmode(psy_ui_TabBar*, uintptr_t tab, TabMode);
psy_ui_Tab* psy_ui_tabbar_tab(psy_ui_TabBar*, uintptr_t tabindex);
const psy_ui_Tab* psy_ui_tabbar_tab_const(const psy_ui_TabBar*, uintptr_t tabindex);

INLINE psy_ui_Component* psy_ui_tabbar_base(psy_ui_TabBar* self)
{
	assert(self);

	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_TABBAR_H */
