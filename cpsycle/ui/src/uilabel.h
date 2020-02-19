// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_LABEL_H
#define psy_ui_LABEL_H

#include "uicomponent.h"

// Label
// Bridge
// Aim: avoid coupling to one platform (win32, xt/motif, etc)
// Abstraction/Refined  psy_ui_Label
// Implementor			psy_ui_LabelImp
// Concrete Implementor	psy_ui_win_LabelImp
//
// psy_ui_Component <>----<> psy_ui_ComponentImp  <---- psy_ui_win_ComponentImp
//      ^                               ^                         |
//      |                               |                         | 
//      |                               |                        <> 
// psy_ui_Label                   psy_ui_LabelImp <---- psy_ui_WinLabelImp

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_LabelImp;

typedef struct {
   psy_ui_Component component;
   struct psy_ui_LabelImp* imp;
   int charnumber;
} psy_ui_Label;

void psy_ui_label_init(psy_ui_Label*, psy_ui_Component* parent);
void psy_ui_label_settext(psy_ui_Label*, const char* text);
void psy_ui_label_text(psy_ui_Label*, char* text);
void psy_ui_label_setcharnumber(psy_ui_Label*, int number);
void psy_ui_label_setstyle(psy_ui_Label*, int style);
void psy_ui_label_settextalignment(psy_ui_Label*, psy_ui_Alignment alignment);
INLINE psy_ui_Component* psy_ui_label_base(psy_ui_Label* self)
{
    return &self->component;
}

// uilabelimp
// vtable function pointers
typedef void (*psy_ui_fp_labelimp_dev_settext)(struct psy_ui_LabelImp*, const char* text);
typedef void (*psy_ui_fp_labelimp_dev_text)(struct psy_ui_LabelImp*, char* text);
typedef void (*psy_ui_fp_labelimp_dev_setstyle)(struct psy_ui_LabelImp*, int style);
typedef void (*psy_ui_fp_labelimp_dev_settextalignment)(struct psy_ui_LabelImp*, psy_ui_Alignment alignment);

typedef struct {
    psy_ui_fp_labelimp_dev_settext dev_settext;
    psy_ui_fp_labelimp_dev_setstyle dev_setstyle;
    psy_ui_fp_labelimp_dev_text dev_text;
    psy_ui_fp_labelimp_dev_settextalignment dev_settextalignment;
} psy_ui_LabelImpVTable;

typedef struct psy_ui_LabelImp {
    psy_ui_ComponentImp component_imp;
    psy_ui_LabelImpVTable* vtable;
} psy_ui_LabelImp;

void psy_ui_labelimp_init(psy_ui_LabelImp*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_LABEL_H */
