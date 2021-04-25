// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#include "../../detail/prefix.h"

#include "languageconfig.h"
// file
#include <dir.h>
// platform
#include "../../detail/portable.h"

static void languageconfig_makelanguagechoice(LanguageConfig*);
static void languageconfig_makelanguagelist(LanguageConfig*);
static int languageconfig_enumlanguagedir(LanguageConfig*, const char* path,
	int flag);
static void languageconfig_setdefaultlanguage(LanguageConfig*);
static const char* languageconfig_defaultlanguagekey(LanguageConfig*);

void languageconfig_init(LanguageConfig* self, psy_Property* parent,
	psy_Translator* translator)
{
	assert(self && parent && translator);

	self->parent = parent;
	self->translator = translator;
	psy_signal_init(&self->signal_changed);
	languageconfig_makelanguagechoice(self);
	languageconfig_makelanguagelist(self);
	languageconfig_setdefaultlanguage(self);
}

void languageconfig_dispose(LanguageConfig* self)
{
	assert(self);

	psy_signal_dispose(&self->signal_changed);
}

void languageconfig_makelanguagechoice(LanguageConfig* self)
{
	self->languagechoice = psy_property_append_choice(self->parent, "lang", 0);
	psy_property_setid(self->languagechoice, PROPERTY_ID_LANG);
	psy_property_settext(self->languagechoice, "settingsview.config.language");
}

void languageconfig_makelanguagelist(LanguageConfig* self)
{
	char currdir[4096];
	
	assert(self);
	
	if (workdir(currdir)) {
		psy_dir_enumerate(self, currdir, "*.ini", 0, (psy_fp_findfile)
			languageconfig_enumlanguagedir);
	}
}

int languageconfig_enumlanguagedir(LanguageConfig* self, const char* path,
	int flag)
{
	char lang[256];

	assert(self);

	if (psy_translator_test(self->translator, path, lang)) {
		psy_property_settext(psy_property_append_str(
			self->languagechoice, lang, path), lang);
	}
	return TRUE;
}

void languageconfig_setdefaultlanguage(LanguageConfig* self)
{
	psy_Property* defaultlang;

	if (defaultlang = psy_property_find(self->languagechoice,
			languageconfig_defaultlanguagekey(self),
			PSY_PROPERTY_TYPE_NONE)) {	
		uintptr_t index;

		index = psy_property_index(defaultlang);
		if (index != psy_INDEX_INVALID) {
			psy_property_setitem_int(self->languagechoice, index);
		}
	}
}

const char* languageconfig_defaultlanguagekey(LanguageConfig* self)
{
	return
#if defined PSYCLE_DEFAULT_LANG_ES
		"es";
#elif defined PSYCLE_DEFAULT_LANG_EN
		"en";
#else
		"es";
#endif
}

void languageconfig_updatelanguage(LanguageConfig* self)
{
	psy_Property* lang;

	assert(self);

	if (lang = psy_property_at_choice(self->languagechoice)) {
		// This updates also the ui components over the translator language
		// changed signal the ui is connected to.
		psy_translator_load(self->translator, psy_property_item_str(lang));
	}
}

bool languageconfig_onchanged(LanguageConfig* self,
	psy_Property* property)
{	
	psy_Property* choice;	

	assert(self && property);

	choice = psy_property_item_choice_parent(property);
	if (choice && psy_property_hasid(choice, PROPERTY_ID_LANG)) {
		languageconfig_updatelanguage(self);
		psy_signal_emit(&self->signal_changed, self, 1, property);
		return TRUE;
	}
	return FALSE;
}

bool languageconfig_hasproperty(const LanguageConfig* self,
	psy_Property* property)
{
	assert(self && self->languagechoice);

	return psy_property_insection(property, self->languagechoice);
}
