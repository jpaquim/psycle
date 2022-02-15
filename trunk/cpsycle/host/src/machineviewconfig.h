/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(MACHINEVIEWCONFIG_H)
#define MACHINEVIEWCONFIG_H

/* container */
#include <properties.h>
#include <signal.h>
/* ui */
#include <uistyles.h>

#ifdef __cplusplus
extern "C" {
#endif

/* MachineViewConfig */

enum {
	PROPERTY_ID_DRAWVUMETERS = 50000,
	PROPERTY_ID_DRAWVIRTUALGENERATORS
};

struct DirConfig;

typedef struct MachineViewConfig {
	/* signals */
	psy_Signal signal_changed;	
	psy_Property* machineview;
	psy_Property* stackview;
	psy_Property* theme;	
	/* references */
	psy_Property* parent;
	struct DirConfig* dirconfig;
} MachineViewConfig;

void machineviewconfig_init(MachineViewConfig*, psy_Property* parent);
void machineviewconfig_dispose(MachineViewConfig*);

void machineviewconfig_setdirectories(MachineViewConfig*, struct DirConfig*);
void machineviewconfig_resettheme(MachineViewConfig*);
void machineviewconfig_settheme(MachineViewConfig*, psy_Property*);
void machineviewconfig_write_styles(MachineViewConfig*);
void machineviewconfig_read_styles(MachineViewConfig*);
bool machineviewconfig_hasthemeproperty(const MachineViewConfig*,
	psy_Property*);
bool machineviewconfig_hasproperty(const MachineViewConfig*, psy_Property*);

bool machineviewconfig_machineindexes(const MachineViewConfig*);
bool machineviewconfig_wirehover(const MachineViewConfig*);
bool machineviewconfig_vumeters(const MachineViewConfig*);
bool machineviewconfig_virtualgenerators(const MachineViewConfig*);
bool machineviewconfig_stackview_drawsmalleffects(const MachineViewConfig*);

bool machineviewconfig_onchanged(MachineViewConfig*, psy_Property*);


#ifdef __cplusplus
}
#endif

#endif /* MACHINEVIEWCONFIG_H */
