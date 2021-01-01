// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"
#include "../../detail/os.h"

#include "uiapp.h"
#include "uifont.h"
#include "uiimpfactory.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "../../detail/portable.h"

#ifndef max
#define max(a, b) (((a > b) ? a : b))
#endif

extern psy_ui_App app;

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
    self->imp = psy_ui_impfactory_allocinit_fontimp(psy_ui_app_impfactory(&app), fontinfo);	
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
static void dev_dispose(psy_ui_FontImp* self) { }
static void dev_copy(psy_ui_FontImp* self, psy_ui_FontImp* other) { }
static psy_ui_FontInfo dev_fontinfo(psy_ui_FontImp* self)
{
    psy_ui_FontInfo rv;
    psy_ui_fontinfo_init(&rv, "tahoma", 8);
    return rv;
}

static psy_ui_FontImpVTable imp_vtable;
static int imp_vtable_initialized = 0;

static void imp_vtable_init(void)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = dev_dispose;		
		imp_vtable.dev_copy = dev_copy;
        imp_vtable.dev_fontinfo = dev_fontinfo;
		imp_vtable_initialized = 1;
	}
}

void psy_ui_font_imp_init(psy_ui_FontImp* self)
{
	imp_vtable_init();	
	self->vtable = &imp_vtable;
}

void psy_ui_fontinfo_init(psy_ui_FontInfo* self, const char* family,
    int height)
{ 
    memset(self, 0, sizeof(psy_ui_FontInfo));
    psy_snprintf(self->lfFaceName, 32, "%s", family);
    self->lfFaceName[31] = '\0';
    self->lfHeight = height;    
}

void psy_ui_fontinfo_init_string(psy_ui_FontInfo* self, const char* text)
{	
	char buffer[256];
	char* token;
	int c = 0;

	psy_snprintf(buffer, 256, "%s", text);
	memset(self, 0, sizeof(psy_ui_FontInfo));
	token = strtok(buffer, ";");
	while (token) {
		if (c == 0) {			
			memcpy(self->lfFaceName, token, max(strlen(token), 32));
			self->lfFaceName[31] = '\0';
		} else
		if (c == 1) {
			self->lfHeight = atoi(token);
		} else {
			break;
		}
		++c;
		token = strtok(NULL, ";");
	}
}

const char* psy_ui_fontinfo_string(psy_ui_FontInfo* self)
{
	static char text[256];
	
	psy_snprintf(text, 256, "%s;%d", self->lfFaceName, self->lfHeight);
	return text;
}
