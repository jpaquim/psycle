// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_KNOB_H
#define psy_ui_KNOB_H

#include "uicomponent.h"
#include "uibitmap.h"

typedef struct {
	psy_ui_Component component;
	psy_Signal signal_tweak;
	psy_ui_Bitmap* bitmap;
	char* label;
	char* desc;
} psy_ui_Knob;

void ui_knob_init(psy_ui_Knob*, psy_ui_Component* parent);
void ui_knob_setbitmap(psy_ui_Knob*, psy_ui_Bitmap* bitmap);
void ui_knob_setlabel(psy_ui_Knob*, const char* text);
void ui_knob_setdesc(psy_ui_Knob*, const char* text);

#endif /* psy_ui_KNOB_H */
