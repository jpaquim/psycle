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

typedef struct VisualConfig {
	/* signals */
	psy_Signal signal_changed;
	psy_Property* visual;
	psy_Property* defaultfont;
	psy_Property* apptheme;
	/* internal */
	PatternViewConfig patview;
	MachineViewConfig macview;
	MachineParamConfig macparam;
	/* references */
	psy_Property* parent;	
} VisualConfig;

void visualconfig_init(VisualConfig*, psy_Property* parent);
void visualconfig_dispose(VisualConfig*);


uintptr_t visualconfig_on_changed(VisualConfig*, psy_Property*);
bool visualconfig_hasproperty(const VisualConfig*, psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* VISUALCONFIG_H */
