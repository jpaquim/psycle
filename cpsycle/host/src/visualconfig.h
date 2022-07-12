/*
** This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
** copyright 2000-2022 members of the psycle project http://psycle.sourceforge.net
*/

#if !defined(VISUALCONFIG_H)
#define VISUALCONFIG_H

/* host*/
#include "languageconfig.h"
#include "machineviewconfig.h"
#include "machineparamconfig.h"
#include "patternviewconfig.h"
#include "viewindex.h"
/* container */
#include <properties.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

/* VisualConfig */

enum {
	PROPERTY_ID_DEFAULTSKIN = 11000,
	PROPERTY_ID_DEFAULTFONT,
	PROPERTY_ID_APPTHEME,
	PROPERTY_ID_LOADSKIN
};

struct DirConfig;
struct psy_audio_Player;

typedef struct VisualConfig {
	/* signals */
	psy_Property* visual;
	psy_Property* defaultfont;
	psy_Property* apptheme;
	/* internal */
	PatternViewConfig patview;
	MachineViewConfig macview;
	MachineParamConfig macparam;
	/* references */
	psy_Property* parent;
	struct DirConfig* dirconfig;
	struct psy_audio_Player* player;
} VisualConfig;

void visualconfig_init(VisualConfig*, psy_Property* parent, struct psy_audio_Player*);
void visualconfig_dispose(VisualConfig*);

void visualconfig_set_directories(VisualConfig* self, struct DirConfig*);
void visualconfig_load_skin(VisualConfig*, const char* path);
void visualconfig_reset_skin(VisualConfig*);
void visualconfig_reset_control_skin(VisualConfig*);
void visualconfig_set_app_theme(VisualConfig*);

const char* visualconfig_default_font_str(const VisualConfig*);
void visualconfig_set_default_font(VisualConfig*, psy_Property*);

bool visualconfig_hasproperty(const VisualConfig*, psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* VISUALCONFIG_H */
