/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_TERMINAL_H
#define psy_ui_TERMINAL_H

/* local */
#include "uicomponent.h"
#include "uieditor.h"
/* thread */
#include <lock.h>

#ifdef __cplusplus
extern "C" {
#endif

/* psy_ui_Terminal */

typedef struct psy_ui_Terminal {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_TextInputor output;
	psy_List* strbuffer;
	psy_Lock outputlock;	
} psy_ui_Terminal;

void psy_ui_terminal_init(psy_ui_Terminal*, psy_ui_Component* parent);

/* output synchronized with ui thread */
void psy_ui_terminal_output(psy_ui_Terminal*, const char* text);
void psy_ui_terminal_clear(psy_ui_Terminal*);

INLINE psy_ui_Component* psy_ui_terminal_base(psy_ui_Terminal* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_TERMINAL_H */
