// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#ifndef psy_ui_native_CHECKBOXIMP_H
#define psy_ui_native_CHECKBOXIMP_H

#include "../../uicheckbox.h"

#ifdef __cplusplus
extern "C" {
#endif

struct psy_ui_Component;

typedef struct psy_ui_native_CheckBoxImp {	
	psy_ui_ComponentImp* imp;
	struct psy_ui_Component* component;
	char* text;
	int multiline;
	int state;
} psy_ui_native_CheckBoxImp;

void psy_ui_native_checkboximp_init(psy_ui_native_CheckBoxImp*,
	struct psy_ui_Component*,
	struct psy_ui_Component* parent);
void psy_ui_native_checkboximp_init_multiline(psy_ui_native_CheckBoxImp*,
	struct psy_ui_Component*,
	struct psy_ui_Component*);

psy_ui_native_CheckBoxImp* psy_ui_native_checkboximp_alloc(void);
psy_ui_native_CheckBoxImp* psy_ui_native_checkboximp_allocinit(
	struct psy_ui_Component* component,
	struct psy_ui_Component* parent);
psy_ui_native_CheckBoxImp* psy_ui_native_checkboximp_allocinit_multiline(
	struct psy_ui_Component* component,
	struct psy_ui_Component* parent);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_native_ComponentImp_H */
