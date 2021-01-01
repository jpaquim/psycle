// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiimpfactory.h"
#include <stdlib.h>

struct psy_ui_BitmapImp* psy_ui_impfactory_allocinit_bitmapimp(psy_ui_ImpFactory* self)
{
	return self->vtable->allocinit_bitmapimp(self);
}

struct psy_ui_GraphicsImp* psy_ui_impfactory_allocinit_graphicsimp(psy_ui_ImpFactory* self, uintptr_t* platformdc)
{
	return self->vtable->allocinit_graphicsimp(self, platformdc);
}

struct psy_ui_FontImp* psy_ui_impfactory_allocinit_fontimp(psy_ui_ImpFactory* self, const struct psy_ui_FontInfo* fontinfo)
{
	return self->vtable->allocinit_fontimp(self, fontinfo);
}

struct psy_ui_ComponentImp* psy_ui_impfactory_allocinit_componentimp(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_componentimp(self, component, parent);
}

struct psy_ui_ComponentImp* psy_ui_impfactory_allocinit_frameimp(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_frameimp(self, component, parent);
}

struct psy_ui_ListBoxImp* psy_ui_impfactory_allocinit_listboximp(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_listboximp(self, component, parent);
}

struct psy_ui_ListBoxImp* psy_ui_impfactory_allocinit_listboximp_multiselect(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_listboximp_multiselect(self, component, parent);
}

struct psy_ui_ComboBoxImp* psy_ui_impfactory_allocinit_comboboximp(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_comboboximp(self, component, parent);
}

struct psy_ui_EditImp* psy_ui_impfactory_allocinit_editimp(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_editimp(self, component, parent);
}

struct psy_ui_EditImp* psy_ui_impfactory_allocinit_editimp_multiline(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_editimp_multiline(self, component, parent);
}

struct psy_ui_CheckBoxImp* psy_ui_impfactory_allocinit_checkboximp(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_checkboximp(self, component, parent);
}

struct psy_ui_CheckBoxImp* psy_ui_impfactory_allocinit_checkboximp_multiline(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_checkboximp_multiline(self, component, parent);
}

struct psy_ui_ColourDialogImp* psy_ui_impfactory_allocinit_colourdialogimp(psy_ui_ImpFactory* self, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_colourdialogimp(self, parent);
}

struct psy_ui_OpenDialogImp* psy_ui_impfactory_allocinit_opendialogimp(psy_ui_ImpFactory* self, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_opendialogimp(self, parent);
}

struct psy_ui_OpenDialogImp* psy_ui_impfactory_allocinit_all_opendialogimp(psy_ui_ImpFactory* self, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir)
{
	return self->vtable->allocinit_all_opendialogimp(self, parent, title,
		filter, defaultextension, initialdir);
}

struct psy_ui_SaveDialogImp* psy_ui_impfactory_allocinit_savedialogimp(psy_ui_ImpFactory* self, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_savedialogimp(self, parent);
}

struct psy_ui_SaveDialogImp* psy_ui_impfactory_allocinit_all_savedialogimp(psy_ui_ImpFactory* self, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir)
{
	return self->vtable->allocinit_all_savedialogimp(self, parent, title,
		filter, defaultextension, initialdir);
}

struct psy_ui_FolderDialogImp* psy_ui_impfactory_allocinit_folderdialogimp(psy_ui_ImpFactory* self, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_folderdialogimp(self, parent);
}

struct psy_ui_FolderDialogImp* psy_ui_impfactory_allocinit_all_folderdialogimp(psy_ui_ImpFactory* self, struct psy_ui_Component* parent,
	const char* title,	
	const char* initialdir)
{
	return self->vtable->allocinit_all_folderdialogimp(self, parent, title,
		 initialdir);
}

struct psy_ui_FontDialogImp* psy_ui_impfactory_allocinit_fontdialogimp(psy_ui_ImpFactory* self, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_fontdialogimp(self, parent);
}

