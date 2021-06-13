/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_LISTBOX_H
#define psy_ui_LISTBOX_H

#include "../../detail/psyconf.h"

#ifdef PSY_USE_PLATFORM_LISTBOX
#include "uicomponent.h"
#else
#include "uiscroller.h"
#include <hashtbl.h>
#endif

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

typedef struct {
    /* inherits */
    psy_ui_Component component;
    /* signals */
    psy_Signal signal_selchanged;
    /* internal */    
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

/* uilistboximp */
struct psy_ui_ComponentImp;

/* vtable function pointers */
typedef int (*psy_ui_fp_listboximp_dev_addtext)(struct psy_ui_ComponentImp*,
    const char* text);
typedef void (*psy_ui_fp_listboximp_dev_settext)(struct psy_ui_ComponentImp*,
    const char* text, intptr_t index);
typedef void (*psy_ui_fp_listboximp_dev_text)(struct psy_ui_ComponentImp*,
    char* text, intptr_t index);
typedef void (*psy_ui_fp_listboximp_dev_setstyle)(struct psy_ui_ComponentImp*,
    int style);
typedef void (*psy_ui_fp_listboximp_dev_clear)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_listboximp_dev_setcursel)(struct psy_ui_ComponentImp*,
    intptr_t index);
typedef intptr_t (*psy_ui_fp_listboximp_dev_cursel)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_listboximp_dev_addsel)(struct psy_ui_ComponentImp*,
    intptr_t index);
typedef void (*psy_ui_fp_listboximp_dev_selitems)(struct psy_ui_ComponentImp*,
    intptr_t* items, intptr_t maxitems);
typedef intptr_t (*psy_ui_fp_listboximp_dev_selcount)(
    struct psy_ui_ComponentImp*);
typedef intptr_t (*psy_ui_fp_listboximp_dev_count)(struct psy_ui_ComponentImp*);

typedef struct {
    psy_ui_fp_listboximp_dev_addtext dev_addtext;
    psy_ui_fp_listboximp_dev_settext dev_settext;
    psy_ui_fp_listboximp_dev_setstyle dev_setstyle;
    psy_ui_fp_listboximp_dev_text dev_text;
    psy_ui_fp_listboximp_dev_clear dev_clear;
    psy_ui_fp_listboximp_dev_setcursel dev_setcursel;
    psy_ui_fp_listboximp_dev_cursel dev_cursel;    
    psy_ui_fp_listboximp_dev_addsel dev_addsel;
    psy_ui_fp_listboximp_dev_selitems dev_selitems;
    psy_ui_fp_listboximp_dev_selcount dev_selcount;
    psy_ui_fp_listboximp_dev_count dev_count;
} psy_ui_ListBoxImpVTable;

void psy_ui_listboximp_extend(psy_ui_ComponentImp*);

INLINE psy_ui_ListBoxImpVTable* psy_ui_listboximp_vtable(psy_ui_ComponentImp* self)
{
    return (psy_ui_ListBoxImpVTable*)self->extended_vtable;
}

INLINE void psy_ui_listbox_text(psy_ui_ListBox* self, char* text,
	uintptr_t index)
{
    psy_ui_listboximp_vtable(self->component.imp)->dev_text(
        (psy_ui_ComponentImp*)self->component.imp->extended_imp, text, index);
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_LISTBOX_H */
