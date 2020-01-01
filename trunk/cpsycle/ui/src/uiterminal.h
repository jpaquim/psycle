// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(UITERMINAL_H)
#define UITERMINAL_H

#include "uicomponent.h"
#include "uieditor.h"

typedef struct {
   psy_ui_Component component;
   psy_ui_Editor output;
} psy_ui_Terminal;

void ui_terminal_init(psy_ui_Terminal*, psy_ui_Component* parent);
void ui_terminal_output(psy_ui_Terminal*, const char* text);
void ui_terminal_clear(psy_ui_Terminal*);

#endif
