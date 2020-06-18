// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uixtimpfactory.h"

#if PSYCLE_USE_TK == PSYCLE_TK_XT

#include "../../detail/psyconf.h"

#include "uiapp.h"
#include "uixtapp.h"
#include "uixtbitmapimp.h"
#include "uixtgraphicsimp.h"
#include "uixtfontimp.h"
#include "uixtcheckboximp.h"
#include "uixtcomponentimp.h"
#include "uixteditimp.h"
#include "uixtlistboximp.h"
#include "uixtcomboboximp.h"
#include "uixtcolordialogimp.h"
#include "uixtopendialogimp.h"
#include "uixtsavedialogimp.h"
#include "uixtfolderdialogimp.h"
#include "uixtfontdialogimp.h"

#include <stdlib.h>

// psy_ui_xt_ImpFactory

static struct psy_ui_BitmapImp* allocinit_bitmapimp(psy_ui_xt_ImpFactory*);
static struct psy_ui_GraphicsImp* allocinit_graphicsimp(psy_ui_xt_ImpFactory*, uintptr_t* platformdc);
static struct psy_ui_FontImp* allocinit_fontimp(psy_ui_xt_ImpFactory*, const psy_ui_FontInfo*);
static struct psy_ui_ComponentImp* allocinit_componentimp(psy_ui_xt_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ComponentImp* allocinit_frameimp(psy_ui_xt_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_EditImp* allocinit_editimp(psy_ui_xt_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_EditImp* allocinit_editimp_multiline(psy_ui_xt_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ListBoxImp* allocinit_listboximp(psy_ui_xt_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ListBoxImp* allocinit_listboximp_multiselect(psy_ui_xt_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ComboBoxImp* allocinit_comboboximp(psy_ui_xt_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_CheckBoxImp* allocinit_checkboximp(psy_ui_xt_ImpFactory*, struct psy_ui_Component* component, struct psy_ui_Component* parent);
static struct psy_ui_ColorDialogImp* allocinit_colordialogimp(psy_ui_xt_ImpFactory*, struct psy_ui_Component* parent);
static struct psy_ui_OpenDialogImp* allocinit_opendialogimp(psy_ui_xt_ImpFactory*, struct psy_ui_Component* parent);
static psy_ui_OpenDialogImp* allocinit_all_opendialogimp(psy_ui_xt_ImpFactory*, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir);
static struct psy_ui_SaveDialogImp* allocinit_savedialogimp(psy_ui_xt_ImpFactory*, struct psy_ui_Component* parent);
static psy_ui_SaveDialogImp* allocinit_all_savedialogimp(psy_ui_xt_ImpFactory*, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir);
static struct psy_ui_FolderDialogImp* allocinit_folderdialogimp(psy_ui_xt_ImpFactory*, struct psy_ui_Component* parent);
static psy_ui_FolderDialogImp* allocinit_all_folderdialogimp(psy_ui_xt_ImpFactory*, struct psy_ui_Component* parent,
	const char* title,	
	const char* initialdir);
static struct psy_ui_FontDialogImp* allocinit_fontdialogimp(psy_ui_xt_ImpFactory*, struct psy_ui_Component* parent);

// VTable init
static psy_ui_ImpFactoryVTable vtable;
static int vtable_initialized = 0;

static void vtable_init(psy_ui_xt_ImpFactory* self)
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

void psy_ui_xt_impfactory_init(psy_ui_xt_ImpFactory* self)
{
	psy_ui_impfactory_init(&self->imp);
	vtable_init(self);
	self->imp.vtable = &vtable;
}

psy_ui_xt_ImpFactory* psy_ui_xt_impfactory_alloc(void)
{
	return (psy_ui_xt_ImpFactory*) malloc(sizeof(psy_ui_xt_ImpFactory));
}

psy_ui_xt_ImpFactory* psy_ui_xt_impfactory_allocinit(void)
{
	psy_ui_xt_ImpFactory* rv;

	rv = psy_ui_xt_impfactory_alloc();
	if (rv) {
		psy_ui_xt_impfactory_init(rv);
	}
	return rv;
}

psy_ui_BitmapImp* allocinit_bitmapimp(psy_ui_xt_ImpFactory* self)
{
	psy_ui_BitmapImp* rv;

	rv = (psy_ui_BitmapImp*) malloc(sizeof(psy_ui_xt_BitmapImp));
	if (rv) {
		psy_ui_xt_bitmapimp_init((psy_ui_xt_BitmapImp*)rv);
	}
	return rv;
}

psy_ui_GraphicsImp* allocinit_graphicsimp(psy_ui_xt_ImpFactory* self, uintptr_t* platformdc)
{
	psy_ui_GraphicsImp* rv;

	rv = (psy_ui_GraphicsImp*)malloc(sizeof(psy_ui_xt_GraphicsImp));
	if (rv) {
		psy_ui_xt_graphicsimp_init((psy_ui_xt_GraphicsImp*)rv, (PlatformXtGC*)platformdc);
	}
	return rv;
}

psy_ui_FontImp* allocinit_fontimp(psy_ui_xt_ImpFactory* self, const psy_ui_FontInfo* fontinfo)
{
	psy_ui_FontImp* rv;

	rv = (psy_ui_FontImp*) malloc(sizeof(psy_ui_xt_FontImp));
	if (rv) {
		psy_ui_xt_fontimp_init((psy_ui_xt_FontImp*)rv, fontinfo);
	}
	return rv;
}

psy_ui_ComponentImp* allocinit_componentimp(psy_ui_xt_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_xt_ComponentImp* rv;
	psy_ui_XtApp* winapp;

	winapp = (psy_ui_XtApp*)app.platform;
	rv = psy_ui_xt_componentimp_allocinit(
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

psy_ui_ComponentImp* allocinit_frameimp(psy_ui_xt_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_xt_ComponentImp* rv;
	psy_ui_XtApp* winapp;

	winapp = (psy_ui_XtApp*)app.platform;
	rv = psy_ui_xt_componentimp_allocinit(
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

psy_ui_EditImp* allocinit_editimp(psy_ui_xt_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_xt_EditImp* rv;
	psy_ui_XtApp* winapp;

	winapp = (psy_ui_XtApp*)app.platform;
	rv = psy_ui_xt_editimp_allocinit(
		component,
		parent ? parent->imp : 0);
	if (rv->xt_component_imp.hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_EditImp*)rv;
}

psy_ui_EditImp* allocinit_editimp_multiline(psy_ui_xt_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_xt_EditImp* rv;
	psy_ui_XtApp* winapp;

	winapp = (psy_ui_XtApp*)app.platform;
	rv = psy_ui_xt_editimp_multiline_allocinit(
		component,
		parent ? parent->imp : 0);
	if (rv->xt_component_imp.hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_EditImp*)rv;
}

psy_ui_ListBoxImp* allocinit_listboximp(psy_ui_xt_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_xt_ListBoxImp* rv;
	psy_ui_XtApp* winapp;

	winapp = (psy_ui_XtApp*)app.platform;
	rv = psy_ui_xt_listboximp_allocinit(
		component,
		parent ? parent->imp : 0);
	if (rv->xt_component_imp.hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_ListBoxImp*)rv;
}


psy_ui_ComboBoxImp* allocinit_comboboximp(psy_ui_xt_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_xt_ComboBoxImp* rv;
	psy_ui_XtApp* winapp;

	winapp = (psy_ui_XtApp*)app.platform;
	rv = psy_ui_xt_comboboximp_allocinit(
		component,
		parent ? parent->imp : 0);
	if (rv->xt_component_imp.hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_ComboBoxImp*)rv;
}

psy_ui_ListBoxImp* allocinit_listboximp_multiselect(psy_ui_xt_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_xt_ListBoxImp* rv;
	psy_ui_XtApp* winapp;

	winapp = (psy_ui_XtApp*)app.platform;
	rv = psy_ui_xt_listboximp_multiselect_allocinit(
		component,
		parent ? parent->imp : 0);
	if (rv->xt_component_imp.hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_ListBoxImp*)rv;
}

psy_ui_CheckBoxImp* allocinit_checkboximp(psy_ui_xt_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	psy_ui_xt_CheckBoxImp* rv;
	psy_ui_XtApp* winapp;

	winapp = (psy_ui_XtApp*)app.platform;
	rv = psy_ui_xt_checkboximp_allocinit(
		component,
		parent ? parent->imp : 0);
	if (rv->xt_component_imp.hwnd == 0) {
		free(rv);
		rv = 0;
	}
	return (psy_ui_CheckBoxImp*)rv;
}

psy_ui_ColorDialogImp* allocinit_colordialogimp(psy_ui_xt_ImpFactory* self, struct psy_ui_Component* parent)
{	
	psy_ui_xt_ColorDialogImp* imp;
	psy_ui_XtApp* xtapp;

	xtapp = (psy_ui_XtApp*) app.platform;
	imp = (psy_ui_xt_ColorDialogImp*) malloc(sizeof(psy_ui_xt_ColorDialogImp));
	if (imp) {
		psy_ui_xt_colordialogimp_init(imp);
		return &imp->imp;
	}
	return 0;
}

psy_ui_OpenDialogImp* allocinit_opendialogimp(psy_ui_xt_ImpFactory* self, struct psy_ui_Component* parent)
{
	psy_ui_xt_OpenDialogImp* imp;
	psy_ui_XtApp* winapp;

	winapp = (psy_ui_XtApp*)app.platform;
	imp = (psy_ui_xt_OpenDialogImp*)malloc(sizeof(psy_ui_xt_OpenDialogImp));
	if (imp) {
		psy_ui_xt_opendialogimp_init(imp, parent);
		return &imp->imp;
	}
	return 0;
}

psy_ui_OpenDialogImp* allocinit_all_opendialogimp(psy_ui_xt_ImpFactory* self, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir)
{
	psy_ui_xt_OpenDialogImp* imp;
	psy_ui_XtApp* winapp;

	winapp = (psy_ui_XtApp*)app.platform;
	imp = (psy_ui_xt_OpenDialogImp*)malloc(sizeof(psy_ui_xt_OpenDialogImp));
	if (imp) {
		psy_ui_xt_opendialogimp_init_all(imp, parent,
			title, filter, defaultextension, initialdir);
		return &imp->imp;
	}
	return 0;
}

psy_ui_SaveDialogImp* allocinit_savedialogimp(psy_ui_xt_ImpFactory* self, struct psy_ui_Component* parent)
{
	psy_ui_xt_SaveDialogImp* imp;
	psy_ui_XtApp* winapp;

	winapp = (psy_ui_XtApp*)app.platform;
	imp = (psy_ui_xt_SaveDialogImp*)malloc(sizeof(psy_ui_xt_SaveDialogImp));
	if (imp) {
		psy_ui_xt_savedialogimp_init(imp, parent);
		return &imp->imp;
	}
	return 0;
}

psy_ui_SaveDialogImp* allocinit_all_savedialogimp(psy_ui_xt_ImpFactory* self, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir)
{
	psy_ui_xt_SaveDialogImp* imp;
	psy_ui_XtApp* winapp;

	winapp = (psy_ui_XtApp*)app.platform;
	imp = (psy_ui_xt_SaveDialogImp*)malloc(sizeof(psy_ui_xt_SaveDialogImp));
	if (imp) {
		psy_ui_xt_savedialogimp_init_all(imp, parent,
			title, filter, defaultextension, initialdir);
		return &imp->imp;
	}
	return 0;
}

psy_ui_FolderDialogImp* allocinit_folderdialogimp(psy_ui_xt_ImpFactory* self, struct psy_ui_Component* parent)
{
	psy_ui_xt_FolderDialogImp* imp;
	psy_ui_XtApp* winapp;

	winapp = (psy_ui_XtApp*)app.platform;
	imp = (psy_ui_xt_FolderDialogImp*)malloc(sizeof(psy_ui_xt_FolderDialogImp));
	if (imp) {
		psy_ui_xt_folderdialogimp_init(imp);
		return &imp->imp;
	}
	return 0;
}

psy_ui_FolderDialogImp* allocinit_all_folderdialogimp(psy_ui_xt_ImpFactory* self, struct psy_ui_Component* parent,
	const char* title,	
	const char* initialdir)
{
	psy_ui_xt_FolderDialogImp* imp;
	psy_ui_XtApp* winapp;

	winapp = (psy_ui_XtApp*)app.platform;
	imp = (psy_ui_xt_FolderDialogImp*)malloc(sizeof(psy_ui_xt_FolderDialogImp));
	if (imp) {
		psy_ui_xt_folderdialogimp_init_all(imp, parent,
			title, initialdir);
		return &imp->imp;
	}
	return 0;
}

psy_ui_FontDialogImp* allocinit_fontdialogimp(psy_ui_xt_ImpFactory* self, struct psy_ui_Component* parent)
{
	psy_ui_xt_FontDialogImp* imp;
	psy_ui_XtApp* winapp;

	winapp = (psy_ui_XtApp*)app.platform;
	imp = (psy_ui_xt_FontDialogImp*) malloc(sizeof(psy_ui_xt_FontDialogImp));
	if (imp) {
		psy_ui_xt_fontdialogimp_init(imp);
		return &imp->imp;
	}
	return 0;
}

#endif
