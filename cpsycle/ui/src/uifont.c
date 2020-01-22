// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "uifont.h"
#include "uiwinfontimp.h"
#include <stdlib.h>

// VTable Prototypes
static void dispose(psy_ui_Font*);
static void copy(psy_ui_Font*, psy_ui_Font* other);

// VTable init
static psy_ui_FontVTable vtable;
static int vtable_initialized = 0;

static void vtable_init(void)
{
	if (!vtable_initialized) {
		vtable.dispose = dispose;		
		vtable.copy = copy;
		vtable_initialized = 1;
	}
}

void psy_ui_font_init(psy_ui_Font* self, const psy_ui_FontInfo* fontinfo)
{
	vtable_init();
	self->vtable = &vtable;
	self->imp = (psy_ui_FontImp*) malloc(sizeof(psy_ui_win_FontImp));
	psy_ui_win_fontimp_init((psy_ui_win_FontImp*)self->imp, fontinfo);	
}

// Delegation Methods to FontImp
void dispose(psy_ui_Font* self)
{	
	self->imp->vtable->dev_dispose(self->imp);
	free(self->imp);
	self->imp = 0;
}

void copy(psy_ui_Font* self, psy_ui_Font* other)
{
	self->imp->vtable->dev_copy(self->imp, other->imp);	
}

// psy_ui_FontImp
static void psy_ui_imp_dispose(psy_ui_FontImp* self) { }
static void psy_ui_imp_copy(psy_ui_FontImp* self, psy_ui_FontImp* other) { }

static psy_ui_FontImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(void)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = psy_ui_imp_dispose;		
		imp_vtable.dev_copy = psy_ui_imp_copy;
		imp_vtable_initialized = 1;
	}
}

void psy_ui_font_imp_init(psy_ui_FontImp* self)
{
	imp_vtable_init();	
	self->vtable = &imp_vtable;
}
