/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "uifont.h"
/* local */
#include "uiapp.h"
#include "uiimpfactory.h"
/* platform */
#include "../../detail/portable.h"

/* psy_ui_FontInfo */
void psy_ui_fontinfo_init(psy_ui_FontInfo* self, const char* family,
	int32_t height)
{
	memset(self, 0, sizeof(psy_ui_FontInfo));
	psy_snprintf(self->lfFaceName, 32, "%s", family);	
	self->lfHeight = height;
}

void psy_ui_fontinfo_init_string(psy_ui_FontInfo* self, const char* text)
{	
	memset(self, 0, sizeof(psy_ui_FontInfo));
	self->lfFaceName[0] = '\0';
	self->lfHeight = 12;
	if (psy_strlen(text) > 0) {
		char buffer[256];
		char* token;
		int c = 0;
		
		psy_snprintf(buffer, 256, "%s", text);
		token = strtok(buffer, ";");
		while (token) {
			if (c == 0) {
				psy_snprintf(self->lfFaceName, 32, "%s", token);			
				self->lfFaceName[31] = '\0';
			} else if (c == 1) {
				self->lfHeight = atoi(token);
			} else {
				break;
			}
			++c;
			token = strtok(NULL, ";");
		}
	}
}

void psy_ui_fontinfo_string(const psy_ui_FontInfo* self, char* rv,
	uintptr_t max_len)
{
	psy_snprintf(rv, max_len, "%s;%d", self->lfFaceName, self->lfHeight);
}

/* psy_ui_Font */
void psy_ui_font_init(psy_ui_Font* self, const psy_ui_FontInfo* fontinfo)
{	
	if (fontinfo) {
		self->imp = psy_ui_impfactory_allocinit_fontimp(
			psy_ui_app_impfactory(psy_ui_app()), fontinfo);
		assert(self->imp);
	} else {		
		self->imp = NULL;
	}
}

void psy_ui_font_dispose(psy_ui_Font* self)
{	
	if (self->imp) {
		self->imp->vtable->dev_dispose(self->imp);
		free(self->imp);
		self->imp = NULL;
	}	
}

void psy_ui_font_copy(psy_ui_Font* self, const psy_ui_Font* other)
{
	if (!other->imp) {
		psy_ui_font_dispose(self);
	} else {
		if (!self->imp) {
			psy_ui_FontInfo fontinfo;

			fontinfo = psy_ui_font_fontinfo(other);
			self->imp = psy_ui_impfactory_allocinit_fontimp(
				psy_ui_app_impfactory(psy_ui_app()), &fontinfo);
		}
		self->imp->vtable->dev_copy(self->imp, other->imp);
	}	
}

psy_ui_FontInfo psy_ui_font_fontinfo(const psy_ui_Font* self)
{
	psy_ui_FontInfo fontinfo;

	if (self->imp) {
		return self->imp->vtable->dev_fontinfo(self->imp);
	}
	psy_ui_fontinfo_init(&fontinfo, "arial", 12);
	return fontinfo;
}

psy_ui_TextMetric psy_ui_font_textmetric(const psy_ui_Font* self)
{
	assert(self->imp);

	return self->imp->vtable->dev_textmetric(self->imp);
}

bool psy_ui_font_equal(const psy_ui_Font* self, const psy_ui_Font* other)
{
	assert(self->imp);

	if (other) {
		return self->imp->vtable->dev_equal(self->imp, other->imp);
	}
	return FALSE;
}

/* psy_ui_FontImp */
static void dev_dispose(psy_ui_FontImp* self) { }
static void dev_copy(psy_ui_FontImp* self, const psy_ui_FontImp* other) { }

static const psy_ui_FontInfo dev_fontinfo(const psy_ui_FontImp* self)
{
    psy_ui_FontInfo rv;

    psy_ui_fontinfo_init(&rv, "tahoma", 8);
    return rv;
}

static psy_ui_TextMetric dev_textmetric(const psy_ui_FontImp* self)
{
	psy_ui_TextMetric rv;	
	return rv;
}

bool dev_equal(const psy_ui_FontImp* self, const psy_ui_FontImp* other)
{
	return TRUE;
}

/* vtable */
static psy_ui_FontImpVTable imp_vtable;
static bool imp_vtable_initialized = FALSE;
/* implementation */
static void imp_vtable_init(psy_ui_FontImp* self)
{
	if (!imp_vtable_initialized) {
		imp_vtable.dev_dispose = dev_dispose;		
		imp_vtable.dev_copy = dev_copy;
        imp_vtable.dev_fontinfo = dev_fontinfo;
		imp_vtable.dev_textmetric = dev_textmetric;
		imp_vtable.dev_equal = dev_equal;
		imp_vtable_initialized = TRUE;
	}
	self->vtable = &imp_vtable;
}
/* implementation */
void psy_ui_font_imp_init(psy_ui_FontImp* self)
{
	imp_vtable_init(self);		
}
