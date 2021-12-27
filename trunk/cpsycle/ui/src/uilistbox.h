/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_LISTBOX_H
#define psy_ui_LISTBOX_H

#include "../../detail/psyconf.h"

/* ui */
#include "uiscroller.h"
#include <hashtbl.h>

/* ListBox */

#ifdef __cplusplus
extern "C" {
#endif

/*
** PSY_USE_PLATFORM_LISTBOX
** Bridge
** Aim: avoid coupling to one platform (win32, xt/motif, etc)
** Abstraction/Refined  psy_ui_ListBox
** Implementor			psy_ui_ComponentImp
** Concrete Implementor	psy_ui_win_ComponentImp
**
** psy_ui_Component <>----<> psy_ui_ComponentImp <----- psy_ui_win_ComponentImp
**      ^                               ^                         |
**      |                               |                         |
**      |                               |                        <>
** psy_ui_ListBox             psy_ui_ComponentImp <------ psy_ui_WinListBoxImp
*/

struct psy_ui_ComponentImp;

struct psy_ui_Component;

typedef struct psy_ui_ListBoxClient {
    /* inherits */
    psy_ui_Component component;
    /* signals */
    psy_Signal signal_selchanged;
    /* internal */
    psy_Table items;
    intptr_t selindex;
    double charnumber;
} psy_ui_ListBoxClient;

void psy_ui_listboxclient_init(psy_ui_ListBoxClient*, psy_ui_Component* parent);
void psy_ui_listboxclient_clear(psy_ui_ListBoxClient*);
intptr_t psy_ui_listboxclient_addtext(psy_ui_ListBoxClient*, const char* text);
void psy_ui_listboxclient_setcursel(psy_ui_ListBoxClient*, intptr_t index);
intptr_t psy_ui_listboxclient_cursel(const psy_ui_ListBoxClient*);
void psy_ui_listboxclient_setcharnumber(psy_ui_ListBoxClient*, double num);

typedef struct psy_ui_ListBox  {
    /* inherits */
    psy_ui_Component component;
    /* signals */
    psy_Signal signal_selchanged;    
    /* internal */
    psy_ui_ListBoxClient client;
    psy_ui_Scroller scroller;
    double charnumber;
} psy_ui_ListBox;

void psy_ui_listbox_init(psy_ui_ListBox*, psy_ui_Component* parent);
void psy_ui_listbox_init_multiselect(psy_ui_ListBox*, psy_ui_Component* parent);
intptr_t psy_ui_listbox_addtext(psy_ui_ListBox*, const char* text);
void psy_ui_listbox_settext(psy_ui_ListBox*, const char* text, intptr_t index);
void psy_ui_listbox_clear(psy_ui_ListBox*);
void psy_ui_listbox_setcursel(psy_ui_ListBox*, intptr_t index);
intptr_t psy_ui_listbox_cursel(const psy_ui_ListBox*);
void psy_ui_listbox_addsel(psy_ui_ListBox*, intptr_t index);
void psy_ui_listbox_selitems(psy_ui_ListBox*, intptr_t* items, intptr_t maxitems);
intptr_t psy_ui_listbox_selcount(psy_ui_ListBox*);
void psy_ui_listbox_setcharnumber(psy_ui_ListBox*, double num);
intptr_t psy_ui_listbox_count(const psy_ui_ListBox*);

INLINE psy_ui_Component* psy_ui_listbox_base(psy_ui_ListBox* self)
{
    return &self->component;
}

void psy_ui_listbox_text(psy_ui_ListBox* self, char* text,
    uintptr_t index);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_LISTBOX_H */
