/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_CHECKBOX_H
#define psy_ui_CHECKBOX_H

/* local */
#include "uicomponent.h"

/* CheckBox */

#ifdef __cplusplus
extern "C" {
#endif

/*
** psy_ui_CheckBox
**
** Bridge
** Aim: avoid coupling to one platform (win32, xt/motif, etc)
** Abstraction/Refined  psy_ui_CheckBox
** Implementor			psy_ui_ComponentImp
** Concrete Implementor	psy_ui_win_CheckBoxImp, psy_ui_native_CheckBoxImp
**
** psy_ui_Component <>----<> psy_ui_ComponentImp <---- psy_ui_win_ComponentImp
**      ^                               ^                        |
**      |                               |                        | 
**      |                               |                        <> 
** psy_ui_CheckBox            psy_ui_ComponentImp <-- psy_ui_WinCheckBoxImp
*/

struct psy_ui_ComponentImp;

typedef struct psy_ui_CheckBox {
   /* inherits */
   psy_ui_Component component;   
   /* signals */
   psy_Signal signal_clicked;
   /* internal */
   char* text;
   char* translation;
   int multiline;   
} psy_ui_CheckBox;

void psy_ui_checkbox_init(psy_ui_CheckBox*, psy_ui_Component* parent);
void psy_ui_checkbox_init_multiline(psy_ui_CheckBox*, psy_ui_Component* parent);
void psy_ui_checkbox_init_text(psy_ui_CheckBox*, psy_ui_Component* parent,
    const char* text);
void psy_ui_checkbox_settext(psy_ui_CheckBox*, const char* text);
const char* psy_ui_checkbox_text(psy_ui_CheckBox*);
void psy_ui_checkbox_check(psy_ui_CheckBox*);
void psy_ui_checkbox_disablecheck(psy_ui_CheckBox*);
int psy_ui_checkbox_checked(psy_ui_CheckBox*);

INLINE psy_ui_Component* psy_ui_checkbox_base(psy_ui_CheckBox* self)
{
    return &self->component;
}

/* uicheckboximp */
struct psy_ui_ComponentImp;
/* vtable function pointers */
typedef void (*psy_ui_fp_checkboximp_dev_settext)(struct psy_ui_ComponentImp*, const char* text);
typedef void (*psy_ui_fp_checkboximp_dev_text)(struct psy_ui_ComponentImp*, char* text);
typedef void (*psy_ui_fp_checkboximp_dev_check)(struct psy_ui_ComponentImp*);
typedef void (*psy_ui_fp_checkboximp_dev_disablecheck)(struct psy_ui_ComponentImp*);
typedef int (*psy_ui_fp_checkboximp_dev_checked)(struct psy_ui_ComponentImp*);

typedef struct {
    psy_ui_fp_checkboximp_dev_settext dev_settext;    
    psy_ui_fp_checkboximp_dev_text dev_text;
    psy_ui_fp_checkboximp_dev_check dev_check;
    psy_ui_fp_checkboximp_dev_disablecheck dev_disablecheck;
    psy_ui_fp_checkboximp_dev_checked dev_checked;
} psy_ui_CheckBoxImpVTable;

void psy_ui_checkboximp_extend(psy_ui_ComponentImp*);

INLINE psy_ui_CheckBoxImpVTable* psy_ui_checkboximp_vtable(psy_ui_ComponentImp* self)
{
    return (psy_ui_CheckBoxImpVTable*)self->extended_vtable;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_CHECKBOX_H */
