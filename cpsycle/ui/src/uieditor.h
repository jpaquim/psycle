// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2019 members of the psycle project http://psycle.sourceforge.net

#if !defined(UIEDITOR_H)
#define UIEDITOR_H

#include "uicomponent.h"

typedef struct {
	ui_component component;	
} ui_editor;

void ui_editor_init(ui_editor*, ui_component* parent);
void ui_editor_load(ui_editor*, const char* path);

void ui_editor_settext(ui_editor*, const char* text);
void ui_editor_addtext(ui_editor*, const char* text);
void ui_editor_clear(ui_editor*);
void ui_editor_setcolor(ui_editor*, uint32_t color);
void ui_editor_setbackgroundcolor(ui_editor*, uint32_t color);

void ui_editor_enableedit(ui_editor*);
void ui_editor_preventedit(ui_editor*);

#endif
