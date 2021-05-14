/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/ 

#if !defined(NAVIGATION_H)
#define NAVIGATION_H

/* host */
#include "workspace.h"
/* ui */
#include "uibutton.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
** Navigation
**
** Button Controls to navigate between previous selected views.
*/

typedef struct Navigation {
	/* inherits */
	psy_ui_Component component;
	/* internal */
	psy_ui_Button prev;
	psy_ui_Button next;
	/* references */
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

#endif /* NAVIGATION_H */
