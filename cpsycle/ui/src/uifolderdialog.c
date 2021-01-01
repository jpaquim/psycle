// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiapp.h"
#include "uifolderdialog.h"
#include "uiimpfactory.h"
#include <stdlib.h>

void psy_ui_folderdialog_init(psy_ui_FolderDialog* self, psy_ui_Component* parent)
{
	self->imp = psy_ui_impfactory_allocinit_folderdialogimp(
		psy_ui_app_impfactory(&app),
		parent);
}

void psy_ui_folderdialog_init_all(psy_ui_FolderDialog* self,
	psy_ui_Component* parent,
	const char* title,
	const char* initialdir)
{
	self->imp = psy_ui_impfactory_allocinit_all_folderdialogimp(
		psy_ui_app_impfactory(&app),
		parent,
		title,	
		initialdir);
}

void psy_ui_folderdialog_dispose(psy_ui_FolderDialog* self)
{
	self->imp->vtable->dev_dispose(self->imp);
	free(self->imp);
	self->imp = 0;
}

int psy_ui_folderdialog_execute(psy_ui_FolderDialog* self)
{	
	return self->imp->vtable->dev_execute(self->imp);
}

const char* psy_ui_folderdialog_path(psy_ui_FolderDialog* self)
{
	return self->imp->vtable->dev_path(self->imp);
}

// psy_ui_FolderImp
static void dev_dispose(psy_ui_FolderDialogImp* self) { }
static int dev_execute(psy_ui_FolderDialogImp* self) { return 0;  }
static const char* dev_path(psy_ui_FolderDialogImp* self) { return 0; }

static psy_ui_FolderDialogImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(void)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = dev_dispose;
		imp_vtable.dev_execute = dev_execute;
		imp_vtable.dev_path = dev_path;
		imp_vtable_initialized = 1;
	}
}

void psy_ui_folderdialogimp_init(psy_ui_FolderDialogImp* self)
{
	imp_vtable_init();
	self->vtable = &imp_vtable;
}
