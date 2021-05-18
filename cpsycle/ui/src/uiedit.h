/* 
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_EDIT_H
#define psy_ui_EDIT_H

#include "uicomponent.h"

/*
** Edit
** Bridge
** Aim: avoid coupling to one platform (win32, xt/motif, etc)
** Abstraction/Refined  psy_ui_Edit
** Implementor			psy_ui_ComponentImp
** Concrete Implementor	psy_ui_win_ComponentImp
**
** psy_ui_Component <>----<> psy_ui_ComponentImp  <---- psy_ui_win_ComponentImp
**      ^                               ^                         |
**      |                               |                         |
**      |                               |                        <>
** psy_ui_Edit                   psy_ui_ComponentImp <------ psy_ui_WinEditImp
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_Edit {
    /* inherits */
	psy_ui_Component component;
    /* signals */
    /* emits any change */
    psy_Signal signal_change;
    /*
    ** emits if edit is inputfield and
    ** - return pressed or
    ** - focus lost or
    ** - clicked outside
    */
    psy_Signal signal_accept;
    /*
    ** emits if edit is inputfield and
    ** - esc pressed
    */
    psy_Signal signal_reject;
    // internal    
	int charnumber;
	int linenumber;
    bool isinputfield;
    bool preventedit;
} psy_ui_Edit;

void psy_ui_edit_init(psy_ui_Edit*, psy_ui_Component* parent);
void psy_ui_edit_multiline_init(psy_ui_Edit*, psy_ui_Component* parent);

void psy_ui_edit_enableinputfield(psy_ui_Edit*);
void psy_ui_edit_settext(psy_ui_Edit*, const char* text);
const char* psy_ui_edit_text(psy_ui_Edit* self);
void psy_ui_edit_setcharnumber(psy_ui_Edit*, int number);
void psy_ui_edit_setlinenumber(psy_ui_Edit*, int number);
void psy_ui_edit_enableedit(psy_ui_Edit*);
void psy_ui_edit_preventedit(psy_ui_Edit*);
void psy_ui_edit_setstyle(psy_ui_Edit*, int style);
void psy_ui_edit_setsel(psy_ui_Edit* self, intptr_t cpmin, intptr_t cpmax);

INLINE psy_ui_Component* psy_ui_edit_base(psy_ui_Edit* self)
{
    return &self->component;
}

/* uieditimp */
/* vtable function pointers */
typedef void (*psy_ui_fp_editimp_dev_settext)(struct psy_ui_ComponentImp*, const char* text);
typedef void (*psy_ui_fp_editimp_dev_text)(struct psy_ui_ComponentImp*, char* text);
typedef void (*psy_ui_fp_editimp_dev_enableedit)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_editimp_dev_preventedit)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_editimp_dev_setstyle)(struct psy_ui_ComponentImp*, int style);
typedef void (*psy_ui_fp_editimp_dev_setsel)(struct psy_ui_ComponentImp*, intptr_t cpmin, intptr_t cpmax);

typedef struct {
    psy_ui_fp_editimp_dev_settext dev_settext;
    psy_ui_fp_editimp_dev_setstyle dev_setstyle;
    psy_ui_fp_editimp_dev_text dev_text;
    psy_ui_fp_editimp_dev_enableedit dev_enableedit;
    psy_ui_fp_editimp_dev_preventedit dev_preventedit;
    psy_ui_fp_editimp_dev_setsel dev_setsel;
} psy_ui_EditImpVTable;

void psy_ui_editimp_extend(psy_ui_ComponentImp*);

INLINE psy_ui_EditImpVTable* psy_ui_editimp_vtable(psy_ui_ComponentImp* self)
{
    return (psy_ui_EditImpVTable*)self->extended_vtable;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_EDIT_H */
