// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UISTATUSBAR_H)
#define UISTATUSBAR_H

#include "uicomponent.h"

typedef struct {
   psy_ui_Component component;
} psy_ui_Statusbar;

void ui_statusbar_init(psy_ui_Statusbar*, psy_ui_Component* parent);
void ui_statusbar_setfields(psy_ui_Statusbar*, int parts, int iStatusWidths[]);
void ui_statusbar_settext(psy_ui_Statusbar*, int field, const char* text);

#endif
