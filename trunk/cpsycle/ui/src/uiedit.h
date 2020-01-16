// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_EDIT_H
#define psy_ui_EDIT_H

#include "uicomponent.h"

typedef struct {
	psy_ui_Component component;
	int charnumber;
	int linenumber;
	psy_Signal signal_change;
} psy_ui_Edit;

void ui_edit_init(psy_ui_Edit*, psy_ui_Component* parent, int styles);
void ui_edit_settext(psy_ui_Edit*, const char* text);
const char* ui_edit_text(psy_ui_Edit*);
void ui_edit_setcharnumber(psy_ui_Edit*, int number);
void ui_edit_setlinenumber(psy_ui_Edit*, int number);
void ui_edit_enableedit(psy_ui_Edit*);
void ui_edit_preventedit(psy_ui_Edit*);

#endif /* psy_ui_EDIT_H */
