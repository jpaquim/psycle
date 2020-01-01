// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(UILABEL_H)
#define UILABEL_H

#include "uicomponent.h"

typedef struct {
   psy_ui_Component component;
   int charnumber;
} psy_ui_Label;

void ui_label_init(psy_ui_Label*, psy_ui_Component* parent);
void ui_label_settext(psy_ui_Label*, const char* text);
void ui_label_setcharnumber(psy_ui_Label*, int number);
void ui_label_setstyle(psy_ui_Label*, int style);

#endif
