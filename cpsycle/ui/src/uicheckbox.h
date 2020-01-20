// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_CHECKBOX_H
#define psy_ui_CHECKBOX_H

#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
   psy_ui_Component component;
   psy_Signal signal_clicked;   
} psy_ui_CheckBox;

void psy_ui_checkbox_init(psy_ui_CheckBox*, psy_ui_Component* parent);
void psy_ui_checkbox_settext(psy_ui_CheckBox*, const char* text);
void psy_ui_checkbox_check(psy_ui_CheckBox*);
void psy_ui_checkbox_disablecheck(psy_ui_CheckBox*);
int psy_ui_checkbox_checked(psy_ui_CheckBox*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_CHECKBOX_H */
