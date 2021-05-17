/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_COMBOBOX_H
#define psy_ui_COMBOBOX_H

/* local */
#include "uicomponent.h"
/* container */
#include <hashtbl.h>

/*
** ComboBox
** Bridge
** Aim: avoid coupling to one platform (win32, xt/motif, etc)
** Abstraction/Refined  psy_ui_ComboBox
** Implementor			psy_ui_ComboBoxImp
** Concrete Implementor	psy_ui_win_ComboBoxImp
**
** psy_ui_Component <>----<> psy_ui_ComponentImp <----- psy_ui_win_ComponentImp
**      ^                               ^                         |
**      |                               |                         |
**      |                               |                        <>
** psy_ui_ComboBox             psy_ui_ComboBoxImp <------ psy_ui_WinComboBoxImp
*/

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_ComboBoxImp;

typedef enum psy_ui_ComboBoxHover {
    psy_ui_COMBOBOXHOVER_NONE = 0,
    psy_ui_COMBOBOXHOVER_DOWN,
    psy_ui_COMBOBOXHOVER_LESS,
    psy_ui_COMBOBOXHOVER_MORE
} psy_ui_ComboBoxHover;

typedef struct psy_ui_ComboBox {
    /* inherits */
    psy_ui_Component component;
    /* signals */
    psy_Signal signal_selchanged;
    /* internal */
    struct psy_ui_ComboBoxImp* imp;       
    int ownerdrawn;   
    psy_ui_ComboBoxHover hover;
    double charnumber;
    psy_Table itemdata;
} psy_ui_ComboBox;

void psy_ui_combobox_init(psy_ui_ComboBox*, psy_ui_Component* parent,
    psy_ui_Component* view);
intptr_t psy_ui_combobox_addtext(psy_ui_ComboBox*, const char* text);
void psy_ui_combobox_clear(psy_ui_ComboBox*);
void psy_ui_combobox_setcursel(psy_ui_ComboBox*, intptr_t index);
intptr_t psy_ui_combobox_cursel(const psy_ui_ComboBox*);
void psy_ui_combobox_setcharnumber(psy_ui_ComboBox*, double num);
void psy_ui_combobox_setitemdata(psy_ui_ComboBox*, uintptr_t index, intptr_t data);
intptr_t psy_ui_combobox_itemdata(psy_ui_ComboBox*, uintptr_t index);

INLINE psy_ui_Component* psy_psy_ui_combobox_base(psy_ui_ComboBox* self)
{
    return &self->component;
}

/* uicomboboximp */
/* vtable function pointers */
typedef int (*psy_ui_fp_comboboximp_dev_addtext)(struct psy_ui_ComboBoxImp*,
    const char* text);
typedef void (*psy_ui_fp_comboboximp_dev_settext)(struct psy_ui_ComboBoxImp*,
    const char* text, intptr_t index);
typedef void (*psy_ui_fp_comboboximp_dev_text)(struct psy_ui_ComboBoxImp*,
    char* text);
typedef void (*psy_ui_fp_comboboximp_dev_setstyle)(struct psy_ui_ComboBoxImp*,
    int style);
typedef void (*psy_ui_fp_comboboximp_dev_clear)(struct psy_ui_ComboBoxImp*);
typedef void (*psy_ui_fp_comboboximp_dev_setcursel)(struct psy_ui_ComboBoxImp*,
    intptr_t index);
typedef intptr_t(*psy_ui_fp_comboboximp_dev_cursel)(
    const struct psy_ui_ComboBoxImp*);
typedef intptr_t(*psy_ui_fp_comboboximp_dev_count)(struct psy_ui_ComboBoxImp*);
typedef void (*psy_ui_fp_comboboximp_dev_selitems)(struct psy_ui_ComboBoxImp*,
    intptr_t* items, intptr_t maxitems);
typedef intptr_t(*psy_ui_fp_comboboximp_dev_selcount)(
    struct psy_ui_ComboBoxImp*);
typedef void (*psy_ui_fp_comboboximp_dev_showdropdown)(
    struct psy_ui_ComboBoxImp*);

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

INLINE intptr_t psy_ui_combobox_count(psy_ui_ComboBox* self)
{
    return self->imp->vtable->dev_count(self->imp);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COMBOBOX_H */
