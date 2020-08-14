// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_LISTBOX_H
#define psy_ui_LISTBOX_H

#include "../../detail/psyconf.h"

#ifdef PSY_USE_PLATFORM_LISTBOX
#include "uicomponent.h"
#else
#include "uiscroller.h"
#include <hashtbl.h>
#endif

// ListBox

#ifdef __cplusplus
extern "C" {
#endif

#ifdef PSY_USE_PLATFORM_LISTBOX

// PSY_USE_PLATFORM_LISTBOX
// Bridge
// Aim: avoid coupling to one platform (win32, xt/motif, etc)
// Abstraction/Refined  psy_ui_ListBox
// Implementor			psy_ui_ListBoxImp
// Concrete Implementor	psy_ui_win_ListBoxImp
//
// psy_ui_Component <>----<> psy_ui_ComponentImp <----- psy_ui_win_ComponentImp
//      ^                               ^                         |
//      |                               |                         | 
//      |                               |                        <> 
// psy_ui_ListBox             psy_ui_ListBoxImp <------ psy_ui_WinListBoxImp

struct psy_ui_ListBoxImp;

typedef struct {
   psy_ui_Component component;
   struct psy_ui_ListBoxImp* imp;
   psy_Signal signal_selchanged;
   uintptr_t charnumber;
} psy_ui_ListBox;

#else

typedef struct psy_ui_ListBoxClient {
    psy_ui_Component component;
    psy_Table items;
    intptr_t selindex;
    psy_Signal signal_selchanged;
    uintptr_t charnumber;
} psy_ui_ListBoxClient;

void psy_ui_listboxclient_init(psy_ui_ListBoxClient*, psy_ui_Component* parent);
void psy_ui_listboxclient_clear(psy_ui_ListBoxClient*);
intptr_t psy_ui_listboxclient_addtext(psy_ui_ListBoxClient*, const char* text);
void psy_ui_listboxclient_setcursel(psy_ui_ListBoxClient*, intptr_t index);
intptr_t psy_ui_listboxclient_cursel(psy_ui_ListBoxClient*);
void psy_ui_listboxclient_setcharnumber(psy_ui_ListBoxClient*, uintptr_t num);

typedef struct {
    psy_ui_Component component;
    psy_ui_ListBoxClient client;
    psy_ui_Scroller scroller;
    psy_Signal signal_selchanged;
} psy_ui_ListBox;

#endif

void psy_ui_listbox_init(psy_ui_ListBox*, psy_ui_Component* parent);
void psy_ui_listbox_init_multiselect(psy_ui_ListBox*, psy_ui_Component* parent);
intptr_t psy_ui_listbox_addtext(psy_ui_ListBox*, const char* text);
void psy_ui_listbox_settext(psy_ui_ListBox*, const char* text, intptr_t index);
void psy_ui_listbox_clear(psy_ui_ListBox*);
void psy_ui_listbox_setcursel(psy_ui_ListBox*, intptr_t index);
intptr_t psy_ui_listbox_cursel(psy_ui_ListBox*);
void psy_ui_listbox_selitems(psy_ui_ListBox*, int* items, int maxitems);
intptr_t psy_ui_listbox_selcount(psy_ui_ListBox*);
void psy_ui_listbox_setcharnumber(psy_ui_ListBox*, uintptr_t num);

INLINE psy_ui_Component* psy_ui_listbox_base(psy_ui_ListBox* self)
{
    return &self->component;
}

// uilistboximp
struct psy_ui_ListBoxImp;

// vtable function pointers
typedef int (*psy_ui_fp_listboximp_dev_addtext)(struct psy_ui_ListBoxImp*, const char* text);
typedef void (*psy_ui_fp_listboximp_dev_settext)(struct psy_ui_ListBoxImp*, const char* text, intptr_t index);
typedef void (*psy_ui_fp_listboximp_dev_text)(struct psy_ui_ListBoxImp*, char* text, intptr_t index);
typedef void (*psy_ui_fp_listboximp_dev_setstyle)(struct psy_ui_ListBoxImp*, int style);
typedef void (*psy_ui_fp_listboximp_dev_clear)(struct psy_ui_ListBoxImp*);
typedef void (*psy_ui_fp_listboximp_dev_setcursel)(struct psy_ui_ListBoxImp*, intptr_t index);
typedef intptr_t (*psy_ui_fp_listboximp_dev_cursel)(struct psy_ui_ListBoxImp*);
typedef void (*psy_ui_fp_listboximp_dev_selitems)(struct psy_ui_ListBoxImp*, int* items, int maxitems);
typedef intptr_t (*psy_ui_fp_listboximp_dev_selcount)(struct psy_ui_ListBoxImp*);

typedef struct {
    psy_ui_fp_listboximp_dev_addtext dev_addtext;
    psy_ui_fp_listboximp_dev_settext dev_settext;
    psy_ui_fp_listboximp_dev_setstyle dev_setstyle;
    psy_ui_fp_listboximp_dev_text dev_text;
    psy_ui_fp_listboximp_dev_clear dev_clear;
    psy_ui_fp_listboximp_dev_setcursel dev_setcursel;
    psy_ui_fp_listboximp_dev_cursel dev_cursel;
    psy_ui_fp_listboximp_dev_selitems dev_selitems;
    psy_ui_fp_listboximp_dev_selcount dev_selcount;
} psy_ui_ListBoxImpVTable;

typedef struct psy_ui_ListBoxImp {
    psy_ui_ComponentImp component_imp;
    psy_ui_ListBoxImpVTable* vtable;
} psy_ui_ListBoxImp;

void psy_ui_listboximp_init(psy_ui_ListBoxImp*);

#ifdef PSY_USE_PLATFORM_LISTBOX

INLINE void psy_psy_ui_listbox_text(psy_ui_ListBox* self, char* text, uintptr_t index)
{
    self->imp->vtable->dev_text(self->imp, text, index);        
}

#endif

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_LISTBOX_H */
