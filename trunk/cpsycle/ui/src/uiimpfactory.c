// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiimpfactory.h"
#include "uiapp.h"
#include "uiwinapp.h"
#include "uiwincomponentimp.h"
#include "uiwinlabelimp.h"
#include "uiwincheckboximp.h"

#include <stdlib.h>

struct psy_ui_ComponentImp* psy_ui_impfactory_allocinit_componentimp(psy_ui_Component* component, psy_ui_Component* parent)
{
	psy_ui_win_ComponentImp* rv;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*) app.platform;
	rv = psy_ui_win_componentimp_allocinit(
		component,
		parent ? parent->imp : 0,
		winapp->componentclass,
		0, 0, 90, 90,
		WS_CHILDWINDOW | WS_VISIBLE,
		0);
	if (rv->hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_ComponentImp*) rv;
}

struct psy_ui_LabelImp* psy_ui_impfactory_allocinit_labelimp(psy_ui_Component* component, psy_ui_Component* parent)
{
	psy_ui_win_LabelImp* rv;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)app.platform;
	rv = psy_ui_win_labelimp_allocinit(
		component,
		parent ? parent->imp : 0);
	if (rv->win_component_imp.hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_LabelImp*)rv;
}

struct psy_ui_CheckBoxImp* psy_ui_impfactory_allocinit_checkboximp(psy_ui_Component* component, psy_ui_Component* parent)
{
	psy_ui_win_CheckBoxImp* rv;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)app.platform;
	rv = psy_ui_win_checkboximp_allocinit(
		component,
		parent ? parent->imp : 0);
	if (rv->win_component_imp.hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_CheckBoxImp*)rv;
}
