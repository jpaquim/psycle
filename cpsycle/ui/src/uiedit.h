// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_EDIT_H
#define psy_ui_EDIT_H

#include "uicomponent.h"

// Edit
// Bridge
// Aim: avoid coupling to one platform (win32, xt/motif, etc)
// Abstraction/Refined  psy_ui_Edit
// Implementor			psy_ui_EditImp
// Concrete Implementor	psy_ui_win_EditImp
//
// psy_ui_Component <>----<> psy_ui_ComponentImp  <---- psy_ui_win_ComponentImp
//      ^                               ^                         |
//      |                               |                         | 
//      |                               |                        <> 
// psy_ui_Edit                   psy_ui_EditImp <------ psy_ui_WinEditImp

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_EditImp;

typedef struct {
	psy_ui_Component component;
    struct psy_ui_EditImp* imp;
	int charnumber;
	int linenumber;
	psy_Signal signal_change;
} psy_ui_Edit;

void psy_ui_edit_init(psy_ui_Edit*, psy_ui_Component* parent);
void psy_ui_edit_multiline_init(psy_ui_Edit*, psy_ui_Component* parent);
void psy_ui_edit_settext(psy_ui_Edit*, const char* text);
const char* psy_ui_edit_text(psy_ui_Edit* self);
void psy_ui_edit_setcharnumber(psy_ui_Edit*, int number);
void psy_ui_edit_setlinenumber(psy_ui_Edit*, int number);
void psy_ui_edit_enableedit(psy_ui_Edit*);
void psy_ui_edit_preventedit(psy_ui_Edit*);
void psy_ui_edit_setstyle(psy_ui_Edit*, int style);
INLINE psy_ui_Component* psy_ui_edit_base(psy_ui_Edit* self)
{
    return &self->component;
}

// uieditimp
// vtable function pointers
typedef void (*psy_ui_fp_editimp_dev_settext)(struct psy_ui_EditImp*, const char* text);
typedef void (*psy_ui_fp_editimp_dev_text)(struct psy_ui_EditImp*, char* text);
typedef void (*psy_ui_fp_editimp_dev_enableedit)(struct psy_ui_EditImp*);
typedef void (*psy_ui_fp_editimp_dev_preventedit)(struct psy_ui_EditImp*);
typedef void (*psy_ui_fp_editimp_dev_setstyle)(struct psy_ui_EditImp*, int style);

typedef struct {
    psy_ui_fp_editimp_dev_settext dev_settext;
    psy_ui_fp_editimp_dev_setstyle dev_setstyle;
    psy_ui_fp_editimp_dev_text dev_text;
    psy_ui_fp_editimp_dev_enableedit dev_enableedit;
    psy_ui_fp_editimp_dev_preventedit dev_preventedit;
} psy_ui_EditImpVTable;

typedef struct psy_ui_EditImp {
    psy_ui_ComponentImp component_imp;
    psy_ui_EditImpVTable* vtable;
} psy_ui_EditImp;

void psy_ui_editimp_init(psy_ui_EditImp*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_EDIT_H */
