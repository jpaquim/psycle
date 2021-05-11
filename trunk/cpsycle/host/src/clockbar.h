// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(CLOCKBAR_H)
#define CLOCKBAR_H

#include "uilabel.h"
#include "uibutton.h"
#include "workspace.h"
// std
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// displays the app time

typedef struct {
	psy_ui_Component component;
	psy_ui_Label header;
	psy_ui_Label position;
	time_t start;
	Workspace* workspace;
} ClockBar;

void clockbar_init(ClockBar*, psy_ui_Component* parent,
	Workspace* workspace);

void clockbar_idle(ClockBar*);

INLINE psy_ui_Component* clockbar_base(ClockBar* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* CLOCKBAR_H */
