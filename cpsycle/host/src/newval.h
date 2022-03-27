/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(NEWVAL_H)
#define NEWVAL_H

/* host */
#include "workspace.h"
/* ui */
#include <uibutton.h>
#include <uilabel.h>
#include <uitextarea.h>

#ifdef __cplusplus
extern "C" {
#endif

/* parameter value window. */

typedef struct NewValView {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Label title;
	psy_ui_Component editbar;
	psy_ui_TextArea edit;
	psy_ui_Label text;
	psy_ui_Button apply;
	psy_ui_Button cancel;	
	intptr_t value;
	intptr_t v_min;
	intptr_t v_max;
	uintptr_t macindex;
	uintptr_t paramindex;
	char dlgtitle[256];
	bool doapply;
	bool docancel;	
	/* references */
	Workspace* workspace;
} NewValView;
	
void newvalview_init(NewValView*, psy_ui_Component* parent,
	uintptr_t mindex, uintptr_t pindex, intptr_t vval, intptr_t vmin, intptr_t vmax, char* title,
	Workspace*);
void newvalview_reset(NewValView*, uintptr_t mindex, uintptr_t pindex, intptr_t vval, intptr_t vmin,
	intptr_t vmax, char* title);

INLINE psy_ui_Component* newvalview_base(NewValView* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* NEWVAL_H */
