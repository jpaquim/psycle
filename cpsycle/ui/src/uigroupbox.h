// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_GROUPBOX_H
#define psy_ui_GROUPBOX_H

#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
   psy_ui_Component component;
} psy_ui_Groupbox;

void psy_ui_groupbox_init(psy_ui_Groupbox* groupbox, psy_ui_Component* parent);
void psy_ui_groupbox_settext(psy_ui_Groupbox* groupbox, const char* text);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_GROUPBOX_H */
