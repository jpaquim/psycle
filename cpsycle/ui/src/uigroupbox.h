// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UIGROUPBOX_H)
#define UIGROUPBOX_H

#include "uicomponent.h"

typedef struct {
   ui_component component;
} ui_groupbox;


void ui_groupbox_init(ui_groupbox* groupbox, ui_component* parent);
void ui_groupbox_settext(ui_groupbox* groupbox, const char* text);

#endif
