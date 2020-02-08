// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiapp.h"
#include "uicolordialog.h"
#include "uiimpfactory.h"
#include <stdlib.h>

extern psy_ui_App app;

void psy_ui_colordialog_init(psy_ui_ColorDialog* self, psy_ui_Component* parent)
{
	self->imp = psy_ui_impfactory_allocinit_colordialogimp(
		psy_ui_app_impfactory(&app),
		parent);
}

void psy_ui_colordialog_dispose(psy_ui_ColorDialog* self)
{
	self->imp->vtable->dev_dispose(self->imp);
	free(self->imp);
	self->imp = 0;
}

int psy_ui_colordialog_execute(psy_ui_ColorDialog* self)
{	
	return self->imp->vtable->dev_execute(self->imp);
}
				
psy_ui_Color psy_ui_colordialog_color(psy_ui_ColorDialog* self)
{
	return self->imp->vtable->dev_color(self->imp);
}

// psy_ui_ColorImp
static void dev_dispose(psy_ui_ColorDialogImp* self) { }
static int dev_execute(psy_ui_ColorDialogImp* self) { return 0; }
static psy_ui_Color dev_color(psy_ui_ColorDialogImp* self) { return 0; }

static psy_ui_ColorDialogImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(void)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = dev_dispose;
		imp_vtable.dev_execute = dev_execute;
		imp_vtable.dev_color = dev_color;
		imp_vtable_initialized = 1;
	}
}

void psy_ui_colordialogimp_init(psy_ui_ColorDialogImp* self)
{
	imp_vtable_init();
	self->vtable = &imp_vtable;
}