static struct psy_ui_BitmapImp* allocinit_bitmapimp(struct psy_ui_ImpFactory* self) { return 0; }
static struct psy_ui_GraphicsImp* allocinit_graphicsimp(struct psy_ui_ImpFactory* self, uintptr_t* platformdc) { return 0; }
static struct psy_ui_FontImp* allocinit_fontimp(psy_ui_ImpFactory* self, const struct psy_ui_FontInfo* fontinfo) { return 0; }
static struct psy_ui_ComponentImp* allocinit_componentimp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return 0; }
static struct psy_ui_ComponentImp* allocinit_frameimp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return 0; }
static struct psy_ui_EditImp* allocinit_editimp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return 0; }
static struct psy_ui_EditImp* allocinit_editimp_multiline(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return 0; }
static struct psy_ui_LabelImp* allocinit_labelimp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return 0; }
static struct psy_ui_ListBoxImp* allocinit_listboximp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return 0; }
static struct psy_ui_ListBoxImp* allocinit_listboximp_multiselect(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return 0; }
static struct psy_ui_ComboBoxImp* allocinit_comboboximp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return 0; }
static struct psy_ui_CheckBoxImp* allocinit_checkboximp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return 0; }
static struct psy_ui_CheckBoxImp* allocinit_checkboximp_multiline(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return 0; }
static struct psy_ui_ColourDialogImp* allocinit_colourdialogimp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* parent) { return 0; }
struct psy_ui_OpenDialogImp* allocinit_opendialogimp(psy_ui_ImpFactory* self, struct psy_ui_Component* parent) { return 0; }
struct psy_ui_OpenDialogImp* allocinit_all_opendialogimp(psy_ui_ImpFactory* self, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir)
{
	return 0;
}
struct psy_ui_SaveDialogImp* allocinit_savedialogimp(psy_ui_ImpFactory* self, struct psy_ui_Component* parent) { return 0; }
struct psy_ui_SaveDialogImp* allocinit_all_savedialogimp(psy_ui_ImpFactory* self, struct psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir)
{
	return 0;
}
struct psy_ui_FolderDialogImp* allocinit_folderdialogimp(psy_ui_ImpFactory* self, struct psy_ui_Component* parent) { return 0; }
struct psy_ui_FolderDialogImp* allocinit_all_folderdialogimp(psy_ui_ImpFactory* self, struct psy_ui_Component* parent,
	const char* title,	
	const char* initialdir)
{
	return 0;
}
struct psy_ui_FontDialogImp* allocinit_fontdialogimp(psy_ui_ImpFactory* self, struct psy_ui_Component* parent) { return 0; }

// VTable init
static psy_ui_ImpFactoryVTable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.allocinit_bitmapimp = vtable.allocinit_bitmapimp;
		vtable.allocinit_graphicsimp = allocinit_graphicsimp;
		vtable.allocinit_fontimp = allocinit_fontimp;
		vtable.allocinit_componentimp = allocinit_componentimp;
		vtable.allocinit_frameimp = allocinit_frameimp;
		vtable.allocinit_editimp = allocinit_editimp;
		vtable.allocinit_editimp_multiline = allocinit_editimp_multiline;		
		vtable.allocinit_listboximp = allocinit_listboximp;
		vtable.allocinit_listboximp_multiselect = allocinit_listboximp_multiselect;
		vtable.allocinit_comboboximp = allocinit_comboboximp;
		vtable.allocinit_checkboximp = allocinit_checkboximp;
		vtable.allocinit_checkboximp_multiline = allocinit_checkboximp_multiline;
		vtable.allocinit_colourdialogimp = allocinit_colourdialogimp;
		vtable.allocinit_opendialogimp = allocinit_opendialogimp;
		vtable.allocinit_all_opendialogimp = allocinit_all_opendialogimp;
		vtable.allocinit_savedialogimp = allocinit_savedialogimp;
		vtable.allocinit_all_savedialogimp = allocinit_all_savedialogimp;
		vtable.allocinit_folderdialogimp = allocinit_folderdialogimp;
		vtable.allocinit_all_folderdialogimp = allocinit_all_folderdialogimp;
		vtable.allocinit_fontdialogimp = allocinit_fontdialogimp;
		vtable_initialized = 1;
	}
}

void psy_ui_impfactory_init(psy_ui_ImpFactory* self)
{
	vtable_init();
	self->vtable = &vtable;
}
