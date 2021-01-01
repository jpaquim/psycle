// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_WIN32
#ifndef psy_ui_win_COMBOBOXIMP_H
#define psy_ui_win_COMBOBOXIMP_H

#include "uicombobox.h"
#include "uiwincomponentimp.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct psy_ui_win_ComboBoxImp {
		psy_ui_ComboBoxImp imp;		
		psy_ui_win_ComponentImp win_component_imp;
		psy_ui_win_ComponentImp win_combo_imp;		
		struct psy_ui_Component* component;
	} psy_ui_win_ComboBoxImp;

	void psy_ui_win_comboboximp_init(psy_ui_win_ComboBoxImp* self,
		struct psy_ui_Component* component,
		struct psy_ui_ComponentImp* parent);

	psy_ui_win_ComboBoxImp* psy_ui_win_comboboximp_alloc(void);
	psy_ui_win_ComboBoxImp* psy_ui_win_comboboximp_allocinit(
		struct psy_ui_Component* component,
		psy_ui_ComponentImp* parent);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_win_ComboBoxImp_H */
#endif /* PSYCLE_TK_WIN32 */
