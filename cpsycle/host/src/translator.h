// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(TRANSLATOR_H)
#define TRANSLATOR_H

#include <properties.h>

typedef struct {	
	psy_Properties* lang;	
} Translator;

void translator_init(Translator*);
void translator_dispose(Translator*);
bool translator_load(Translator* self, const char* path);
bool translator_test(Translator* self, const char* path, char* id);
const char* translator_translate(Translator*, const char* key);

#endif /* TRANSLATOR_H */
