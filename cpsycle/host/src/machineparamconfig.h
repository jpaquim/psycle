// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(MACHINEPARAMCONFIG_H)
#define MACHINEPARAMCONFIG_H

// host
#include "skincoord.h"
// ui
#include <uibitmap.h>
// container
#include <properties.h>
#include <signal.h>

#ifdef __cplusplus
extern "C" {
#endif

// ParamSkin
typedef struct ParamSkin {
    psy_ui_Colour fonttopcolour;
    psy_ui_Colour fontbottomcolour;
    psy_ui_Colour fonthtopcolour;
    psy_ui_Colour fonthbottomcolour;
    psy_ui_Colour fonttitlecolour;
    psy_ui_Colour topcolour;
    psy_ui_Colour bottomcolour;
    psy_ui_Colour htopcolour;
    psy_ui_Colour hbottomcolour;
    psy_ui_Colour titlecolour;
    psy_ui_Bitmap knobbitmap;
    psy_ui_Bitmap mixerbitmap;
    char* bitmappath;
    SkinCoord slider;
    SkinCoord knob;
    SkinCoord sliderknob;
    SkinCoord vuoff;
    SkinCoord vuon;
    SkinCoord switchon;
    SkinCoord switchoff;
    SkinCoord checkon;
    SkinCoord checkoff;
    double paramwidth;
    double paramwidth_small;
} ParamSkin;

psy_ui_RealSize mpfsize(ParamSkin* skin, const psy_ui_TextMetric* tm,
	uintptr_t paramtype, bool issmall);

// MachineParamConfig

enum {
	PROPERTY_ID_LOADCONTROLSKIN = 9000,
	PROPERTY_ID_DEFAULTCONTROLSKIN
};

typedef struct MachineParamConfig {
	// signals
	psy_Signal signal_changed;
	psy_Signal signal_themechanged;
	// internal data
	psy_Property* paramview;
	psy_Property* theme;	
	// references
	psy_Property* parent;
} MachineParamConfig;

void machineparamconfig_init(MachineParamConfig*, psy_Property* parent);
void machineparamconfig_dispose(MachineParamConfig*);

void machineparamconfig_resettheme(MachineParamConfig*);
void machineparamconfig_settheme(MachineParamConfig*, psy_Property* theme);
ParamSkin* machineparamconfig_skin(MachineParamConfig*);
void machineparamconfig_releaseskin(void);

bool machineparamconfig_hasthemeproperty(const MachineParamConfig*,
	psy_Property*);
bool machineparamconfig_hasproperty(const MachineParamConfig*, psy_Property*);

const char* machineparamconfig_dialbpm(const MachineParamConfig*);
void machineparamconfig_setdialbpm(MachineParamConfig*, const char* filename);

psy_ui_FontInfo machineparamconfig_fontinfo(const MachineParamConfig*);

bool machineparamconfig_onchanged(MachineParamConfig*, psy_Property*);
bool machineparamconfig_onthemechanged(MachineParamConfig*, psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* MACHINEPARAMCONFIG_H */
