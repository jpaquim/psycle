// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(NEWVAL_H)
#define NEWVAL_H

// host
#include "workspace.h"
// ui
#include <uibutton.h>
#include <uiedit.h>
#include <uilabel.h>

#ifdef __cplusplus
extern "C" {
#endif

// parameter value window.

typedef struct NewValView {
	// inherits
	psy_ui_Component component;
	// ui elements
	psy_ui_Label title;
	psy_ui_Component editbar;
	psy_ui_Edit	edit;
	psy_ui_Label text;
	psy_ui_Button apply;
	psy_ui_Button cancel;
	// internal data;
	int value;
	int v_min;
	int v_max;
	int macindex;
	int paramindex;
	char dlgtitle[256];
	bool doapply;
	bool docancel;	
	// references
	Workspace* workspace;
} NewValView;
	
void newvalview_init(NewValView*, psy_ui_Component* parent,
	int mindex, int pindex, int vval, int vmin, int vmax, char* title,
	Workspace*);
void newvalview_reset(NewValView*, int mindex, int pindex, int vval, int vmin,
	int vmax, char* title);

INLINE psy_ui_Component* newvalview_base(NewValView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* NEWVAL_H */
