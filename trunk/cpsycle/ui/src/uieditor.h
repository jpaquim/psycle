// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_EDITOR_H
#define psy_ui_EDITOR_H

#include "uicomponent.h"

typedef struct {
	psy_ui_Component component;	
} psy_ui_Editor;

void ui_editor_init(psy_ui_Editor*, psy_ui_Component* parent);
void ui_editor_load(psy_ui_Editor*, const char* path);

void ui_editor_settext(psy_ui_Editor*, const char* text);
void ui_editor_addtext(psy_ui_Editor*, const char* text);
void ui_editor_clear(psy_ui_Editor*);
void ui_editor_setcolor(psy_ui_Editor*, uint32_t color);
void ui_editor_setbackgroundcolor(psy_ui_Editor*, uint32_t color);

void ui_editor_enableedit(psy_ui_Editor*);
void ui_editor_preventedit(psy_ui_Editor*);

#endif /* psy_ui_EDITOR_H */
