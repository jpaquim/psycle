/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#ifndef psy_ui_VIEWCOMPONENTIMP_H
#define psy_ui_VIEWCOMPONENTIMP_H

#include "uicomponent.h"

/* psy_ui_ViewComponentImp */
typedef struct psy_ui_ViewComponentImp {
	/* inherits */
	psy_ui_ComponentImp imp;
	/* internal */
	struct psy_ui_Component* component;	
	uintptr_t winid;		
	int preventwmchar;
	psy_ui_RealRectangle position;
	int dbg;	
	bool visible;
	psy_ui_Component* view;
	psy_ui_Component* parent;
	psy_List* viewcomponents;
} psy_ui_ViewComponentImp;

void psy_ui_viewcomponentimp_init(psy_ui_ViewComponentImp* self,
	struct psy_ui_Component* component,
	psy_ui_Component* parent,
	psy_ui_Component* view,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand);

psy_ui_ViewComponentImp* psy_ui_viewcomponentimp_alloc(void);
psy_ui_ViewComponentImp* psy_ui_viewcomponentimp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_Component* parent,
	psy_ui_Component* view,
	const char* classname,
	int x, int y, int width, int height,
	uint32_t dwStyle,
	int usecommand);


#ifdef __cplusplus
}
#endif

#endif /* psy_ui_VIEWCOMPONENTIMP_H */
