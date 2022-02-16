/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"

#include "uiframe.h"
/* local */
#include "uiapp.h"
#include "uiimpfactory.h"


/* implementation */
void psy_ui_frame_init(psy_ui_Frame* self, psy_ui_Component* parent)
{	
	psy_ui_ComponentImp* imp;

	imp = psy_ui_impfactory_allocinit_frameimp(
		psy_ui_app_impfactory(psy_ui_app()), self, parent);
	psy_ui_component_init_imp(self, parent, NULL, imp);	
	psy_ui_component_setstyletype(self, psy_ui_STYLE_ROOT);
}

void psy_ui_toolframe_init(psy_ui_Frame* self, psy_ui_Component* parent)
{
	psy_ui_ComponentImp* imp;

	imp = psy_ui_impfactory_allocinit_toolframeimp(
		psy_ui_app_impfactory(psy_ui_app()), self, parent);
	psy_ui_component_init_imp(self, parent, NULL, imp);	
	psy_ui_component_setstyletype(self, psy_ui_STYLE_ROOT);
}

void psy_ui_frame_init_main(psy_ui_Frame* self)
{
	psy_ui_frame_init(self, NULL);
	psy_ui_app_setmain(psy_ui_app(), self);	
}
