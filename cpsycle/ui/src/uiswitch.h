/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
**
*/

#ifndef psy_ui_SWITCH_H
#define psy_ui_SWITCH_H

#include "uicomponent.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_Switch {
	/* inherits */
	psy_ui_Component component;
	/* signals */
	psy_Signal signal_clicked;
	/* internal */
	bool state;
	/* references */
	psy_Property* property;
} psy_ui_Switch;

void psy_ui_switch_init(psy_ui_Switch*, psy_ui_Component* parent);
void psy_ui_switch_init_exchange(psy_ui_Switch*, psy_ui_Component* parent,
	psy_Property*);

psy_ui_Switch* psy_ui_switch_alloc(void);
psy_ui_Switch* psy_ui_switch_allocinit(psy_ui_Component* parent);
psy_ui_Switch* psy_ui_switch_allocinit_exchange(psy_ui_Component* parent, psy_Property*);

void psy_ui_switch_data_exchange(psy_ui_Switch*, psy_Property*);

void psy_ui_switch_check(psy_ui_Switch*);
void psy_ui_switch_uncheck(psy_ui_Switch*);
bool psy_ui_switch_checked(const psy_ui_Switch*);

INLINE psy_ui_Component* psy_ui_switch_base(psy_ui_Switch* self)
{
	return &self->component;
}

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_SWITCH_H */
