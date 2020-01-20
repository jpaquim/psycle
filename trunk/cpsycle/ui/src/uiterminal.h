// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_TERMINAL_H
#define psy_ui_TERMINAL_H

#include "uicomponent.h"
#include "uieditor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
   psy_ui_Component component;
   psy_ui_Editor output;
} psy_ui_Terminal;

void psy_ui_terminal_init(psy_ui_Terminal*, psy_ui_Component* parent);
void psy_ui_terminal_output(psy_ui_Terminal*, const char* text);
void psy_ui_terminal_clear(psy_ui_Terminal*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_TERMINAL_H */
