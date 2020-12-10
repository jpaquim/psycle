// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "languageconfig.h"
// ui
#include <uicomponent.h> // access ui translator
// file
#include <dir.h>
// std
#include <stdio.h>
#include <string.h>
// platform
#include "../../detail/portable.h"

static void languageconfig_makelanguagelist(LanguageConfig*, psy_Property*);
static int languageconfig_onaddlanguage(LanguageConfig*, const char* path, int flag);
static const char* languageconfig_languagekey(LanguageConfig*);
static int languageconfig_onaddlanguage(LanguageConfig* self, const char* path, int flag);

void languageconfig_init(LanguageConfig* self, psy_Property* parent)
{
	self->parent = parent;
	languageconfig_makelanguagelist(self, parent);
}

void languageconfig_makelanguagelist(LanguageConfig* self, psy_Property* parent)
{
	char currdir[4096];
	psy_Property* defaultlang;

	assert(self);

	self->language = psy_property_setid(psy_property_settext(
		psy_property_append_choice(parent, "lang", 0),
		"settingsview.language"),
		PROPERTY_ID_LANG);
	if (workdir(currdir)) {
		psy_dir_enumerate(self, currdir, "*.ini", 0, (psy_fp_findfile)
			languageconfig_onaddlanguage);
	}
#if defined PSYCLE_DEFAULT_LANG_ES
	defaultlang = psy_property_find(self->language, "es",
		PSY_PROPERTY_TYPE_NONE);
#elif defined PSYCLE_DEFAULT_LANG_EN
	defaultlang = psy_property_find(self->language, "en",
		PSY_PROPERTY_TYPE_NONE);
#else
	defaultlang = psy_property_find(self->language, "es",
		PSY_PROPERTY_TYPE_NONE);
#endif
	if (defaultlang) {
		uintptr_t index;

		index = psy_property_index(defaultlang);
		if (index != UINTPTR_MAX) {
			psy_property_setitem_int(self->language, index);
		}
	}
}

int languageconfig_onaddlanguage(LanguageConfig* self, const char* path, int flag)
{
	char languageid[256];

	assert(self);

	if (psy_translator_test(psy_ui_translator(), path, languageid)) {
		psy_property_settext(
			psy_property_append_string(self->language, languageid, path),
			languageid);
	}
	return TRUE;
}


void languageconfig_configlanguage(LanguageConfig* self)
{
	psy_Property* langchoice;

	assert(self);

	langchoice = psy_property_at_choice(self->language);
	if (langchoice) {
		psy_translator_load(psy_ui_translator(),
			psy_property_item_str(langchoice));
	}
}

const char* languageconfig_languagekey(LanguageConfig* self)
{
	psy_Property* p;
	const char* rv = 0;

	assert(self);

	if (p = psy_property_at(self->parent, "lang", PSY_PROPERTY_TYPE_NONE)) {
		if (p = psy_property_at_choice(p)) {
			rv = psy_property_item_str(p);
		}
	}
	return rv;
}
