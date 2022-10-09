/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_LISTBOX_H
#define psy_ui_LISTBOX_H

/* local */
#include "uiscroller.h"
/* container */
#include <hashtbl.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** psy_ui_ListBoxPane
**
** psy_ui_Component
**      ^
**      |
** psy_ui_ListBox <>----<> psy_ui_ListBoxPane           
*/


typedef struct psy_ui_ListItem {
	char* text;
	bool selected;
} psy_ui_ListItem;

void psy_ui_listitem_init(psy_ui_ListItem*, const char* text);
void psy_ui_listitem_dispose(psy_ui_ListItem*);

psy_ui_ListItem* psy_ui_listitem_alloc(void);
psy_ui_ListItem* psy_ui_listitem_alloc_init(const char* text);

INLINE void psy_ui_listitem_select(psy_ui_ListItem* self)
{
	self->selected = TRUE;
}

INLINE void psy_ui_listitem_deselect(psy_ui_ListItem* self)
{
	self->selected = FALSE;
}

INLINE const char* psy_ui_listitem_text(const psy_ui_ListItem* self)
{
	return self->text;
}


/* psy_ui_ListBoxPane */
typedef struct psy_ui_ListBoxPane {
    /* inherits */
    psy_ui_Component component;
    /* signals */
    psy_Signal signal_selchanged;
    /* internal */
    psy_Table items;
    intptr_t selindex;
    double charnumber;
    bool multiselect;    
} psy_ui_ListBoxPane;

void psy_ui_listboxpane_init(psy_ui_ListBoxPane*, psy_ui_Component* parent);
void psy_ui_listboxpane_clear(psy_ui_ListBoxPane*);
intptr_t psy_ui_listboxpane_add_text(psy_ui_ListBoxPane*, const char* text);
void psy_ui_listboxpane_set_cur_sel(psy_ui_ListBoxPane*, intptr_t index);
intptr_t psy_ui_listboxpane_cur_sel(const psy_ui_ListBoxPane*);
void psy_ui_listboxpane_set_char_number(psy_ui_ListBoxPane*, double num);
psy_ui_ListItem* psy_ui_listboxpane_item(psy_ui_ListBoxPane*, uintptr_t index);
const psy_ui_ListItem* psy_ui_listboxpane_item_const(const psy_ui_ListBoxPane*,
	uintptr_t index);


/* psy_ui_ListBox */
typedef struct psy_ui_ListBox  {
    /* inherits */
    psy_ui_Component component;
    /* signals */
    psy_Signal signal_selchanged;    
    /* internal */
    psy_ui_ListBoxPane pane;
    psy_ui_Scroller scroller;
    double charnumber;
} psy_ui_ListBox;

void psy_ui_listbox_init(psy_ui_ListBox*, psy_ui_Component* parent);
void psy_ui_listbox_init_multi_select(psy_ui_ListBox*, psy_ui_Component* parent);
intptr_t psy_ui_listbox_add_text(psy_ui_ListBox*, const char* text);
void psy_ui_listbox_set_text(psy_ui_ListBox*, const char* text, intptr_t index);
void psy_ui_listbox_clear(psy_ui_ListBox*);
void psy_ui_listbox_set_cur_sel(psy_ui_ListBox*, intptr_t index);
intptr_t psy_ui_listbox_cur_sel(const psy_ui_ListBox*);
void psy_ui_listbox_add_sel(psy_ui_ListBox*, intptr_t index);
void psy_ui_listbox_sel_items(psy_ui_ListBox*, intptr_t* items, intptr_t maxitems);
intptr_t psy_ui_listbox_sel_count(psy_ui_ListBox*);
void psy_ui_listbox_set_char_number(psy_ui_ListBox*, double num);
intptr_t psy_ui_listbox_count(const psy_ui_ListBox*);

INLINE psy_ui_Component* psy_ui_listbox_base(psy_ui_ListBox* self)
{
    return &self->component;
}

void psy_ui_listbox_text(psy_ui_ListBox* self, char* text, uintptr_t index);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_LISTBOX_H */
