// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(KEYBOARDMISCCONFIG_H)
#define KEYBOARDMISCCONFIG_H

// container
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

// MachineViewConfig
//

enum {
	PROPERTY_ID_DEFAULTLINES = 10000
};

typedef struct KeyboardMiscConfig {	
	psy_Property* keyboard;
	psy_Property* keyboard_misc;
	// references
	psy_Property* parent;	
} KeyboardMiscConfig;

void keyboardmiscconfig_init(KeyboardMiscConfig*, psy_Property* parent);

bool keyboardandmisc_ft2home(const KeyboardMiscConfig*);
bool keyboardandmisc_ft2delete(const KeyboardMiscConfig*);
bool keyboardandmisc_effcursoralwaysdown(const KeyboardMiscConfig*);
bool keyboardandmisc_playstartwithrctrl(KeyboardMiscConfig*);
bool keyboardandmisc_movecursoronestep(const KeyboardMiscConfig*);
bool keyboardandmisc_savereminder(const KeyboardMiscConfig*);
bool keyboardandmisc_patdefaultlines(const KeyboardMiscConfig*);
bool keyboardandmisc_allowmultipleinstances(const KeyboardMiscConfig*);
bool keyboardandmisc_recordtweaksastws(const KeyboardMiscConfig*);
bool keyboardandmisc_advancelineonrecordtweak(const KeyboardMiscConfig*);
int keyboardandmisc_pgupdowntype(const KeyboardMiscConfig*);
int keyboardandmisc_pgupdownstep(const KeyboardMiscConfig*);

#ifdef __cplusplus
}
#endif

#endif /* KEYBOARDMISCCONFIG_H */
