// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(CHECKUNSAVED_H)
#define CHECKUNSAVED_H

// host
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	CONFIRM_YES,
	CONFIRM_NO,
	CONFIRM_CONTINUE
} ConfirmBoxButton;

typedef struct ConfirmBox {
	// inherits
	psy_ui_Component component;
	// signals
	psy_Signal signal_execute;
	// ui elements
	psy_ui_Component view;
	psy_ui_Label title;
	psy_ui_Label header;
	psy_ui_Button yes;
	psy_ui_Button no;
	psy_ui_Button cont;			
	ConfirmBoxAction mode;
	// internal data
	char* titlestr;
	char* yesstr;
	char* nostr;	
	// references
	Workspace* workspace;
} ConfirmBox;

void confirmbox_init(ConfirmBox*, psy_ui_Component* parent, Workspace*);
void confirmbox_setexitmode(ConfirmBox*);
void confirmbox_setnewsongmode(ConfirmBox*);
void confirmbox_setlabels(ConfirmBox* self, const char* title,
	const char* yesstr, const char* nostr);

INLINE psy_ui_Component* confirmbox_base(ConfirmBox* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* CHECKUNSAVED_H */
