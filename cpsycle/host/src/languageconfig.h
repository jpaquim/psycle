// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(LANGUAGECONFIG_H)
#define LANGUAGECONFIG_H

// container
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

// LanguageConfig
//
// Configures and selects the language

enum {
	PROPERTY_ID_LANG = 3000
};

typedef struct LanguageConfig {
	psy_Property* language;
	// references
	psy_Property* parent;
} LanguageConfig;

void languageconfig_init(LanguageConfig*, psy_Property* parent);
void languageconfig_configlanguage(LanguageConfig*);

#ifdef __cplusplus
}
#endif

#endif /* LANGUAGECONFIG_H */
