// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_SAVEDIALOG_H
#define psy_ui_SAVEDIALOG_H

#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_SaveDialog {
	psy_ui_Component* parent;
	char* title;
	char* filter;
	char* defaultextension;
	char* initialdir;
	char* filename;
} psy_ui_SaveDialog;

void psy_ui_savedialog_init(psy_ui_SaveDialog*, psy_ui_Component* parent);
void psy_ui_savedialog_init_all(psy_ui_SaveDialog*,
	psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir);
void psy_ui_savedialog_dispose(psy_ui_SaveDialog*);

int psy_ui_savedialog_execute(psy_ui_SaveDialog*);
const char* psy_ui_savedialog_filename(psy_ui_SaveDialog*);

#ifdef __cplusplus
}
#endif

#endif
