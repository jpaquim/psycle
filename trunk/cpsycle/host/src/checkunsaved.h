// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(CHECKUNSAVED_H)
#define CHECKUNSAVED_H

#include "workspace.h"

#include <uibutton.h>
#include <uilabel.h>

typedef enum {
	CHECKUNSAVE_SAVE,
	CHECKUNSAVE_NOSAVE,
	CHECKUNSAVE_CONTINUE
} CheckUnsaveButton;

typedef struct {
	psy_ui_Component component;	
	psy_ui_Component view;
	psy_ui_Label title;
	psy_ui_Label header;
	psy_ui_Button saveandexit;
	psy_ui_Button exit;
	psy_ui_Button cont;	
	Workspace* workspace;
	psy_Signal signal_execute;
	CheckUnsaveMode mode;
	char* titlestr;
	char* savestr;
	char* nosavestr;
} CheckUnsavedBox;

void checkunsavedbox_init(CheckUnsavedBox*, psy_ui_Component* parent, Workspace*);
void checkunsavedbox_setexitmode(CheckUnsavedBox*);
void checkunsavedbox_setnewsongmode(CheckUnsavedBox*);
void checkunsavedbox_setlabels(CheckUnsavedBox* self, const char* title,
	const char* savestr, const char* nosavestr);

INLINE psy_ui_Component* checkunsavedbox_base(CheckUnsavedBox* self)
{
	return &self->component;
}

#endif /* CHECKUNSAVED_H */
