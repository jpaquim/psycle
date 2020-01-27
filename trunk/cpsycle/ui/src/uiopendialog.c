// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiopendialog.h"
#include "uiwinopendialogimp.h"
#include <string.h>
#include <stdlib.h>

void psy_ui_opendialog_init(psy_ui_OpenDialog* self, psy_ui_Component* parent)
{
	psy_ui_win_OpenDialogImp* imp;

	imp = (psy_ui_win_OpenDialogImp*) malloc(sizeof(psy_ui_win_OpenDialogImp));
	psy_ui_win_opendialogimp_init(imp, parent);
	self->imp = (psy_ui_OpenDialogImp*) imp;
}

void psy_ui_opendialog_init_all(psy_ui_OpenDialog* self,
	psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir)
{	
	psy_ui_win_OpenDialogImp* imp;

	imp = (psy_ui_win_OpenDialogImp*) malloc(sizeof(psy_ui_win_OpenDialogImp));
	psy_ui_win_opendialogimp_init_all(imp, parent, title, filter, defaultextension, initialdir);
	self->imp = (psy_ui_OpenDialogImp*) imp;
}

void psy_ui_opendialog_dispose(psy_ui_OpenDialog* self)
{
	self->imp->vtable->dev_dispose(self->imp);
	free(self->imp);
	self->imp = 0;
}

int psy_ui_opendialog_execute(psy_ui_OpenDialog* self)
{
	return self->imp->vtable->dev_execute(self->imp);
}

const char* psy_ui_opendialog_filename(psy_ui_OpenDialog* self)
{
	return self->imp->vtable->dev_filename(self->imp);
}

// psy_ui_OpenImp
static void dev_dispose(psy_ui_OpenDialogImp* self) { }
static int dev_execute(psy_ui_OpenDialogImp* self) { return 0; }
static const char* dev_filename(psy_ui_OpenDialogImp* self) { return 0; }

static psy_ui_OpenDialogImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(void)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = dev_dispose;
		imp_vtable.dev_execute = dev_execute;
		imp_vtable.dev_filename = dev_filename;
		imp_vtable_initialized = 1;
	}
}

void psy_ui_opendialogimp_init(psy_ui_OpenDialogImp* self)
{
	imp_vtable_init();
	self->vtable = &imp_vtable;
}
