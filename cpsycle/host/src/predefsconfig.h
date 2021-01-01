// This source is free software ; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation ; either version 2, or (at your option) any later version.
// copyright 2000-2021 members of the psycle project http://psycle.sourceforge.net

#if !defined(PREDEFSCONFIG_H)
#define PREDEFSCONFIG_H

// audio
#include <instrument.h>

// container
#include <properties.h>

#ifdef __cplusplus
extern "C" {
#endif

// PredefsConfig

// aim : Stores predefinitions of instruments in the configuration like in FT2
// status: atm only envelopes

typedef struct PredefsConfig {
	// signals
	psy_Signal signal_changed;	
	// internal
	psy_Property* predefs;
	// references
	psy_Property* parent;	
} PredefsConfig;

void predefsconfig_init(PredefsConfig*, psy_Property* parent);
void predefsconfig_dispose(PredefsConfig*);

void predefsconfig_predef(PredefsConfig*, int index, psy_dsp_EnvelopeSettings* rv);
void predefsconfig_storepredef(PredefsConfig*, int index, psy_dsp_EnvelopeSettings* rv);

// Will be called if settingview was changed
bool predefsconfig_onchanged(PredefsConfig*, psy_Property*);
bool predefsconfig_hasproperty(const PredefsConfig*, psy_Property*);

#ifdef __cplusplus
}
#endif

#endif /* PREDEFSCONFIG_H */
