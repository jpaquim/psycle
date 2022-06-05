/*
** This source is free software; you can redistribute itand /or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uiopendialog.h"
/* local */
#include "uiapp.h"
#include "uiimpfactory.h"
/* std */
#include <string.h>
#include <stdlib.h>

/* implementation */
void psy_ui_opendialog_init(psy_ui_OpenDialog* self, psy_ui_Component* parent)
{
	self->imp = psy_ui_impfactory_allocinit_opendialogimp(
		psy_ui_app_impfactory(psy_ui_app()),
		parent);
}

void psy_ui_opendialog_init_all(psy_ui_OpenDialog* self,
	psy_ui_Component* parent,
	const char* title,
	const char* filter,
	const char* defaultextension,
	const char* initialdir)
{	
	self->imp = psy_ui_impfactory_allocinit_all_opendialogimp(
		psy_ui_app_impfactory(psy_ui_app()),
		parent, title, filter, defaultextension, initialdir);	
}

void psy_ui_opendialog_dispose(psy_ui_OpenDialog* self)
{
	self->imp->vtable->dev_dispose(self->imp);
	free(self->imp);
	self->imp = NULL;
}

psy_ui_OpenDialog* psy_ui_opendialog_alloc(void)
{
	return (psy_ui_OpenDialog*)malloc(sizeof(psy_ui_OpenDialog*));
}

psy_ui_OpenDialog* psy_ui_opendialog_allocinit(psy_ui_Component* parent)
{
	psy_ui_OpenDialog* rv;

	rv = psy_ui_opendialog_alloc();
	if (rv) {
		psy_ui_opendialog_init(rv, parent);
	}
	return rv;
}

int psy_ui_opendialog_execute(psy_ui_OpenDialog* self)
{
	return self->imp->vtable->dev_execute(self->imp);
}

const psy_Path* psy_ui_opendialog_path(const psy_ui_OpenDialog* self)
{
	return self->imp->vtable->dev_path(self->imp);
}

/* psy_ui_OpenDialogImp */
static psy_Path path;
static bool path_initialized = FALSE;

/* prototypes */
static void dev_dispose(psy_ui_OpenDialogImp* self) { }
static int dev_execute(psy_ui_OpenDialogImp* self) { return 0; }
static const psy_Path* dev_path(const psy_ui_OpenDialogImp* self)
{
	if (!path_initialized) {
		psy_path_init(&path, NULL);
		path_initialized = TRUE;
	}
	return &path;
}

/* vtable */
static psy_ui_OpenDialogImpVTable imp_vtable;
static int imp_vtable_initialized = FALSE;

static void imp_vtable_init(psy_ui_OpenDialogImp* self)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = dev_dispose;
		imp_vtable.dev_execute = dev_execute;
		imp_vtable.dev_path = dev_path;
		imp_vtable_initialized = TRUE;
	}
	self->vtable = &imp_vtable;
}

/* implementation */
void psy_ui_opendialogimp_init(psy_ui_OpenDialogImp* self)
{
	imp_vtable_init(self);	
}
