/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiframe.h"
/* local */
#include "uiapp.h"
#include "uiimpfactory.h"
/* std */
#include <stdlib.h>

/* implementation */
void psy_ui_frame_init(psy_ui_Frame* self, psy_ui_Component* parent)
{	
	psy_ui_ComponentImp* imp;

	imp = psy_ui_impfactory_allocinit_frameimp(
		psy_ui_app_impfactory(psy_ui_app()), self, parent);
	psy_ui_component_init_imp(self, parent, NULL, imp);	
	psy_ui_component_set_style_type(self, psy_ui_STYLE_ROOT);
}

void psy_ui_toolframe_init(psy_ui_Frame* self, psy_ui_Component* parent)
{
	psy_ui_ComponentImp* imp;

	imp = psy_ui_impfactory_allocinit_toolframeimp(
		psy_ui_app_impfactory(psy_ui_app()), self, parent);
	psy_ui_component_init_imp(self, parent, NULL, imp);	
	psy_ui_component_set_style_type(self, psy_ui_STYLE_ROOT);
}

void psy_ui_frame_init_main(psy_ui_Frame* self)
{
	psy_ui_frame_init(self, NULL);
	psy_ui_app_setmain(psy_ui_app(), self);	
}

psy_ui_Component* psy_ui_frame_alloc(void)
{
	return (psy_ui_Component*)malloc(sizeof(psy_ui_Component));
}

psy_ui_Component* psy_ui_frame_allocinit(psy_ui_Component* parent)
{
	psy_ui_Component* rv;

	rv = psy_ui_component_alloc();
	if (rv) {
		psy_ui_frame_init(rv, parent);
		psy_ui_component_deallocate_after_destroyed(rv);
	}
	return rv;
}

psy_ui_Component* psy_ui_toolframe_alloc(void)
{
	return (psy_ui_Component*)malloc(sizeof(psy_ui_Component));
}

psy_ui_Component* psy_ui_toolframe_allocinit(psy_ui_Component* parent)
{
	psy_ui_Component* rv;

	rv = psy_ui_component_alloc();
	if (rv) {
		psy_ui_toolframe_init(rv, parent);
		psy_ui_component_deallocate_after_destroyed(rv);
	}
	return rv;
}
