// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2020 members of the psycle project http://psycle.sourceforge.net

#if !defined(KEYBOARDMISCCONFIG_H)
#define KEYBOARDMISCCONFIG_H

// container
#include <properties.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

// MachineViewConfig
//

enum {
	PROPERTY_ID_DEFAULTLINES = 10000
};

typedef struct KeyboardMiscConfig {	
	// signals
	psy_Signal signal_changed;
	// internal data
	psy_Property* keyboard;
	psy_Property* keyboard_misc;
	// references
	psy_Property* parent;	
} KeyboardMiscConfig;

void keyboardmiscconfig_init(KeyboardMiscConfig*, psy_Property* parent);
void keyboardmiscconfig_dispose(KeyboardMiscConfig*);

bool keyboardmiscconfig_ft2home(const KeyboardMiscConfig*);
bool keyboardmiscconfig_ft2delete(const KeyboardMiscConfig*);
bool keyboardmiscconfig_effcursoralwaysdown(const KeyboardMiscConfig*);
bool keyboardmiscconfig_playstartwithrctrl(KeyboardMiscConfig*);
bool keyboardmiscconfig_movecursoronestep(const KeyboardMiscConfig*);
bool keyboardmiscconfig_savereminder(const KeyboardMiscConfig*);
bool keyboardmiscconfig_patdefaultlines(const KeyboardMiscConfig*);
bool keyboardmiscconfig_allowmultipleinstances(const KeyboardMiscConfig*);
bool keyboardmiscconfig_recordtweaksastws(const KeyboardMiscConfig*);
bool keyboardmiscconfig_advancelineonrecordtweak(const KeyboardMiscConfig*);
int keyboardmiscconfig_pgupdowntype(const KeyboardMiscConfig*);
int keyboardmiscconfig_pgupdownstep(const KeyboardMiscConfig*);

void keyboardmiscconfig_onchanged(KeyboardMiscConfig*, psy_Property*);
bool keyboardmiscconfig_hasproperty(const KeyboardMiscConfig*, psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* KEYBOARDMISCCONFIG_H */
