// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UIGROUPBOX_H)
#define UIGROUPBOX_H

#include "uicomponent.h"

typedef struct {
   psy_ui_Component component;
} psy_ui_Groupbox;


void ui_groupbox_init(psy_ui_Groupbox* groupbox, psy_ui_Component* parent);
void ui_groupbox_settext(psy_ui_Groupbox* groupbox, const char* text);

#endif
