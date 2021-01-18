// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uix11impfactory.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT

#include "../../detail/psyconf.h"

#include "uiapp.h"
#include "uix11app.h"
#include "uix11bitmapimp.h"
#include "uix11graphicsimp.h"
#include "uix11fontimp.h"
#include "uix11checkboximp.h"
#include "uix11componentimp.h"
#include "uix11editimp.h"
#include "uix11listboximp.h"
#include "uix11comboboximp.h"
#include "uix11colordialogimp.h"
#include "uix11opendialogimp.h"
#include "uix11savedialogimp.h"
#include "uix11folderdialogimp.h"
#include "uix11fontdialogimp.h"

#include <stdlib.h>

// psy_ui_x11_ImpFactory

static struct psy_ui_BitmapImp* allocinit_bitmapimp(psy_ui_x11_ImpFactory*);
static struct psy_ui_GraphicsImp* allocinit_graphicsimp(psy_ui_x11_ImpFactory*, uintptr_t* platformdc);
static struct psy_ui_FontImp* allocinit_fontimp(psy_ui_x11_ImpFactory*, const psy_ui_FontInfo*);
static struct psy_ui_ComponentImp* allocinit_componentimp(psy_ui_x11_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ComponentImp* allocinit_frameimp(psy_ui_x11_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_EditImp* allocinit_editimp(psy_ui_x11_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_EditImp* allocinit_editimp_multiline(psy_ui_x11_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ListBoxImp* allocinit_listboximp(psy_ui_x11_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ListBoxImp* allocinit_listboximp_multiselect(psy_ui_x11_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ComboBoxImp* allocinit_comboboximp(psy_ui_x11_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_CheckBoxImp* allocinit_checkboximp(psy_ui_x11_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ColorDialogImp* allocinit_colordialogimp(psy_ui_x11_ImpFactory*, struct psy_ui_Component* parent);
static struct psy_ui_OpenDialogImp* allocinit_opendialogimp(psy_ui_x11_ImpFactory*, struct psy_ui_Component* parent);
static psy_ui_OpenDialogImp* allocinit_all_opendialogimp(psy_ui_x11_ImpFactory*, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir);
static struct psy_ui_SaveDialogImp* allocinit_savedialogimp(psy_ui_x11_ImpFactory*, struct psy_ui_Component* parent);
static psy_ui_SaveDialogImp* allocinit_all_savedialogimp(psy_ui_x11_ImpFactory*, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir);
static struct psy_ui_FolderDialogImp* allocinit_folderdialogimp(psy_ui_x11_ImpFactory*, struct psy_ui_Component* parent);
static psy_ui_FolderDialogImp* allocinit_all_folderdialogimp(psy_ui_x11_ImpFactory*, struct psy_ui_Component* parent,
	const char* title,	
	const char* initialdir);
static struct psy_ui_FontDialogImp* allocinit_fontdialogimp(psy_ui_x11_ImpFactory*, struct psy_ui_Component* parent);

// VTable init
static psy_ui_ImpFactoryVTable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_x11_ImpFactory* self)
{
	if (!vtable_initialized) {
		vtable = *self->imp.vtable;
		vtable.allocinit_bitmapimp = (psy_ui_fp_impfactory_allocinit_bitmapimp) allocinit_bitmapimp;
		vtable.allocinit_graphicsimp = (psy_ui_fp_impfactory_allocinit_graphicsimp) allocinit_graphicsimp;
		vtable.allocinit_fontimp = (psy_ui_fp_impfactory_allocinit_fontimp) allocinit_fontimp;
		vtable.allocinit_componentimp = (psy_ui_fp_impfactory_allocinit_componentimp) allocinit_componentimp;
		vtable.allocinit_frameimp = (psy_ui_fp_impfactory_allocinit_frameimp) allocinit_frameimp;
		vtable.allocinit_editimp = (psy_ui_fp_impfactory_allocinit_editimp) allocinit_editimp;
		vtable.allocinit_editimp_multiline = (psy_ui_fp_impfactory_allocinit_editimp_multiline) allocinit_editimp_multiline;
		vtable.allocinit_listboximp = (psy_ui_fp_impfactory_allocinit_listboximp) allocinit_listboximp;
		vtable.allocinit_listboximp_multiselect = (psy_ui_fp_impfactory_allocinit_listboximp_multiselect) allocinit_listboximp_multiselect;
		vtable.allocinit_checkboximp = (psy_ui_fp_impfactory_allocinit_checkboximp) allocinit_checkboximp;
		vtable.allocinit_comboboximp = (psy_ui_fp_impfactory_allocinit_comboboximp) allocinit_comboboximp;
		vtable.allocinit_colordialogimp = (psy_ui_fp_impfactory_allocinit_colordialogimp) allocinit_colordialogimp;
		vtable.allocinit_opendialogimp = (psy_ui_fp_impfactory_allocinit_opendialogimp) allocinit_opendialogimp;
		vtable.allocinit_all_opendialogimp = (psy_ui_fp_impfactory_allocinit_all_opendialogimp) allocinit_all_opendialogimp;
		vtable.allocinit_savedialogimp = (psy_ui_fp_impfactory_allocinit_savedialogimp) allocinit_savedialogimp;
		vtable.allocinit_all_savedialogimp = (psy_ui_fp_impfactory_allocinit_all_savedialogimp) allocinit_all_savedialogimp;
		vtable.allocinit_folderdialogimp = (psy_ui_fp_impfactory_allocinit_folderdialogimp) allocinit_folderdialogimp;
		vtable.allocinit_all_folderdialogimp = (psy_ui_fp_impfactory_allocinit_all_folderdialogimp) allocinit_all_folderdialogimp;
		vtable.allocinit_fontdialogimp = (psy_ui_fp_impfactory_allocinit_fontdialogimp) allocinit_fontdialogimp;
		vtable_initialized = 1;
	}
}

void psy_ui_x11_impfactory_init(psy_ui_x11_ImpFactory* self)
{
	psy_ui_impfactory_init(&self->imp);
	vtable_init(self);
	self->imp.vtable = &vtable;
}

psy_ui_x11_ImpFactory* psy_ui_x11_impfactory_alloc(void)
{
	return (psy_ui_x11_ImpFactory*) malloc(sizeof(psy_ui_x11_ImpFactory));
}

psy_ui_x11_ImpFactory* psy_ui_x11_impfactory_allocinit(void)
{
	psy_ui_x11_ImpFactory* rv;

	rv = psy_ui_x11_impfactory_alloc();
	if (rv) {
		psy_ui_x11_impfactory_init(rv);
	}
	return rv;
}

psy_ui_BitmapImp* allocinit_bitmapimp(psy_ui_x11_ImpFactory* self)
{
	psy_ui_BitmapImp* rv;

	rv = (psy_ui_BitmapImp*) malloc(sizeof(psy_ui_x11_BitmapImp));
	if (rv) {
		psy_ui_x11_bitmapimp_init((psy_ui_x11_BitmapImp*)rv);
	}
	return rv;
}

psy_ui_GraphicsImp* allocinit_graphicsimp(psy_ui_x11_ImpFactory* self, uintptr_t* platformdc)
{
	psy_ui_GraphicsImp* rv;

	rv = (psy_ui_GraphicsImp*)malloc(sizeof(psy_ui_x11_GraphicsImp));
	if (rv) {
		psy_ui_x11_graphicsimp_init((psy_ui_x11_GraphicsImp*)rv, (PlatformXtGC*)platformdc);
	}
	return rv;
}

psy_ui_FontImp* allocinit_fontimp(psy_ui_x11_ImpFactory* self, const psy_ui_FontInfo* fontinfo)
{
	psy_ui_FontImp* rv;

	rv = (psy_ui_FontImp*) malloc(sizeof(psy_ui_x11_FontImp));
	if (rv) {
		psy_ui_x11_fontimp_init((psy_ui_x11_FontImp*)rv, fontinfo);
	}
	return rv;
}

psy_ui_ComponentImp* allocinit_componentimp(psy_ui_x11_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_x11_ComponentImp* rv;
	psy_ui_X11App* winapp;

	winapp = (psy_ui_X11App*)app.platform;
	rv = psy_ui_x11_componentimp_allocinit(
		component,
		parent ? parent->imp : 0,
		winapp->componentclass,
		0, 0, 90, 90,
		0, // WS_CHILDWINDOW | WS_VISIBLE,
		0);
	if (rv && rv->hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_ComponentImp*)rv;
}

psy_ui_ComponentImp* allocinit_frameimp(psy_ui_x11_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_x11_ComponentImp* rv;
	psy_ui_X11App* winapp;

	winapp = (psy_ui_X11App*)app.platform;
	rv = psy_ui_x11_componentimp_allocinit(
		component,
		parent ? parent->imp : 0,
		winapp->appclass,
		0, 0, 800, 600, 0,
		//CW_USEDEFAULT, CW_USEDEFAULT,
		//CW_USEDEFAULT, CW_USEDEFAULT,
		//WS_OVERLAPPEDWINDOW,
		0);
	if (rv->hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_ComponentImp*)rv;
}

psy_ui_EditImp* allocinit_editimp(psy_ui_x11_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_x11_EditImp* rv;
	psy_ui_X11App* winapp;

	winapp = (psy_ui_X11App*)app.platform;
	rv = psy_ui_x11_editimp_allocinit(
		component,
		parent ? parent->imp : 0);
	if (rv->x11_component_imp.hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_EditImp*)rv;
}

psy_ui_EditImp* allocinit_editimp_multiline(psy_ui_x11_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_x11_EditImp* rv;
	psy_ui_X11App* winapp;

	winapp = (psy_ui_X11App*)app.platform;
	rv = psy_ui_x11_editimp_multiline_allocinit(
		component,
		parent ? parent->imp : 0);
	if (rv->x11_component_imp.hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_EditImp*)rv;
}

psy_ui_ListBoxImp* allocinit_listboximp(psy_ui_x11_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_x11_ListBoxImp* rv;
	psy_ui_X11App* winapp;

	winapp = (psy_ui_X11App*)app.platform;
	rv = psy_ui_x11_listboximp_allocinit(
		component,
		parent ? parent->imp : 0);
	if (rv->x11_component_imp.hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_ListBoxImp*)rv;
}


psy_ui_ComboBoxImp* allocinit_comboboximp(psy_ui_x11_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_x11_ComboBoxImp* rv;
	psy_ui_X11App* winapp;

	winapp = (psy_ui_X11App*)app.platform;
	rv = psy_ui_x11_comboboximp_allocinit(
		component,
		parent ? parent->imp : 0);
	if (rv->x11_component_imp.hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_ComboBoxImp*)rv;
}

psy_ui_ListBoxImp* allocinit_listboximp_multiselect(psy_ui_x11_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_x11_ListBoxImp* rv;
	psy_ui_X11App* winapp;

	winapp = (psy_ui_X11App*)app.platform;
	rv = psy_ui_x11_listboximp_multiselect_allocinit(
		component,
		parent ? parent->imp : 0);
	if (rv->x11_component_imp.hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_ListBoxImp*)rv;
}

psy_ui_CheckBoxImp* allocinit_checkboximp(psy_ui_x11_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_x11_CheckBoxImp* rv;
	psy_ui_X11App* winapp;

	winapp = (psy_ui_X11App*)app.platform;
	rv = psy_ui_x11_checkboximp_allocinit(
		component,
		parent ? parent->imp : 0);
	if (rv->x11_component_imp.hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_CheckBoxImp*)rv;
}

psy_ui_ColorDialogImp* allocinit_colordialogimp(psy_ui_x11_ImpFactory* self, struct psy_ui_Component* parent)
{	
	psy_ui_x11_ColorDialogImp* imp;
	psy_ui_X11App* xtapp;

	xtapp = (psy_ui_X11App*) app.platform;
	imp = (psy_ui_x11_ColorDialogImp*) malloc(sizeof(psy_ui_x11_ColorDialogImp));
	if (imp) {
		psy_ui_x11_colordialogimp_init(imp);
		return &imp->imp;
	}
	return 0;
}

psy_ui_OpenDialogImp* allocinit_opendialogimp(psy_ui_x11_ImpFactory* self, struct psy_ui_Component* parent)
{
	psy_ui_x11_OpenDialogImp* imp;
	psy_ui_X11App* winapp;

	winapp = (psy_ui_X11App*)app.platform;
	imp = (psy_ui_x11_OpenDialogImp*)malloc(sizeof(psy_ui_x11_OpenDialogImp));
	if (imp) {
		psy_ui_x11_opendialogimp_init(imp, parent);
		return &imp->imp;
	}
	return 0;
}

psy_ui_OpenDialogImp* allocinit_all_opendialogimp(psy_ui_x11_ImpFactory* self, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir)
{
	psy_ui_x11_OpenDialogImp* imp;
	psy_ui_X11App* winapp;

	winapp = (psy_ui_X11App*)app.platform;
	imp = (psy_ui_x11_OpenDialogImp*)malloc(sizeof(psy_ui_x11_OpenDialogImp));
	if (imp) {
		psy_ui_x11_opendialogimp_init_all(imp, parent,
			title, filter, defaultextension, initialdir);
		return &imp->imp;
	}
	return 0;
}

psy_ui_SaveDialogImp* allocinit_savedialogimp(psy_ui_x11_ImpFactory* self, struct psy_ui_Component* parent)
{
	psy_ui_x11_SaveDialogImp* imp;
	psy_ui_X11App* winapp;

	winapp = (psy_ui_X11App*)app.platform;
	imp = (psy_ui_x11_SaveDialogImp*)malloc(sizeof(psy_ui_x11_SaveDialogImp));
	if (imp) {
		psy_ui_x11_savedialogimp_init(imp, parent);
		return &imp->imp;
	}
	return 0;
}

psy_ui_SaveDialogImp* allocinit_all_savedialogimp(psy_ui_x11_ImpFactory* self, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir)
{
	psy_ui_x11_SaveDialogImp* imp;
	psy_ui_X11App* winapp;

	winapp = (psy_ui_X11App*)app.platform;
	imp = (psy_ui_x11_SaveDialogImp*)malloc(sizeof(psy_ui_x11_SaveDialogImp));
	if (imp) {
		psy_ui_x11_savedialogimp_init_all(imp, parent,
			title, filter, defaultextension, initialdir);
		return &imp->imp;
	}
	return 0;
}

psy_ui_FolderDialogImp* allocinit_folderdialogimp(psy_ui_x11_ImpFactory* self, struct psy_ui_Component* parent)
{
	psy_ui_x11_FolderDialogImp* imp;
	psy_ui_X11App* winapp;

	winapp = (psy_ui_X11App*)app.platform;
	imp = (psy_ui_x11_FolderDialogImp*)malloc(sizeof(psy_ui_x11_FolderDialogImp));
	if (imp) {
		psy_ui_x11_folderdialogimp_init(imp);
		return &imp->imp;
	}
	return 0;
}

psy_ui_FolderDialogImp* allocinit_all_folderdialogimp(psy_ui_x11_ImpFactory* self, struct psy_ui_Component* parent,
	const char* title,	
	const char* initialdir)
{
	psy_ui_x11_FolderDialogImp* imp;
	psy_ui_X11App* winapp;

	winapp = (psy_ui_X11App*)app.platform;
	imp = (psy_ui_x11_FolderDialogImp*)malloc(sizeof(psy_ui_x11_FolderDialogImp));
	if (imp) {
		psy_ui_x11_folderdialogimp_init_all(imp, parent,
			title, initialdir);
		return &imp->imp;
	}
	return 0;
}

psy_ui_FontDialogImp* allocinit_fontdialogimp(psy_ui_x11_ImpFactory* self, struct psy_ui_Component* parent)
{
	psy_ui_x11_FontDialogImp* imp;
	psy_ui_X11App* winapp;

	winapp = (psy_ui_X11App*)app.platform;
	imp = (psy_ui_x11_FontDialogImp*) malloc(sizeof(psy_ui_x11_FontDialogImp));
	if (imp) {
		psy_ui_x11_fontdialogimp_init(imp);
		return &imp->imp;
	}
	return 0;
}

#endif
