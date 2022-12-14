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
	PROPERTY_ID_DRAWVIRTUALGENERATORS,
	PROPERTY_ID_MACHINESKIN
};

struct DirConfig;

typedef struct MachineViewConfig {		
	psy_Property* machineview;
	psy_Property* stackview;
	psy_Property* theme;	
	psy_Property* machineskins;	
	/* references */
	psy_Property* parent;
	struct DirConfig* dirconfig;
} MachineViewConfig;

void machineviewconfig_init(MachineViewConfig*, psy_Property* parent);
void machineviewconfig_dispose(MachineViewConfig*);

bool machineviewconfig_connect(MachineViewConfig*, const char* key,
	void* context, void* fp);
psy_Property* machineviewconfig_property(MachineViewConfig*, const char* key);

void machineviewconfig_setdirectories(MachineViewConfig*, struct DirConfig*);
void machineviewconfig_update_machine_skins(MachineViewConfig*);
void machineviewconfig_resettheme(MachineViewConfig*);
const char* machineviewconfig_machine_skin_name(MachineViewConfig* self);
void machineviewconfig_settheme(MachineViewConfig*, psy_Property*);
void machineviewconfig_load(MachineViewConfig*);
void machineviewconfig_save(MachineViewConfig*);

bool machineviewconfig_hasthemeproperty(const MachineViewConfig*,
	psy_Property*);
bool machineviewconfig_hasproperty(const MachineViewConfig*, psy_Property*);

bool machineviewconfig_machineindexes(const MachineViewConfig*);
bool machineviewconfig_wirehover(const MachineViewConfig*);
bool machineviewconfig_vumeters(const MachineViewConfig*);
bool machineviewconfig_virtualgenerators(const MachineViewConfig*);
bool machineviewconfig_stackview_drawsmalleffects(const MachineViewConfig*);
/* theme */
void machineviewconfig_set_background_colour(MachineViewConfig*,
	psy_ui_Colour);
void machineviewconfig_set_poly_colour(MachineViewConfig*,
	psy_ui_Colour);
void machineviewconfig_set_wire_colour(MachineViewConfig*,
	psy_ui_Colour);
void machineviewconfig_set_generator_colour(MachineViewConfig*,
	psy_ui_Colour);
void machineviewconfig_set_effect_colour(MachineViewConfig*,
	psy_ui_Colour);

#ifdef __cplusplus
}
#endif

#endif /* MACHINEVIEWCONFIG_H */
