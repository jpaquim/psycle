// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_COMBOBOX_H
#define psy_ui_COMBOBOX_H

#include "uicomponent.h"

// ComboBox
// Bridge
// Aim: avoid coupling to one platform (win32, xt/motif, etc)
// Abstraction/Refined  psy_ui_ComboBox
// Implementor			psy_ui_ComboBoxImp
// Concrete Implementor	psy_ui_win_ComboBoxImp
//
// psy_ui_Component <>----<> psy_ui_ComponentImp <----- psy_ui_win_ComponentImp
//      ^                               ^                         |
//      |                               |                         | 
//      |                               |                        <> 
// psy_ui_ComboBox             psy_ui_ComboBoxImp <------ psy_ui_WinComboBoxImp

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_ComboBoxImp;

typedef struct {
   psy_ui_Component component;   
   struct psy_ui_ComboBoxImp* imp;   
   psy_Signal signal_selchanged;
   int ownerdrawn;   
   int hover;
   int charnumber;
} psy_ui_ComboBox;

void psy_ui_combobox_init(psy_ui_ComboBox*, psy_ui_Component* parent);
intptr_t psy_ui_combobox_addtext(psy_ui_ComboBox*, const char* text);
void psy_ui_combobox_clear(psy_ui_ComboBox*);
void psy_ui_combobox_setcursel(psy_ui_ComboBox*, intptr_t index);
intptr_t psy_ui_combobox_cursel(psy_ui_ComboBox*);
void psy_ui_combobox_setcharnumber(psy_ui_ComboBox*, int num);

INLINE psy_ui_Component* psy_psy_ui_combobox_base(psy_ui_ComboBox* self)
{
    return &self->component;
}

// uicomboboximp
// vtable function pointers
typedef int (*psy_ui_fp_comboboximp_dev_addtext)(struct psy_ui_ComboBoxImp*, const char* text);
typedef void (*psy_ui_fp_comboboximp_dev_settext)(struct psy_ui_ComboBoxImp*, const char* text, intptr_t index);
typedef void (*psy_ui_fp_comboboximp_dev_text)(struct psy_ui_ComboBoxImp*, char* text);
typedef void (*psy_ui_fp_comboboximp_dev_setstyle)(struct psy_ui_ComboBoxImp*, int style);
typedef void (*psy_ui_fp_comboboximp_dev_clear)(struct psy_ui_ComboBoxImp*);
typedef void (*psy_ui_fp_comboboximp_dev_setcursel)(struct psy_ui_ComboBoxImp*, intptr_t index);
typedef intptr_t(*psy_ui_fp_comboboximp_dev_cursel)(struct psy_ui_ComboBoxImp*);
typedef intptr_t(*psy_ui_fp_comboboximp_dev_count)(struct psy_ui_ComboBoxImp*);
typedef void (*psy_ui_fp_comboboximp_dev_selitems)(struct psy_ui_ComboBoxImp*, int* items, int maxitems);
typedef intptr_t(*psy_ui_fp_comboboximp_dev_selcount)(struct psy_ui_ComboBoxImp*);
typedef void (*psy_ui_fp_comboboximp_dev_showdropdown)(struct psy_ui_ComboBoxImp*);

typedef struct {
    psy_ui_fp_comboboximp_dev_addtext dev_addtext;
    psy_ui_fp_comboboximp_dev_settext dev_settext;
    psy_ui_fp_comboboximp_dev_setstyle dev_setstyle;
    psy_ui_fp_comboboximp_dev_text dev_text;
    psy_ui_fp_comboboximp_dev_clear dev_clear;
    psy_ui_fp_comboboximp_dev_setcursel dev_setcursel;
    psy_ui_fp_comboboximp_dev_cursel dev_cursel;
    psy_ui_fp_comboboximp_dev_count dev_count;
    psy_ui_fp_comboboximp_dev_selitems dev_selitems;
    psy_ui_fp_comboboximp_dev_selcount dev_selcount;
    psy_ui_fp_comboboximp_dev_showdropdown dev_showdropdown;
} psy_ui_ComboBoxImpVTable;

typedef struct psy_ui_ComboBoxImp {
    psy_ui_ComponentImp component_imp;
    psy_ui_ComboBoxImpVTable* vtable;
} psy_ui_ComboBoxImp;

void psy_ui_comboboximp_init(psy_ui_ComboBoxImp*);

INLINE void psy_psy_ui_combobox_text(psy_ui_ComboBox* self, char* text)
{
    self->imp->vtable->dev_text(self->imp, text);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COMBOBOX_H */
