// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#ifndef psy_ui_SCROLLER_H
#define psy_ui_SCROLLER_H

#include "uiscrollbar.h"

typedef struct {
	psy_ui_Component component;
	psy_ui_ScrollBar vscroll;
	psy_ui_Component bottom;
	psy_ui_ScrollBar hscroll;
	psy_ui_Component spacer;
	psy_ui_Component* client;	
} psy_ui_Scroller;

void psy_ui_scroller_init(psy_ui_Scroller*, psy_ui_Component* client,
	psy_ui_Component* parent);
//void psy_ui_scroller_updateoverflow(psy_ui_Scroller*);

//void psy_ui_scroller_connectclient(psy_ui_Scroller* self);
//void psy_ui_scroller_init_reparent(psy_ui_Scroller* self,
	//psy_ui_Component* client, psy_ui_Component* parent);

INLINE psy_ui_Component* psy_ui_scroller_base(psy_ui_Scroller* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_SCROLLBAR_H */
