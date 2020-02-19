// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/psyconf.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT
#ifndef psy_ui_xt_COMBOBOXIMP_H
#define psy_ui_xt_COMBOBOXIMP_H

#include "uicombobox.h"
#include "uixtcomponentimp.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef struct psy_ui_xt_ComboBoxImp {
		psy_ui_ComboBoxImp imp;		
		psy_ui_xt_ComponentImp xt_component_imp;
		psy_ui_xt_ComponentImp xt_combo_imp;		
		struct psy_ui_Component* component;
	} psy_ui_xt_ComboBoxImp;

	void psy_ui_xt_comboboximp_init(psy_ui_xt_ComboBoxImp* self,
		struct psy_ui_Component* component,
		struct psy_ui_ComponentImp* parent);

	psy_ui_xt_ComboBoxImp* psy_ui_xt_comboboximp_alloc(void);
	psy_ui_xt_ComboBoxImp* psy_ui_xt_comboboximp_allocinit(
		struct psy_ui_Component* component,
		psy_ui_ComponentImp* parent);

#ifdef __cplusplus
}
#endif

#endif /* psy_ui_xt_ComboBoxImp_H */
#endif /* PSYCLE_TK_XT */
