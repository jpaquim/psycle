/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(MACHINEPARAMCONFIG_H)
#define MACHINEPARAMCONFIG_H

/* ui */
#include <uibitmap.h>
/* container */
#include <properties.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PARAMWIDTH 26.0
#define PARAMWIDTH_SMALL 18.0

/* MachineParamConfig */

enum {
	PROPERTY_ID_LOADCONTROLSKIN = 9000,
	PROPERTY_ID_DEFAULTCONTROLSKIN
};

struct DirConfig;

typedef struct MachineParamConfig {
    /* signals */
	psy_Signal signal_changed;
	psy_Signal signal_themechanged;
    /* internal */
	psy_Property* paramview;
	psy_Property* theme;	
    /* references */
	psy_Property* parent;
	struct DirConfig* dirconfig;
} MachineParamConfig;

void machineparamconfig_init(MachineParamConfig*, psy_Property* parent);
void machineparamconfig_dispose(MachineParamConfig*);

void machineparamconfig_setdirectories(MachineParamConfig*, struct DirConfig*);
void machineparamconfig_resettheme(MachineParamConfig*);
void machineparamconfig_settheme(MachineParamConfig*, psy_Property* theme);
void machineparamconfig_update_styles(MachineParamConfig*);

bool machineparamconfig_hasthemeproperty(const MachineParamConfig*,
	psy_Property*);
bool machineparamconfig_hasproperty(const MachineParamConfig*, psy_Property*);

const char* machineparamconfig_dialbpm(const MachineParamConfig*);
void machineparamconfig_set_dial_bpm(MachineParamConfig*, const char* filename);

psy_ui_FontInfo machineparamconfig_fontinfo(const MachineParamConfig*);

uintptr_t machineparamconfig_onchanged(MachineParamConfig*, psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* MACHINEPARAMCONFIG_H */
