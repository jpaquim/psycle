/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_COMPONENTBACKGROUND_H
#define psy_ui_COMPONENTBACKGROUND_H

/* local */
#include "uigraphics.h"

struct psy_ui_Component;

typedef struct psy_ui_ComponentBackground {
	struct psy_ui_Component* component;
	uintptr_t bgframetimer;
	uintptr_t currbgframe;
} psy_ui_ComponentBackground;

void psy_ui_componentbackground_init(psy_ui_ComponentBackground*,
	struct psy_ui_Component* component);
void psy_ui_componentbackground_dispose(psy_ui_ComponentBackground*);

void psy_ui_componentbackground_draw(psy_ui_ComponentBackground*,
	psy_ui_Graphics*);
void psy_ui_componentbackground_idle(psy_ui_ComponentBackground*);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_COMPONENTBACKGROUND_H */
