/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_CHECKBOX_H
#define psy_ui_CHECKBOX_H

/* local */
#include "uilabel.h"

/* CheckBox */

#ifdef __cplusplus
extern "C" {
#endif

/*
** psy_ui_CheckBox
**
** psy_ui_Component <>----<> psy_ui_ComponentImp
**         ^
**         |
**         |
** psy_ui_CheckBox <>----- psy_ui_Component
**                 <>----- psy_ui_Label
*/

typedef struct psy_ui_CheckBox {
   /* inherits */
   psy_ui_Component component;   
   /* signals */
   psy_Signal signal_clicked;
   /* internal */
   psy_ui_Component checkmark;
   psy_ui_Label text;   
   /* references */
   psy_Property* property;
} psy_ui_CheckBox;

void psy_ui_checkbox_init(psy_ui_CheckBox*, psy_ui_Component* parent);
void psy_ui_checkbox_init_text(psy_ui_CheckBox*, psy_ui_Component* parent,
    const char* text);

void psy_ui_checkbox_set_text(psy_ui_CheckBox*, const char* text);
const char* psy_ui_checkbox_text(psy_ui_CheckBox*);
void psy_ui_checkbox_check(psy_ui_CheckBox*);
void psy_ui_checkbox_disable_check(psy_ui_CheckBox*);
bool psy_ui_checkbox_checked(const psy_ui_CheckBox*);
void psy_ui_checkbox_prevent_wrap(psy_ui_CheckBox*);
void psy_ui_checkbox_enablewrap(psy_ui_CheckBox*);
void psy_ui_checkbox_data_exchange(psy_ui_CheckBox*, psy_Property*);

INLINE psy_ui_Component* psy_ui_checkbox_base(psy_ui_CheckBox* self)
{
    return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_CHECKBOX_H */
