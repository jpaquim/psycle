// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_EDIT_H
#define psy_ui_EDIT_H

#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	psy_ui_Component component;
	int charnumber;
	int linenumber;
	psy_Signal signal_change;
} psy_ui_Edit;

void psy_ui_edit_init(psy_ui_Edit*, psy_ui_Component* parent, int styles);
void psy_ui_edit_settext(psy_ui_Edit*, const char* text);
const char* psy_ui_edit_text(psy_ui_Edit*);
void psy_ui_edit_setcharnumber(psy_ui_Edit*, int number);
void psy_ui_edit_setlinenumber(psy_ui_Edit*, int number);
void psy_ui_edit_enableedit(psy_ui_Edit*);
void psy_ui_edit_preventedit(psy_ui_Edit*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_EDIT_H */
