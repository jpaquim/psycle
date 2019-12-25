// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UITERMINAL_H)
#define UITERMINAL_H

#include "uicomponent.h"
#include "uieditor.h"

typedef struct {
   ui_component component;
   ui_editor output;
} ui_terminal;

void ui_terminal_init(ui_terminal*, ui_component* parent);
void ui_terminal_output(ui_terminal*, const char* text);
void ui_terminal_clear(ui_terminal*);

#endif
