// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(TRANSLATOR_H)
#define TRANSLATOR_H

#include <properties.h>

// Translator
//
// Defines a dictionary for localization with a default set in english. It can
// test "ini" files for language definitions and can load a dictionary from an
// "ini" file.
//
// Translator <>----- psy_Properties        ; dictionary
//    |
//    - - <<use>>- -> psy_propertiesio      ; dictionary load
//
// Structure of properties/ini-file:
// root.lang: sets the language id used to identify the language (de, en, es)
// the dictionary has predefined sections and words
// sections:
// [file] [undo] [play] [main] [help] [machinebar] [edit] [lpb] [render] [gear]
// [settingsview] [instrumentview] [machineview] [newmachine] [samplesview]
// [cmds]
// word definitions: see translator.c definekeys

typedef struct Translator {
	psy_Properties dictionary;	
} Translator;

void translator_init(Translator*);
void translator_dispose(Translator*);
// reset to default (english)
void translator_reset(Translator*);
// loads a.ini file containing a dictionary
// first it resets to default (english) and overwrites available properties
// with the new definitions
bool translator_load(Translator* self, const char* path);
// tests root.lang property. (workspace uses this to scan the language list)
bool translator_test(const Translator*, const char* path, char* id);
// translate a key
// word definitions: see translator.c definekeys
// example  translate("file.load")
//   returns: Load (for 'en')
// translate("help.load")
//   returns: load (not found in help, return key without section prefix)
const char* translator_translate(Translator*, const char* key);

#endif /* TRANSLATOR_H */
