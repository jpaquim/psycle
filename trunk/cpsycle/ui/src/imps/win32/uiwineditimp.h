/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
#ifndef psy_ui_win_EDITIMP_H
#define psy_ui_win_EDITIMP_H

#include "../../uiedit.h"
#include "uiwincomponentimp.h"

#ifdef __cplusplus
extern "C" {
#endif

void psy_ui_win_editimp_init(psy_ui_win_ComponentImp* self,
	struct psy_ui_Component* component,
	struct psy_ui_ComponentImp* parent);
void psy_ui_win_editimp_multiline_init(psy_ui_win_ComponentImp* self,
	psy_ui_Component* component,
	psy_ui_ComponentImp* parent);

psy_ui_win_ComponentImp* psy_ui_win_editimp_alloc(void);
psy_ui_win_ComponentImp* psy_ui_win_editimp_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent);
psy_ui_win_ComponentImp* psy_ui_win_editimp_multiline_allocinit(
	struct psy_ui_Component* component,
	psy_ui_ComponentImp* parent);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_win_ComponentImp_H */
#endif /* PSYCLE_TK_WIN32 */
