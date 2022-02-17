/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
**  copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"

#define HASHSIZE 401

#include "translator.h"
/* local */
#include "inireader.h"
/* platform */
#include "../../detail/portable.h"
/* std */
#include <assert.h>

static bool translator_hastranslation(const char* key,
	const char* translation);
static const char* translator_remove_section(psy_Translator*, const char* key);
static void psy_translator_build(psy_Translator*, psy_Property* lang);
static int psy_translator_enum(psy_Translator*, psy_Property*,
	uintptr_t level);
static void psy_translator_onread(psy_Translator*, psy_IniReader* sender,
	const char* key, const char* value);
static void psy_translator_ontest(psy_Translator*, psy_IniReader* sender,
	const char* key, const char* value);

void psy_translator_init(psy_Translator* self)
{
	assert(self);
	
	psy_table_init_keysize(&self->container, HASHSIZE);
	psy_signal_init(&self->signal_languagechanged);
	self->testid = NULL;
}

void psy_translator_dispose(psy_Translator* self)
{
	assert(self);

	psy_table_disposeall(&self->container, NULL);
	free(self->testid);
	self->testid = NULL;
	psy_signal_dispose(&self->signal_languagechanged);
}

void psy_translator_setdefault(psy_Translator* self, const psy_Property* lang)
{
	assert(self);
	
	if (lang) {		
		psy_translator_build(self, (psy_Property*)lang);
	}	
}

void psy_translator_reset(psy_Translator* self)
{
	assert(self);
	
	psy_table_disposeall(&self->container, NULL);
	psy_table_init_keysize(&self->container, HASHSIZE);
}

bool psy_translator_load(psy_Translator* self, const char* path)
{	
	int success;
	psy_Path dicpath;	
	psy_IniReader inireader;

	assert(self);

	psy_path_init(&dicpath, path);	
	psy_inireader_init(&inireader);
	psy_signal_connect(&inireader.signal_read, self, 
		psy_translator_onread);
	success = inireader_load(&inireader, &dicpath);
	psy_inireader_dispose(&inireader);
	psy_path_dispose(&dicpath);
	psy_signal_emit(&self->signal_languagechanged, self, 0);
	return success;	
}

bool psy_translator_test(const psy_Translator* self, const char* path, char* id)
{	
	psy_Path dicpath;	
	psy_IniReader inireader;

	assert(self);

	psy_path_init(&dicpath, path);	
	free(((psy_Translator*)self)->testid);
	((psy_Translator*)self)->testid = NULL;
	psy_inireader_init(&inireader);
	psy_signal_connect(&inireader.signal_read, (psy_Translator*)self, 
		psy_translator_ontest);
	id[0] = '\0';
	if (inireader_load(&inireader, &dicpath) == PSY_OK) {
		if (self->testid) {
			psy_snprintf(id, 256, "%s", self->testid);
		}
	}
	psy_inireader_dispose(&inireader);
	psy_path_dispose(&dicpath);
	return psy_strlen(id) != 0;	
}

const char* psy_translator_translate(psy_Translator* self, const char* key)
{		
	assert(self);

	if (psy_strlen(key) > 0) {
		const char* rv;

		rv = psy_table_at_strhash(&self->container, key);
		if (rv) {
			return rv;
		}
	}
	return key;
}

bool translator_hastranslation(const char* key, const char* translation)
{
	return key != translation;	
}

const char* psy_translator_langid(const psy_Translator* self)
{	
	const char* rv;

	rv = psy_table_at_strhash(&((psy_Translator*)self)->container, "lang");
	if (rv) {
		return rv;
	}
	return "en";
}

void psy_translator_build(psy_Translator* self, psy_Property* lang)
{
	psy_translator_reset(self);	
	if (lang) {
		psy_property_enumerate(lang, self,
			(psy_PropertyCallback)
			psy_translator_enum);
	}
}

int psy_translator_enum(psy_Translator* self, psy_Property* property, uintptr_t level)
{
	if (psy_property_type(property) == PSY_PROPERTY_TYPE_STRING) {
		char_dyn_t* key;
		char_dyn_t* oldkey;

		key = psy_property_fullkey(property);
		if (oldkey = psy_table_at_strhash(&self->container, key)) {
			free(oldkey);
		}
		psy_table_insert_strhash(&self->container, key,
			psy_strdup(psy_property_item_str(property)));
		free(key);
	}
	return 1;
}

void psy_translator_onread(psy_Translator* self, psy_IniReader* sender,
	const char* key, const char* value)
{
	char* fullkey;
	char* curr;
	uintptr_t len;

	len = psy_strlen(sender->section) + psy_strlen(key) + 1;
	fullkey = (char*)malloc(len + 1);
	if (sender->section) {
		psy_snprintf(fullkey, len + 1, "%s.%s", sender->section, key);
	} else {
		psy_snprintf(fullkey, len + 1, "%s", key);
	}
	if (curr = psy_table_at_strhash(&self->container, fullkey)) {
		free(curr);
		psy_table_insert_strhash(&self->container, fullkey, psy_strdup(value));
	}
	free(fullkey);
	fullkey = NULL;
}

void psy_translator_ontest(psy_Translator* self, psy_IniReader* sender,
	const char* key, const char* value)
{
	if (psy_strlen(sender->section) == 0 && strcmp(key, "lang") == 0) {
		psy_strreset(&self->testid, value);
	}
}
