// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiapp.h"
#include "uifontdialog.h"
#include "uiimpfactory.h"
#include <stdlib.h>

void psy_ui_fontdialog_init(psy_ui_FontDialog* self, psy_ui_Component* parent)
{
	self->imp = psy_ui_impfactory_allocinit_fontdialogimp(
		psy_ui_app_impfactory(&app),
		parent);
}

void psy_ui_fontdialog_dispose(psy_ui_FontDialog* self)
{
	self->imp->vtable->dev_dispose(self->imp);
	free(self->imp);
	self->imp = 0;
}

int psy_ui_fontdialog_execute(psy_ui_FontDialog* self)
{	
	return self->imp->vtable->dev_execute(self->imp);
}
				
psy_ui_FontInfo psy_ui_fontdialog_fontinfo(psy_ui_FontDialog* self)
{
	return self->imp->vtable->dev_fontinfo(self->imp);
}

// psy_ui_FontImp
static void dev_dispose(psy_ui_FontDialogImp* self) { }
static int dev_execute(psy_ui_FontDialogImp* self) { return 0; }
static psy_ui_FontInfo dev_fontinfo(psy_ui_FontDialogImp* self) {
	psy_ui_FontInfo rv;

	psy_ui_fontinfo_init(&rv, "arial", 8);
	return rv;
}

static psy_ui_FontDialogImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(void)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = dev_dispose;
		imp_vtable.dev_execute = dev_execute;
		imp_vtable.dev_fontinfo = dev_fontinfo;
		imp_vtable_initialized = 1;
	}
}

void psy_ui_fontdialogimp_init(psy_ui_FontDialogImp* self)
{
	imp_vtable_init();
	self->vtable = &imp_vtable;
}
