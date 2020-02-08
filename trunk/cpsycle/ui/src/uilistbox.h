// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_LISTBOX_H
#define psy_ui_LISTBOX_H

#include "uicomponent.h"

// ListBox
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

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_ListBoxImp;

typedef struct {
   psy_ui_Component component;
   struct psy_ui_ListBoxImp* imp;
   psy_Signal signal_selchanged;
} psy_ui_ListBox;

void psy_ui_listbox_init(psy_ui_ListBox*, psy_ui_Component* parent);
void psy_ui_listbox_init_multiselect(psy_ui_ListBox*, psy_ui_Component* parent);
intptr_t psy_ui_listbox_addtext(psy_ui_ListBox*, const char* text);
void psy_ui_listbox_settext(psy_ui_ListBox*, const char* text, intptr_t index);
void psy_ui_listbox_clear(psy_ui_ListBox*);
void psy_ui_listbox_setcursel(psy_ui_ListBox*, intptr_t index);
intptr_t psy_ui_listbox_cursel(psy_ui_ListBox*);
void psy_ui_listbox_selitems(psy_ui_ListBox*, int* items, int maxitems);
intptr_t psy_ui_listbox_selcount(psy_ui_ListBox*);

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

INLINE psy_ui_Component* psy_psy_ui_listbox_base(psy_ui_ListBox* self)
{
    return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_LISTBOX_H */
