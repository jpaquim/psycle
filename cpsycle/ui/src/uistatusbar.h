// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net


#if !defined(UISTATUSBAR)
#define UISTATUSBAR

#include "uicomponent.h"

typedef struct {
   ui_component component;
} ui_statusbar;

void ui_statusbar_init(void* self, ui_statusbar* statusbar, ui_component* parent);
void ui_statusbar_setfields(ui_statusbar* self, int parts, int iStatusWidths[]);
void ui_statusbar_settext(ui_statusbar* self, int field, const char* text);

#endif
