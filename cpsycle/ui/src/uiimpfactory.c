/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiimpfactory.h"
/* std */
#include <stdlib.h>

struct psy_ui_AppImp* psy_ui_impfactory_allocinit_appimp(psy_ui_ImpFactory* self,
	struct psy_ui_App* app, uintptr_t instance)
{
	return self->vtable->allocinit_appimp(self, app, instance);
}

struct psy_ui_BitmapImp* psy_ui_impfactory_allocinit_bitmapimp(psy_ui_ImpFactory* self, psy_ui_RealSize size)
{
	return self->vtable->allocinit_bitmapimp(self, size);
}

struct psy_ui_GraphicsImp* psy_ui_impfactory_allocinit_graphicsimp(psy_ui_ImpFactory* self, uintptr_t* platformdc)
{
	return self->vtable->allocinit_graphicsimp(self, platformdc);
}

struct psy_ui_GraphicsImp* psy_ui_impfactory_allocinit_graphicsimp_bitmap(psy_ui_ImpFactory* self, struct psy_ui_Bitmap* bitmap)
{
	return self->vtable->allocinit_graphicsimp_bitmap(self, bitmap);
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

struct psy_ui_ComponentImp* psy_ui_impfactory_allocinit_toolframeimp(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_toolframeimp(self, component, parent);
}

struct psy_ui_ComponentImp* psy_ui_impfactory_allocinit_listboximp(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_listboximp(self, component, parent);
}

struct psy_ui_ComponentImp* psy_ui_impfactory_allocinit_listboximp_multiselect(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_listboximp_multiselect(self, component, parent);
}

struct psy_ui_ComponentImp* psy_ui_impfactory_allocinit_comboboximp(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent, struct psy_ui_Component* view)
{
	return self->vtable->allocinit_comboboximp(self, component, parent, view);
}

struct psy_ui_ComponentImp* psy_ui_impfactory_allocinit_editimp(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_editimp(self, component, parent);
}

struct psy_ui_ComponentImp* psy_ui_impfactory_allocinit_editimp_multiline(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_editimp_multiline(self, component, parent);
}

struct psy_ui_ComponentImp* psy_ui_impfactory_allocinit_checkboximp(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
{
	return self->vtable->allocinit_checkboximp(self, component, parent);
}

struct psy_ui_ComponentImp* psy_ui_impfactory_allocinit_checkboximp_multiline(psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent)
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

static struct psy_ui_BitmapImp* allocinit_bitmapimp(struct psy_ui_ImpFactory* self) { return NULL; }
static struct psy_ui_GraphicsImp* allocinit_graphicsimp(struct psy_ui_ImpFactory* self, uintptr_t* platformdc) { return NULL; }
static struct psy_ui_GraphicsImp* allocinit_graphicsimp_bitmap(struct psy_ui_ImpFactory* self, struct psy_ui_Bitmap* bitmap) { return NULL; }
static struct psy_ui_FontImp* allocinit_fontimp(psy_ui_ImpFactory* self, const struct psy_ui_FontInfo* fontinfo) { return NULL; }
static struct psy_ui_ComponentImp* allocinit_componentimp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return NULL; }
static struct psy_ui_ComponentImp* allocinit_frameimp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return NULL; }
static struct psy_ui_ComponentImp* allocinit_toolframeimp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return NULL; }
static struct psy_ui_ComponentImp* allocinit_editimp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return NULL; }
static struct psy_ui_ComponentImp* allocinit_editimp_multiline(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return NULL; }
static struct psy_ui_LabelImp* allocinit_labelimp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return NULL; }
static struct psy_ui_ComponentImp* allocinit_listboximp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return NULL; }
static struct psy_ui_ComponentImp* allocinit_listboximp_multiselect(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return NULL; }
static struct psy_ui_ComponentImp* allocinit_comboboximp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent, struct psy_ui_Component* view) { return NULL; }
static struct psy_ui_ComponentImp* allocinit_checkboximp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return NULL; }
static struct psy_ui_ComponentImp* allocinit_checkboximp_multiline(struct psy_ui_ImpFactory* self, struct psy_ui_Component* component, struct psy_ui_Component* parent) { return NULL; }
static struct psy_ui_ColourDialogImp* allocinit_colourdialogimp(struct psy_ui_ImpFactory* self, struct psy_ui_Component* parent) { return NULL; }
struct psy_ui_OpenDialogImp* allocinit_opendialogimp(psy_ui_ImpFactory* self, struct psy_ui_Component* parent) { return NULL; }
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

/* vtable */
static psy_ui_ImpFactoryVTable vtable;
static bool vtable_initialized = FALSE;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.allocinit_appimp = psy_ui_impfactory_allocinit_appimp;
		vtable.allocinit_bitmapimp = psy_ui_impfactory_allocinit_bitmapimp;
		vtable.allocinit_graphicsimp = allocinit_graphicsimp;
		vtable.allocinit_graphicsimp_bitmap = allocinit_graphicsimp_bitmap;
		vtable.allocinit_fontimp = allocinit_fontimp;
		vtable.allocinit_componentimp = allocinit_componentimp;
		vtable.allocinit_frameimp = allocinit_frameimp;
		vtable.allocinit_toolframeimp = allocinit_toolframeimp;
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
		vtable_initialized = TRUE;
	}
}

void psy_ui_impfactory_init(psy_ui_ImpFactory* self)
{
	vtable_init();
	self->vtable = &vtable;
}
