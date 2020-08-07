// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT
#ifndef psy_ui_x11_CHECKBOXIMP_H
#define psy_ui_x11_CHECKBOXIMP_H

#include "uicheckbox.h"
#include "uix11componentimp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct psy_ui_x11_CheckBoxImp {
	psy_ui_CheckBoxImp imp;
	psy_ui_x11_ComponentImp x11_component_imp;	
	struct psy_ui_Component* component;	
} psy_ui_x11_CheckBoxImp;

void psy_ui_x11_checkboximp_init(psy_ui_x11_CheckBoxImp* self,
	struct psy_ui_Component* component,
	struct psy_ui_ComponentImp* parent);

psy_ui_x11_CheckBoxImp* psy_ui_x11_checkboximp_alloc(void);
psy_ui_x11_CheckBoxImp* psy_ui_x11_checkboximp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_x11_ComponentImp_H */
#endif /* PSYCLE_TK_XT */
