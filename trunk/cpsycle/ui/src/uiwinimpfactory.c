// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/psyconf.h"

#include "uiwinimpfactory.h"
#include "uiapp.h"
#include "uiwinapp.h"
#include "uiwinbitmapimp.h"
#include "uiwingraphicsimp.h"
#include "uiwincheckboximp.h"
#include "uiwincomponentimp.h"
#include "uiwineditimp.h"
#include "uiwinlabelimp.h"
#include "uiwinlistboximp.h"

#include <stdlib.h>

// psy_ui_win_ImpFactory

static struct psy_ui_BitmapImp* allocinit_bitmapimp(psy_ui_win_ImpFactory*);
static struct psy_ui_GraphicsImp* allocinit_graphicsimp(psy_ui_win_ImpFactory*, uintptr_t* platformdc);
static struct psy_ui_ComponentImp* allocinit_componentimp(psy_ui_win_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ComponentImp* allocinit_frameimp(psy_ui_win_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_EditImp* allocinit_editimp(psy_ui_win_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_EditImp* allocinit_editimp_multiline(psy_ui_win_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_LabelImp* allocinit_labelimp(psy_ui_win_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ListBoxImp* allocinit_listboximp(psy_ui_win_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ListBoxImp* allocinit_listboximp_multiselect(psy_ui_win_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_CheckBoxImp* allocinit_checkboximp(psy_ui_win_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);

// VTable init
static psy_ui_ImpFactoryVTable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_win_ImpFactory* self)
{
	if (!vtable_initialized) {
		vtable = *self->imp.vtable;
		vtable.allocinit_bitmapimp = (psy_ui_fp_impfactory_allocinit_bitmapimp) allocinit_bitmapimp;
		vtable.allocinit_graphicsimp = (psy_ui_fp_impfactory_allocinit_graphicsimp) allocinit_graphicsimp;
		vtable.allocinit_componentimp = (psy_ui_fp_impfactory_allocinit_componentimp) allocinit_componentimp;
		vtable.allocinit_frameimp = (psy_ui_fp_impfactory_allocinit_frameimp) allocinit_frameimp;
		vtable.allocinit_editimp = (psy_ui_fp_impfactory_allocinit_editimp) allocinit_editimp;
		vtable.allocinit_editimp_multiline = (psy_ui_fp_impfactory_allocinit_editimp_multiline) allocinit_editimp_multiline;
		vtable.allocinit_labelimp = (psy_ui_fp_impfactory_allocinit_labelimp) allocinit_labelimp;
		vtable.allocinit_listboximp = (psy_ui_fp_impfactory_allocinit_listboximp) allocinit_listboximp;
		vtable.allocinit_listboximp_multiselect = (psy_ui_fp_impfactory_allocinit_listboximp_multiselect) allocinit_listboximp_multiselect;
		vtable.allocinit_checkboximp = (psy_ui_fp_impfactory_allocinit_checkboximp) allocinit_checkboximp;
		vtable_initialized = 1;
	}
}

void psy_ui_win_impfactory_init(psy_ui_win_ImpFactory* self)
{
	psy_ui_impfactory_init(&self->imp);
	vtable_init(self);
	self->imp.vtable = &vtable;
}

psy_ui_win_ImpFactory* psy_ui_win_impfactory_alloc(void)
{
	return (psy_ui_win_ImpFactory*) malloc(sizeof(psy_ui_win_ImpFactory));
}

psy_ui_win_ImpFactory* psy_ui_win_impfactory_allocinit(void)
{
	psy_ui_win_ImpFactory* rv;

	rv = psy_ui_win_impfactory_alloc();
	if (rv) {
		psy_ui_win_impfactory_init(rv);
	}
	return rv;
}

psy_ui_BitmapImp* allocinit_bitmapimp(psy_ui_win_ImpFactory* self)
{
	psy_ui_BitmapImp* rv;

	rv = (psy_ui_BitmapImp*) malloc(sizeof(psy_ui_win_BitmapImp));
	psy_ui_win_bitmapimp_init((psy_ui_win_BitmapImp*)rv);
	return rv;
}

psy_ui_GraphicsImp* allocinit_graphicsimp(psy_ui_win_ImpFactory* self, uintptr_t* platformdc)
{
	psy_ui_GraphicsImp* rv;

	rv = (psy_ui_GraphicsImp*)malloc(sizeof(psy_ui_win_GraphicsImp));
	psy_ui_win_graphicsimp_init((psy_ui_win_GraphicsImp*)rv, (HDC)platformdc);
	return rv;
}

psy_ui_ComponentImp* allocinit_componentimp(psy_ui_win_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_win_ComponentImp* rv;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)app.platform;
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
	return (psy_ui_ComponentImp*)rv;
}

psy_ui_ComponentImp* allocinit_frameimp(psy_ui_win_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_win_ComponentImp* rv;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)app.platform;
	rv = psy_ui_win_componentimp_allocinit(
		component,
		parent ? parent->imp : 0,
		winapp->appclass,
		CW_USEDEFAULT, CW_USEDEFAULT,
		CW_USEDEFAULT, CW_USEDEFAULT,
		WS_OVERLAPPEDWINDOW,
		0);
	if (rv->hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_ComponentImp*)rv;
}

psy_ui_EditImp* allocinit_editimp(psy_ui_win_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_win_EditImp* rv;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)app.platform;
	rv = psy_ui_win_editimp_allocinit(
		component,
		parent ? parent->imp : 0);
	if (rv->win_component_imp.hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_EditImp*)rv;
}

psy_ui_EditImp* allocinit_editimp_multiline(psy_ui_win_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_win_EditImp* rv;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)app.platform;
	rv = psy_ui_win_editimp_multiline_allocinit(
		component,
		parent ? parent->imp : 0);
	if (rv->win_component_imp.hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_EditImp*)rv;
}

psy_ui_LabelImp* allocinit_labelimp(psy_ui_win_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
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

psy_ui_ListBoxImp* allocinit_listboximp(psy_ui_win_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_win_ListBoxImp* rv;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)app.platform;
	rv = psy_ui_win_listboximp_allocinit(
		component,
		parent ? parent->imp : 0);
	if (rv->win_component_imp.hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_ListBoxImp*)rv;
}

psy_ui_ListBoxImp* allocinit_listboximp_multiselect(psy_ui_win_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_win_ListBoxImp* rv;
	psy_ui_WinApp* winapp;

	winapp = (psy_ui_WinApp*)app.platform;
	rv = psy_ui_win_listboximp_multiselect_allocinit(
		component,
		parent ? parent->imp : 0);
	if (rv->win_component_imp.hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_ListBoxImp*)rv;
}

psy_ui_CheckBoxImp* allocinit_checkboximp(psy_ui_win_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
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
