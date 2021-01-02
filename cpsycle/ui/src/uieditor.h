// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_EDITOR_H
#define psy_ui_EDITOR_H

// local
#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

// Editor component (implementaion scintilla based)

typedef struct psy_ui_Editor {
	// inherits
	psy_ui_Component component;
} psy_ui_Editor;

void psy_ui_editor_init(psy_ui_Editor*, psy_ui_Component* parent);
void psy_ui_editor_load(psy_ui_Editor*, const char* path);
void psy_ui_editor_save(psy_ui_Editor*, const char* path);
void psy_ui_editor_settext(psy_ui_Editor*, const char* text);
void psy_ui_editor_addtext(psy_ui_Editor*, const char* text);
char* psy_ui_editor_text(psy_ui_Editor*, uintptr_t maxlength, char* text);
uintptr_t psy_ui_editor_length(psy_ui_Editor*);
void psy_ui_editor_clear(psy_ui_Editor*);
void psy_ui_editor_setcolour(psy_ui_Editor*, psy_ui_Colour);
void psy_ui_editor_setbackgroundcolour(psy_ui_Editor*, psy_ui_Colour);
void psy_ui_editor_enableedit(psy_ui_Editor*);
void psy_ui_editor_preventedit(psy_ui_Editor*);
void psy_ui_editor_enablewrap(psy_ui_Editor*);
void psy_ui_editor_disablewrap(psy_ui_Editor*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_EDITOR_H */
