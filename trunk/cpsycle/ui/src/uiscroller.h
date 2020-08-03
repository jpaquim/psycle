// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_SCROLLER_H
#define psy_ui_SCROLLER_H

#include "uiscrollbar.h"

typedef struct {
	psy_ui_Component component;
	psy_ui_ScrollBar right;
	psy_ui_ScrollBar bottom;
	psy_ui_Component* client;
} psy_ui_Scroller;

void psy_ui_scroller_init(psy_ui_Scroller*, psy_ui_Component* client, psy_ui_Component* parent);
// void psy_ui_scroller_updateoverflow(psy_ui_Scroller*);

INLINE psy_ui_Component* psy_ui_scroller_base(psy_ui_Scroller* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_SCROLLBAR_H */
