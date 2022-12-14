/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(LANGUAGECONFIG_H)
#define LANGUAGECONFIG_H

/* container */
#include <properties.h>
#include <translator.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
** LanguageConfig
**
** Configures and selects the language of a psy_Translator
*/

enum {
	PROPERTY_ID_LANG = 40000
};

typedef struct LanguageConfig {
	/* signals */
	psy_Signal signal_changed;
	psy_Property* languagechoice;
	/* references */
	psy_Property* parent;
	psy_Translator* translator;
} LanguageConfig;

void languageconfig_init(LanguageConfig*, psy_Property* parent,
	psy_Translator*);
void languageconfig_dispose(LanguageConfig*);
void languageconfig_update_language(LanguageConfig*);

bool languageconfig_has_property(const LanguageConfig*, psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* LANGUAGECONFIG_H */
