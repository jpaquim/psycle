/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#include "../../detail/prefix.h"


#include "translator.h"
/* local */
#include "inireader.h"
/* platform */
#include "../../detail/portable.h"
#include "../../detail/os.h"


/* psy_Translator */

/* prototypes */
static void psy_translator_on_read(psy_Translator*, psy_IniReader* sender,
	const char* key, const char* value);
static void psy_translator_on_test(psy_Translator*, psy_IniReader* sender,
	const char* key, const char* value);

/* implementation */
void psy_translator_init(psy_Translator* self)
{
	assert(self);
	
	psy_dictionary_init(&self->dictionary);
	psy_signal_init(&self->signal_languagechanged);
	self->testid = NULL;
}

void psy_translator_dispose(psy_Translator* self)
{
	assert(self);

	psy_dictionary_dispose(&self->dictionary);	
	free(self->testid);
	self->testid = NULL;
	psy_signal_dispose(&self->signal_languagechanged);
}

void psy_translator_set_default(psy_Translator* self, const psy_Dictionary* lang)
{
	assert(self);
	
	psy_dictionary_reset(&self->dictionary);
	if (lang) {
		psy_TableIterator it;

		for (it = psy_table_begin(&((psy_Dictionary*)lang)->container);
				!psy_tableiterator_equal(&it, psy_table_end());
				psy_tableiterator_inc(&it)) {
			uintptr_t src_key;
			const char* src_value;

			src_key = psy_tableiterator_key(&it);
			src_value = (const char*)psy_tableiterator_value(&it);
			psy_dictionary_set_hash(&self->dictionary, src_key, src_value);
		}
	}
}

void psy_translator_reset(psy_Translator* self)
{
	assert(self);
	
	psy_dictionary_reset(&self->dictionary);	
}

bool psy_translator_load(psy_Translator* self, const char* path)
{	
	int success;
	psy_IniReader inireader;

	assert(self);
	
	psy_inireader_init(&inireader);
#if defined DIVERSALIS__OS__POSIX
	inireader.dos_to_utf8 = TRUE;	
#endif
	psy_signal_connect(&inireader.signal_read, self, 
		psy_translator_on_read);
	success = inireader_load(&inireader, path);
	psy_inireader_dispose(&inireader);	
	psy_signal_emit(&self->signal_languagechanged, self, 0);
	return success;	
}

bool psy_translator_test(const psy_Translator* self, const char* path, char* id)
{	
	psy_IniReader inireader;

	assert(self);
	
	free(((psy_Translator*)self)->testid);
	((psy_Translator*)self)->testid = NULL;
	psy_inireader_init(&inireader);
#if defined DIVERSALIS__OS__POSIX
	inireader.dos_to_utf8 = TRUE;
#endif	
	psy_signal_connect(&inireader.signal_read, (psy_Translator*)self, 
		psy_translator_on_test);
	id[0] = '\0';
	if (inireader_load(&inireader, path) == PSY_OK) {
		if (self->testid) {
			psy_snprintf(id, 256, "%s", self->testid);
		}
	}
	psy_inireader_dispose(&inireader);	
	return psy_strlen(id) != 0;	
}

const char* psy_translator_translate(psy_Translator* self, const char* key)
{		
	const char* rv;

	assert(self);

	rv = psy_dictionary_at(&self->dictionary, key);
	if (rv) {
		return rv;
	}
	return key;
}

const char* psy_translator_lang_id(const psy_Translator* self)
{	
	const char* rv;

	rv = psy_dictionary_at(&self->dictionary, "lang");
	if (rv) {
		return rv;
	}
	return "en";
}

void psy_translator_on_read(psy_Translator* self, psy_IniReader* sender,
	const char* key, const char* value)
{
	char* full_key;	
	uintptr_t len;

	len = psy_strlen(sender->section) + psy_strlen(key) + 1;
	full_key = (char*)malloc(len + 1);
	if (sender->section) {
		psy_snprintf(full_key, len + 1, "%s.%s", sender->section, key);		
	} else {
		psy_snprintf(full_key, len + 1, "%s", key);
	}
	if (psy_dictionary_at(&self->dictionary, full_key)) {		
		psy_dictionary_set(&self->dictionary, full_key, value);
	}
	free(full_key);
	full_key = NULL;
}

void psy_translator_on_test(psy_Translator* self, psy_IniReader* sender,
	const char* key, const char* value)
{
	if (psy_strlen(sender->section) == 0 && strcmp(key, "lang") == 0) {
		psy_strreset(&self->testid, value);
	}
}
