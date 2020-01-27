// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_CHECKBOX_H
#define psy_ui_CHECKBOX_H

#include "uicomponent.h"

// CheckBox
// Bridge
// Aim: avoid coupling to one platform (win32, xt/motif, etc)
// Abstraction/Refined  psy_ui_Label
// Implementor			psy_ui_LabelImp
// Concrete Implementor	psy_ui_win_LabelImp
//
// psy_ui_Component <>----<> psy_ui_ComponentImp <---- psy_ui_win_ComponentImp
//      ^                               ^                        |
//      |                               |                        | 
//      |                               |                        <> 
// psy_ui_CheckBox            psy_ui_CheckBoxImp <-- psy_ui_WinCheckBoxImp

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
   psy_ui_Component component;
   struct psy_ui_CheckBoxImp* imp;
   psy_Signal signal_clicked;   
} psy_ui_CheckBox;

void psy_ui_checkbox_init(psy_ui_CheckBox*, psy_ui_Component* parent);
void psy_ui_checkbox_settext(psy_ui_CheckBox*, const char* text);
void psy_ui_checkbox_check(psy_ui_CheckBox*);
void psy_ui_checkbox_disablecheck(psy_ui_CheckBox*);
int psy_ui_checkbox_checked(psy_ui_CheckBox*);
INLINE psy_ui_Component* psy_ui_checkbox_base(psy_ui_CheckBox* self)
{
    return &self->component;
}

// uicheckboximp
// vtable function pointers
typedef void (*psy_ui_fp_checkboximp_dev_settext)(struct psy_ui_CheckBoxImp*, const char* text);
typedef void (*psy_ui_fp_checkboximp_dev_text)(struct psy_ui_CheckBoxImp*, char* text);
typedef void (*psy_ui_fp_checkboximp_dev_check)(struct psy_ui_CheckBoxImp*);
typedef void (*psy_ui_fp_checkboximp_dev_disablecheck)(struct psy_ui_CheckBoxImp*);
typedef int (*psy_ui_fp_checkboximp_dev_checked)(struct psy_ui_CheckBoxImp*);

typedef struct {
    psy_ui_fp_checkboximp_dev_settext dev_settext;    
    psy_ui_fp_checkboximp_dev_text dev_text;
    psy_ui_fp_checkboximp_dev_check dev_check;
    psy_ui_fp_checkboximp_dev_disablecheck dev_disablecheck;
    psy_ui_fp_checkboximp_dev_checked dev_checked;
} psy_ui_CheckBoxImpVTable;

typedef struct psy_ui_CheckBoxImp {
    psy_ui_ComponentImp component_imp;    
    psy_ui_CheckBoxImpVTable* vtable;
} psy_ui_CheckBoxImp;

void psy_ui_checkboximp_init(psy_ui_CheckBoxImp*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_CHECKBOX_H */
