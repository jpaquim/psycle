// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(NAVIGATION_H)
#define NAVIGATION_H

#include "uibutton.h"
#include "workspace.h"

#ifdef __cplusplus
extern "C" {
#endif

// aim: browser like navigation for the previous selected views.

typedef struct {
	psy_ui_Component component;
	psy_ui_Button prev;
	psy_ui_Button next;
	Workspace* workspace;
} Navigation;

void navigation_init(Navigation*, psy_ui_Component* parent, Workspace*);

INLINE psy_ui_Component* navigation_base(Navigation* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif
