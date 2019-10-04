// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UILABEL_H)
#define UILABEL_H

#include "uicomponent.h"

typedef struct {
   ui_component component;
   int charnumber;
} ui_label;


void ui_label_init(ui_label* label, ui_component* parent);
void ui_label_settext(ui_label* label, const char* text);
void ui_label_setcharnumber(ui_label*, int number);

#endif