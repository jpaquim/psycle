/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "translator.h"
/* local */
#include "propertiesio.h"
/* platform */
#include "../../detail/portable.h"
/* std */
#include <assert.h>

static bool translator_hastranslation(const char* key,
	const char* translation);
static const char* translator_remove_section(psy_Translator*, const char* key);

void psy_translator_init(psy_Translator* self)
{
	assert(self);

	self->dictionary = NULL;
	self->defaultdictionary = NULL;
	psy_signal_init(&self->signal_languagechanged);	
}

void psy_translator_dispose(psy_Translator* self)
{
	assert(self);

	psy_property_deallocate(self->dictionary);
	psy_property_deallocate (self->defaultdictionary);
	psy_signal_dispose(&self->signal_languagechanged);
}

void psy_translator_setdefault(psy_Translator* self, const psy_Property* lang)
{
	assert(self);
	
	psy_translator_reset(self);
	psy_property_deallocate(self->defaultdictionary);
	self->defaultdictionary = NULL;
	if (lang) {
		self->defaultdictionary = psy_property_clone(lang);
		self->dictionary = psy_property_clone(lang);
	}	
}

void psy_translator_reset(psy_Translator* self)
{
	assert(self);
	
	psy_property_deallocate(self->dictionary);
	self->dictionary = NULL;	
}

bool psy_translator_load(psy_Translator* self, const char* path)
{	
	assert(self);

	if (self->dictionary) {
		int success;
		psy_Path dicpath;

		assert(self);

		psy_path_init(&dicpath, path);
		success = propertiesio_load(self->dictionary, &dicpath, FALSE);
		psy_path_dispose(&dicpath);
		psy_signal_emit(&self->signal_languagechanged, self, 0);
		return success;
	}
	return FALSE;
}

bool psy_translator_test(const psy_Translator* self, const char* path, char* id)
{
	psy_Property* lang;
	psy_Path dicpath;

	assert(self);

	lang = psy_property_allocinit_key(NULL);	

	assert(self);

	psy_path_init(&dicpath, path);
	if (propertiesio_load(lang, &dicpath, 1) == PSY_OK) {
		psy_Property* p;
		
		p = psy_property_at(lang, "lang", PSY_PROPERTY_TYPE_NONE);
		if (p) {
			psy_snprintf(id, 256, "%s", psy_property_item_str(p));
			psy_property_deallocate(lang);
			psy_path_dispose(&dicpath);
			return TRUE;
		}
	}
	psy_path_dispose(&dicpath);
	psy_property_deallocate(lang);
	return FALSE;
}

const char* psy_translator_translate(psy_Translator* self, const char* key)
{		
	assert(self);

	if (self->dictionary && key) {
		const char* rv;

		rv = psy_property_at_str(self->dictionary, key, key);
		if (!translator_hastranslation(rv, key)) {
			return translator_remove_section(self, rv);
		}
		return rv;
	}
	return key;
}

bool translator_hastranslation(const char* key, const char* translation)
{
	return key != translation;	
}

const char* translator_remove_section(psy_Translator* self, const char* key)
{
	const char* rv;

	assert(self);

	if (key) {
		rv = strrchr(key, '.');
		rv = (rv != NULL)
			? rv + 1
			: key;
	} else {
		rv = NULL;
	}
	if (rv == NULL) {
		rv = "";
	}
	return rv;
}

const char* psy_translator_langid(const psy_Translator* self)
{
	psy_Property* p;

	p = psy_property_at(self->dictionary, "lang", PSY_PROPERTY_TYPE_NONE);
	if (p) {
		return psy_property_item_str(p);
	}
	return "en";
}
