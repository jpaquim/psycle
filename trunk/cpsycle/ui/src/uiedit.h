// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UIEDIT_H)
#define UIEDIT_H

#include "uicomponent.h"

typedef struct {
	ui_component component;
	int charnumber;
	int linenumber;
	psy_Signal signal_change;
} ui_edit;

void ui_edit_init(ui_edit*, ui_component* parent, int styles);
void ui_edit_settext(ui_edit*, const char* text);
const char* ui_edit_text(ui_edit*);
void ui_edit_setcharnumber(ui_edit*, int number);
void ui_edit_setlinenumber(ui_edit*, int number);
void ui_edit_enableedit(ui_edit*);
void ui_edit_preventedit(ui_edit*);

#endif