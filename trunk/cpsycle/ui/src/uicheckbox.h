// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(UICHECKBOX_H)
#define UICHECKBOX_H

#include "uicomponent.h"

typedef struct {
   psy_ui_Component component;
   psy_Signal signal_clicked;   
} psy_ui_CheckBox;

void psy_ui_checkbox_init(psy_ui_CheckBox*, psy_ui_Component* parent);
void psy_ui_checkbox_settext(psy_ui_CheckBox*, const char* text);
void psy_ui_checkbox_check(psy_ui_CheckBox*);
void psy_ui_checkbox_disablecheck(psy_ui_CheckBox*);
int psy_ui_checkbox_checked(psy_ui_CheckBox*);

#endif
