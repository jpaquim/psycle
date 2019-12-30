// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(PSY_UI_STATUSBAR_H)
#define PSY_UI_STATUSBAR_H

#include "uicomponent.h"

typedef struct {
   psy_ui_Component component;
} psy_ui_StatusBar;

void psy_ui_statusbar_init(psy_ui_StatusBar*, psy_ui_Component* parent);
void psy_ui_statusbar_setfields(psy_ui_StatusBar*, int parts, int iStatusWidths[]);
void psy_ui_statusbar_settext(psy_ui_StatusBar*, int field, const char* text);

#endif
