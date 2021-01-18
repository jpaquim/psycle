// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "uiapp.h"
#include "uicolordialog.h"
#include "uiimpfactory.h"
#include <stdlib.h>

void psy_ui_colourdialog_init(psy_ui_ColourDialog* self, psy_ui_Component* parent)
{
	assert(self);

	self->imp = psy_ui_impfactory_allocinit_colourdialogimp(
		psy_ui_app_impfactory(psy_ui_app()),
		parent);
}

void psy_ui_colourdialog_dispose(psy_ui_ColourDialog* self)
{
	assert(self);

	self->imp->vtable->dev_dispose(self->imp);
	free(self->imp);
	self->imp = NULL;
}

int psy_ui_colourdialog_execute(psy_ui_ColourDialog* self)
{	
	assert(self);

	return self->imp->vtable->dev_execute(self->imp);
}
				
psy_ui_Colour psy_ui_colourdialog_colour(psy_ui_ColourDialog* self)
{
	assert(self);

	return self->imp->vtable->dev_colour(self->imp);
}

void psy_ui_colourdialog_setcolour(psy_ui_ColourDialog* self, psy_ui_Colour colour)
{
	assert(self);

	self->imp->vtable->dev_setcolour(self->imp, colour);
}

// psy_ui_ColourImp
static void dev_dispose(psy_ui_ColourDialogImp* self) { }
static int dev_execute(psy_ui_ColourDialogImp* self) { return 0; }
static psy_ui_Colour dev_colour(psy_ui_ColourDialogImp* self) { return psy_ui_colour_make(0); }
static void dev_setcolour(psy_ui_ColourDialogImp* self, psy_ui_Colour colour) { }

static psy_ui_ColourDialogImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;

static psy_ui_ColourDialogImpVTable* imp_vtable_init(void)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = dev_dispose;
		imp_vtable.dev_execute = dev_execute;
		imp_vtable.dev_colour = dev_colour;
		imp_vtable.dev_setcolour = dev_setcolour;
		imp_vtable_initialized = TRUE;
	}
	return &imp_vtable;
}

void psy_ui_colourdialogimp_init(psy_ui_ColourDialogImp* self)
{	
	assert(self);

	self->vtable = imp_vtable_init();
}
